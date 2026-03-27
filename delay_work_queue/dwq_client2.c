/**********************************************
 * dwq_client2.c — 用户空间 client（支持普通请求 + callback 模式）
 *
 * 用法：
 *   ./dwq_client2 sync  "message"   — 同步 request-reply
 *   ./dwq_client2 async "message"   — 异步 callback 模式
 *
 * Callback 模式流程：
 *   1. ioctl(REGISTER, handle=2)    ← 注册自己为 callback 接收端
 *   2. write(handle=1, callback_handle=2, cmd=WORK)
 *   3. 另一线程 read() 等事件        ← server 会向 handle=2 发 EVENT
 *
 * Compile: gcc -pthread -o dwq_client2 dwq_client2.c
 * Run:     ./dwq_client2 sync "hello"
 *          ./dwq_client2 async "hello async"
 *********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include "dwq_uapi.h"

#define DEV_PATH       "/dev/dwq"
#define TARGET_HANDLE  1
#define CB_HANDLE      2    /* client 用作 callback 接收端的 handle */

static int g_fd;
static volatile int g_cb_received = 0;

/* callback 接收线程：等 server 回调 */
static void *cb_thread(void *arg)
{
    struct dwq_reply event;
    (void)arg;

    printf("[client2/cb] waiting for callback event on handle=%d...\n",
           CB_HANDLE);

    ssize_t n = read(g_fd, &event, sizeof(event));
    if (n < 0) {
        perror("cb read");
        return NULL;
    }

    printf("[client2/cb] CALLBACK received: cmd=%u tx_id=%u result=%d "
           "data=\"%.*s\"\n",
           event.cmd, event.tx_id, event.result,
           (int)event.data_len, event.data);

    g_cb_received = 1;
    return NULL;
}

int main(int argc, char *argv[])
{
    const char *mode = (argc > 1) ? argv[1] : "sync";
    const char *data = (argc > 2) ? argv[2] : "hello from client2";

    g_fd = open(DEV_PATH, O_RDWR);
    if (g_fd < 0) { perror("open"); return 1; }

    printf("[client2] pid=%d  mode=%s\n", getpid(), mode);

    if (strcmp(mode, "async") == 0) {
        /* ---- Callback 模式 ---- */
        struct dwq_reg_info reg;
        struct dwq_msg      req;
        pthread_t           tid;

        /* 1. 注册 callback handle（client 自己也成为一个服务端点）*/
        /* 类比 client 创建 BnMyCallback 并传给 server */
        memset(&reg, 0, sizeof(reg));
        reg.handle = CB_HANDLE;
        strncpy(reg.name, "client-callback", sizeof(reg.name) - 1);
        if (ioctl(g_fd, DWQ_IOC_REGISTER, &reg) < 0) {
            perror("ioctl REGISTER callback");
            close(g_fd);
            return 1;
        }
        printf("[client2] registered callback handle=%d\n", CB_HANDLE);

        /* 2. 启动 callback 接收线程 */
        pthread_create(&tid, NULL, cb_thread, NULL);

        /* 3. 发送请求，带 callback_handle */
        memset(&req, 0, sizeof(req));
        req.handle          = TARGET_HANDLE;
        req.cmd             = DWQ_CMD_WORK;
        req.callback_handle = CB_HANDLE;       /* 告诉 server 回调这里 */
        req.data_len        = strlen(data);
        strncpy(req.data, data, DWQ_MAX_DATA - 1);

        ssize_t n = write(g_fd, &req, sizeof(req));
        if (n < 0) { perror("write"); close(g_fd); return 1; }
        printf("[client2] sent async request data=\"%s\"\n", data);
        printf("[client2] NOT blocking — doing other work...\n");

        /* 模拟 client 做其他事 */
        sleep(1);
        printf("[client2] still doing other work...\n");

        /* 等 callback 线程收到事件 */
        pthread_join(tid, NULL);

        /* 注销 callback handle */
        ioctl(g_fd, DWQ_IOC_UNREGISTER, 0);

    } else {
        /* ---- 同步 Request-Reply 模式 ---- */
        struct dwq_msg   req;
        struct dwq_reply reply;

        memset(&req, 0, sizeof(req));
        req.handle   = TARGET_HANDLE;
        req.cmd      = DWQ_CMD_WORK;
        req.data_len = strlen(data);
        strncpy(req.data, data, DWQ_MAX_DATA - 1);

        ssize_t n = write(g_fd, &req, sizeof(req));
        if (n < 0) { perror("write"); close(g_fd); return 1; }
        printf("[client2] sent sync request data=\"%s\"\n", data);

        printf("[client2] blocking on read()...\n");
        n = read(g_fd, &reply, sizeof(reply));
        if (n < 0) { perror("read"); close(g_fd); return 1; }

        printf("[client2] REPLY: tx_id=%u result=%d data=\"%.*s\"\n",
               reply.tx_id, reply.result,
               (int)reply.data_len, reply.data);
    }

    close(g_fd);
    return 0;
}

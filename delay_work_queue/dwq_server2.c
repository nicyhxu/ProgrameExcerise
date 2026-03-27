/**********************************************
 * dwq_server2.c — 用户空间 server 进程
 *
 * 类比 Android Binder Server（带 joinThreadPool looper）：
 *   1. open("/dev/dwq")
 *   2. ioctl(DWQ_IOC_REGISTER, handle=1)   ← addService
 *   3. 循环：
 *        read()  → 阻塞等请求 (BR_TRANSACTION)
 *        处理
 *        write(REPLY)                       ← BC_REPLY
 *
 * Compile: gcc -o dwq_server2 dwq_server2.c
 * Run:     ./dwq_server2
 *********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "dwq_uapi.h"

#define DEV_PATH  "/dev/dwq"
#define MY_HANDLE 1           /* 本 server 注册的 handle 编号 */

static volatile int running = 1;
static void on_signal(int s) { (void)s; running = 0; }

int main(void)
{
    int fd;
    struct dwq_reg_info reg;
    struct dwq_msg      req;
    struct dwq_msg      reply_msg;

    signal(SIGINT,  on_signal);
    signal(SIGTERM, on_signal);

    fd = open(DEV_PATH, O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    /* ---- 注册服务 (类比 addService) ---- */
    memset(&reg, 0, sizeof(reg));
    reg.handle = MY_HANDLE;
    strncpy(reg.name, "echo-service", sizeof(reg.name) - 1);

    if (ioctl(fd, DWQ_IOC_REGISTER, &reg) < 0) {
        perror("ioctl REGISTER");
        close(fd);
        return 1;
    }
    printf("[server2] pid=%d  registered handle=%d name=\"%s\"\n",
           getpid(), MY_HANDLE, reg.name);
    printf("[server2] waiting for requests...\n");

    /* ---- looper (类比 joinThreadPool) ---- */
    while (running) {
        ssize_t n;

        /* 阻塞等请求 — 类比 BR_TRANSACTION */
        n = read(fd, &req, sizeof(req));
        if (n < 0) {
            if (running) perror("read");
            break;
        }

        printf("[server2] got request: tx_id=%u cmd=%u data=\"%.*s\"\n",
               req.tx_id, req.cmd,
               (int)req.data_len, req.data);

        /* 模拟处理 */
        sleep(1);

        /* ---- 回复 (类比 BC_REPLY) ---- */
        memset(&reply_msg, 0, sizeof(reply_msg));
        reply_msg.handle   = MY_HANDLE;
        reply_msg.cmd      = DWQ_CMD_REPLY;
        reply_msg.tx_id    = req.tx_id;          /* 必须回传 tx_id */
        snprintf(reply_msg.data, DWQ_MAX_DATA,
                 "echo: %.*s", (int)req.data_len, req.data);
        reply_msg.data_len = strlen(reply_msg.data);

        n = write(fd, &reply_msg, sizeof(reply_msg));
        if (n < 0) { perror("write reply"); break; }

        printf("[server2] replied tx_id=%u \"%s\"\n",
               reply_msg.tx_id, reply_msg.data);
    }

    /* ---- 注销 (类比 removeService) ---- */
    ioctl(fd, DWQ_IOC_UNREGISTER, 0);
    close(fd);
    printf("[server2] exited\n");
    return 0;
}

/**********************************************
 * dwq_server2.c — 用户空间 server（支持 callback + broadcast）
 *
 * 流程：
 *   1. 注册 handle=1
 *   2. read() 等请求
 *   3. 若请求带 callback_handle：
 *        处理完后 write(handle=callback_handle, cmd=EVENT) 回调 client
 *        （不走 REPLY 路径）
 *      否则：
 *        write(cmd=REPLY, tx_id=原值) 普通回复
 *   4. ioctl(BROADCAST) 广播事件给所有 subscriber
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
#define MY_HANDLE 1

static volatile int running = 1;
static void on_signal(int s) { (void)s; running = 0; }

static int g_fd;

/* 广播事件给所有订阅者 */
static void do_broadcast(unsigned int handle, unsigned int event_id,
                          const char *msg)
{
    struct dwq_broadcast bc;
    memset(&bc, 0, sizeof(bc));
    bc.handle   = handle;
    bc.event_id = event_id;
    bc.data_len = strlen(msg);
    strncpy(bc.data, msg, DWQ_MAX_DATA - 1);

    if (ioctl(g_fd, DWQ_IOC_BROADCAST, &bc) < 0)
        perror("ioctl BROADCAST");
    else
        printf("[server2] BROADCAST handle=%u event=%u msg=\"%s\"\n",
               handle, event_id, msg);
}

int main(void)
{
    struct dwq_reg_info reg;
    struct dwq_msg      req;
    int req_count = 0;

    signal(SIGINT,  on_signal);
    signal(SIGTERM, on_signal);

    g_fd = open(DEV_PATH, O_RDWR);
    if (g_fd < 0) { perror("open"); return 1; }

    /* ---- 注册服务 ---- */
    memset(&reg, 0, sizeof(reg));
    reg.handle = MY_HANDLE;
    strncpy(reg.name, "echo-service", sizeof(reg.name) - 1);
    if (ioctl(g_fd, DWQ_IOC_REGISTER, &reg) < 0) {
        perror("ioctl REGISTER");
        close(g_fd);
        return 1;
    }
    printf("[server2] pid=%d  registered handle=%d\n", getpid(), MY_HANDLE);
    printf("[server2] waiting for requests...\n");

    while (running) {
        ssize_t n;
        char reply_data[DWQ_MAX_DATA];

        n = read(g_fd, &req, sizeof(req));
        if (n < 0) {
            if (running) perror("read");
            break;
        }

        req_count++;
        printf("[server2] request #%d tx_id=%u cmd=%u cb_handle=%u data=\"%.*s\"\n",
               req_count, req.tx_id, req.cmd, req.callback_handle,
               (int)req.data_len, req.data);

        /* 模拟处理 */
        sleep(1);

        snprintf(reply_data, sizeof(reply_data),
                 "echo[%d]: %.*s", req_count, (int)req.data_len, req.data);

        if (req.callback_handle != 0) {
            /* ---- Callback 模式：反向调用 client 注册的 callback handle ---- */
            /* 类比 Binder server 持有 client IBinder 并调用 onCallback()      */
            struct dwq_msg cb_msg;
            memset(&cb_msg, 0, sizeof(cb_msg));
            cb_msg.handle   = req.callback_handle;  /* 目标=client 的 callback handle */
            cb_msg.cmd      = DWQ_CMD_EVENT;
            cb_msg.tx_id    = req.tx_id;
            cb_msg.data_len = strlen(reply_data);
            strncpy(cb_msg.data, reply_data, DWQ_MAX_DATA - 1);

            n = write(g_fd, &cb_msg, sizeof(cb_msg));
            if (n < 0) {
                /* callback handle 已注销（client 退出），忽略此次回调 */
                fprintf(stderr, "[server2] CALLBACK handle=%u gone (client exited), skip\n",
                        req.callback_handle);
            } else {
                printf("[server2] CALLBACK → handle=%u \"%s\"\n",
                       req.callback_handle, reply_data);
            }

        } else {
            /* ---- 普通 Reply 模式 ---- */
            struct dwq_msg reply_msg;
            memset(&reply_msg, 0, sizeof(reply_msg));
            reply_msg.handle   = MY_HANDLE;
            reply_msg.cmd      = DWQ_CMD_REPLY;
            reply_msg.tx_id    = req.tx_id;
            reply_msg.data_len = strlen(reply_data);
            strncpy(reply_msg.data, reply_data, DWQ_MAX_DATA - 1);

            n = write(g_fd, &reply_msg, sizeof(reply_msg));
            if (n < 0)
                perror("write reply");
            else
                printf("[server2] REPLY tx_id=%u \"%s\"\n",
                       req.tx_id, reply_data);
        }

        /* ---- 每处理 3 个请求就广播一次状态事件 ---- */
        if (req_count % 3 == 0) {
            char bc_msg[DWQ_MAX_DATA];
            snprintf(bc_msg, sizeof(bc_msg),
                     "server processed %d requests", req_count);
            do_broadcast(MY_HANDLE, req_count, bc_msg);
        }
    }

    ioctl(g_fd, DWQ_IOC_UNREGISTER, 0);
    close(g_fd);
    printf("[server2] exited\n");
    return 0;
}

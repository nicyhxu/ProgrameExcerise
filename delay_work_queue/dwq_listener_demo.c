/**********************************************
 * dwq_listener_demo.c — Listener / Subscribe 模式演示
 *
 * 类比 Android IServiceCallback / DeathRecipient：
 *   1. open("/dev/dwq")
 *   2. ioctl(DWQ_IOC_SUBSCRIBE, handle=1)  ← 订阅 handle=1 的广播
 *   3. 循环 read() 阻塞等广播事件           ← 类比 onEvent() 回调
 *
 * 可以启动多个 listener 实例，server2 广播时所有人都收到。
 *
 * Compile: gcc -o dwq_listener_demo dwq_listener_demo.c
 * Run:     ./dwq_listener_demo
 *********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "dwq_uapi.h"

#define DEV_PATH       "/dev/dwq"
#define SUBSCRIBE_HANDLE  1

static volatile int running = 1;
static void on_signal(int s) { (void)s; running = 0; }

int main(void)
{
    int fd;
    struct dwq_reply event;
    unsigned int h = SUBSCRIBE_HANDLE;

    signal(SIGINT,  on_signal);
    signal(SIGTERM, on_signal);

    fd = open(DEV_PATH, O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    /* ---- 订阅广播 (类比 registerCallback) ---- */
    if (ioctl(fd, DWQ_IOC_SUBSCRIBE, h) < 0) {
        perror("ioctl SUBSCRIBE");
        close(fd);
        return 1;
    }
    printf("[listener] pid=%d  subscribed handle=%u\n", getpid(), h);
    printf("[listener] waiting for broadcast events...\n");

    /* ---- 事件循环 (类比 onEvent 回调被触发) ---- */
    while (running) {
        ssize_t n = read(fd, &event, sizeof(event));
        if (n < 0) {
            if (running) perror("read");
            break;
        }

        /* cmd=DWQ_CMD_EVENT 表示广播事件 */
        printf("[listener] EVENT handle=%u tx_id=%u result=%d data=\"%.*s\"\n",
               event.handle, event.tx_id, event.result,
               (int)event.data_len, event.data);
    }

    /* ---- 取消订阅 ---- */
    ioctl(fd, DWQ_IOC_UNSUBSCRIBE, h);
    close(fd);
    printf("[listener] exited\n");
    return 0;
}

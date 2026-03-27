/**********************************************
 * dwq_client2.c — 用户空间 client 进程
 *
 * 类比 Android Binder Client (BpBinder::transact)：
 *   1. open("/dev/dwq")
 *   2. write(handle=1, cmd=WORK, data)  → BC_TRANSACTION
 *      驱动查 handle 表 → 路由到 server2 的 pending_list → 唤醒 server2
 *   3. read()  → 阻塞等 server2 回复  (BR_REPLY)
 *
 * Compile: gcc -o dwq_client2 dwq_client2.c
 * Run:     ./dwq_client2 [message]
 *********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "dwq_uapi.h"

#define DEV_PATH     "/dev/dwq"
#define TARGET_HANDLE 1       /* 和 server2 注册的 handle 一致 */

int main(int argc, char *argv[])
{
    int              fd;
    struct dwq_msg   req;
    struct dwq_reply reply;
    const char      *data = (argc > 1) ? argv[1] : "hello from client2";
    ssize_t          n;

    fd = open(DEV_PATH, O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    printf("[client2] pid=%d  target handle=%d\n", getpid(), TARGET_HANDLE);

    /* ---- 发送请求 (BC_TRANSACTION) ---- */
    memset(&req, 0, sizeof(req));
    req.handle   = TARGET_HANDLE;
    req.cmd      = DWQ_CMD_WORK;
    req.data_len = strlen(data);
    strncpy(req.data, data, DWQ_MAX_DATA - 1);
    /* tx_id 由驱动自动分配，这里填 0 */

    n = write(fd, &req, sizeof(req));
    if (n < 0) { perror("write"); close(fd); return 1; }
    printf("[client2] sent request data=\"%s\"\n", data);

    /* ---- 等待回复 (BR_REPLY) ---- */
    printf("[client2] waiting for reply...\n");
    n = read(fd, &reply, sizeof(reply));
    if (n < 0) { perror("read"); close(fd); return 1; }

    printf("[client2] reply: tx_id=%u result=%d data=\"%.*s\"\n",
           reply.tx_id, reply.result,
           (int)reply.data_len, reply.data);

    close(fd);
    return (reply.result == DWQ_RESULT_OK) ? 0 : 1;
}

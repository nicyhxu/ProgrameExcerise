/**********************************************
 * test_dwq.c — userspace test program for /dev/dwq
 *
 * Compile:  gcc -o test_dwq test_dwq.c
 * Usage:    ./test_dwq [ping|work|query]
 *
 * Demonstrates the Binder-like handle/message protocol:
 *   write(fd, &dwq_msg,   sizeof(dwq_msg))   -- send request
 *   read (fd, &dwq_reply, sizeof(dwq_reply)) -- receive reply (blocks)
 *********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "dwq_uapi.h"

#define DEV_PATH "/dev/dwq"

static void print_reply(const struct dwq_reply *r)
{
    printf("  reply.handle  = %u\n",  r->handle);
    printf("  reply.cmd     = %u\n",  r->cmd);
    printf("  reply.result  = %d (%s)\n", r->result,
           r->result == DWQ_RESULT_OK ? "OK" : "ERROR");
    printf("  reply.data    = \"%.*s\"\n", (int)r->data_len, r->data);
}

/* send a message and wait for the reply */
static int transact(int fd, __u32 handle, __u32 cmd,
                    const char *data, struct dwq_reply *out)
{
    struct dwq_msg msg;
    ssize_t n;

    memset(&msg, 0, sizeof(msg));
    msg.handle = handle;
    msg.cmd    = cmd;
    if (data) {
        size_t len = strlen(data);
        if (len >= DWQ_MAX_DATA) len = DWQ_MAX_DATA - 1;
        memcpy(msg.data, data, len);
        msg.data_len = (__u32)len;
    }

    /* --- write request --- */
    n = write(fd, &msg, sizeof(msg));
    if (n < 0) { perror("write"); return -1; }

    /* --- read reply (blocks for DWQ_CMD_WORK until work finishes) --- */
    n = read(fd, out, sizeof(*out));
    if (n < 0) { perror("read"); return -1; }

    return 0;
}

int main(int argc, char *argv[])
{
    const char *cmd_str = (argc > 1) ? argv[1] : "work";
    struct dwq_reply reply;
    int fd, ret = 0;

    fd = open(DEV_PATH, O_RDWR);
    if (fd < 0) {
        perror("open " DEV_PATH);
        fprintf(stderr, "hint: sudo mknod /dev/dwq c <major> 0 && sudo chmod 666 /dev/dwq\n");
        return 1;
    }

    printf("[test_dwq] cmd=%s  handle=%d\n", cmd_str, DWQ_HANDLE_DEFAULT);

    if (strcmp(cmd_str, "ping") == 0) {
        /* ---- PING: expect immediate pong ---- */
        printf("Sending PING to handle %d ...\n", DWQ_HANDLE_DEFAULT);
        ret = transact(fd, DWQ_HANDLE_DEFAULT, DWQ_CMD_PING, "hello", &reply);

    } else if (strcmp(cmd_str, "work") == 0) {
        /* ---- WORK: blocks ~3 seconds ---- */
        printf("Sending WORK to handle %d (will block ~3s) ...\n",
               DWQ_HANDLE_DEFAULT);
        ret = transact(fd, DWQ_HANDLE_DEFAULT, DWQ_CMD_WORK,
                       "payload-from-userspace", &reply);

    } else if (strcmp(cmd_str, "query") == 0) {
        /* ---- QUERY: ask status ---- */
        printf("Sending QUERY to handle %d ...\n", DWQ_HANDLE_DEFAULT);
        ret = transact(fd, DWQ_HANDLE_DEFAULT, DWQ_CMD_QUERY, NULL, &reply);

    } else if (strcmp(cmd_str, "badhandle") == 0) {
        /* ---- test unknown handle ---- */
        printf("Sending PING to handle 7 (unregistered) ...\n");
        ret = transact(fd, 7, DWQ_CMD_PING, NULL, &reply);

    } else {
        fprintf(stderr, "unknown cmd: %s\n", cmd_str);
        fprintf(stderr, "usage: %s [ping|work|query|badhandle]\n", argv[0]);
        close(fd);
        return 1;
    }

    if (ret == 0) {
        printf("Reply received:\n");
        print_reply(&reply);
    }

    close(fd);
    return (ret == 0 && reply.result == DWQ_RESULT_OK) ? 0 : 1;
}

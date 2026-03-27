/**********************************************
 * dwq_server.c — server process
 *
 * Like a Binder service: mmap /dev/dwq, then poll the shared memory
 * for incoming requests (state == PENDING), process them, write reply.
 *
 * Compile: gcc -o dwq_server dwq_server.c
 * Run:     ./dwq_server
 *********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "dwq_uapi.h"

/* matches kernel struct dwq_shm */
struct dwq_shm {
    unsigned int seq;
    unsigned int handle;
    unsigned int cmd;
    unsigned int state;             /* 0=idle 1=pending 2=done */
    char         data[DWQ_MAX_DATA];
    char         reply_data[DWQ_MAX_DATA];
};

#define DWQ_SHM_IDLE     0
#define DWQ_SHM_PENDING  1
#define DWQ_SHM_DONE     2

static void handle_request(struct dwq_shm *shm)
{
    printf("[server] seq=%u handle=%u cmd=%u data=\"%.*s\"\n",
           shm->seq, shm->handle, shm->cmd,
           DWQ_MAX_DATA, shm->data);

    /* simulate processing */
    sleep(1);

    /* write reply into shared memory */
    snprintf(shm->reply_data, DWQ_MAX_DATA,
             "server handled seq=%u cmd=%u", shm->seq, shm->cmd);

    /* publish: mark done so client sees it */
    __sync_synchronize();       /* memory barrier */
    shm->state = DWQ_SHM_DONE;

    printf("[server] replied: \"%s\"\n", shm->reply_data);
}

int main(void)
{
    int fd;
    struct dwq_shm *shm;
    unsigned int last_seq = 0;

    fd = open("/dev/dwq", O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    /* map the shared physical page – same page the client will map */
    shm = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) { perror("mmap"); close(fd); return 1; }

    printf("[server] pid=%d  shm mapped at %p\n", getpid(), shm);
    printf("[server] waiting for requests...\n");

    while (1) {
        /* spin-wait for a new pending request from client             */
        /* In a real driver you'd use poll()/select() or a futex here  */
        __sync_synchronize();

        if (shm->state == DWQ_SHM_PENDING && shm->seq != last_seq) {
            last_seq = shm->seq;
            handle_request(shm);
        }
        usleep(1000);   /* 1ms poll interval */
    }

    munmap(shm, 4096);
    close(fd);
    return 0;
}

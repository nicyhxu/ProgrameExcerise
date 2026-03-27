/**********************************************
 * dwq_uapi.h — shared user/kernel ABI for /dev/dwq
 *
 * Modelled on Binder's binder.h:
 *   dwq_msg       ≈ binder_transaction_data
 *   dwq_reply     ≈ binder_transaction_data (reply direction)
 *   dwq_reg_info  ≈ flat_binder_object (service registration)
 *   handle        ≈ flat_binder_object.handle
 *   tx_id         ≈ binder_transaction.debug_id
 *********************************************/
#ifndef DWQ_UAPI_H
#define DWQ_UAPI_H

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/ioctl.h>
#else
#include <stdint.h>
#include <sys/ioctl.h>
typedef uint32_t __u32;
typedef int32_t  __s32;
#endif

/* ------------------------------------------------------------------ */
/* Command codes  (dwq_msg.cmd)                                        */
/* ------------------------------------------------------------------ */
#define DWQ_CMD_PING    1   /* client→server: echo test, reply immediately  */
#define DWQ_CMD_WORK    2   /* client→server: do async work                 */
#define DWQ_CMD_QUERY   3   /* client→server: query handle status           */
#define DWQ_CMD_REPLY   4   /* server→driver: reply to a client transaction */

/* ------------------------------------------------------------------ */
/* Handle IDs                                                           */
/*   handle=0  : built-in workqueue service (backward compat)         */
/*   handle=1~7: user-space server registered via DWQ_IOC_REGISTER    */
/* ------------------------------------------------------------------ */
#define DWQ_HANDLE_DEFAULT  0
#define DWQ_HANDLE_MAX      8

/* ------------------------------------------------------------------ */
/* Request / Reply message                                             */
/* ------------------------------------------------------------------ */
#define DWQ_MAX_DATA  128

struct dwq_msg {
    __u32  handle;              /* target handle                              */
    __u32  cmd;                 /* DWQ_CMD_xxx                                */
    __u32  data_len;            /* valid bytes in data[]                      */
    __u32  tx_id;               /* transaction ID:                            */
                                /*   client→server: driver fills automatically */
                                /*   server→driver (REPLY): echo back tx_id  */
    char   data[DWQ_MAX_DATA];
};

struct dwq_reply {
    __u32  handle;
    __u32  cmd;
    __u32  tx_id;               /* matches the original request tx_id        */
    __s32  result;              /* 0=OK, <0=error                             */
    __u32  data_len;
    char   data[DWQ_MAX_DATA];
};

/* ------------------------------------------------------------------ */
/* Service registration  (server calls DWQ_IOC_REGISTER)              */
/* 类比 Binder addService / flat_binder_object                         */
/* ------------------------------------------------------------------ */
struct dwq_reg_info {
    __u32  handle;              /* handle to register as (1~7)               */
    char   name[32];            /* service name (debug)                       */
};

/* ------------------------------------------------------------------ */
/* Result codes                                                         */
/* ------------------------------------------------------------------ */
#define DWQ_RESULT_OK       0
#define DWQ_RESULT_ENODEV  -19  /* handle not registered             */
#define DWQ_RESULT_EBUSY   -16  /* handle already has a server       */
#define DWQ_RESULT_EINVAL  -22  /* bad argument                      */
#define DWQ_RESULT_EPERM   -1   /* operation not permitted           */

/* ------------------------------------------------------------------ */
/* ioctl commands                                                       */
/* ------------------------------------------------------------------ */
#define DWQ_IOC_MAGIC      'd'
#define DWQ_IOC_STOP       _IO ('d', 2)                         /* cancel workqueue work     */
#define DWQ_IOC_STATUS     _IOR('d', 3, int)                    /* query pending queue depth */
#define DWQ_IOC_REGISTER   _IOW('d', 4, struct dwq_reg_info)   /* server: register handle   */
#define DWQ_IOC_UNREGISTER _IO ('d', 5)                         /* server: unregister        */

#endif /* DWQ_UAPI_H */

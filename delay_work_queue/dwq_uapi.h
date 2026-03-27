/**********************************************
 * dwq_uapi.h — shared user/kernel ABI for /dev/dwq
 *
 * Modelled on Binder's binder.h:
 *   dwq_msg        ≈ binder_transaction_data
 *   dwq_reply      ≈ binder_transaction_data (reply direction)
 *   dwq_reg_info   ≈ flat_binder_object (service registration)
 *   callback_handle≈ client passes own IBinder to server for callback
 *   SUBSCRIBE      ≈ IServiceCallback / DeathRecipient registration
 *   BROADCAST      ≈ server-initiated event push to all listeners
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
#define DWQ_CMD_PING      1   /* client→server: echo test                   */
#define DWQ_CMD_WORK      2   /* client→server: do async work               */
#define DWQ_CMD_QUERY     3   /* client→server: query handle status         */
#define DWQ_CMD_REPLY     4   /* server→driver: reply to a client tx        */
#define DWQ_CMD_EVENT     5   /* server→client: async event/callback push   */

/* ------------------------------------------------------------------ */
/* Handle IDs                                                           */
/*   handle=0  : built-in workqueue service                           */
/*   handle=1~7: user-space server (DWQ_IOC_REGISTER)                 */
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
    __u32  tx_id;               /* transaction ID (driver fills on TX)        */
    __u32  callback_handle;     /* CALLBACK mode: client's own handle for     */
                                /* server to call back on; 0 = not used       */
                                /* 类比 client 把自己的 IBinder 传给 server   */
    char   data[DWQ_MAX_DATA];
};

struct dwq_reply {
    __u32  handle;
    __u32  cmd;
    __u32  tx_id;
    __s32  result;
    __u32  data_len;
    char   data[DWQ_MAX_DATA];
};

/* ------------------------------------------------------------------ */
/* Service registration                                                 */
/* ------------------------------------------------------------------ */
struct dwq_reg_info {
    __u32  handle;
    char   name[32];
};

/* ------------------------------------------------------------------ */
/* Broadcast event  (used with DWQ_IOC_BROADCAST)                     */
/* ------------------------------------------------------------------ */
struct dwq_broadcast {
    __u32  handle;              /* which handle's subscribers to notify       */
    __u32  event_id;            /* application-defined event type             */
    __u32  data_len;
    char   data[DWQ_MAX_DATA];
};

/* ------------------------------------------------------------------ */
/* Result codes                                                         */
/* ------------------------------------------------------------------ */
#define DWQ_RESULT_OK       0
#define DWQ_RESULT_ENODEV  -19
#define DWQ_RESULT_EBUSY   -16
#define DWQ_RESULT_EINVAL  -22
#define DWQ_RESULT_EPERM   -1

/* ------------------------------------------------------------------ */
/* ioctl commands                                                       */
/* ------------------------------------------------------------------ */
#define DWQ_IOC_MAGIC       'd'
#define DWQ_IOC_STOP        _IO ('d', 2)
#define DWQ_IOC_STATUS      _IOR('d', 3, int)
#define DWQ_IOC_REGISTER    _IOW('d', 4, struct dwq_reg_info)
#define DWQ_IOC_UNREGISTER  _IO ('d', 5)
#define DWQ_IOC_SUBSCRIBE   _IOW('d', 6, unsigned int)   /* arg = handle to subscribe  */
#define DWQ_IOC_UNSUBSCRIBE _IOW('d', 7, unsigned int)   /* arg = handle to unsub      */
#define DWQ_IOC_BROADCAST   _IOW('d', 8, struct dwq_broadcast) /* server push event   */

#endif /* DWQ_UAPI_H */

/**********************************************
 * dwq_uapi.h — shared user/kernel ABI for /dev/dwq
 *
 * Both the kernel driver and userspace test program include this file.
 * Modelled loosely on Binder's flat_binder_object / binder_transaction_data.
 *********************************************/
#ifndef DWQ_UAPI_H
#define DWQ_UAPI_H

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
typedef uint32_t __u32;
typedef int32_t  __s32;
#endif

/* ------------------------------------------------------------------ */
/* Command codes  (dwq_msg.cmd)                                        */
/* ------------------------------------------------------------------ */
#define DWQ_CMD_PING    1   /* echo back immediately, no workqueue      */
#define DWQ_CMD_WORK    2   /* queue a delayed work on the target handle */
#define DWQ_CMD_QUERY   3   /* query current state of a handle           */

/* ------------------------------------------------------------------ */
/* Handle IDs                                                           */
/*   Like Binder handles: an integer that names a "service" inside     */
/*   the kernel driver.  Currently only handle 0 is registered.        */
/* ------------------------------------------------------------------ */
#define DWQ_HANDLE_DEFAULT  0   /* the built-in delayed-work service */
#define DWQ_HANDLE_MAX      8   /* max registered handles            */

/* ------------------------------------------------------------------ */
/* Request message  (userspace → kernel via write())                   */
/* ------------------------------------------------------------------ */
#define DWQ_MAX_DATA  128

struct dwq_msg {
    __u32  handle;              /* target handle (like Binder's flat_binder_object.handle) */
    __u32  cmd;                 /* DWQ_CMD_xxx                                              */
    __u32  data_len;            /* number of valid bytes in data[]                          */
    char   data[DWQ_MAX_DATA];  /* optional payload (e.g. work parameters)                  */
};

/* ------------------------------------------------------------------ */
/* Reply message  (kernel → userspace via read())                      */
/* ------------------------------------------------------------------ */
#define DWQ_RESULT_OK       0
#define DWQ_RESULT_ENODEV  -19  /* handle not registered  */
#define DWQ_RESULT_EBUSY   -16  /* handle already running */
#define DWQ_RESULT_EINVAL  -22  /* bad cmd or data        */

struct dwq_reply {
    __u32  handle;              /* mirrors the request handle   */
    __u32  cmd;                 /* mirrors the request cmd      */
    __s32  result;              /* DWQ_RESULT_xxx               */
    __u32  data_len;
    char   data[DWQ_MAX_DATA];  /* response payload             */
};

/* ------------------------------------------------------------------ */
/* ioctl definitions  (kept from previous version)                     */
/* ------------------------------------------------------------------ */
#define DWQ_IOC_MAGIC   'd'
#define DWQ_IOC_START   _IOW('d', 1, unsigned long) /* arg = delay ms   */
#define DWQ_IOC_STOP    _IO ('d', 2)
#define DWQ_IOC_STATUS  _IOR('d', 3, int)

#endif /* DWQ_UAPI_H */

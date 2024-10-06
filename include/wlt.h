#ifndef __WLT_H__
#define __WLT_H__

#include <stddef.h>
#include <stdint.h>
#include <linux/wireless.h>

#define WLT_WDEV_NAMSIZ IFNAMSIZ

/*
 * When setting a wireless device mode, user have to pass a mode value.
 * So user have to know which type is for and we can provide uni-type
 * by typedef.
 */
typedef uint32_t wlt_wdev_mode_t;

/* Structure for wireless device in Wireless Tools */
struct wlt_wdev {
    intptr_t descriptor;		/* wireless device descriptor */
    int index;
    wlt_wdev_mode_t prevmode;	/* previous mode of wireless device */
    wlt_wdev_mode_t mode;		/* changed mode by the program */
    char name[WLT_WDEV_NAMSIZ];	/* wireless device name */
    int32_t chann;		/* current channel of wireless device */
};

struct wlt_wdev *wlt_wdev_init(char *name);

void wlt_wdev_destroy(struct wlt_wdev *wdev);

/*
 * Basic operations for wlt_wdev structure. These operations contain
 * OS dependent codes.
 */

int wlt_wdev_open(struct wlt_wdev *wdevp);

ssize_t wlt_wdev_read(struct wlt_wdev *wdevp,
		      uint8_t *buffp,
		      unsigned int size);

void wlt_wdev_close(struct wlt_wdev *wdevp);

#include "io_multiplex.h"

/* wlt_wdev_getdesc: return descriptor for io multiplexing */
io_desc_t wlt_wdev_getdesc(struct wlt_wdev *iop);

#include "wlt_ieee80211_pcap.h"

#endif

#ifndef __WLT_H__
#define __WLT_H__

#include <stdint.h>
#include <linux/wireless.h>

/* Structure for wireless device in Wireless Tools */
struct wlt_dev {
    int sockfd;			/* socket for wireless device */
    int index;
    uint32_t prevmode;		/* initial mode of wireless device */
    uint32_t mode;		/* changed mode by the program */
    char name[IFNAMSIZ + 1];	/* wireless device name */
    int32_t chann;		/* current channel of wireless device */
};

struct wlt_dev *wlt_init(char *name);
void wlt_destroy(struct wlt_dev *wdev);

#include "wlt_ieee80211_pcap.h"

#endif

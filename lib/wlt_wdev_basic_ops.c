#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include "wlt.h"

/*
 * Basic operations for wlt_wdev structure. These operations contain
 * OS dependent codes.
 */

int wlt_wdev_open(struct wlt_wdev *wdevp)
{
    if (!wdevp)
	goto OUT;
    
    wdevp->descriptor = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (wdevp->descriptor == -1) {
	perror("WLT_WDEV_SOCK_OPEN (socket)");
	goto OUT;
    }
    return 0;

OUT:
    return -1;
}

ssize_t wlt_wdev_read(struct wlt_wdev *wdevp,
		      uint8_t *buffp,
		      unsigned int size)
{
    if (!wdevp || !buffp)
	return -1;

    return read(wdevp->descriptor, buffp, size);
}

void wlt_wdev_close(struct wlt_wdev *wdevp)
{
    if (!wdevp)
	return ;

    close(wdevp->descriptor);
}

/* wlt_wdev_getdesc: return descriptor for io multiplexing */
io_desc_t wlt_wdev_getdesc(struct wlt_wdev *iop)
{
    if (!iop)
	return -1;
    
    return iop->descriptor;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/wireless.h>
#include "wlt.h"
#include "wlt_wnetdev.h"

/* wlt_ioctl_if: do ioctl of given (network interface) name */
static int wlt_ioctl_if(int fd,
			const char *name,
			unsigned long req,
			struct ifreq *ifr)
{
    if (fd < 0 || name == NULL || ifr == NULL)
	goto OUT;

    strncpy(WLT_GETMEMB_IF_IFNAME(ifr), name, IFNAMSIZ);
    if (ioctl(fd, req, ifr) == -1) {
	perror("ioctl");
	goto OUT;
    }
    return 0;

OUT:
    return -1;
}

/* wlt_ioctl_iw: do ioctl of given (wireless interface) name */
static int wlt_ioctl_iw(int fd,
			const char *name,
			unsigned long req,
			struct iwreq *iwr)
{
    if (fd < 0 || name == NULL || iwr == NULL)
	goto OUT;

    strncpy(WLT_GETMEMB_IW_IFNAME(iwr), name, IFNAMSIZ);
    if (ioctl(fd, req, iwr) == -1) {
	perror("ioctl");
	goto OUT;
    }
    return 0;

OUT:
    return -1;
}

#define WLT_GET_IW_MODE(fd, name, iwr)			\
    wlt_ioctl_iw((fd), (name), (SIOCGIWMODE), (iwr))
#define WLT_GET_IW_CHANN(fd, name, iwr)			\
    wlt_ioctl_iw((fd), (name), (SIOCGIWFREQ), (iwr))
#define WLT_SET_IW_MODE(fd, name, iwr)			\
    wlt_ioctl_iw((fd), (name), (SIOCSIWMODE), (iwr))
#define WLT_SET_IW_CHANN(fd, name, iwr)			\
    wlt_ioctl_iw((fd), (name), (SIOCSIWFREQ), (iwr))

#define WLT_SET_IF_FLAG(fd, name, ifr)			\
    wlt_ioctl_if((fd), (name), (SIOCSIFFLAGS), (ifr))
#define WLT_GET_IF_INDEX(fd, name, ifr)			\
    wlt_ioctl_if((fd), (name), (SIOCGIFINDEX), (ifr))
#define WLT_GET_IF_HWADDR(fd, name, ifr)		\
    wlt_ioctl_if((fd), (name), (SIOCGIFHWADDR), (ifr))

int wlt_getindex(struct wlt_wdev *wdevp)
{
    struct ifreq ifr;
    
    if (!wdevp)
	goto OUT;

    if (WLT_GET_IF_INDEX(wdevp->descriptor, wdevp->name, &ifr) == -1)
	goto OUT;
    
    wdevp->index = WLT_GETMEMB_IF_IFINDEX(&ifr);
    return 0;

OUT:
    return -1;
}

int wlt_getmode(struct wlt_wdev *wdevp)
{
    struct iwreq iwr;

    if (!wdevp)
	goto OUT;

    if (WLT_GET_IW_MODE(wdevp->descriptor, wdevp->name, &iwr) == -1)
	goto OUT;

    wdevp->prevmode = WLT_GETMEMB_IW_MODE(&iwr);
    wdevp->mode = WLT_GETMEMB_IW_MODE(&iwr);
    return 0;

OUT:
    return -1;
}

int wlt_setmode(struct wlt_wdev *wdevp, wlt_wdev_mode_t mode)
{
    char *modes[] = {
	"driver decided mode",
	"single cell network",
	"multi cell network, roaming",
	"synchronization master or access point",
	"wireless repeater (forwarder)",
	"secondary master / repeater (backup)",
	"passive monitor (listen only)",
	"mesh (IEEE 802.11s) network"
    };
    int res;
    struct ifreq ifr;
    struct iwreq iwr;
    
    if (!wdevp)
	goto OUT;

    puts("Interface down..");

    WLT_WDEV_IFDOWN(&ifr);
    if (WLT_SET_IF_FLAG(wdevp->descriptor, wdevp->name, &ifr) == -1)
	goto OUT;

    printf("Changing mode from %s to %s..\n", modes[wdevp->prevmode],
	   modes[mode]);

    WLT_SETMEMB_IW_MODE(&iwr, mode);
    res = WLT_SET_IW_MODE(wdevp->descriptor, wdevp->name, &iwr);
    if (res == -1)		/* although it's failed, just move on */
	puts("failed");

    puts("Interface up..");

    WLT_WDEV_IFUP(&ifr);
    if (WLT_SET_IF_FLAG(wdevp->descriptor, wdevp->name, &ifr) == -1)
	goto OUT;
    
    if (res == -1)
	goto OUT;
    
    wdevp->mode = mode;
    return 0;

OUT:
    return -1;
}

int wlt_setchann(struct wlt_wdev *wdevp, int chann)
{
    struct iwreq iwr;
    
    if (!wdevp)
	goto OUT;

    WLT_SETMEMB_IW_FREQM(&iwr, WLT_IEEE80211_24_CHANN2FREQ_M(chann));
    WLT_SETMEMB_IW_FREQE(&iwr, 1);
    if (WLT_SET_IW_CHANN(wdevp->descriptor, wdevp->name, &iwr) == -1)
	goto OUT;

    wdevp->chann = chann;
    return 0;

OUT:
    return -1;
}

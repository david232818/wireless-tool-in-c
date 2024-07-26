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
#include "wlt_ioctl.h"

/* prv_wlt_ioctl_if: do ioctl of given (network interface) name */
static int _wlt_ioctl_if(const int fd,
			   const char *name,
			   unsigned long req,
			   struct ifreq *ifr)
{
    if (fd < 0 || name == NULL || ifr == NULL)
	return -1;

    strncpy(ifr->ifr_name, name, IFNAMSIZ);
    if (ioctl(fd, req, ifr) == -1) {
	perror("ioctl");
	return -1;
    }
    return 0;
}

/* prv_wlt_ioctl_iw: do ioctl of given (wireless interface) name */
static int _wlt_ioctl_iw(const int fd,
			   const char *name,
			   const unsigned long req,
			   struct iwreq *iwr)
{
    if (fd < 0 || name == NULL || iwr == NULL)
	return -1;

    strncpy(iwr->ifr_ifrn.ifrn_name, name, IFNAMSIZ);
    if (ioctl(fd, req, iwr) == -1) {
	perror("ioctl");
	return -1;
    }
    return 0;
}

#define WLT_GET_IW_MODE(fd, name, iwr)				\
    _wlt_ioctl_iw((fd), (name), (SIOCGIWMODE), (iwr))
#define WLT_GET_IW_CHANN(fd, name, iwr)				\
    _wlt_ioctl_iw((fd), (name), (SIOCGIWFREQ), (iwr))
#define WLT_SET_IW_MODE(fd, name, iwr)				\
    _wlt_ioctl_iw((fd), (name), (SIOCSIWMODE), (iwr))
#define WLT_SET_IW_CHANN(fd, name, iwr)				\
    _wlt_ioctl_iw((fd), (name), (SIOCSIWFREQ), (iwr))

#define WLT_SET_IF_FLAG(fd, name, ifr)				\
    _wlt_ioctl_if((fd), (name), (SIOCSIFFLAGS), (ifr))
#define WLT_GET_IF_INDEX(fd, name, ifr)				\
    _wlt_ioctl_if((fd), (name), (SIOCGIFINDEX), (ifr))
#define WLT_GET_IF_HWADDR(fd, name, ifr)			\
    _wlt_ioctl_if((fd), (name), (SIOCGIFHWADDR), (ifr))

int wlt_getindex(struct wlt_dev *wdev)
{
    struct ifreq ifr;
    
    if (wdev == NULL)
	return -1;

    if (WLT_GET_IF_INDEX(wdev->sockfd, wdev->name, &ifr) == -1)
	return -1;
    wdev->index = ifr.ifr_ifindex;
    return 0;
}

int wlt_getmode(struct wlt_dev *wdev)
{
    struct iwreq iwr;

    if (wdev == NULL)
	return -1;

    if (WLT_GET_IW_MODE(wdev->sockfd, wdev->name, &iwr) == -1)
	return -1;
    wdev->prevmode = iwr.u.mode;
    wdev->mode = iwr.u.mode;
    return 0;
}

int wlt_setmode(struct wlt_dev *wdev, const uint32_t mode)
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
    
    if (wdev == NULL)
	return -1;

    puts("Interface down..");
    ifr.ifr_flags |= ((short) ~0) & ~IFF_UP;
    if (WLT_SET_IF_FLAG(wdev->sockfd, wdev->name, &ifr) == -1)
	return -1;

    printf("Changing mode from %s to %s.. ", modes[wdev->prevmode],
	   modes[mode]);
    iwr.u.mode = mode;
    res = WLT_SET_IW_MODE(wdev->sockfd, wdev->name, &iwr);
    if (res == -1)
	puts("failed");

    puts("Interface up..");
    ifr.ifr_flags |= IFF_UP;
    if (WLT_SET_IF_FLAG(wdev->sockfd, wdev->name, &ifr) == -1)
	return -1;
    if (res == -1)
	return -1;
    wdev->mode = mode;
    return 0;
}

int wlt_setchann(struct wlt_dev *wdev, int chann)
{
    struct iwreq iwr;
    
    if (wdev == NULL)
	return -1;

    iwr.u.freq.m = WLT_IEEE80211_24_CHANN2FREQ_M(chann);
    iwr.u.freq.e = 1;
    if (WLT_SET_IW_CHANN(wdev->sockfd, wdev->name, &iwr) == -1)
	return -1;
    wdev->chann = chann;
    return 0;
}

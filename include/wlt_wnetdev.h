#ifndef __WLT_WNETDEV_H__
#define __WLT_WNETDEV_H__

#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/wireless.h>
#include "wlt.h"

#define WLT_IS_WDEV_MODE_MONITOR(mode) ((mode) == IW_MODE_MONITOR)

#define WLT_GETMEMB_IF_IFNAME(ifrp) ((ifrp)->ifr_name)

#define WLT_GETMEMB_IW_IFNAME(iwrp) ((iwrp)->ifr_ifrn.ifrn_name)

#define WLT_GETMEMB_IF_IFINDEX(ifrp) ((ifrp)->ifr_ifindex)

#define WLT_GETMEMB_IW_MODE(iwrp) ((iwrp)->u.mode)

#define WLT_SETMEMB_IF_IFFLAGS(ifrp, flags) \
    ((ifrp)->ifr_flags |= ((short) ~0) & (flags))

#define WLT_SETMEMB_IW_MODE(iwrp, mode) ((iwrp)->u.mode = (mode))

#define WLT_SETMEMB_IW_FREQM(iwrp, freqm) ((iwrp)->u.freq.m = (freqm))

#define WLT_SETMEMB_IW_FREQE(iwrp, freqe) ((iwrp)->u.freq.e = (freqe))

#define WLT_WDEV_IFDOWN(ifrp) WLT_SETMEMB_IF_IFFLAGS((ifrp), ~IFF_UP)

#define WLT_WDEV_IFUP(ifrp) WLT_SETMEMB_IF_IFFLAGS((ifrp), IFF_UP)

int wlt_getindex(struct wlt_wdev *wdev);

int wlt_getmode(struct wlt_wdev *wdev);

int wlt_setmode(struct wlt_wdev *wdev, wlt_wdev_mode_t mode);

int wlt_setchann(struct wlt_wdev *wdev, int chann);

#define WLT_WDEV_SET_MONITOR(wdevp) wlt_setmode((wdevp), IW_MODE_MONITOR)

#endif

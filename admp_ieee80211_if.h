#ifndef __IEEE80211_IF_H__
#define __IEEE80211_IF_H__

#include <stdint.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>

#include "admp_err.h"

struct admp_ieee80211_if_info {
	int fd;
	int index;
	uint32_t prev_mode;
	uint32_t mode;
	char name[IFNAMSIZ + 1];
	char presence[IFNAMSIZ + 1];
	struct sockaddr hwaddr;
};

admp_res_t ieee80211_ioctl_if(const int fd,
			      const char *name,
			      unsigned long req,
			      struct ifreq *ifr);

admp_res_t ieee80211_ioctl_iw(const int fd,
			      const char *name,
			      const unsigned long req,
			      struct iwreq *iwr);

#define IEEE80211_GET_IW_BY_NAME(fd, name, iwr)			\
    ieee80211_ioctl_iw((fd), (name), (SIOCGIWNAME), (iwr))
#define IEEE80211_GET_IW_MODE(fd, name, iwr)			\
    ieee80211_ioctl_iw((fd), (name), (SIOCGIWMODE), (iwr))
#define IEEE80211_GET_IW_CHANN(fd, name, iwr)			\
    ieee80211_ioctl_iw((fd), (name), (SIOCGIWFREQ), (iwr))
#define IEEE80211_SET_IW_MODE(fd, name, iwr)			\
    ieee80211_ioctl_iw((fd), (name), (SIOCSIWMODE), (iwr))

#define IEEE80211_SET_IF_FLAG(fd, name, ifr)			\
    ieee80211_ioctl_if((fd), (name), (SIOCSIFFLAGS), (ifr))
#define IEEE80211_GET_IF_INDEX(fd, name, ifr)			\
    ieee80211_ioctl_if((fd), (name), (SIOCGIFINDEX), (ifr))
#define IEEE80211_GET_IF_HWADDR(fd, name, ifr)			\
    ieee80211_ioctl_if((fd), (name), (SIOCGIFHWADDR), (ifr))

admp_res_t admp_ieee80211_if_init(const char *name,
				struct admp_ieee80211_if_info *info);
admp_res_t admp_ieee80211_if_destroy(struct admp_ieee80211_if_info *info);

#endif

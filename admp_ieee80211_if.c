#define _GNU_SOURCE /* To get defns of NI_MAXSERV and NI_MAXHOST */

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <linux/wireless.h>
#include "admp_ieee80211_if.h"
#include "admp_err.h"

admp_res_t ieee80211_ioctl_if(const int fd,
			      const char *name,
			      unsigned long req,
			      struct ifreq *ifr)
{
    if (ifr == NULL) {
	admp_printerr(ADMP_ERR_INVALID_ARGS, "ieee80211_ioctl_if");
	return -ADMP_ERR_INVALID_ARGS;
    }

    strncpy(ifr->ifr_name, name, IFNAMSIZ);
    if (ioctl(fd, req, ifr) == -1) {
	admp_printerr(ADMP_ERR_IFCMD, "ioctl");
	return -ADMP_ERR_IFCMD;
    }
    return ADMP_SUCCESS;
}

admp_res_t ieee80211_ioctl_iw(const int fd,
			      const char *name,
			      const unsigned long req,
			      struct iwreq *iwr)
{
    if (iwr == NULL) {
	admp_printerr(ADMP_ERR_INVALID_ARGS, "ieee80211_ioctl_iw");
	return -ADMP_ERR_INVALID_ARGS;
    }

    strncpy(iwr->ifr_ifrn.ifrn_name, name, IFNAMSIZ);
    if (ioctl(fd, req, iwr) == -1) {
	admp_printerr(ADMP_ERR_IWCMD, "ioctl");
	return -ADMP_ERR_IWCMD;
    }
    return ADMP_SUCCESS;
}

static admp_res_t ieee80211_set_if_mode(struct admp_ieee80211_if_info *info,
					struct ifreq *ifr,
					struct iwreq *iwr,
					const unsigned long mode)
{
    admp_res_t res;
    char *modes[8] = {
	"driver decided mode",
	"single cell network",
	"multi cell network, roaming",
	"synchronization master or access point",
	"wireless repeater (forwarder)",
	"secondary master / repeater (backup)",
	"passive monitor (listen only)",
	"mesh (IEEE 802.11s) network"
    };

    if (info == NULL || ifr == NULL || iwr == NULL) {
	admp_printerr(ADMP_ERR_INVALID_ARGS, "ieee80211_set_if_mode");
	return -ADMP_ERR_INVALID_ARGS;
    }

    puts("Interface Down..");
    ifr->ifr_flags |= (short) 0xffff;
    ifr->ifr_flags &= ~IFF_UP;
    res = IEEE80211_SET_IF_FLAG(info->fd, info->name, ifr);
    if (res != ADMP_SUCCESS)
	goto OUT;

    printf("%s to %s\n", modes[info->mode], modes[mode]);
    iwr->u.mode = mode;
    res = IEEE80211_SET_IW_MODE(info->fd, info->name, iwr);
    if (res != ADMP_SUCCESS)
	goto OUT;

    info->mode = mode;

    puts("Interface Up..");
    ifr->ifr_flags |= IFF_UP;
    res = IEEE80211_SET_IF_FLAG(info->fd, info->name, ifr);
    if (res != ADMP_SUCCESS)
	goto OUT;

OUT:
    return res;
}

admp_res_t admp_ieee80211_if_init(const char *name,
				  struct admp_ieee80211_if_info *info)
{
    admp_res_t res;
    struct iwreq iwr;
    struct ifreq ifr;

    if (name == NULL) {
	res = -ADMP_ERR_INVALID_ARGS;
	admp_printerr(-res,
		      "admp_ieee80211_if_init");
	goto OUT;
    }

    info->fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (info->fd == -1) {
	res = -ADMP_ERR_SOCKET;
	admp_printerr(-res, "socket");
	goto OUT;
    }

    strncpy(info->name, name, IFNAMSIZ);
    strncpy(iwr.ifr_ifrn.ifrn_name, name, IFNAMSIZ);

    res = IEEE80211_GET_IW_BY_NAME(info->fd, info->name, &iwr);
    if (res != ADMP_SUCCESS)
	goto CLOSE_AND_OUT;
    strncpy(info->presence, iwr.u.name, IFNAMSIZ);

    /* printf("%s: %s\n", info->name, iwr.u.name); */

    res = IEEE80211_GET_IF_INDEX(info->fd, info->name, &ifr);
    if (res != ADMP_SUCCESS)
	goto CLOSE_AND_OUT;
    info->index = ifr.ifr_ifindex;

    res = IEEE80211_GET_IF_HWADDR(info->fd, info->name, &ifr);
    if (res != ADMP_SUCCESS)
	goto CLOSE_AND_OUT;
    info->hwaddr = ifr.ifr_hwaddr;

    res = IEEE80211_GET_IW_MODE(info->fd, info->name, &iwr);
    if (res != ADMP_SUCCESS)
	goto CLOSE_AND_OUT;
    info->prev_mode = iwr.u.mode;
    info->mode = iwr.u.mode;

    if (iwr.u.mode != IW_MODE_MONITOR) {
	puts("Interface is not in monitor mode..");
	puts("Changing it to monitor mode..");

	res = ieee80211_set_if_mode(info,
				    &ifr,
				    &iwr,
				    IW_MODE_MONITOR);
	if (res != ADMP_SUCCESS)
	    goto CLOSE_AND_OUT;
    }

    return ADMP_SUCCESS;

CLOSE_AND_OUT:
    close(info->fd);
    info->fd = -1;

OUT:
    return res;
}

admp_res_t admp_ieee80211_if_destroy(struct admp_ieee80211_if_info *info)
{
    struct iwreq iwr;
    struct ifreq ifr;
    admp_res_t res;

    if (info == NULL) {
	admp_printerr(ADMP_ERR_INVALID_ARGS,
		      "admp_ieee80211_if_destroy");
	return -ADMP_ERR_INVALID_ARGS;

    }

    strncpy(iwr.ifr_ifrn.ifrn_name, info->name, IFNAMSIZ);
    if (info->prev_mode != IW_MODE_MONITOR) {
	/*
	 * Setting interface mode to a previous mode may fail but in this phase,
	 * this is not an fatal error. So just move on.
	 */
	ieee80211_set_if_mode(info, &ifr, &iwr, info->prev_mode);
    }

    if (close(info->fd) == -1) {
	res = -ADMP_ERR_CLOSE;
	admp_printerr(-res, "close");
	return res;
    }
    memset(info, 0x0, sizeof(*info));
    return ADMP_SUCCESS;
}

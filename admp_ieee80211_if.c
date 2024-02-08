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

#include "admp_ieee80211_if.h"
#include "admp_err.h"

/* prv_ieee80211_ioctl_if: do ioctl of given (network interface) name */
admp_res_t prv_ieee80211_ioctl_if(const int fd,
				  const char *name,
				  unsigned long req,
				  struct ifreq *ifr)
{
    if (fd == -1 || name == NULL || ifr == NULL) {
	admp_printerr(ADMP_ERR_INVALID_ARGS, "prv_ieee80211_ioctl_if");
	return -ADMP_ERR_INVALID_ARGS;
    }

    strncpy(ifr->ifr_name, name, IFNAMSIZ);
    if (ioctl(fd, req, ifr) == -1) {
	admp_printerr(ADMP_ERR_IFCMD, "ioctl");
	return -ADMP_ERR_IFCMD;
    }
    return ADMP_SUCCESS;
}

/* prv_ieee80211_ioctl_iw: do ioctl of given (wireless interface) name */
admp_res_t prv_ieee80211_ioctl_iw(const int fd,
				  const char *name,
				  const unsigned long req,
				  struct iwreq *iwr)
{
    if (fd == -1 || name == NULL || iwr == NULL) {
	admp_printerr(ADMP_ERR_INVALID_ARGS, "prv_ieee80211_ioctl_iw");
	return -ADMP_ERR_INVALID_ARGS;
    }

    strncpy(iwr->ifr_ifrn.ifrn_name, name, IFNAMSIZ);
    if (ioctl(fd, req, iwr) == -1) {
	admp_printerr(ADMP_ERR_IWCMD, "ioctl");
	return -ADMP_ERR_IWCMD;
    }
    return ADMP_SUCCESS;
}

/* prv_ieee80211_set_if_mode: set wireless interface mode to given mode */
static admp_res_t prv_ieee80211_set_if_mode(struct admp_ieee80211_if_info *info,
					    const unsigned long mode)
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
    struct ifreq ifr;
    struct iwreq iwr;
    admp_res_t res;

    if (info == NULL) {
	res = -ADMP_ERR_INVALID_ARGS;
	admp_printerr(-res, "prv_ieee80211_set_if_mode");
	goto OUT;
    }

    puts("Interface Down..");
    ifr.ifr_flags |= ((short) ~0) & ~IFF_UP;
    res = IEEE80211_SET_IF_FLAG(info->fd, info->name, &ifr);
    if (res != ADMP_SUCCESS)
	goto OUT;

    if (!ADMP_IS_X_IN_RANGE(0, info->mode, sizeof(modes))
	|| !ADMP_IS_X_IN_RANGE(0, mode, sizeof(modes))) {
	res = -ADMP_ERR_INVALID_ARGS;
	admp_printerr(-res, "prv_ieee80211_set_if_mode");
	goto OUT;
    }
    printf("%s to %s\n", modes[info->mode], modes[mode]);
    iwr.u.mode = mode;
    res = IEEE80211_SET_IW_MODE(info->fd, info->name, &iwr);
    if (res != ADMP_SUCCESS)
	goto OUT;
    info->mode = mode;

    puts("Interface Up..");
    ifr.ifr_flags |= IFF_UP;
    res = IEEE80211_SET_IF_FLAG(info->fd, info->name, &ifr);
    if (res != ADMP_SUCCESS)
	goto OUT;

    return ADMP_SUCCESS;

OUT:
    return res;
}

/* admp_ieee80211_if_init: initialize wireless interface info to given name */
admp_res_t admp_ieee80211_if_init(const char *name,
				  struct admp_ieee80211_if_info *info,
				  int64_t mode)
{
    admp_res_t res;
    struct iwreq iwr;
    struct ifreq ifr;

    if (name == NULL || info == NULL) {
	admp_printerr(ADMP_ERR_INVALID_ARGS,
		      "admp_ieee80211_if_init");
	res = -ADMP_FAIL;
	goto OUT;
    }

    info->fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (info->fd == -1) {
	admp_printerr(-ADMP_ERR_SOCKET, "socket");
	res = -ADMP_FAIL;
	goto OUT;
    }

    strncpy(info->name, name, IFNAMSIZ);

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

    if (mode < 0) {		/*
				 * By default, function sets interface mode
				 * to monitor mode
				 */
	if (iwr.u.mode != IW_MODE_MONITOR) {
	    puts("Interface is not in monitor mode..");
	    puts("Changing it to monitor mode..");

	    res = ieee80211_set_if_mode(info,
					IW_MODE_MONITOR);
	    if (res != ADMP_SUCCESS)
		goto CLOSE_AND_OUT;
	}
    } else {
	res = ieee80211_set_if_mode(info, mode);
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

/* admp_ieee80211_if_destroy: destroy wireless interface info data */
admp_res_t admp_ieee80211_if_destroy(struct admp_ieee80211_if_info *info)
{
    struct iwreq iwr;
    struct ifreq ifr;
    admp_res_t res;

    if (info == NULL) {
	res = -ADMP_ERR_INVALID_ARGS;
	admp_printerr(-res, "admp_ieee80211_if_destroy");
	goto FAIL;
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
	goto FAIL;
    }
    memset(info, 0x0, sizeof(*info));
    return ADMP_SUCCESS;

FAIL:
    return res;
}

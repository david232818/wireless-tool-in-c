#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <linux/wireless.h>
#include "wlt.h"
#include "wlt_ioctl.h"
#include "wlt_ieee80211_pcap.h"
#include "wlt_ieee80211_ap.h"

#define WLT_IS_X_IN_RANGE(min, x, max) (((x) >= (min)) && ((x) < (max)))
#define WLT_IS_X_GT_LT(min, x, max) (((x) > (min)) && ((x) < (max)))

static int ___wlt_ieee80211_beacon_parser(struct wlt_ieee80211_mgmt_frm *frm,
					  ssize_t len,
					  struct wlt_ieee80211_ap *ap)
{
    int res, done;
    struct wlt_ieee80211_beacon *bean;
    struct wlt_ieee80211_tld *ie;
    
    if (frm == NULL || len < 0 || ap == NULL)
	return -1;

    bean = (struct wlt_ieee80211_beacon *) frm->variable;
    ie = (struct wlt_ieee80211_tld *) bean->variable;

    res = 0;
    done = 0;
    while (len > 0 && done == 0) {
	if (!WLT_IS_X_GT_LT(0, ie->len, len)) {
	    res = -1;
	    break;
	}

	switch (ie->type) {
	case 0x00:
	    /* SSID */
	    strncpy(ap->ssid, (char *) ie->data, ie->len);
	    done = 1;
	    break;
	default:
	    done = 1;
	    break;
	}

	ie = (struct wlt_ieee80211_tld *) ((uint8_t *) ie + ie->len);
	len -= ie->len;
    }
    return res;
}

static int
___wlt_ieee80211_assoc_req_parser(struct wlt_ieee80211_mgmt_frm *frm,
				  ssize_t len,
				  struct wlt_ieee80211_ap *ap)
{
    int res, done;
    struct wlt_ieee80211_assoc_req *assoc_req;
    struct wlt_ieee80211_tld *ie;

    if (frm == NULL || len < 0 || ap == NULL)
	return -1;

    assoc_req = (struct wlt_ieee80211_assoc_req *) frm->variable;
    ie = (struct wlt_ieee80211_tld *) assoc_req->variable;

    done = 0;
    res = 0;
    while (len > 0 && done == 0) {
	if (!WLT_IS_X_GT_LT(0, ie->len, len)) {
	    res = -1;
	    break;
	}

	switch (ie->type) {
	case 0x00:
	    /* SSID */
	    strncpy(ap->ssid, (char *) ie->data, ie->len);
	    done = 1;
	    break;
	default:
	    done = 1;
	    break;
	}

	ie = (struct wlt_ieee80211_tld *) ((uint8_t *) ie + ie->len);
	len -= ie->len;
    }
    return res;
}

static int __wlt_ieee80211_mgmt_parser(void *buff,
				       ssize_t len,
				       struct wlt_ieee80211_ap *ap)
{
    int res;
    struct wlt_ieee80211_mgmt_frm *frm;
    uint8_t *bssid;

    if (buff == NULL || len < 0 || ap == NULL)
	return -1;

    frm = buff;
    switch (frm->fc.ds) {
    case 0b00:
	/* IBSS or BSS */
	bssid = frm->addr3;
	break;
    case 0b01:
	/* From AP */
	bssid = frm->addr2;
	break;
    case 0b10:
	/* To AP */
	bssid = frm->addr1;
	break;
    case 0b11:
	/* Wireless bridge */
	bssid = NULL;
	break;
    default:
	bssid = NULL;
	break;
    }

    if (bssid) {
	memcpy(ap->bssid, bssid, 6);
	ap->bssid[6] = !0;
    } else {
	memset(ap->bssid, 0x00, 7);
    }

    switch (frm->fc.subtype) {
    case 0b0000:
	/* Association request frame */
	/* puts("assoc"); */
	res = ___wlt_ieee80211_assoc_req_parser(frm, len, ap);
	break;
    case 0b1000:
	/* Beacon frame */
	res = ___wlt_ieee80211_beacon_parser(frm, len, ap);
	break;
    default:
	res = -1;
	break;
    }
    return res;
}

static int __wlt_ieee80211_mac_parser(void *buff,
				      ssize_t len,
				      struct wlt_ieee80211_ap *ap)
{
    int res;
    struct wlt_ieee80211_mac_frm *frm;

    if (buff == NULL || len < 0 || ap == NULL)
	return -1;

    res = -1;
    frm = buff;
    switch (frm->fc.type) {
    case 0b00:
	/* Management frame */
	res = __wlt_ieee80211_mgmt_parser(buff, len, ap);
	break;
    case 0b01:
	/* Control frame */
	break;
    case 0b10:
	/* Data frame */
	break;
    default:
	res = -1;
	break;
    }
    return res;
}

static int ___wlt_ieee80211_radiotap_off(uint64_t present, unsigned int bitloc)
{
    unsigned int offtab[] = {
	sizeof(uint64_t),	/* TSFT */
	sizeof(uint8_t),	/* Flags */
	sizeof(uint8_t),	/* Rate */
	sizeof(uint16_t),	/* Channel */
	sizeof(uint8_t) + sizeof(uint8_t) /* hop set + hop pattern */

	/* [TODO] Add defined fields */
    };
    unsigned int off, i;
    uint64_t mask;

    off = 0;
    for (i = 0; i < bitloc; i++) {
	mask = 1 << i;
	if (mask & present)
	    off += offtab[i];
    }
    return off;
}

static void *__wlt_ieee80211_radiotap_parser(void *buff,
					     ssize_t len,
					     struct wlt_ieee80211_ap *ap)
{
    int antsig_off;
    struct wlt_ieee80211_radiotap_hdr *hdr;
    uint8_t *data;

    if (buff == NULL || len < 0)
	return NULL;

    hdr = buff;

    antsig_off = ___wlt_ieee80211_radiotap_off(*((uint64_t *) &hdr->present),
					       WLT_RADIOTAP_ANTSIG_BITLOC);
    if (antsig_off == -1)
	return NULL;

    if (WLT_IS_RADIOTAP_PRESENCE_EXTENDED(hdr->present)) {
	data = hdr->variable + antsig_off + sizeof(uint32_t);
    } else {
	data = hdr->variable + antsig_off;
    }
    ap->antsig = (int8_t) *data;
    /* printf("antsig: %d\n", antsig); */

    if (ap->antsig >= 0)
	return NULL;
    
    if (!WLT_IS_X_IN_RANGE(0, hdr->len, len))
	return NULL;
    return (buff + hdr->len);
}

static int _wlt_ieee80211_parser(void *buff,
				 ssize_t len,
				 struct wlt_ieee80211_ap *ap)
{
    void *buffp;
    
    if (buff == NULL || len < 0)
	return -1;

    /* puts("There is a packet.."); */

    buffp = __wlt_ieee80211_radiotap_parser(buff, len, ap);
    if (buffp == NULL)
	return -1;

    if (__wlt_ieee80211_mac_parser(buffp, len, ap) == -1)
	return -1;
    return 0;
}

#define MAX_EVTCNT 128

/* wlt_ieee80211_pcap: capture ieee80211 packets */
int wlt_ieee80211_pcap(struct wlt_dev *wdev)
{
    int fdflags, epollfd, evtcnt, i;
    int res;
    struct epoll_event evts[2], epoll_evts[MAX_EVTCNT];
    struct wlt_ieee80211_ap *apbuf;
    ssize_t frmlen;
    uint8_t buff[WLT_IEEE80211_BUFSIZ];

    if (wdev == NULL)
	return -1;

    if (wdev->mode != IW_MODE_MONITOR)
	if (wlt_setmode(wdev, IW_MODE_MONITOR) == -1)
	    return -1;

    fdflags = fcntl(wdev->sockfd, F_GETFL);
    fdflags |= O_NONBLOCK;
    if (fcntl(wdev->sockfd, F_SETFL, fdflags) < 0) {
	perror("fcntl");
	return -1;
    }
    
    epollfd = epoll_create(2);
    if (epollfd < 0) {
	perror("epoll_create");
	return -1;
    }
    
    evts[0].events = EPOLLIN;
    evts[0].data.fd = wdev->sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, wdev->sockfd, &evts[0]) < 0) {
	perror("epoll_ctl");
	close(epollfd);
	return -1;
    }

    evts[1].events = EPOLLIN;
    evts[1].data.fd = STDIN_FILENO;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &evts[1]) < 0) {
	perror("epoll_ctl");
	close(epollfd);
	return -1;
    }

    apbuf = wlt_ieee80211_ap_init();
    if (apbuf == NULL) {
	close(epollfd);
	return -1;
    }

    while (1) {
	evtcnt = epoll_wait(epollfd, epoll_evts, MAX_EVTCNT, 5000);
	if (evtcnt < 0) {
	    perror("epoll_wait");
	    close(epollfd);
	    return -1;
	}

	for (i = 0; i < evtcnt; i++) {
	    if (epoll_evts[i].data.fd == wdev->sockfd) {
		memset(buff, 0x00, sizeof(buff));
		frmlen = read(wdev->sockfd, buff, WLT_IEEE80211_BUFSIZ);
		if (frmlen >= 0) {
		    res = _wlt_ieee80211_parser(buff, frmlen, apbuf);
		}

		if (!res) {
		    printf("%s - %d\n", apbuf->ssid, apbuf->antsig);
		    memset(apbuf, 0x00, sizeof(*apbuf));
		}
	    }

	    if (epoll_evts[i].data.fd == STDIN_FILENO) {
		/* printf("user key: %c\n", getchar()); */
		if (getchar() == 'q')
		    goto OUT;
	    }
	}

	puts("channel hopping..");
	/* printf("channel: %d\n", wdev->chann); */
	if (wlt_setchann(wdev,
			 WLT_IEEE80211_24_HOP_CHANN(wdev->chann)) == -1) {
	    close(epollfd);
	    free(apbuf);
	    return -1;
	}
    }
    
OUT:
    close(epollfd);
    free(apbuf);
    return 0;
}

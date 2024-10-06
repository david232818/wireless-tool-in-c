#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "wlt.h"
#include "wlt_wnetdev.h"
#include "wlt_ieee80211_pcap.h"
#include "wlt_ieee80211_ap.h"
#include "io_multiplex.h"
#include "fd.h"

#define WLT_IS_X_IN_RANGE(min, x, max) (((x) >= (min)) && ((x) < (max)))
#define WLT_IS_X_GT_LT(min, x, max) (((x) > (min)) && ((x) < (max)))

#define PARSER_OTHER_FRAME 1

static int wlt_ieee80211_beacon_parser(struct wlt_ieee80211_mgmt_frm *mgmtp,
				       ssize_t len,
				       struct wlt_ieee80211_ap *ap)
{
    struct wlt_ieee80211_beacon *beanp;
    struct wlt_ieee80211_tld *iep;
    
    if (mgmtp == NULL || len < 0 || ap == NULL)
	goto OUT;

    beanp = (struct wlt_ieee80211_beacon *) mgmtp->variable;
    iep = (struct wlt_ieee80211_tld *) beanp->variable;

    while (len > 0) {
	if (!WLT_IS_X_GT_LT(0, iep->len, len))
	    goto OUT;

	switch (iep->type) {
	case 0x00:
	    /* SSID */
	    if (iep->len > WLT_IEEE80211_APNAMSIZ - 1)
		goto OUT;
	    
	    strncpy(ap->ssid, (char *) iep->data, iep->len);
	    ap->ssid[iep->len] = '\0';
	    break;
	default:
	    goto FINISH;
	}

	iep = (struct wlt_ieee80211_tld *) ((uint8_t *) iep + iep->len);
	len -= iep->len;
    }

FINISH:
    return 0;

OUT:
    return -1;
}

static int
wlt_ieee80211_assoc_req_parser(struct wlt_ieee80211_mgmt_frm *mgmtp,
			       ssize_t len,
			       struct wlt_ieee80211_ap *ap)
{
    struct wlt_ieee80211_assoc_req *assocp;
    struct wlt_ieee80211_tld *iep;

    if (mgmtp == NULL || len < 0 || ap == NULL)
	goto OUT;

    assocp = (struct wlt_ieee80211_assoc_req *) mgmtp->variable;
    iep = (struct wlt_ieee80211_tld *) assocp->variable;

    while (len > 0) {
	if (!WLT_IS_X_GT_LT(0, iep->len, len))
	    goto OUT;

	switch (iep->type) {
	case 0x00:
	    /* SSID */
	    if (iep->len > WLT_IEEE80211_APNAMSIZ - 1)
		goto OUT;
	    
	    strncpy(ap->ssid, (char *) iep->data, iep->len);
	    ap->ssid[iep->len] = '\0';
	    break;
	default:
	    goto FINISH;
	}

	iep = (struct wlt_ieee80211_tld *) ((uint8_t *) iep + iep->len);
	len -= iep->len;
    }

FINISH:
    return 0;

OUT:
    return -1;
}

static int wlt_ieee80211_mgmt_parser(void *buffp,
				     ssize_t len,
				     struct wlt_ieee80211_ap *ap)
{
    int res;
    struct wlt_ieee80211_mgmt_frm *mgmtp;
    uint8_t *bssidp;

    if (buffp == NULL || len < 0 || ap == NULL)
	goto OUT;

    mgmtp = buffp;
    switch (mgmtp->fc.ds & 0x3) {
    case 0:
	/* IBSS or BSS */
	bssidp = mgmtp->addr3;
	break;
    case 1:
	/* From AP */
	bssidp = mgmtp->addr2;
	break;
    case 2:
	/* To AP */
	bssidp = mgmtp->addr1;
	break;
    case 3:
	/* Wireless bridge */
	bssidp = NULL;
	break;
    default:
	bssidp = NULL;
	break;
    }

    if (bssidp) {
	memcpy(ap->bssid, bssidp, 6);
	ap->bssid[6] = !0;
    } else {
	memset(ap->bssid, 0x00, 7);
    }

    switch (mgmtp->fc.subtype & 0xf) {
    case 0:
	/* Association request frame */
	/* puts("assoc"); */
	res = wlt_ieee80211_assoc_req_parser(mgmtp, len, ap);
	break;
    case 8:
	/* Beacon frame */
	res = wlt_ieee80211_beacon_parser(mgmtp, len, ap);
	break;
    default:
	res = PARSER_OTHER_FRAME;
	break;
    }
    return res;

OUT:
    return -1;
}

static int wlt_ieee80211_mac_parser(void *buffp,
				    ssize_t len,
				    struct wlt_ieee80211_ap *ap)
{
    int res;
    struct wlt_ieee80211_mac_frm *frmp;

    if (buffp == NULL || len < 0 || ap == NULL)
	goto OUT;

    frmp = buffp;
    switch (frmp->fc.type & 0x3) {
    case 0:
	/* Management frame */
	res = wlt_ieee80211_mgmt_parser(buffp, len, ap);
	break;
    case 1:
	/* Control frame */
	res = PARSER_OTHER_FRAME;
	break;
    case 2:
	/* Data frame */
	res = PARSER_OTHER_FRAME;
	break;
    default:
	res = PARSER_OTHER_FRAME;
	break;
    }
    return res;

OUT:
    return -1;
}

static unsigned int wlt_ieee80211_radiotap_off(uint64_t present,
					       unsigned int bitloc)
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

static void *wlt_ieee80211_radiotap_parser(void *buffbasep,
					   ssize_t len,
					   struct wlt_ieee80211_ap *ap)
{
    unsigned int antsig_off;
    struct wlt_ieee80211_radiotap_hdr *radiohp;
    uint8_t *datap;

    if (buffbasep == NULL || len < 0)
	goto OUT;

    radiohp = buffbasep;

    antsig_off = wlt_ieee80211_radiotap_off(*((uint64_t *) &radiohp->present),
					    WLT_RADIOTAP_ANTSIG_BITLOC);

    if (WLT_IS_RADIOTAP_PRESENCE_EXTENDED(radiohp->present)) {
	datap = radiohp->variable + antsig_off + sizeof(uint32_t);
    } else {
	datap = radiohp->variable + antsig_off;
    }
    ap->antsig = (int8_t) *datap;
    /* printf("antsig: %d\n", antsig); */

    if (ap->antsig >= 0)
	goto OUT;
    
    if (!WLT_IS_X_IN_RANGE(0, radiohp->len, len))
	goto OUT;
    
    return (void *) ((uint8_t *) buffbasep + radiohp->len);

OUT:
    return NULL;
}

static int wlt_ieee80211_parser(void *buffbasep,
				ssize_t len,
				struct wlt_ieee80211_ap *ap)
{
    void *buffp;
    
    if (buffbasep == NULL || len < 0)
	goto OUT;

    /* puts("There is a packet.."); */

    buffp = wlt_ieee80211_radiotap_parser(buffbasep, len, ap);
    if (buffp == NULL)
	goto OUT;

    if (wlt_ieee80211_mac_parser(buffp, len, ap) == -1)
	goto OUT;
    
    return 0;

OUT:
    return -1;
}

#define MAX_EVTCNT 128

/* wlt_ieee80211_pcap: heart of IEEE 802.11 sniffing */
int wlt_ieee80211_pcap(struct wlt_wdev *wdevp)
{
    int evtcnt, i;
    int res;
    struct io_event *io_evtp;
    struct wlt_ieee80211_ap *apbufp, *ap;
    aplist_t auth_aplist;
    ssize_t frmlen;
    uint8_t buff[WLT_IEEE80211_BUFSIZ];
    io_desc_t *iosp;

    res = -1;
    
    if (!wdevp)
	goto OUT;

    if (!WLT_IS_WDEV_MODE_MONITOR(wdevp->mode))
	if (WLT_WDEV_SET_MONITOR(wdevp) == -1)
	    goto OUT;

    io_evtp = io_event_init(2, MAX_EVTCNT, 5000);
    if (!io_evtp)
	goto OUT;

    if (io_event_add(io_evtp, wlt_wdev_getdesc(wdevp)) == -1)
	goto OUT_DESTROY_IOEVENT;
    
    if (io_event_add(io_evtp, file_getdesc(stdin)) == -1)
	goto OUT_DESTROY_IOEVENT;

    apbufp = wlt_ieee80211_ap_init();
    if (apbufp == NULL)
	goto OUT_DESTROY_IOEVENT;

    APLIST_INIT(&auth_aplist);
    
    while (1) {
	evtcnt = io_event_cap(io_evtp);
	if (evtcnt < 0) {
	    res = -1;
	    goto OUT_DESTROY;
	}

	for (i = 0; i < evtcnt; i++) {
	    iosp = IO_EVENT_GET_IOSBASE(io_evtp);
	    
	    if (IO_EVENT_GETDESC_FROM_IOSPTR_AND_ADVANCE(io_evtp, i, iosp)) {
		apbufp->ssid[0] = '\0';
		memset(buff, 0x00, sizeof(buff));
		frmlen = wlt_wdev_read(wdevp, buff, WLT_IEEE80211_BUFSIZ);
		if (frmlen >= 0) {
		    res = wlt_ieee80211_parser(buff, frmlen, apbufp);
		}

		/* if (!res) { */
		/*     printf("%s - %d\n", apbuf->ssid, apbuf->antsig); */
		/*     memset(apbuf, 0x00, sizeof(*apbuf)); */
		/* } */

		if (!res && (wlt_ieee80211_ap_search(&auth_aplist, apbufp)
			     == WLT_IEEE80211_AP_NOT_FOUND)) {
		    ap = wlt_ieee80211_ap_init();
		    if (ap == NULL) {
			res = -1;
			goto OUT_DESTROY;
		    }

		    memcpy(ap, apbufp, offsetof(struct wlt_ieee80211_ap,
						node));
		    wlt_ieee80211_ap_add_tail(&auth_aplist, ap);
		    printf("%s\t%02x:%02x:%02x:%02x:%02x:%02x\t%d\n",
			   (ap->ssid[0] != '\0') ? ap->ssid : "Unknown",
			   ap->bssid[0],
			   ap->bssid[1],
			   ap->bssid[2],
			   ap->bssid[3],
			   ap->bssid[4],
			   ap->bssid[5],
			   ap->antsig);
		}
	    }

	    if (IO_EVENT_GETDESC_FROM_IOSPTR_AND_ADVANCE(io_evtp, \
							 i, \
							 iosp)) {
		/* printf("user key: %c\n", getchar()); */
		if (getchar() == 'q') {
		    res = 0;
		    goto OUT_DESTROY;
		}
	    }
	}

	/* puts("channel hopping.."); */
	/* printf("channel: %d\n", wdev->chann); */
	if (wlt_setchann(wdevp,
			 WLT_IEEE80211_24_HOP_CHANN(wdevp->chann)) == -1) {
	    res = -1;
	    goto OUT_DESTROY;
	}
    }

OUT_DESTROY:
    wlt_ieee80211_ap_destroy(apbufp);
    wlt_ieee80211_ap_list_clear(&auth_aplist);

OUT_DESTROY_IOEVENT:
    io_event_destroy(io_evtp);

OUT:
    return res;
}

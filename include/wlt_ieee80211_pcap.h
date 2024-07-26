#ifndef __WLT_IEEE80211_PCAP__
#define __WLT_IEEE80211_PCAP__

#include <stdio.h>
#include <stdint.h>

struct wlt_ieee80211_radiotap_hdr {
    uint8_t ver;		/* Version */
    uint8_t pad;		/* Padding */
    uint16_t len;		/* Length */
    uint32_t present;
    uint8_t variable[];
} __attribute__((__packed__));

#define WLT_IS_RADIOTAP_PRESENCE_EXTENDED(present) \
    (((present) >> 31) & 1)

#define WLT_RADIOTAP_ANTSIG_BITLOC 5
#define WLT_IS_RADIOTAP_ANTENNA_SIG_PRESENT(present) \
    (((present) >> 5) & 1)

struct wlt_ieee80211_frm_ctl {
    uint16_t proto: 2,
	    type: 2,
	    subtype: 4,
	    ds: 2,		/* [tods][fromds] */
	    moreflag: 1,
	    retry: 1,
	    pwrmgmt: 1,
	    moredata: 1,
	    wep: 1,
	    order: 1;
} __attribute__((__packed__));

struct wlt_ieee80211_seq_ctl {
    uint16_t fragnum: 4,
	    seqnum: 12;
} __attribute__((__packed__));

/*
 * This header structure is for generic case. But in IEEE 802.11,
 * header structure varies by frame categories.
 */
struct wlt_ieee80211_mac_frm {
    struct wlt_ieee80211_frm_ctl fc;
    uint16_t durid;		/* Duration / ID */
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    struct wlt_ieee80211_seq_ctl seqctl;
    uint8_t addr4[6];
    uint8_t variable[];
} __attribute__((__packed__));

struct wlt_ieee80211_mac_trailer {
    uint32_t fcs;
} __attribute__((__packed__));

struct wlt_ieee80211_mgmt_frm {
    /* header */
    struct wlt_ieee80211_frm_ctl fc;
    uint16_t durid;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    struct wlt_ieee80211_seq_ctl seqctl;
    uint8_t variable[];
} __attribute__((__packed__));

struct wlt_ieee80211_beacon {
    uint64_t tmstamp;		/* Timestamp */
    uint16_t interval;		/* Beacon interval */
    uint16_t capinfo;		/* Capability information */
    uint8_t variable[];		/* Information Elements */
} __attribute__((__packed__));

struct wlt_ieee80211_assoc_req {
    uint16_t capinfo;		/* Capability information */
    uint16_t interval;		/* Listening interval */
    uint8_t variable[];		/* Information Elements */
} __attribute__((__packed__));

struct wlt_ieee80211_tld {
    uint8_t type;		/* EID */
    uint8_t len;		/* Length */
    uint8_t data[];		/* Data */
} __attribute__((__packed__));

#define WLT_IEEE80211_BUFSIZ 4096

int wlt_ieee80211_pcap(struct wlt_dev *wdev);

#endif

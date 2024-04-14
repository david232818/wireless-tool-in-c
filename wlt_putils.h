#ifndef __ADMP_PUTILS_H__
#define __ADMP_PUTILS_H__

#include <stdint.h>
#include <stddef.h>

#include "admp_ieee80211_if.h"
#include "admp_err.h"

/* #define ADMP_IEEE80211_FRAME_MAX_LEN 2346 */

struct admp_ieee80211_radiotap_hdr {
    uint8_t ver;		/* Version */
    uint8_t pad;		/* Padding */
    uint16_t len;		/* Length */
    uint32_t present;
    uint8_t variable[];
} __attribute__((__packed__));

#define ADMP_IS_RADIOTAP_PRESENCE_EXTENDED(present) \
    (((present) >> 31) & 1)

#define ADMP_RADIOTAP_ANTSIG_BIT_LOC 5
#define ADMP_IS_RADIOTAP_ANTENNA_SIG_PRESENT(present) \
    (((present) >> 5) & 1)

struct admp_ieee80211_frame_control {
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

struct admp_ieee80211_sequence_control {
    uint16_t fragnum: 4,
	    seqnum: 12;
} __attribute__((__packed__));

/*
 * This header structure is for generic case. But in IEEE 802.11,
 * header structure varies by frame categories.
 */
struct admp_ieee80211_mac_frame {
    struct admp_ieee80211_frame_control fc;
    uint16_t durid;		/* Duration / ID */
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    struct admp_ieee80211_sequence_control seqctl;
    uint8_t addr4[6];
    uint8_t variable[];
} __attribute__((__packed__));

struct admp_ieee80211_mac_trailer {
    uint32_t fcs;
} __attribute__((__packed__));

struct admp_ieee80211_mgmt_frame {
    /* header */
    struct admp_ieee80211_frame_control fc;
    uint16_t durid;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    struct admp_ieee80211_sequence_control seqctl;
    uint8_t variable[];
} __attribute__((__packed__));

struct admp_ieee80211_beacon {
    uint64_t tmstamp;		/* Timestamp */
    uint16_t interval;		/* Beacon interval */
    uint16_t capinfo;		/* Capability information */
    uint8_t variable[];		/* Information Elements */
} __attribute__((__packed__));

struct admp_ieee80211_assoc_req {
    uint16_t capinfo;		/* Capability information */
    uint16_t interval;		/* Listening interval */
    uint8_t variable[];		/* Information Elements */
} __attribute__((__packed__));

struct admp_ieee80211_tlv {
    uint8_t type;		/* EID */
    uint8_t len;		/* Length */
    uint8_t value[];		/* Data */
} __attribute__((__packed__));

struct admp_ieee80211_ap_info {
    int8_t antsig;
    uint8_t bssid[7];
    char ssid[BUFSIZ];
};

admp_res_t admp_ieee80211_pcap(struct admp_ieee80211_if_info *,
			       struct admp_ieee80211_ap_info *);

#endif

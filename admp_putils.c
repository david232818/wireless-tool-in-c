#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>

#include "admp_ieee80211_if.h"
#include "admp_err.h"
#include "admp_putils.h"
#include "admp_utils.h"

/* prv_ieee80211_beacon_parser: parse given frame as a beacon frame */
static admp_res_t
prv_ieee80211_beacon_parser(struct admp_ieee80211_mgmt_frame *frame,
			    ssize_t len,
			    struct admp_ieee80211_ap_info *ap)
{
    unsigned int done;
    struct admp_ieee80211_beacon *beacon;
    struct admp_ieee80211_tlv *ie;

    if (frame == NULL || ap == NULL || len < 0) {
	admp_printerr(ADMP_ERR_INVALID_ARGS, "prv_ieee80211_mgmt_parser");
	return -ADMP_ERR_INVALID_ARGS;
    }

    beacon = (struct admp_ieee80211_beacon *) frame->variable;
    ie = (struct admp_ieee80211_tlv *) beacon->variable;
    done = 0;
    while (len > 0 && done == 0) {
	if (!ADMP_IS_X_IN_RANGE(0, ie->len, len))
	    break;
	
	switch (ie->type) {
	case 0x00:
	    /* SSID */
	    strncpy(ap->ssid, (char *) ie->value, ie->len);
	    break;
	default:
	    done = 1;
	    break;
	}
	ie = (struct admp_ieee80211_tlv *) ((uint8_t *) ie + ie->len);
	len -= ie->len;
    }
    return ADMP_SUCCESS;
}

/*
 * prv_ieee80211_assoc_req_parser: parse given frame as a association
 * request frame
 */
static admp_res_t
prv_ieee80211_assoc_req_parser(struct admp_ieee80211_mgmt_frame *frame,
			       ssize_t len,
			       struct admp_ieee80211_ap_info *ap)
{
    unsigned int done;
    struct admp_ieee80211_assoc_req *assoc_req;
    struct admp_ieee80211_tlv *ie;

    if (frame == NULL || ap == NULL || len < 0) {
	admp_printerr(ADMP_ERR_INVALID_ARGS, "prv_ieee80211_mgmt_parser");
	return -ADMP_ERR_INVALID_ARGS;
    }

    assoc_req = (struct admp_ieee80211_assoc_req *) frame;
    ie = (struct admp_ieee80211_tlv *) assoc_req->variable;
    done = 0;
    while ((len > 0) && (done == 0)) {
	/* printf("%d %d\n", ie->len, len); */
	if (!ADMP_IS_X_IN_RANGE(0, ie->len, len))
	    break;

	/* printf("type: %d\n", ie->type); */
	switch (ie->type) {
	case 0x00:
	    /* SSID */
	    strncpy(ap->ssid, (char *) ie->value, ie->len);
	    break;
	default:
	    done = 1;
	    break;
	}
	ie = (struct admp_ieee80211_tlv *) ((uint8_t *) ie + ie->len);
	len -= ie->len;
    }
    return ADMP_SUCCESS;
}

/* prv_ieee80211_mgmt_parser: parse given buff as a management frame */
static admp_res_t prv_ieee80211_mgmt_parser(void *buff,
					    ssize_t len,
					    struct admp_ieee80211_ap_info *ap)
{
    struct admp_ieee80211_mgmt_frame *frame;
    uint8_t *bssid;
    admp_res_t res;

    if (buff == NULL || ap == NULL || len < 0) {
	admp_printerr(ADMP_ERR_INVALID_ARGS, "prv_ieee80211_mgmt_parser");
	return -ADMP_ERR_INVALID_ARGS;
    }
    
    frame = buff;

    switch (frame->fc.ds) {
    case 0b00:
	/* IBSS or BSS */
	bssid = frame->addr3;
	break;
    case 0b01:
	/* From AP */
	bssid = frame->addr2;
	break;
    case 0b10:
	/* To AP */
	bssid = frame->addr1;
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
	ap->bssid[0] = bssid[0];
	ap->bssid[1] = bssid[1];
	ap->bssid[2] = bssid[2];
	ap->bssid[3] = bssid[3];
	ap->bssid[4] = bssid[4];
	ap->bssid[5] = bssid[5];
	ap->bssid[6] = !0;
    } else {
	memset(ap->bssid, 0x0, sizeof(ap->bssid));
    }

    /* printf("%d\n", frame->fc.subtype); */
    switch (frame->fc.subtype) {
    case 0b0000:
	/* Association request frame */
	res = prv_ieee80211_assoc_req_parser(frame, len, ap);
	break;
    case 0b1000:
	/* Beacon frame */
	res = prv_ieee80211_beacon_parser(frame, len, ap);
	break;
    default:
	res = -ADMP_ERR_INVALID_FRAME;
	break;
    }
    return res;
}

/* prv_ieee80211_parser: parse given buff as a generic frame */
static admp_res_t prv_ieee80211_parser(void *buff,
				       ssize_t len,
				       struct admp_ieee80211_ap_info *ap)
{
    struct admp_ieee80211_mac_frame *frame;
    admp_res_t res;

    if (buff == NULL || ap == NULL || len < 0) {
	admp_printerr(ADMP_ERR_INVALID_ARGS, "prv_ieee80211_parser");
	return -ADMP_ERR_INVALID_ARGS;
    }
    
    frame = buff;
    switch (frame->fc.type) {
    case 0b00:
	/* Management frame */
	res = prv_ieee80211_mgmt_parser(buff, len, ap);
	break;
    case 0b01:
	/* Control frame */
	break;
    case 0b10:
	/* Data frame */
	break;
    default:
	res = -ADMP_ERR_INVALID_FRAME;
	break;
    }
    return res;
}

/*
 * prv_ieee80211_radiotap_get_offset: get offset of specific field in the
 * radiotap header described by a bit location
 */
static unsigned int prv_ieee80211_radiotap_get_offset(uint64_t present,
						      unsigned int bit_loc)
{
    unsigned int offsettab[] = {
	sizeof(uint64_t),	/* TSFT */
	sizeof(uint8_t),	/* Flags */
	sizeof(uint8_t),	/* Rate */
	sizeof(uint16_t),	/* Channel */
	sizeof(uint8_t) + sizeof(uint8_t) /* hop set + hop pattern */

	/* [TODO] Add defined fields */
    };
    unsigned int offset;
    uint64_t mask, i;

    offset = 0;
    for (i = 0; i < bit_loc; i++) {
	mask = 1 << i;
	if (mask & present)
	    offset += offsettab[i];
    }
    return offset;
}

/* prv_ieee80211_radiotap_parser: parse buff as a radiotap header */
static void *prv_ieee80211_radiotap_parser(void *buff,
					   ssize_t len,
					   struct admp_ieee80211_ap_info *ap)
{
    unsigned int antsig_offset;
    struct admp_ieee80211_radiotap_hdr *header;
    uint8_t *data;

    if (buff == NULL || ap == NULL || len < 0) {
	admp_printerr(ADMP_ERR_INVALID_ARGS, "admp_parser");
	return NULL;
    }

    header = buff;
    /* printf("%d %08x ", header->len, header->present); */

    antsig_offset = prv_ieee80211_radiotap_get_offset(
	*((uint64_t *) &header->present),
	ADMP_RADIOTAP_ANTSIG_BIT_LOC);

    if (ADMP_IS_RADIOTAP_PRESENCE_EXTENDED(header->present)) {
	data = header->variable + antsig_offset + sizeof(uint32_t);
    } else {
	data = header->variable + antsig_offset;
    }
    ap->antsig = (int8_t) *data;
    /* printf("%d ", *((int8_t *) data)); */
    
    /* if (header->len > 0 && header->len < len) { */
    /* 	printf("\n"); */
    /* 	admp_hexdump(header, header->len); */
    /* } else { */
    /* 	printf("\n"); */
    /* 	return NULL; */
    /* } */

    if (!ADMP_IS_X_IN_RANGE(0, header->len, len))
	return NULL;
	
    return buff + header->len;
}

/* admp_pcap: capture packet and hexdump it */
admp_res_t admp_ieee80211_pcap(struct admp_ieee80211_if_info *info,
			       struct admp_ieee80211_ap_info *ap)
{
    struct iwreq iwr;
    ssize_t len;
    uint8_t buff[BUFSIZ], *buffp;
    admp_res_t res;

    if (info == NULL) {
	admp_printerr(ADMP_ERR_INVALID_ARGS, "admp_pcap");
	return -ADMP_ERR_INVALID_ARGS;
    }

    printf("|Radiotap header length\t|Flags\t|Signal strength\t|\n");

    res = ADMP_SUCCESS;
    while ((len = read(info->fd,
		       buff,
		       BUFSIZ)) >= 0) {
	buffp = prv_ieee80211_radiotap_parser(buff, len, ap);
	if (buffp == NULL) {	/*
				 * There are weird packets. And we do not
				 * have to process it.
				 */
	    memset(buff, 0x0, BUFSIZ);
	    memset(ap, 0x0, sizeof(*ap));
	    continue;
	}

	res = prv_ieee80211_parser(buffp, len - (buffp - buff), ap);
	if (res != ADMP_SUCCESS) {
	    memset(buff, 0x0, BUFSIZ);
	    memset(ap, 0x0, sizeof(*ap));
	    continue;
	}

	/* admp_hexdump(buffp, len); */
	
	res = IEEE80211_GET_IW_CHANN(info->fd, info->name, &iwr);
	if (res != ADMP_SUCCESS)
	    break;

	printf("%s ", ap->ssid);
	if (ap->bssid[6]) {
	    printf("[%02x:%02x:%02x:%02x:%02x:%02x] ",
		   ap->bssid[0],
		   ap->bssid[1],
		   ap->bssid[2],
		   ap->bssid[3],
		   ap->bssid[4],
		   ap->bssid[5]);
	} else {
	    printf("No BSSID ");
	}
	printf("%d\n", ap->antsig);

	puts("");
	memset(buff, 0x0, BUFSIZ);
	memset(ap, 0x0, sizeof(*ap));
    }
    return res;
}

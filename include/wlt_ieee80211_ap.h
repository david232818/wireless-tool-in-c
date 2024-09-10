#ifndef __WLT_IEEE80211_AP_H__
#define __WLT_IEEE80211_AP_H__

#include <stdint.h>
#include "ll.h"

#define WLT_IEEE80211_APNAMSIZ 32

struct wlt_ieee80211_ap {
    int8_t antsig;
    uint8_t bssid[7];		/* 6th byte to check bssid is not NULL */
    char ssid[WLT_IEEE80211_APNAMSIZ];

    struct ll_node node;
};

typedef struct ll_node aplist_t;

#define APLIST_INIT(headp) LL_HEAD_NODE_INIT((headp))

enum ap_search_flag {
    WLT_IEEE80211_AP_NOT_FOUND = 0,
    WLT_IEEE80211_AP_FOUND = 1,
    WLT_SEARCH_FLAG_MAX
};

int wlt_ieee80211_ap_search(aplist_t *aplist,
			    struct wlt_ieee80211_ap *target);
void wlt_ieee80211_ap_add_tail(aplist_t *aplist,
			       struct wlt_ieee80211_ap *ap);
struct wlt_ieee80211_ap *wlt_ieee80211_ap_init(void);
void wlt_ieee80211_ap_destroy(void *ap);
void wlt_ieee80211_ap_list_clear(aplist_t *aplist);

#endif

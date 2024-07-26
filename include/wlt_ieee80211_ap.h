#ifndef __WLT_IEEE80211_AP_H__
#define __WLT_IEEE80211_AP_H__

#include <stdint.h>
#include "ll.h"

#define WLT_IEEE80211_APNAMSIZ 32

struct wlt_ieee80211_ap {
    int8_t antsig;
    uint8_t bssid[7];		/* 6th byte to check bssid is not NULL */
    char ssid[WLT_IEEE80211_APNAMSIZ];

    struct ll_node link;
};

struct wlt_ieee80211_ap *wlt_ieee80211_ap_init(void);
void wlt_ieee80211_ap_destroy(struct wlt_ieee80211_ap *ap);

#endif

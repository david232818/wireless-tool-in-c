#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wlt_ieee80211_ap.h"

int wlt_ieee80211_ap_search(struct wlt_ieee80211_ap *aplist,
			    struct wlt_ieee80211_ap *target)
{
    struct ll_node *curr;
    struct wlt_ieee80211_ap *ap;

    LL_NODE_FOREACH(&(aplist->node), curr) {
	ap = curr->obj;
	if (memcmp(ap->bssid, target->bssid, 6) == 0)
	    return WLT_IEEE80211_AP_FOUND;
    }
    return WLT_IEEE80211_AP_NOT_FOUND;
}

void wlt_ieee80211_ap_add_tail(struct wlt_ieee80211_ap *aplist,
			       struct wlt_ieee80211_ap *ap)
{
    ll_node_add_tail(&(aplist)->node, &(ap->node));
}

struct wlt_ieee80211_ap *wlt_ieee80211_ap_init(void)
{
    struct wlt_ieee80211_ap *ap;

    ap = malloc(sizeof(*ap));
    if (ap == NULL) {
	perror("malloc");
	return NULL;
    }
    LL_NODE_INIT(&(ap->node), struct wlt_ieee80211_ap, node);
    return ap;
}

void wlt_ieee80211_ap_destroy(void *ap)
{
    free(ap);
}

void wlt_ieee80211_ap_list_clear(struct wlt_ieee80211_ap *aplist)
{
    ll_node_clear(&(aplist->node), wlt_ieee80211_ap_destroy);
}

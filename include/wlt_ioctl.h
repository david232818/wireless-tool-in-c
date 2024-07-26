#ifndef __WLT_IOCTL_H__
#define __WLT_IOCTL_H__

int wlt_getindex(struct wlt_dev *wdev);
int wlt_getmode(struct wlt_dev *wdev);
int wlt_setmode(struct wlt_dev *wdev, const uint32_t mode);
int wlt_setchann(struct wlt_dev *wdev, int chann);

/* IEEE 802.11 2.4GHz macros */
#define WLT_IEEE80211_24_BASE_FREQ 2407
#define WLT_IEEE80211_24_FREQ_INTERVAL 5
#define WLT_IEEE80211_24_FREQ2CHANN(freq) \
    (((freq) - WLT_IEEE80211_24_BASE_FREQ) / WLT_IEEE80211_24_FREQ_INTERVAL)
#define WLT_IEEE80211_24_CHANN2FREQ_M(chann) \
    (((chann) * WLT_IEEE80211_24_FREQ_INTERVAL + WLT_IEEE80211_24_BASE_FREQ) \
     * 100000)
#define WLT_IEEE80211_24_HOP_CHANN(currchan) (((currchan) + 1) % 13 + 1)

#endif

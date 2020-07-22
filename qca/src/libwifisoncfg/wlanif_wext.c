/*
 * Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
 *
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "wlanif_wext.h"

#ifdef GMOCK_UNIT_TESTS
#include "strlcpy.h"
#endif

#ifdef SON_MEMORY_DEBUG

#include "qca-son-mem-debug.h"

#undef QCA_MOD_INPUT
#define QCA_MOD_INPUT QCA_MOD_LIBWIFISONCFG
#include "son-mem-debug.h"

#endif /* SON_MEMORY_DEBUG */

#define TRACE_ENTRY() dbgf(sonwextDbgS.dbgModule, DBGINFO, "%s: Enter \n",__func__)
#define TRACE_EXIT() dbgf(sonwextDbgS.dbgModule, DBGINFO, "%s: Exit \n",__func__)
#define TRACE_EXIT_ERR() dbgf(sonwextDbgS.dbgModule, DBGERR, "%s: Exit with err %d\n",__func__,ret)

/*IOCTL socket creation*/
static inline int wext_priv_sock_init(struct wlanif_wext_priv * priv)
{
    int ret;

    if ((ret = priv->Sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        goto out;
    }

    if ((ret = fcntl(priv->Sock, F_SETFL, fcntl(priv->Sock, F_GETFL) | O_NONBLOCK))) {
        goto err;
    }

    return 0;
err:
    close(priv->Sock);
out:
    return -1;
}

/*close socket connection*/
static inline void wext_priv_sock_close(struct wlanif_wext_priv * priv)
{
    close(priv->Sock);
}

/* Function to get name of the dev */
int getName_wext(void *ctx, const char * ifname, char *name )
{
    struct iwreq Wrq;
    int ret;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy( Wrq.ifr_name, ifname, IFNAMSIZ );

    if ((ret = ioctl(priv->Sock, SIOCGIWNAME, &Wrq)) < 0) {
        goto err;
    }

    strlcpy( name, Wrq.u.name ,IFNAMSIZ );

    TRACE_EXIT();

    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/* Function to check whether the current device is AP */
int isAP_wext(void *ctx, const char * ifname, uint32_t *result)
{
    int ret;
    struct iwreq Wrq;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    *result = 0;

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );

    if ((ret = ioctl(priv->Sock, SIOCGIWMODE, &Wrq)) < 0) {
        goto err;
    }

    *result = (Wrq.u.mode == IW_MODE_MASTER ? 1 : 0);

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/* Function to get BSSID address */
int getBSSID_wext(void *ctx, const char * ifname, struct ether_addr *BSSID )
{
    struct iwreq Wrq;
    int ret;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );

    if ((ret = ioctl(priv->Sock, SIOCGIWAP, &Wrq)) < 0) {
        goto err;
    }

    memcpy( BSSID, &Wrq.u.ap_addr.sa_data, ETH_ALEN );

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/* Funtion to get ESSID info */
int getESSID_wext(void *ctx, const char * ifname, void *buf, uint32_t *len )
{
    struct iwreq Wrq;
    int ret;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    memset(&Wrq, 0, sizeof(Wrq));
    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    Wrq.u.data.pointer = buf;
    Wrq.u.data.length = *len;

    if ((ret = ioctl(priv->Sock, SIOCGIWESSID, &Wrq)) < 0) {
        goto err;
    }

    /* Update the ssid length if the ioctl succeeds */
    *len = Wrq.u.data.length;

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/* Function to get frequency info*/
int getFreq_wext(void *ctx, const char * ifname, int32_t * freq)
{
    int ret;
    struct iwreq Wrq;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    if ((ret = ioctl(priv->Sock, SIOCGIWFREQ, &Wrq)) < 0) {
        goto err;
    }

    *freq = Wrq.u.freq.m;

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/* Function to get channel width*/
int getChannelWidth_wext(void *ctx, const char * ifname, int * chwidth)
{
    struct iwreq Wrq;
    int ret,iwparam;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    iwparam = IEEE80211_PARAM_CHWIDTH;
    memcpy(Wrq.u.name, &iwparam, sizeof(iwparam));
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_GETPARAM, &Wrq)) < 0) {
        goto err;
    }

    memcpy(chwidth, Wrq.u.name, sizeof(int));

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/* Function to get channel extoffset */
int getChannelExtOffset_wext(void *ctx, const char * ifname, int * choffset)
{
    struct iwreq Wrq;
    int ret, iwparam;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    iwparam = IEEE80211_PARAM_CHEXTOFFSET;
    memcpy(Wrq.u.name, &iwparam, sizeof(iwparam));
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_GETPARAM, &Wrq)) < 0) {
        goto err;
    }
    memcpy(choffset, Wrq.u.name, sizeof(int));

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;

}

/*Function to get channel Bandwidth */
int getChannelBandwidth_wext(void *ctx, const char * ifname, int * bandwidth)
{
    struct iwreq Wrq;
    int ret, iwparam;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    iwparam = IEEE80211_PARAM_BANDWIDTH;
    memcpy(Wrq.u.name, &iwparam, sizeof(iwparam));
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_GETPARAM, &Wrq)) < 0) {
        goto err;
    }
    memcpy(bandwidth, Wrq.u.name, sizeof(int));

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;

}

/*Fucntion to handle Generic commands - parameter cmd is required for some nl based functions*/
int getGenericInfoCmd_wext(void *ctx, const char * ifname, int cmd, void * chanInfo, int chanInfoSize)
{
    struct iwreq Wrq;
    int ret;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    Wrq.u.data.pointer = chanInfo;
    Wrq.u.data.length = chanInfoSize;

    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_CONFIG_GENERIC, &Wrq)) < 0) {
        goto err;
    }

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/*Function to handle Generic commands*/
int getGenericInfo_wext(void *ctx, const char * ifname, void * chanInfo, int chanInfoSize)
{
    struct iwreq Wrq;
    int ret;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    Wrq.u.data.pointer = chanInfo;
    Wrq.u.data.length = chanInfoSize;

    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_CONFIG_GENERIC, &Wrq)) < 0) {
        goto err;
    }

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/*Function to get center Frequency*/
int getCfreq2_wext(void *ctx, const char * ifname, int32_t * cfreq2)
{
    int ret,iwparam;
    struct iwreq Wrq;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    iwparam = IEEE80211_PARAM_SECOND_CENTER_FREQ;
    memcpy(Wrq.u.name, &iwparam, sizeof(iwparam));
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_GETPARAM, &Wrq)) < 0) {
        goto err;
    }
    memcpy(cfreq2, Wrq.u.name, sizeof(int));

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/* Function to get channel info*/
int getChannelInfo_wext(void *ctx, const char * ifname, void * chanInfo, int chanInfoSize)
{
    struct iwreq Wrq;
    int ret;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    Wrq.u.data.pointer = chanInfo;
    Wrq.u.data.length = chanInfoSize;

    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_GETCHANINFO, &Wrq)) < 0) {
        goto err;
    }

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;

}

/* Function to get channel info160*/
int getChannelInfo160_wext(void *ctx, const char * ifname, void * chanInfo, int chanInfoSize)
{
    struct iwreq Wrq;
    int ret;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    Wrq.u.data.pointer = chanInfo;
    Wrq.u.data.length = chanInfoSize;

    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_CONFIG_GENERIC, &Wrq)) < 0) {
        goto err;
    }

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/* Function to get station info*/
int getStationInfo_wext(void *ctx, const char * ifname, void *buf, int * len)
{

    struct iwreq iwr;
    int ret;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(iwr.ifr_name, ifname, IFNAMSIZ );
    iwr.u.data.pointer = buf;
    iwr.u.data.length = *len;

    if(( ret = ioctl( priv->Sock, IEEE80211_IOCTL_STA_INFO, &iwr )) < 0 )
    {
        goto err;
    }

    *len = iwr.u.data.length;

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/* Function to get acs state*/
int getAcsState_wext(void * ctx, const char *ifname, int * acsstate)
{
    int ret,iwparam;
    struct iwreq Wrq;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    iwparam = IEEE80211_PARAM_GET_ACS;
    memcpy(Wrq.u.name, &iwparam, sizeof(iwparam));
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_GETPARAM, &Wrq)) < 0) {
        goto err;
    }
    memcpy(acsstate, Wrq.u.name, sizeof(int));

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/*Function to get cac state*/
int getCacState_wext(void * ctx, const char *ifname, int * cacstate)
{
    int ret,iwparam;
    struct iwreq Wrq;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    iwparam = IEEE80211_PARAM_GET_CAC;
    memcpy(Wrq.u.name, &iwparam, sizeof(iwparam));
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_GETPARAM, &Wrq)) < 0) {
        goto err;
    }
    memcpy(cacstate, Wrq.u.name, sizeof(int));

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;

}

/*Function to get parent index info*/
int getParentIfindex_wext(void * ctx, const char *ifname, int * parenIndex)
{
    int ret,iwparam;
    struct iwreq Wrq;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    iwparam = IEEE80211_PARAM_PARENT_IFINDEX;
    memcpy(Wrq.u.name, &iwparam, sizeof(iwparam));
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_GETPARAM, &Wrq)) < 0) {
        goto err;
    }
    memcpy(parenIndex, Wrq.u.name, sizeof(int));

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;

}

/* Function to get smart monitor*/
int getSmartMonitor_wext(void * ctx, const char *ifname, int * smartmonitor)
{
    int ret,iwparam;
    struct iwreq Wrq;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    iwparam = IEEE80211_PARAM_RX_FILTER_SMART_MONITOR;;
    memcpy(Wrq.u.name, &iwparam, sizeof(iwparam));
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_GETPARAM, &Wrq)) < 0) {
        goto err;
    }
    memcpy(smartmonitor, Wrq.u.name, sizeof(int));

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/*Function to handle setparam maccmd*/
int setParamMaccmd_wext(void * ctx, const char *ifname, void *param, uint32_t len)
{
    int ret;
    struct iwreq Wrq;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    memcpy(Wrq.u.name, param, len);
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_SETPARAM, &Wrq)) < 0) {
        goto err;
    }

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;

}

/* Function to handle setparam in driver*/
int setParam_wext(void *ctx, const char *ifname,int cmd, void *param, uint32_t len)
{
    int ret;
    struct iwreq Wrq;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    memcpy(Wrq.u.name, &cmd, sizeof(int));
    memcpy(&Wrq.u.name[sizeof(cmd)], param, len);
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_SETPARAM, &Wrq)) < 0) {
        goto err;
    }

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;

}

/*Function to get range */
int getRange_wext(void *ctx, const char *ifname, int *we_version)
{
    int ret;
    struct iw_range *range;
    struct iwreq iwr;
    struct wlanif_wext_priv *priv;
    size_t buflen;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    /*
     * Use larger buffer than struct iw_range in order to allow the
     * structure to grow in the future.
     */
    buflen = sizeof(struct iw_range) + 500;
    range = malloc(buflen);
    if (range == NULL)
        return -1;

    memset(range, 0, buflen);
    memset(&iwr, 0, sizeof(iwr));

    strlcpy(iwr.ifr_name, ifname, IFNAMSIZ);
    iwr.u.data.pointer = (caddr_t) range;
    iwr.u.data.length = buflen;

    if ((ret = ioctl(priv->Sock, SIOCGIWRANGE, &iwr)) < 0) {
        goto err;
    }

    *we_version = range->we_version_compiled;

    free(range);
    TRACE_EXIT();
    return 0;

err:
    free(range);
    TRACE_EXIT_ERR();
    return ret;
}

/*Function to get station stats*/
int getStaStats_wext(void * ctx , const char *ifname, void *data , uint32_t data_len)
{
    struct iwreq iwr;
    int ret;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(iwr.ifr_name, ifname, IFNAMSIZ);

    iwr.u.data.pointer = data;
    iwr.u.data.length = data_len;
    if(( ret = ioctl( priv->Sock, IEEE80211_IOCTL_STA_STATS, &iwr )) < 0 )
    {
        goto err;
    }

    TRACE_EXIT();
    return 0;
err:
    TRACE_EXIT_ERR();
    return ret;

}

/* Function to get dbreq info*/
int getDbgreq_wext(void * ctx , const char *ifname, void *data , uint32_t data_len)
{

    struct iwreq iwr;
    int ret;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(iwr.ifr_name, ifname, IFNAMSIZ);

    iwr.u.data.pointer = data;
    iwr.u.data.length = data_len;
    if(( ret = ioctl( priv->Sock, IEEE80211_IOCTL_DBGREQ, &iwr )) < 0 )
    {
        goto err;
    }

    TRACE_EXIT();
    return ret;
err:
    TRACE_EXIT_ERR();
    return ret;
}

/*Funtion to get extended subcommands */
int getExtended_wext(void * ctx , const char *ifname, void *data , uint32_t data_len)
{

    struct ifreq ifr;
    int ret;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);

    ifr.ifr_data = data;
    if(( ret = ioctl( priv->Sock, SIOCGATHEXTENDED, &ifr )) < 0 )
    {
        goto err;
    }

    TRACE_EXIT();
    return 0;
err:
    TRACE_EXIT_ERR();
    return ret;
}

/*Funtion to handle Add/Del/Kick Mac commands*/
int addDelKickMAC_wext(void * ctx , const char *ifname, int operation, void *data, uint32_t len)
{

    struct iwreq iwr;
    int ret;
    struct wlanif_wext_priv * priv;
    int ioctl_id = -1;

    TRACE_ENTRY();

    switch (operation)
    {
        case IO_OPERATION_ADDMAC:
            ioctl_id = IEEE80211_IOCTL_ADDMAC;
            break;
        case IO_OPERATION_DELMAC:
            ioctl_id = IEEE80211_IOCTL_DELMAC;
            break;
        case IO_OPERATION_KICKMAC:
            ioctl_id = IEEE80211_IOCTL_KICKMAC;
            break;
        default:
            /*Unsupported operation*/
            ret = -1;
            goto err;
            break;
    }
    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(iwr.ifr_name, ifname, IFNAMSIZ);

    memcpy(iwr.u.name, data, len);

    if(( ret = ioctl( priv->Sock, ioctl_id , &iwr )) < 0 )
    {
        goto err;
    }

    TRACE_EXIT();
    return 0;
err:
    TRACE_EXIT_ERR();
    return ret;
}

/*Function to set filter command */
int setFilter_wext(void * ctx , const char *ifname, void *data, uint32_t len)
{
	struct iwreq iwr;
	int ret;
	struct wlanif_wext_priv * priv;

	TRACE_ENTRY();
	priv = (struct wlanif_wext_priv *) ctx;
	assert(priv != NULL);

	memset(&iwr, 0, sizeof(iwr));

	strlcpy(iwr.ifr_name, ifname, IFNAMSIZ);
	iwr.u.data.pointer = data;
        iwr.u.data.length = len;

	if(( ret = ioctl( priv->Sock, IEEE80211_IOCTL_FILTERFRAME, &iwr )) < 0 )
	{
		goto err;
	}

	TRACE_EXIT();
	return 0;
err:
	TRACE_EXIT_ERR();
	return ret;
}

/*Function to get Wireless mode from driver*/
int getWirelessMode_wext(void * ctx , const char *ifname, void *data, uint32_t len)
{
	struct iwreq iwr;
	int ret;
	struct wlanif_wext_priv * priv;
    char *chr=(char *)data;

	TRACE_ENTRY();
	priv = (struct wlanif_wext_priv *) ctx;
	assert(priv != NULL);

	strlcpy(iwr.ifr_name, ifname, IFNAMSIZ);
	iwr.u.data.pointer = data;
	iwr.u.data.length = sizeof(data);

    if(( ret = ioctl( priv->Sock, IEEE80211_IOCTL_GETMODE, &iwr )) < 0 )
	{
		goto err;
	}
    chr[iwr.u.data.length] = '\0';

	TRACE_EXIT();
	return 0;
err:
	TRACE_EXIT_ERR();
	return ret;
}

/* Function to send mgmt packet*/
int sendMgmt_wext(void * ctx , const char *ifname, void *data, uint32_t len)
{
	struct iwreq iwr;
	int ret;
	struct wlanif_wext_priv * priv;

	TRACE_ENTRY();
	priv = (struct wlanif_wext_priv *) ctx;
	assert(priv != NULL);

	strlcpy(iwr.ifr_name, ifname, IFNAMSIZ);
	iwr.u.data.pointer = data;
	iwr.u.data.length = len;

	if(( ret = ioctl( priv->Sock, IEEE80211_IOCTL_SEND_MGMT, &iwr )) < 0 )
	{
		    goto err;
	}

	TRACE_EXIT();
	return 0;
err:
	TRACE_EXIT_ERR();
	return ret;
}

/* Function to get station count */
int getStaCount_wext(void *ctx, const char * ifname, int32_t * result)
{
    int ret, iwparam;
    struct iwreq Wrq;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ);
    iwparam = IEEE80211_PARAM_STA_COUNT;
    memcpy(Wrq.u.name, &iwparam, sizeof(iwparam));
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_GETPARAM, &Wrq)) < 0)
    {
        goto err;
    }
    memcpy(result, Wrq.u.name, sizeof(int));

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/// @see setIntfMode(const void *, const char *, const char *, u_int8_t);
int setIntfMode_wext(void *ctx, const char * ifname, const char * mode, u_int8_t len)
{
    int ret;
    struct iwreq Wrq;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    memset(&Wrq, 0, sizeof(struct iwreq));
    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ);
    Wrq.u.data.pointer = (void *)mode;
    Wrq.u.data.length = len;

    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_SETMODE, &Wrq)) < 0) {
        goto err;
    }

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

int setParamVapInd_wext(void * ctx, const char *ifname, void *param, uint32_t len)
{
    int ret;
    struct iwreq Wrq;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    memcpy(Wrq.u.name, param, len);
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_SETPARAM, &Wrq)) < 0) {
        goto err;
    }

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

int setFreq_wext(void *ctx, const char * ifname, int freq)
{
    int ret;
    struct wlanif_wext_priv * priv;
    struct iwreq Wrq = { { { 0 } } };

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ );
    Wrq.u.freq.m = freq;
    Wrq.u.freq.e = 0;
    Wrq.u.freq.flags = IW_FREQ_FIXED;

    if ((ret = ioctl(priv->Sock, SIOCSIWFREQ, &Wrq)) < 0) {
        goto err;
    }

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/// @see setCFreq2(const void *, const char *, int)
int setCFreq2_wext(void *ctx, const char *ifname, int chan_num) {
    int ret;
    struct iwreq Wrq;
    struct wlanif_wext_priv *priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *)ctx;
    assert(priv != NULL);

    int param[2] = {IEEE80211_PARAM_SECOND_CENTER_FREQ, chan_num};

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ);
    memcpy(Wrq.u.name, &param, sizeof(param));
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_SETPARAM, &Wrq)) < 0) {
        goto err;
    }

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

/// @see getPrivArgs(const void *, const char *, size_t *)
struct iw_priv_args *getPrivArgs_wext(void *ctxt, const char *ifname,
                                      size_t *numIoctls) {
    struct iwreq wrq;
    struct wlanif_wext_priv *priv;
    struct iw_priv_args *privArgs = NULL;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *)ctxt;
    assert(priv != NULL);

    // We do not know how many actual ioctls there are, so allocate a
    // reasonably large number to start with. If it is insufficient, the
    // kernel will tell us and we can re-allocate.
    size_t maxNumEntries = 256;
    privArgs = calloc(maxNumEntries, sizeof(struct iw_priv_args));
    if (!privArgs) {
        printf("%s: Initial memory allocation failed for private ioctl buffer",
             __func__);
        TRACE_EXIT();
        return NULL;
    }

    memset(&wrq, 0, sizeof(wrq));
    strlcpy(wrq.ifr_name, ifname, sizeof(wrq.ifr_name));
    wrq.u.data.pointer = privArgs;
    wrq.u.data.length = maxNumEntries;

    while (ioctl(priv->Sock, SIOCGIWPRIV, &wrq) < 0) {
        if (E2BIG == errno) {
            // Kernel can provide a hint of how many entries we need.
            if (wrq.u.data.length > maxNumEntries) {
                maxNumEntries = wrq.u.data.length;
            } else {
                maxNumEntries *= 2;
            }

            struct iw_priv_args *privArgs2 = NULL;
            privArgs2 = realloc(privArgs, maxNumEntries * sizeof(struct iw_priv_args));
            if (!privArgs2) {
                printf("%s: Failed to realloc for %zu entries",
                     __func__, maxNumEntries);
                free(privArgs);
                TRACE_EXIT();
                return NULL;
            }

            privArgs = privArgs2;
            wrq.u.data.pointer = privArgs;
            wrq.u.data.length = maxNumEntries;
        } else {
            printf("%s: ioctl failed on %s for unknown reason: %d",
                 __func__, wrq.ifr_name, errno);
            free(privArgs);
            TRACE_EXIT();
            return NULL;
        }
    }

    *numIoctls = wrq.u.data.length;
    TRACE_EXIT();

    return privArgs;
}

/// @see getACSReport(void *, const char *, u_int8_t *, ieee80211_acs_report_t[],
///                   u_int8_t *, ieee80211_neighbor_info_t[], u_int8_t[])
int getACSReport_wext(void *ctx, const char *ifName, u_int8_t* numChans,
                      ieee80211_acs_report_t *chanData, u_int8_t* numNeighbors,
                      ieee80211_neighbor_info_t *neighborData, u_int8_t neighborChans[]) {
    int ret;
    struct iwreq iwr;
    struct wlanif_wext_priv *priv = (struct wlanif_wext_priv *)ctx;

    TRACE_ENTRY();

    struct ieee80211req_athdbg req;
    memset(&req, 0, sizeof(struct ieee80211req_athdbg));
    (void)memset(&iwr, 0, sizeof(iwr));

    if (strlcpy(iwr.ifr_name, ifName, sizeof(iwr.ifr_name)) >= sizeof(iwr.ifr_name)) {
        return -EINVAL;
    }

    iwr.u.data.pointer = (void *)&req;
    iwr.u.data.length = (sizeof(struct ieee80211req_athdbg));
    req.cmd = IEEE80211_DBGREQ_GETACSREPORT;

    struct ieee80211_acs_dbg *acs = NULL;
    acs = (struct ieee80211_acs_dbg *)calloc(1, sizeof(struct ieee80211_acs_dbg));
    if (!acs) {
        return -ENOMEM;
    }

    // First read to get number of channels
    req.data.acs_rep.data_addr = acs;
    req.data.acs_rep.data_size = sizeof(struct ieee80211_acs_dbg);
    req.data.acs_rep.index = 0;
    acs->entry_id = 0;
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_DBGREQ, &iwr)) < 0) {
        TRACE_EXIT_ERR();
        goto done;
    }

    acs->neighbor_list = (void *)calloc(IEEE80211_MAX_NEIGHBOURS, sizeof(ieee80211_neighbor_info));
    acs->neighbor_size = sizeof(ieee80211_neighbor_info) * (IEEE80211_MAX_NEIGHBOURS);
    if (!acs->neighbor_list) {
        ret = ENOMEM;
        TRACE_EXIT_ERR();
        goto done;
    }

    // Loop through each channel data and get the neighbors
    u_int8_t idxChan = 0, idxNeighbor = 0;
    for (idxChan = 0; idxChan < *numChans && idxChan < acs->nchans; ++idxChan) {
        acs->entry_id = idxChan;
        req.cmd = IEEE80211_DBGREQ_GETACSREPORT;

        ret = ioctl(priv->Sock, IEEE80211_IOCTL_DBGREQ, &iwr);
        if (ret < 0) {
            TRACE_EXIT_ERR();
            goto done;
        }

        // Number of Neighbors scanned greater than allocated memory
        if (ret == E2BIG) {
            if (acs->neighbor_list) {
                free(acs->neighbor_list);
                acs->neighbor_list = NULL;
            }

            acs->neighbor_list = (void *)malloc(sizeof(ieee80211_neighbor_info) * acs->chan_nbss);
            if (NULL == acs->neighbor_list) {
                ret = ENOMEM;
                TRACE_EXIT_ERR();
                goto done;
            } else {
                memset(acs->neighbor_list, 0, sizeof(ieee80211_neighbor_info) * acs->chan_nbss);
                acs->neighbor_size = sizeof(ieee80211_neighbor_info) * acs->chan_nbss;
                if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_DBGREQ, &iwr)) < 0) {
                    TRACE_EXIT_ERR();
                    goto done;
                }

            }
        }

        // Copy the channel data
        memcpy(&chanData[idxChan], acs, sizeof(struct ieee80211_acs_dbg));

        // Append the neighbor data
        u_int8_t j;
        for (j = 0; j < acs->chan_nbss && idxNeighbor < *numNeighbors; j++) {
            memcpy(&neighborData[idxNeighbor], &acs->neighbor_list[j],
                   sizeof(ieee80211_neighbor_info));
            neighborChans[idxNeighbor] = acs->ieee_chan;
            idxNeighbor++;
        }
    }

    *numChans = idxChan;
    *numNeighbors = idxNeighbor;
    TRACE_EXIT();

done:
    if (acs->neighbor_list) {
        free(acs->neighbor_list);
    }
    free(acs);
    return ret;
}

int setAcsChanList_wext(void *ctx, const char *ifName, u_int8_t numChans, u_int8_t *channels) {
    int ret = -1;
    struct iwreq iwr;
    struct wlanif_wext_priv *priv;
    struct ieee80211req_athdbg req;
    ieee80211_user_chanlist_t chanlist;
    u_int8_t *chan = NULL;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *)ctx;
    assert(priv != NULL);

    memset(&req, 0, sizeof(struct ieee80211req_athdbg));
    (void)memset(&iwr, 0, sizeof(iwr));

    if (strlcpy(iwr.ifr_name, ifName, sizeof(iwr.ifr_name)) >= sizeof(iwr.ifr_name)) {
        return -EINVAL;
    }

    chan = (u_int8_t *)malloc(sizeof(u_int8_t) * numChans);
    if (NULL == chan) {
        goto done;
    }

    chanlist.n_chan = numChans;
    req.data.param[0] = (u_long)&chanlist; /*typecasting to avoid warning */

    u_int8_t i;
    for (i = 0; i < numChans; i++) {
        chanlist.chans[i].chan = channels[i];
        chanlist.chans[i].band = 0; //TODO: should pass proper band index for 6Ghz support
    }

    iwr.u.data.pointer = (void *)&req;
    iwr.u.data.length = (sizeof(struct ieee80211req_athdbg));

    req.cmd = IEEE80211_DBGREQ_SETACSUSERCHANLIST;
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_DBGREQ, &iwr)) < 0) {
        TRACE_EXIT_ERR();
        goto done;
    }

    TRACE_EXIT();

done:
    free(chan);
    return ret;
}

/* Init Function*/
int wlanif_wext_init(struct wlanif_config *wext_conf)
{
    struct wlanif_wext_priv * priv;
    int ret;

    /* Initialize debug module used for logging */
    sonwextDbgS.dbgModule = dbgModuleFind("libsonwext");
    sonwextDbgS.dbgModule->Level=DBGERR;

    assert(wext_conf != NULL);

    wext_conf->IsCfg80211 = 0;
    wext_conf->ctx = malloc(sizeof(struct wlanif_wext_priv));

    if (wext_conf->ctx == NULL)
    {
        printf("%s: Failed\n",__func__);
        return -ENOMEM;
    }

    priv = (struct wlanif_wext_priv *) wext_conf->ctx;
    assert(priv != NULL);

    if( (ret = wext_priv_sock_init(priv)) )
    {
        return ret;
    }

    return 0;
}

/*Deinit Function*/
void wlanif_wext_deinit(struct wlanif_config *wext_conf)
{
    struct wlanif_wext_priv * priv;

    assert(wext_conf != NULL);

    priv = (struct wlanif_wext_priv *) wext_conf->ctx;

    assert(priv != NULL);
    wext_priv_sock_close(priv);

    free(priv);
}

/*Function to get country code*/
int getCountryCode_wext(void *ctx, const char *ifname, size_t size, char *countryCode) {
    struct wlanif_wext_priv *priv;
    struct iwreq iwr;
    int ret, iwparam;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *)ctx;
    assert(priv != NULL);

    strlcpy(iwr.ifr_name, ifname, IFNAMSIZ);
    iwparam = IEEE80211_PARAM_COUNTRYCODE;
    memcpy(iwr.u.name, &iwparam, sizeof(iwparam));
    if(( ret = ioctl( priv->Sock, IEEE80211_IOCTL_GETPARAM, &iwr )) < 0 ) {
        goto err;
    }

    memcpy(countryCode, iwr.u.name, size);
    TRACE_EXIT();
    return 0;
err:
    TRACE_EXIT_ERR();
    return ret;
}

/*Function to get MAP Bss Role*/
static int getMapBssRole_wext(void *ctx, const char *ifname, u_int8_t *mapBssRole) {
    int ret,iwparam;
    struct iwreq Wrq;
    struct wlanif_wext_priv *priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *)ctx;
    assert(priv != NULL);

    strlcpy(Wrq.ifr_name, ifname, IFNAMSIZ);
    iwparam = IEEE80211_PARAM_MAP_BSS_TYPE;
    memcpy(Wrq.u.name, &iwparam, sizeof(iwparam));
    if ((ret = ioctl(priv->Sock, IEEE80211_IOCTL_GETPARAM, &Wrq)) < 0) {
        goto err;
    }
    memcpy(mapBssRole, Wrq.u.name, sizeof(u_int8_t));

    TRACE_EXIT();
    return 0;

err:
    TRACE_EXIT_ERR();
    return ret;
}

int setCACTimeout_wext(void *ctx, const char *ifname, int cac_timeout) {
    TRACE_ENTRY();

    TRACE_EXIT();
    return 0;
}

int getPhyStats_wext(void * ctx , const char *ifname, void *data, uint32_t len)
{
    struct ifreq ifr;
    int ret;
    struct wlanif_wext_priv * priv;

    TRACE_ENTRY();

    priv = (struct wlanif_wext_priv *) ctx;
    assert(priv != NULL);

    strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);

    ifr.ifr_data = data;
    if(( ret = ioctl( priv->Sock, SIOCGATHPHYSTATS, &ifr )) < 0 )
    {
        goto err;
    }

    TRACE_EXIT();
    return 0;
err:
    TRACE_EXIT_ERR();
    return ret;
}

/*ioctl ops function list*/
static struct wlanif_config_ops wlanif_wext_ops = {
    .init = wlanif_wext_init,
    .deinit = wlanif_wext_deinit,
    .getName = getName_wext,
    .isAP = isAP_wext,
    .getBSSID = getBSSID_wext,
    .getESSID = getESSID_wext,
    .getFreq = getFreq_wext,
    .getChannelWidth = getChannelWidth_wext,
    .getChannelExtOffset = getChannelExtOffset_wext,
    .getChannelBandwidth = getChannelBandwidth_wext,
    .getAcsState = getAcsState_wext,
    .getCacState = getCacState_wext,
    .getParentIfindex = getParentIfindex_wext,
    .getSmartMonitor = getSmartMonitor_wext,
    .getGenericInfoAtf = getGenericInfoCmd_wext,
    .getGenericInfoAld = getGenericInfo_wext,
    .getGenericHmwds = getGenericInfo_wext,
    .getGenericNac = getGenericInfo_wext,
    .getCfreq2 = getCfreq2_wext,
    .getChannelInfo = getChannelInfo_wext,
    .getChannelInfo160 = getChannelInfo160_wext,
    .getStationInfo = getStationInfo_wext,
    .getDbgreq = getDbgreq_wext,
    .getExtended = getExtended_wext,
    .addDelKickMAC = addDelKickMAC_wext,
    .setFilter = setFilter_wext,
    .getWirelessMode = getWirelessMode_wext,
    .sendMgmt = sendMgmt_wext,
    .setParamMaccmd = setParamMaccmd_wext,
    .setParam = setParam_wext,
    .getStaStats = getStaStats_wext,
    .getRange = getRange_wext,
    .getStaCount = getStaCount_wext,
    .setIntfMode = setIntfMode_wext,
    .setParamVapInd = setParamVapInd_wext,
    .setFreq = setFreq_wext,
    .setCFreq2 = setCFreq2_wext,
    .getPrivArgs = getPrivArgs_wext,
    .getACSReport = getACSReport_wext,
    .getCountryCode = getCountryCode_wext,
    .setAcsChanList = setAcsChanList_wext,
    .getMapBssRole = getMapBssRole_wext,
    .setCACTimeout = setCACTimeout_wext,
    .getPhyStats = getPhyStats_wext,
};

struct wlanif_config_ops * wlanif_wext_get_ops()
{
    return &wlanif_wext_ops;
}

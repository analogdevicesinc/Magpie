/*************************************************************************************************/
/*!
 *  Copyright (c) 2011-2019 Arm Ltd. All Rights Reserved.
 *
 *  Copyright (c) 2019 Packetcraft, Inc.
 *
 *  Portions Copyright (c) 2022-2023 Analog Devices, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
/*************************************************************************************************/

#include <stdint.h>
#include <string.h>
#include "wsf_types.h"
#include "util/bstream.h"
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "hci_api.h"
#include "dm_api.h"
#include "att_api.h"
#include "smp_api.h"
#include "app_api.h"
#include "app_db.h"
#include "app_ui.h"
#include "app_hw.h"
#include "app_main.h"
#include "svc_ch.h"
#include "svc_core.h"
//#include "svc_hrs.h"
#include "svc_dis.h"
#include "svc_batt.h"
//#include "svc_rscs.h"
#include "gatt/gatt_api.h"
#include "bas/bas_api.h"
#include "hrps/hrps_api.h"
#include "rscp/rscp_api.h"
#include "pal_btn.h"
#include "tmr.h"
#include "magpie_api.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! WSF message event starting value */
#define MAGPIE_MSG_START 0xA0

/* Default Running Speed and Cadence Measurement period (seconds) */
#define MAGPIE_DEFAULT_RSCM_PERIOD 1

/*! WSF message event enumeration */
enum {
    MAGPIE_CLOCK_TIMER_IND = MAGPIE_MSG_START, /*! Heart rate measurement timer expired */
    MAGPIE_BATT_TIMER_IND, /*! Battery measurement timer expired */
    MAGPIE_SDCARD_TIMER_IND /*! SD Card measurement timer expired */
};

/*! Button press handling constants */
#define BTN_SHORT_MS 200
#define BTN_MED_MS 500
#define BTN_LONG_MS 1000

#define BTN_1_TMR MXC_TMR2
#define BTN_2_TMR MXC_TMR3

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! Application message type */
typedef union {
    wsfMsgHdr_t hdr;
    dmEvt_t dm;
    attsCccEvt_t ccc;
    attEvt_t att;
} magpieMsg_t;

/**************************************************************************************************
  Configurable Parameters
**************************************************************************************************/

/*! configurable parameters for advertising */
static const appAdvCfg_t magpieAdvCfg = {
    { 60000, 0, 0 }, /*! Advertising durations in ms */
    { 800, 0, 0 } /*! Advertising intervals in 0.625 ms units */
};

/*! configurable parameters for slave */
static const appSlaveCfg_t magpieSlaveCfg = {
    MAGPIE_CONN_MAX, /*! Maximum connections */
};

/*! configurable parameters for security */
//TODO: Figure out securing pairing
static const appSecCfg_t magpieSecCfg = {
    DM_AUTH_BOND_FLAG | DM_AUTH_SC_FLAG, /*! Authentication and bonding flags */
    0, /*! Initiator key distribution flags */
    DM_KEY_DIST_LTK, /*! Responder key distribution flags */
    FALSE, /*! TRUE if Out-of-band pairing data is present */
    TRUE /*! TRUE to initiate security upon connection */
};

/*! configurable parameters for connection parameter update */
static const appUpdateCfg_t magpieUpdateCfg = {
    6000,
    /*! ^ Connection idle period in ms before attempting
    connection parameter update; set to zero to disable */
    640, /*! Minimum connection interval in 1.25ms units */
    800, /*! Maximum connection interval in 1.25ms units */
    0, /*! Connection latency */
    900, /*! Supervision timeout in 10ms units */
    5 /*! Number of update attempts before giving up */
};

/*! heart rate measurement configuration */
//static const hrpsCfg_t magpieHrpsCfg = {
//    2000 /*! Measurement timer expiration period in ms */
//};

/*! battery measurement configuration */
static const basCfg_t magpieBasCfg = {
    30, /*! Battery measurement timer expiration period in seconds */
    1, /*! Perform battery measurement after this many timer periods */
    100 /*! Send battery level notification to peer when below this level. */
};

/*! SMP security parameter configuration */
static const smpCfg_t magpieSmpCfg = {
    500, /*! 'Repeated attempts' timeout in msec */
    SMP_IO_NO_IN_NO_OUT, /*! I/O Capability */
    7, /*! Minimum encryption key length */
    16, /*! Maximum encryption key length */
    1, /*! Attempts to trigger 'repeated attempts' timeout */
    0, /*! Device authentication requirements */
    64000, /*! Maximum repeated attempts timeout in msec */
    64000, /*! Time msec before attemptExp decreases */
    2 /*! Repeated attempts multiplier exponent */
};

/**************************************************************************************************
  Advertising Data
**************************************************************************************************/

/*! advertising data, discoverable mode */
static const uint8_t magpieAdvDataDisc[] = {
    /*! flags */
    2, /*! length */
    DM_ADV_TYPE_FLAGS, /*! AD type */
    DM_FLAG_LE_GENERAL_DISC | /*! flags */
        DM_FLAG_LE_BREDR_NOT_SUP,

    /*! tx power */
    2, /*! length */
    DM_ADV_TYPE_TX_POWER, /*! AD type */
    0, /*! tx power */

    /*! service UUID list */
    5, /*! length */  //UUID is 1 byte, the rest is 2 bytes for each service
    DM_ADV_TYPE_16_UUID, /*! AD type */  //takes 1 byte
    //UINT16_TO_BYTES(ATT_UUID_HEART_RATE_SERVICE), UINT16_TO_BYTES(ATT_UUID_RUNNING_SPEED_SERVICE),
    UINT16_TO_BYTES(ATT_UUID_DEVICE_INFO_SERVICE), UINT16_TO_BYTES(ATT_UUID_BATTERY_SERVICE),

	2,
	DM_ADV_TYPE_MANUFACTURER,
	0xFF   //put whatever here
};

/*! scan data, discoverable mode */
static const uint8_t magpieScanDataDisc[] = {
    /*! device name */
    15, /*! length */
    DM_ADV_TYPE_LOCAL_NAME, /*! AD type */
    'M', 'a', 'g','p','i','e','R','e','c','o','r','d','e','r'
};

/**************************************************************************************************
  Client Characteristic Configuration Descriptors
**************************************************************************************************/

/*! enumeration of client characteristic configuration descriptors */
enum {
    MAGPIE_GATT_SC_CCC_IDX, /*! GATT service, service changed characteristic */
    MAGPIE_CLOCK_CLOCKMS_CCC_IDX, /*! Clock service, clock monitor characteristic */
    MAGPIE_BATT_LVL_CCC_IDX, /*! Battery service, battery level characteristic */
    MAGPIE_SD_MS_CCC_IDX, /*! SD Card measurement characteristic */
    MAGPIE_NUM_CCC_IDX
};

/*! client characteristic configuration descriptors settings, indexed by above enumeration */
static const attsCccSet_t magpieCccSet[MAGPIE_NUM_CCC_IDX] = {
    /* cccd handle          value range               security level */
    { GATT_SC_CH_CCC_HDL, ATT_CLIENT_CFG_INDICATE, DM_SEC_LEVEL_NONE }, /* MAGPIE_GATT_SC_CCC_IDX */
    { BATT_LVL_CH_CCC_HDL, ATT_CLIENT_CFG_NOTIFY, DM_SEC_LEVEL_NONE } /* MAGPIE_BATT_LVL_CCC_IDX */

};

//Removed:   { HRS_HRM_CH_CCC_HDL, ATT_CLIENT_CFG_NOTIFY, DM_SEC_LEVEL_NONE }, /* MAGPIE_HRS_HRM_CCC_IDX */
//Removed:   { RSCS_RSM_CH_CCC_HDL, ATT_CLIENT_CFG_NOTIFY, DM_SEC_LEVEL_NONE } /* MAGPIE_RSCS_SM_CCC_IDX */

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! WSF handler ID */
wsfHandlerId_t magpieHandlerId;

/* WSF Timer to send running speed and cadence measurement data */
wsfTimer_t magpieRscmTimer;

/* Running Speed and Cadence Measurement period - Can be changed at runtime to vary period */
static uint16_t magpieRscmPeriod = MAGPIE_DEFAULT_RSCM_PERIOD;

/* Heart Rate Monitor feature flags */
static uint8_t magpieHrmFlags = CH_HRM_FLAGS_VALUE_8BIT | CH_HRM_FLAGS_ENERGY_EXP;

/*************************************************************************************************/
/*!
 *  \brief  Application DM callback.
 *
 *  \param  pDmEvt  DM callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void magpieDmCback(dmEvt_t *pDmEvt)
{
    dmEvt_t *pMsg;
    uint16_t len;

    len = DmSizeOfEvt(pDmEvt);

    if ((pMsg = WsfMsgAlloc(len)) != NULL) {
        memcpy(pMsg, pDmEvt, len);
        WsfMsgSend(magpieHandlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Application ATT callback.
 *
 *  \param  pEvt    ATT callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void magpieAttCback(attEvt_t *pEvt)
{
    attEvt_t *pMsg;

    if ((pMsg = WsfMsgAlloc(sizeof(attEvt_t) + pEvt->valueLen)) != NULL) {
        memcpy(pMsg, pEvt, sizeof(attEvt_t));
        pMsg->pValue = (uint8_t *)(pMsg + 1);
        memcpy(pMsg->pValue, pEvt->pValue, pEvt->valueLen);
        WsfMsgSend(magpieHandlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Application ATTS client characteristic configuration callback.
 *
 *  \param  pDmEvt  DM callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void magpieCccCback(attsCccEvt_t *pEvt)
{
    attsCccEvt_t *pMsg;
    appDbHdl_t dbHdl;

    /* If CCC not set from initialization and there's a device record and currently bonded */
    if ((pEvt->handle != ATT_HANDLE_NONE) &&
        ((dbHdl = AppDbGetHdl((dmConnId_t)pEvt->hdr.param)) != APP_DB_HDL_NONE) &&
        AppCheckBonded((dmConnId_t)pEvt->hdr.param)) {
        /* Store value in device database. */
        AppDbSetCccTblValue(dbHdl, pEvt->idx, pEvt->value);
    }

    if ((pMsg = WsfMsgAlloc(sizeof(attsCccEvt_t))) != NULL) {
        memcpy(pMsg, pEvt, sizeof(attsCccEvt_t));
        WsfMsgSend(magpieHandlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
*  \brief  Send a Running Speed and Cadence Measurement Notification.
*
*  \param  connId    connection ID
*
*  \return None.
*/
/*************************************************************************************************/
static void magpieSendRunningSpeedMeasurement(dmConnId_t connId)
{
    if (AttsCccEnabled(connId, MAGPIE_SD_MS_CCC_IDX)) {
        static uint8_t walk_run = 1;

        /* TODO: Set Running Speed and Cadence Measurement Parameters */

        RscpsSetParameter(RSCP_SM_PARAM_SPEED, 1);
        RscpsSetParameter(RSCP_SM_PARAM_CADENCE, 2);
        RscpsSetParameter(RSCP_SM_PARAM_STRIDE_LENGTH, 3);
        RscpsSetParameter(RSCP_SM_PARAM_TOTAL_DISTANCE, 4);

        /* Toggle running/walking */
        walk_run = walk_run ? 0 : 1;
        RscpsSetParameter(RSCP_SM_PARAM_STATUS, walk_run);

        RscpsSendSpeedMeasurement(connId);
    }

    /* Configure and start timer to send the next measurement */
    magpieRscmTimer.msg.event = MAGPIE_SDCARD_TIMER_IND;
    magpieRscmTimer.msg.status = MAGPIE_SD_MS_CCC_IDX;
    magpieRscmTimer.handlerId = magpieHandlerId;
    magpieRscmTimer.msg.param = connId;

    WsfTimerStartSec(&magpieRscmTimer, magpieRscmPeriod);
}

/*************************************************************************************************/
/*!
 *  \brief  Process CCC state change.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void magpieProcCccState(magpieMsg_t *pMsg)
{
    APP_TRACE_INFO3("ccc state ind value:%d handle:%d idx:%d", pMsg->ccc.value, pMsg->ccc.handle,
                    pMsg->ccc.idx);

    /* handle heart rate measurement CCC */
    if (pMsg->ccc.idx == MAGPIE_CLOCK_CLOCKMS_CCC_IDX) {
        if (pMsg->ccc.value == ATT_CLIENT_CFG_NOTIFY) {
            HrpsMeasStart((dmConnId_t)pMsg->ccc.hdr.param, MAGPIE_CLOCK_TIMER_IND, MAGPIE_CLOCK_CLOCKMS_CCC_IDX);
        } else {
            HrpsMeasStop((dmConnId_t)pMsg->ccc.hdr.param);
        }
        return;
    }

    /* handle running speed and cadence measurement CCC */
    if (pMsg->ccc.idx == MAGPIE_SD_MS_CCC_IDX) {
        if (pMsg->ccc.value == ATT_CLIENT_CFG_NOTIFY) {
            magpieSendRunningSpeedMeasurement((dmConnId_t)pMsg->ccc.hdr.param);
        } else {
            WsfTimerStop(&magpieRscmTimer);
        }
        return;
    }

    /* handle battery level CCC */
    if (pMsg->ccc.idx == MAGPIE_BATT_LVL_CCC_IDX) {
        if (pMsg->ccc.value == ATT_CLIENT_CFG_NOTIFY) {
            BasMeasBattStart((dmConnId_t)pMsg->ccc.hdr.param, MAGPIE_BATT_TIMER_IND,
                             MAGPIE_BATT_LVL_CCC_IDX);
        } else {
            BasMeasBattStop((dmConnId_t)pMsg->ccc.hdr.param);
        }
        return;
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Perform UI actions on connection close.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void magpieClose(magpieMsg_t *pMsg)
{
    /* stop heart rate measurement */
    HrpsMeasStop((dmConnId_t)pMsg->hdr.param);

    /* stop battery measurement */
    BasMeasBattStop((dmConnId_t)pMsg->hdr.param);

    /* Stop running speed and cadence timer */
    WsfTimerStop(&magpieRscmTimer);
}

/*************************************************************************************************/
/*!
 *  \brief  Set up advertising and other procedures that need to be performed after
 *          device reset.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void magpieSetup(magpieMsg_t *pMsg)
{
    /* set advertising and scan response data for discoverable mode */
    AppAdvSetData(APP_ADV_DATA_DISCOVERABLE, sizeof(magpieAdvDataDisc), (uint8_t *)magpieAdvDataDisc);
    AppAdvSetData(APP_SCAN_DATA_DISCOVERABLE, sizeof(magpieScanDataDisc), (uint8_t *)magpieScanDataDisc);

    /* set advertising and scan response data for connectable mode */
    AppAdvSetData(APP_ADV_DATA_CONNECTABLE, 0, NULL);
    AppAdvSetData(APP_SCAN_DATA_CONNECTABLE, 0, NULL);

    /* start advertising; automatically set connectable/discoverable mode and bondable mode */
    AppAdvStart(APP_MODE_AUTO_INIT);
}

/*************************************************************************************************/
/*!
 *  \brief  Button press callback.
 *
 *  \param  btn    Button press.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void magpieBtnCback(uint8_t btn)
{
    dmConnId_t connId;
    static uint8_t heartRate = 78; /* for testing/demonstration */

    /* button actions when connected */
    if ((connId = AppConnIsOpen()) != DM_CONN_ID_NONE) {
        switch (btn) {
        case APP_UI_BTN_1_SHORT:
            /* increment the heart rate */
            AppHwHrmTest(++heartRate);
            break;

        case APP_UI_BTN_1_MED:
            break;

        case APP_UI_BTN_1_LONG:
            AppConnClose(connId);
            break;

        case APP_UI_BTN_2_SHORT:
            /* decrement the heart rate */
            AppHwHrmTest(--heartRate);
            break;

        case APP_UI_BTN_2_MED:
            /* Toggle HRM Sensor DET flags */
            if (!(magpieHrmFlags & (CH_HRM_FLAGS_SENSOR_DET | CH_HRM_FLAGS_SENSOR_NOT_DET))) {
                magpieHrmFlags |= CH_HRM_FLAGS_SENSOR_DET;
            } else if (magpieHrmFlags & CH_HRM_FLAGS_SENSOR_DET) {
                magpieHrmFlags &= ~CH_HRM_FLAGS_SENSOR_DET;
                magpieHrmFlags |= CH_HRM_FLAGS_SENSOR_NOT_DET;
            } else {
                magpieHrmFlags &= ~CH_HRM_FLAGS_SENSOR_NOT_DET;
            }

            HrpsSetFlags(magpieHrmFlags);
            break;

        case APP_UI_BTN_2_LONG:
            /* Toggle HRM RR Interval feature flag */
            if (magpieHrmFlags & CH_HRM_FLAGS_RR_INTERVAL) {
                magpieHrmFlags &= ~CH_HRM_FLAGS_RR_INTERVAL;
            } else {
                magpieHrmFlags |= CH_HRM_FLAGS_RR_INTERVAL;
            }

            HrpsSetFlags(magpieHrmFlags);
            break;

        default:
            break;
        }
    } else { /* button actions when not connected */
        switch (btn) {
        case APP_UI_BTN_1_SHORT:
            /* start or restart advertising */
            AppAdvStart(APP_MODE_AUTO_INIT);
            break;

        case APP_UI_BTN_1_MED:
            /* enter discoverable and bondable mode */
            AppSetBondable(TRUE);
            AppAdvStart(APP_MODE_DISCOVERABLE);
            break;

        case APP_UI_BTN_1_LONG:
            /* clear all bonding info */
            AppSlaveClearAllBondingInfo();

            /* restart advertising */
            AppAdvStart(APP_MODE_AUTO_INIT);
            break;

        case APP_UI_BTN_2_SHORT:
            /* Toggle HRM Flag for 8 and 16 bit values */
            if (magpieHrmFlags & CH_HRM_FLAGS_VALUE_16BIT) {
                magpieHrmFlags &= ~CH_HRM_FLAGS_VALUE_16BIT;
            } else {
                magpieHrmFlags |= CH_HRM_FLAGS_VALUE_16BIT;
            }

            HrpsSetFlags(magpieHrmFlags);
            break;

        default:
            break;
        }
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Process messages from the event handler.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void magpieProcMsg(magpieMsg_t *pMsg)
{
    uint8_t uiEvent = APP_UI_NONE;

    switch (pMsg->hdr.event) {
    case MAGPIE_SDCARD_TIMER_IND:
        magpieSendRunningSpeedMeasurement((dmConnId_t)pMsg->ccc.hdr.param);
        break;

    case MAGPIE_CLOCK_TIMER_IND:
        HrpsProcMsg(&pMsg->hdr);
        break;

    case MAGPIE_BATT_TIMER_IND:
        BasProcMsg(&pMsg->hdr);
        break;

    case ATTS_HANDLE_VALUE_CNF:
        HrpsProcMsg(&pMsg->hdr);
        BasProcMsg(&pMsg->hdr);
        break;

    case ATTS_CCC_STATE_IND:
        magpieProcCccState(pMsg);
        break;

    case DM_RESET_CMPL_IND:
        AttsCalculateDbHash();
        DmSecGenerateEccKeyReq();
        magpieSetup(pMsg);
        uiEvent = APP_UI_RESET_CMPL;
        break;

    case DM_ADV_SET_START_IND:
        uiEvent = APP_UI_ADV_SET_START_IND;
        break;

    case DM_ADV_SET_STOP_IND:
        uiEvent = APP_UI_ADV_SET_STOP_IND;
        break;

    case DM_ADV_START_IND:
        uiEvent = APP_UI_ADV_START;
        break;

    case DM_ADV_STOP_IND:
        uiEvent = APP_UI_ADV_STOP;
        break;

    case DM_CONN_OPEN_IND:
        HrpsProcMsg(&pMsg->hdr);
        BasProcMsg(&pMsg->hdr);
        uiEvent = APP_UI_CONN_OPEN;
        break;

    case DM_CONN_CLOSE_IND:
        magpieClose(pMsg);
        uiEvent = APP_UI_CONN_CLOSE;
        break;

    case DM_SEC_PAIR_CMPL_IND:
        DmSecGenerateEccKeyReq();
        uiEvent = APP_UI_SEC_PAIR_CMPL;
        break;

    case DM_SEC_PAIR_FAIL_IND:
        DmSecGenerateEccKeyReq();
        uiEvent = APP_UI_SEC_PAIR_FAIL;
        break;

    case DM_SEC_ENCRYPT_IND:
        uiEvent = APP_UI_SEC_ENCRYPT;
        break;

    case DM_SEC_ENCRYPT_FAIL_IND:
        uiEvent = APP_UI_SEC_ENCRYPT_FAIL;
        break;

    case DM_SEC_AUTH_REQ_IND:
        AppHandlePasskey(&pMsg->dm.authReq);
        break;

    case DM_SEC_ECC_KEY_IND:
        DmSecSetEccKey(&pMsg->dm.eccMsg.data.key);
        break;

    case DM_SEC_COMPARE_IND:
        AppHandleNumericComparison(&pMsg->dm.cnfInd);
        break;

    case DM_PRIV_CLEAR_RES_LIST_IND:
        APP_TRACE_INFO1("Clear resolving list status 0x%02x", pMsg->hdr.status);
        break;

    case DM_HW_ERROR_IND:
        uiEvent = APP_UI_HW_ERROR;
        break;

    default:
        break;
    }

    if (uiEvent != APP_UI_NONE) {
        AppUiAction(uiEvent);
    }
}

/*************************************************************************************************/
/*!
 *  \brief     Platform button press handler.
 *
 *  \param[in] btnId  button ID.
 *  \param[in] state  button state. See ::PalBtnPos_t.
 *
 *  \return    None.
 */
/*************************************************************************************************/
static void btnPressHandler(uint8_t btnId, PalBtnPos_t state)
{
    if (btnId == 1) {
        /* Start/stop button timer */
        if (state == PAL_BTN_POS_UP) {
            /* Button Up, stop the timer, call the action function */
            unsigned btnUs = MXC_TMR_SW_Stop(BTN_1_TMR);
            if ((btnUs > 0) && (btnUs < BTN_SHORT_MS * 1000)) {
                AppUiBtnTest(APP_UI_BTN_1_SHORT);
            } else if (btnUs < BTN_MED_MS * 1000) {
                AppUiBtnTest(APP_UI_BTN_1_MED);
            } else if (btnUs < BTN_LONG_MS * 1000) {
                AppUiBtnTest(APP_UI_BTN_1_LONG);
            } else {
                AppUiBtnTest(APP_UI_BTN_1_EX_LONG);
            }
        } else {
            /* Button down, start the timer */
            MXC_TMR_SW_Start(BTN_1_TMR);
        }
    } else if (btnId == 2) {
        /* Start/stop button timer */
        if (state == PAL_BTN_POS_UP) {
            /* Button Up, stop the timer, call the action function */
            unsigned btnUs = MXC_TMR_SW_Stop(BTN_2_TMR);
            if ((btnUs > 0) && (btnUs < BTN_SHORT_MS * 1000)) {
                AppUiBtnTest(APP_UI_BTN_2_SHORT);
            } else if (btnUs < BTN_MED_MS * 1000) {
                AppUiBtnTest(APP_UI_BTN_2_MED);
            } else if (btnUs < BTN_LONG_MS * 1000) {
                AppUiBtnTest(APP_UI_BTN_2_LONG);
            } else {
                AppUiBtnTest(APP_UI_BTN_2_EX_LONG);
            }
        } else {
            /* Button down, start the timer */
            MXC_TMR_SW_Start(BTN_2_TMR);
        }
    } else {
        APP_TRACE_ERR0("Undefined button");
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Application handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID.
 *
 *  \return None.
 */
/*************************************************************************************************/
void MagpieHandlerInit(wsfHandlerId_t handlerId)
{
    uint8_t addr[6] = { 0 };
    APP_TRACE_INFO0("MagpieHandlerInit");
    AppGetBdAddr(addr);
    APP_TRACE_INFO6("MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x", addr[5], addr[4], addr[3], addr[2],
                    addr[1], addr[0]);
    APP_TRACE_INFO1("Adv local name: %s", &magpieScanDataDisc[2]);

    /* store handler ID */
    magpieHandlerId = handlerId;

    /* Set configuration pointers */
    pAppAdvCfg = (appAdvCfg_t *)&magpieAdvCfg;
    pAppSlaveCfg = (appSlaveCfg_t *)&magpieSlaveCfg;
    pAppSecCfg = (appSecCfg_t *)&magpieSecCfg;
    pAppUpdateCfg = (appUpdateCfg_t *)&magpieUpdateCfg;

    /* Initialize application framework */
    AppSlaveInit();
    AppServerInit();

    /* Set stack configuration pointers */
    pSmpCfg = (smpCfg_t *)&magpieSmpCfg;

    /* initialize heart rate profile sensor */
//    HrpsInit(handlerId, (hrpsCfg_t *)&magpieHrpsCfg);
//    HrpsSetFlags(magpieHrmFlags);

    /* initialize battery service server */
    BasInit(handlerId, (basCfg_t *)&magpieBasCfg);
}

/*************************************************************************************************/
/*!
 *  \brief  WSF event handler for application.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void MagpieHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
    if (pMsg != NULL) {
        APP_TRACE_INFO1("Magpie got evt %d", pMsg->event);

        /* process ATT messages */
        if (pMsg->event >= ATT_CBACK_START && pMsg->event <= ATT_CBACK_END) {
            /* process server-related ATT messages */
            AppServerProcAttMsg(pMsg);
        } else if (pMsg->event >= DM_CBACK_START && pMsg->event <= DM_CBACK_END) {
            /* process DM messages */
            /* process advertising and connection-related messages */
            AppSlaveProcDmMsg((dmEvt_t *)pMsg);

            /* process security-related messages */
            AppSlaveSecProcDmMsg((dmEvt_t *)pMsg);
        }

        /* perform profile and user interface-related operations */
        magpieProcMsg((magpieMsg_t *)pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Start the application.
 *
 *  \return None.
 */
/*************************************************************************************************/
void MagpieStart(void)
{
    /* Register for stack callbacks */
    DmRegister(magpieDmCback);
    DmConnRegister(DM_CLIENT_ID_APP, magpieDmCback);
    AttRegister(magpieAttCback);
    AttConnRegister(AppServerConnCback);
    AttsCccRegister(MAGPIE_NUM_CCC_IDX, (attsCccSet_t *)magpieCccSet, magpieCccCback);

    /* Register for app framework callbacks */
    AppUiBtnRegister(magpieBtnCback);
    /* Initialize with button press handler */
    PalBtnInit(btnPressHandler);
    /* Initialize attribute server database */
    SvcCoreGattCbackRegister(GattReadCback, GattWriteCback);
    SvcCoreAddGroup();
//    SvcHrsCbackRegister(NULL, HrpsWriteCback);
//    SvcHrsAddGroup();
    SvcDisAddGroup();
    SvcBattCbackRegister(BasReadCback, NULL);
    SvcBattAddGroup();
    //SvcRscsAddGroup();

    /* Set Service Changed CCCD index. */
    GattSetSvcChangedIdx(MAGPIE_GATT_SC_CCC_IDX);

    /* Set running speed and cadence features */
    //RscpsSetFeatures(RSCS_ALL_FEATURES);

    /* Reset the device */
    DmDevReset();
}

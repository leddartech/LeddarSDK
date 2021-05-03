////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	shared/comm/LtComLeddarEngine.h
///
/// \brief	Defines for LeddarEngine protocol
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "LtComEthernetPublic.h"

namespace LtComLeddarTechPublic
{

    enum eLtDistance
    {
        LT_COMM_LE_NO_DATA      = 0xFFFFFFFA, // Data were not available to make a detection.
        LT_COMM_LE_NO_DETECTION = 0xFFFFFFF5  //  No detection value
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum	eLtComLeddarEngineSystemState
    ///
    /// \brief	LeddarEngine system state values
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum eLtComLeddarEngineSystemState
    {
        LT_COMM_LE_SYSSTATE_STANDBY = 1, //!< LeddarEngine ready to be used.
        LT_COMM_LE_SYSSTATE_NORMAL,      //!< LeddarEngine is used to acquire data.
        LT_COMM_LE_SYSSTATE_SHUTDOWN     //!< LeddarEngine is shutting down.
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum	eLtComLeddarEngineTriggerMode
    ///
    /// \brief	LeddarEngine trigger mode values
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum eLtComLeddarEngineTriggerMode
    {
        LT_COMM_LE_TRIGGERMODE_EXTSOFT = 1,
        LT_COMM_LE_TRIGGERMODE_EXTHARD,
        LT_COMM_LE_TRIGGERMODE_INTERNAL
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eLtCommCfgSrvLeddarEngineRequestCodes
    ///
    /// \brief  Configuration server LeddarEngine request codes.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef enum eLtCommCfgSrvLeddarEngineRequestCodes
    {
        LT_COMM_CFGSRV_REQUEST_LE_INVALID = LT_COMM_CFGSRV_REQUEST_PLATFORM_SPECIFIC_BASE, ///< (0x1000) Configuration server start base of platform specific request
        LT_COMM_CFGSRV_REQUEST_LE_START_ACQUISITION,                                       ///< (0x1001) Start acquisition
        LT_COMM_CFGSRV_REQUEST_LE_STOP_ACQUISITION,                                        ///< (0x1002) Stop acquisition
        LT_COMM_CFGSRV_REQUEST_LE_EXTERNAL_SOFT_TRIG                                       ///< (0x1003) Software trig for a single acquisition
    } eLtCommCfgSrvLeddarEngineRequestCodes;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eLtCommLeddarEngineConstantsElementsIdCodes
    ///
    /// \brief  Communication IDs
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef enum eLtCommLeddarEngineConstantsElementsIdCodes
    {
        LT_COM_ID_LE_INVALID                = LT_COMM_ID_PLATFORM_SPECIFIC_BASE_V2, ///< (0x2000) Product specific element ids starting base
        LT_COMM_ID_LE_FRM_QTY               = 0x2001,                               ///< (0x2001) {uint16_t} Frame config: Frame config quantity
        LT_COMM_ID_LE_FRM_OPTICAL_TILES_QTY = 0x2002,                               ///< (0x2002) {uint32_t[]} Frame config: Optical tiles quantity
        LT_COMM_ID_LE_FRM_VSEG              = 0x2003,                               ///< (0x2003) {uint32_t[]} Frame config: Vertical segment size
        LT_COMM_ID_LE_FRM_HSEG              = 0x2004,                               ///< (0x2004) {uint32_t[]} Frame config: Horizontal segment size
        LT_COMM_ID_LE_FRM_FRAME_SIZE        = 0x2005,                               ///< (0x2005) {uint32_t[]} Frame config: Frame size
        LT_COMM_ID_LE_FRM_FRAME_RATE        = 0x2006,                               ///< (0x2006) {uint32_t[]} Frame config: Frame rate

        LT_COMM_ID_LE_OPT_TOTAL_QTY             = 0x2010, ///< (0x2010) {uint32_t} Optical tile config: Optical tile config total size
        LT_COMM_ID_LE_OPT_FRAME_CFG_INDEX       = 0x2011, ///< (0x2011) {uint32_t[]} Optical tile config: Frame config index
        LT_COMM_ID_LE_OPT_OPTICAL_TILE_INDEX    = 0x2012, ///< (0x2012) {uint32_t[]} Optical tile config: Optical tile index
        LT_COMM_ID_LE_OPT_ACQUISITION_TILES_QTY = 0x2013, ///< (0x2013) {uint32_t[]} Optical tile config: Acquisition tiles quantity
        LT_COMM_ID_LE_OPT_VSEG                  = 0x2014, ///< (0x2014) {uint32_t[]} Optical tile config: Vertical segment size
        LT_COMM_ID_LE_OPT_HSEG                  = 0x2015, ///< (0x2015) {uint32_t[]} Optical tile config: Horizontal segment size
        LT_COMM_ID_LE_OPT_TILE_SEQ_NUMBER       = 0x2016, ///< (0x2016) {uint32_t[]} Optical tile config: Optical tile sequence number
        LT_COMM_ID_LE_OPT_DBSD_TRANSITION_TIME  = 0x2017, ///< (0x2017) {uint32_t[]} Optical tile config: DBSD transition time
        LT_COMM_ID_LE_OPT_DBSD_POSITION         = 0x2018, ///< (0x2018) {uint32_t[]} Optical tile config: DBSD position

        LT_COMM_ID_LE_ACQ_TOTAL_QTY              = 0x2040, ///< (0x2040) {uint32_t} Acquisition tile config: Acquisition tile total size
        LT_COMM_ID_LE_ACQ_FRAME_CFG_INDEX        = 0x2041, ///< (0x2041) {uint32_t[]} Acquisition tile config: Frame config index
        LT_COMM_ID_LE_ACQ_OPTICAL_TILE_INDEX     = 0x2042, ///< (0x2042) {uint32_t[]} Acquisition tile config: Optical tile index
        LT_COMM_ID_LE_ACQ_ACQUISITION_TILE_INDEX = 0x2043, ///< (0x2043) {uint32_t[]} Acquisition tile config: Acquisition tile index
        LT_COMM_ID_LE_ACQ_SEGMENTS_QTY           = 0x2044, ///< (0x2044) {uint32_t[]} Acquisition tile config: Segments quantity
        LT_COMM_ID_LE_ACQ_VSEG                   = 0x2045, ///< (0x2045) {uint32_t[]} Acquisition tile config: Vertical segments size
        LT_COMM_ID_LE_ACQ_HSEG                   = 0x2046, ///< (0x2046) {uint32_t[]} Acquisition tile config: Horizontal segments size
        LT_COMM_ID_LE_ACQ_TILE_SEQ_NUMBER        = 0x2047, ///< (0x2047) {uint32_t[]} Acquisition tile config: Tile sequence number
        LT_COMM_ID_LE_ACQ_ACCUMULATIONS          = 0x2048, ///< (0x2048) {uint32_t[]} Acquisition tile config: Number of accumulations
        LT_COMM_ID_LE_ACQ_OVERSAMPLING           = 0x2049, ///< (0x2049) {uint32_t[]} Acquisition tile config: Number of oversampling
        LT_COMM_ID_LE_ACQ_BASE_POINTS            = 0x204A, ///< (0x204A) {uint32_t[]} Acquisition tile config: Number of base points
        LT_COMM_ID_LE_ACQ_LASERS_ENABLED = 0x204B, ///< (0x204B) {uint32_t[]} Acquisition tile config: Laser enabled, bits 0-15 correspond to the 16 lasers that may be enabled

        LT_COMM_ID_LE_SYS_ACQ_VSEG_MAX              = 0x2100, ///< (0x2100) {uint32_t} Maximum number of vertical segments
        LT_COMM_ID_LE_SYS_ACQ_HSEG_MAX              = 0x2101, ///< (0x2101) {uint32_t} Maximum number of horizontal segments
        LT_COMM_ID_LE_SYS_ACQ_MAX_DETECTION_PER_SEG = 0x2102, ///< (0x2102) {uint32_t} Maximum number of detections per segment
        LT_COMM_ID_LE_SYS_ACQ_TRIGGER_MODE          = 0x2103, ///< (0x2103) {uint32_t} Trigger mode
        LT_COMM_ID_LE_SYS_ACQ_BASE_POINT_STEP = 0x2104 ///< (0x2104) {uint32_t} Distance in meters between two base points. IMPORTANT: Divide by oversampling to obtain sample point
                                                       ///< step. Fixed point value type (Q16.16)

    } eLtCommLeddarEngineConstantsElementsIdCodes;

    typedef enum eLtCommLeddarEngineConfigElementsIdCodes
    {
        LT_COMM_ID_LE_FRAME_CFG_INDEX          = 0x2500, ///< (0x2500) Current frame config selected
        LT_COMM_ID_LE_WAVEFORM_ROI             = 0x2501, ///< (0x2501) Current monitored optical tile
        LT_COMM_ID_LE_UDP_RX_IP                = 0x2502, ///< (0x2502) UDP receiver IP
        LT_COMM_ID_LE_UDP_RX_DETECTIONS_PORT   = 0x2503, ///< (0x2503) UDP receiver detections port
        LT_COMM_ID_LE_UDP_RX_RAW_WF_PORT       = 0x2504, ///< (0x2504) UDP receiver raw waveforms port
        LT_COMM_ID_LE_UDP_RX_PROCESSED_WF_PORT = 0x2505  ///< (0x2505) UDP receiver processed waveforms port
    } eLtCommLeddarEngineConfigElementsIdCodes;
} // namespace LtComLeddarTechPublic
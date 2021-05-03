////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   shared/comm/Legacy/M16/LtComM16.h
///
/// \brief  Structures and defines for M16 sensor communication.
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "comm/LtComEthernetPublic.h"

namespace LtComM16
{

    //*****************************************************************************
    //*************** Constants and Macros ****************************************
    //*****************************************************************************
#define M16_MAX_ECHOES                      96       ///< Maximum number of echoes.
#define M16_LED_INTENSITY_MAX               16       ///< Number of led intensity.
#define M16_NUMBER_CHANNELS                 16       ///< Number of Leddar channels.
#define M16_MAX_ECHOES_BY_CHANNEL           6        ///< Maximum number of echoes by channel

#define M16_CFG_REQUEST_COMPUTE(_offset)        (LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_PLATFORM_SPECIFIC_BASE + _offset)
#define M16_ID_COMPUTE(_offset)                 (LtComLeddarTechPublic::LT_COMM_ID_PLATFORM_SPECIFIC_BASE + _offset)
#define IS16_ID_COMPUTE(_offset)                (LtComLeddarTechPublic::LT_COMM_ID_PRODUCT_SPECIFIC_BASE + _offset)

#define M16_FILTERED_SCALE                  8192

    const int8_t SMOOTHINGLIMITS[2]  = {-17, 16};
    const uint8_t M16_ZONESDET_NB_NODES_MAX = 64; //Maximum number of expression nodes used for zones detector.


    //*****************************************************************************
    //*************** Data Type Definition ****************************************
    //*****************************************************************************

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eLtCommPlatformM16ElementIdCodes
    ///
    /// \brief  Element Id definitions: (beware some IDs in this range are defined at the LtCom level.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef enum eLtCommPlatformM16ElementIdCodes
    {
        /// \brief  M16 platform specific elements ID codes.
        M16_ID_CFG_THRESHOLD_TABLE_OFFSET   = M16_ID_COMPUTE( 0x0000 ),   ///< (0x1000) {LtFixedPoint} Threshold table offset in M16_FILTERED_SCALE.
        M16_ID_CAL_CHAN_TIMEBASE_DELAY      = M16_ID_COMPUTE( 0x0014 ),   ///< (0x1014) {LtFixedPoint}[number of channels] Calibration time base delay of each segments.
        M16_ID_CFG_TRANS_IMP_GAIN           = M16_ID_COMPUTE( 0x007B ),   ///< (0x107B) {LtUInt8} FPGA parameter: transimpedance gain from M16_DEFAULT_TRANS_IMP_GAIN_MIN to M16_DEFAULT_TRANS_IMP_GAIN_MAX.
        M16_ID_CFG_LED_INTENSITY            = M16_ID_COMPUTE( 0x0080 ),   ///< (0x1080) {LtUInt8} Led intensity from M16_DEFAULT_LED_INTENSITY_MIN to M16_DEFAULT_LED_INTENSITY_MAX.
        M16_ID_CFG_SENSOR_POSITION_X        = M16_ID_COMPUTE( 0x0090 ),   ///< (0x1090) {LtFloat32} Sensor X position.
        M16_ID_CFG_SENSOR_POSITION_Y        = M16_ID_COMPUTE( 0x0091 ),   ///< (0x1091) {LtFloat32} Sensor Y position.
        M16_ID_CFG_SENSOR_POSITION_Z        = M16_ID_COMPUTE( 0x0092 ),   ///< (0x1092) {LtFloat32} Sensor Z position.
        M16_ID_CFG_SENSOR_ORIENTATION_YAW   = M16_ID_COMPUTE( 0x0093 ),   ///< (0x1093) {LtFloat32} Sensor yaw orientation.
        M16_ID_CFG_SENSOR_ORIENTATION_PITCH = M16_ID_COMPUTE( 0x0094 ),   ///< (0x1094) {LtFloat32} Sensor pitch orientation.
        M16_ID_CFG_SENSOR_ORIENTATION_ROLL  = M16_ID_COMPUTE( 0x0095 ),   ///< (0x1095) {LtFloat32} Sensor roll orientation.
        M16_ID_CFG_SERIAL_PORT_BAUDRATE     = M16_ID_COMPUTE( 0x0096 ),   ///< (0x1096) {LtUInt32}[Number of serial port] Modbus Baudrate value.
        M16_ID_CFG_SERIAL_PORT_DATA_BITS    = M16_ID_COMPUTE( 0x0097 ),   ///< (0x1097) {LtUInt8}[Number of serial port] Number of bits per data (8 or 9).
        M16_ID_CFG_SERIAL_PORT_PARITY       = M16_ID_COMPUTE( 0x0098 ),   ///< (0x1098) {LtUInt8}[Number of serial port] Parity.
        M16_ID_CFG_SERIAL_PORT_STOP_BITS    = M16_ID_COMPUTE( 0x0099 ),   ///< (0x1099) {LtUInt8}[Number of serial port] Stop bits count.
        M16_ID_CFG_SERIAL_PORT_ADDRESS      = M16_ID_COMPUTE( 0x009A ),   ///< (0x109A) {LtUInt8}[Number of serial port] ModBus address (1 to 247).
        M16_ID_CFG_SERIAL_PORT_FLOW_CONTROL = M16_ID_COMPUTE( 0x009B ),   ///< (0x109B) {LtUInt8}[Number of serial port] Flow control mode. 0 disabled, 1 enabled
        M16_ID_CFG_CAN_PORT_BAUDRATE        = M16_ID_COMPUTE( 0x009C ),   ///< (0x109C) {LtUInt32}[Number of CAN port] CANBus Baudrate value.
        M16_ID_CFG_CAN_PORT_TX_MSG_BASE_ID  = M16_ID_COMPUTE( 0x009D ),   ///< (0x109D) {LtUInt32}[Number of CAN port] Tx message base ID.
        M16_ID_CFG_CAN_PORT_RX_MSG_BASE_ID  = M16_ID_COMPUTE( 0x009E ),   ///< (0x109E) {LtUInt32}[Number of CAN port] Rx message base ID.
        M16_ID_CFG_CAN_PORT_FRAME_FORMAT    = M16_ID_COMPUTE( 0x009F ),   ///< (0x109F) {LtUInt8}[Number of CAN port] Tx Rx frame format. 0 standard, 1 extended

        M16_ID_CFG_ZONESDET_NB_VALID_NODES              = M16_ID_COMPUTE( 0x00A0 ), ///< (0x10A0) {LtUInt8} Number of valid zones detector expression node. Must be <= M16_ZONESDET_NB_NODES_MAX.
        M16_ID_CFG_ZONESDET_OPTIONS                     = M16_ID_COMPUTE( 0x00A1 ), ///< (0x10A1) {LtUInt8} Zones detector bits field options. See \ref eLtCommM16ZonesDetectorOptions.
        M16_ID_CFG_ZONESDET_CMP_VALUE                   = M16_ID_COMPUTE( 0x00A2 ), ///< (0x10A2) {LtUInt32}[M16_ZONESDET_NB_NODES_MAX] Value to compare.
        M16_ID_CFG_ZONESDET_OPERATOR                    = M16_ID_COMPUTE( 0x00A3 ), ///< (0x10A3) {LtUInt16}[M16_ZONESDET_NB_NODES_MAX] Operator. See \ref eLtCommM16OperatorDefinitions.
        M16_ID_CFG_ZONESDET_OPERAND1                    = M16_ID_COMPUTE( 0x00A4 ), ///< (0x10A4) {LtUInt8}[M16_ZONESDET_NB_NODES_MAX] First operand:  cond = start segment index, logic = index of expression operator to get result.
        M16_ID_CFG_ZONESDET_OPERAND2                    = M16_ID_COMPUTE( 0x00A5 ), ///< (0x10A5) {LtUInt8}[M16_ZONESDET_NB_NODES_MAX] Second operand: cond = stop segment index,  logic = index of expression operator to get result.
        M16_ID_DISCRETE_OUTPUTS                         = M16_ID_COMPUTE( 0x00A6 ), ///< (0x10A6) {LtUInt32} Bits fields discrete outputs state.
        M16_ID_CFG_DISCRETE_OUTPUTS_RISING_DEBOUNCE     = M16_ID_COMPUTE( 0x00A7 ), ///< (0x10A7) {LtUInt8}[number of supported output] Rising debouncing value in number of samples (from deasserted to asserted).
        M16_ID_CFG_DISCRETE_OUTPUTS_FALLING_DEBOUNCE    = M16_ID_COMPUTE( 0x00A8 ), ///< (0x10A8) {LtUInt8}[number of supported output] Falling debouncing value in number of samples (from asserted to deasserted).


        M16_ID_CFG_ACQ_OPTIONS                          = M16_ID_COMPUTE( 0x00A9 ),   ///< (0x10A9) {LtUInt16} Bits field of acquisition options. See \ref eLtCommPlatformM16AcqOptions.
        M16_ID_CFG_AUTO_ACQ_AVG_FRM                     = M16_ID_COMPUTE( 0x00AA ),   ///< (0x10AA) {LtUInt16} Number of frame to evaluate new automatic acquisition parameters from M16_DEFAULT_AUTO_AVG_FRM_MIN to M16_DEFAULT_AUTO_AVG_FRM_MAX.
        M16_ID_ACQ_CURRENT_PARAMS                       = M16_ID_COMPUTE( 0x00AB ),   ///< (0x10AB) {LtUInt32} Current acquisition parameters states. See \ref eLtCommPlateformGalaxyAcqCurrentParams
        M16_ID_MEASUREMENT_RATE                         = M16_ID_COMPUTE( 0x00AC ),   ///< (0x10AC) {LtFixedPoint} Measurement rate in M16_MEASUREMENT_RATE_SCALE.
        M16_ID_MEASUREMENT_RATE_LIST                    = M16_ID_COMPUTE( 0x00AD ),   ///< (0x10AD) {LtFixedPoint}[] List of supported measurement rate in M16_MEASUREMENT_RATE_SCALE.
        M16_ID_BEAM_RANGE                               = M16_ID_COMPUTE( 0x00AE ),   ///< (0x10AE) {LtFixedPoint} Beam range in M16_DISTANCE_SCALE for displaying accommodation.
        M16_ID_LED_POWER                                = M16_ID_COMPUTE( 0x00AF ),   ///< (0x10AF) {LtUInt8} Led power in percent.
        M16_ID_LED_POWER_LIST                           = M16_ID_COMPUTE( 0x00B0 ),   ///< (0x10B0) {LtUInt8}[] List of supported led power in percent.
        M16_ID_LIMIT_CFG_THRESHOLD_TABLE_OFFSET         = M16_ID_COMPUTE( 0x00B1 ),   ///< (0x10B1) {LtFixedPoint}[2] Minimum and maximum threshold table offset permitted to M16_ID_CFG_THRESHOLD_TABLE_OFFSET
        ///<                            from the current configuration. Call \ref M16_CFGSRV_REQUEST_PARAMS_TO_THRES_TABLE_OFFSET_MIN
        ///<                            to get the minimum value with a different configuration.
        M16_ID_LIMIT_CFG_LED_INTENSITY                  = M16_ID_COMPUTE( 0x00B2 ),   ///< (0x10B2) {LtUInt8}[2] Minimum and maximum led intensity permitted to M16_ID_CFG_LED_INTENSITY.
        M16_ID_LIMIT_CFG_AUTO_ACQ_AVG_FRM               = M16_ID_COMPUTE( 0x00B3 ),   ///< (0x10B3) {LtUInt16}[2] Minimum and maximum number of auto acq avg frame permitted to M16_ID_CFG_AUTO_ACQ_AVG_FRM.
        M16_ID_CFG_SERIAL_PORT_MAX_ECHOES               = M16_ID_COMPUTE( 0x00B4 ),   ///< (0x10B4) {LtUInt8}[Number of serial port] Maximum number of echoes to send on the serial port.
        M16_ID_LIMIT_CFG_SERIAL_PORT_MAX_ECHOES         = M16_ID_COMPUTE( 0x00B5 ),   ///< (0x10B5) {LtUInt8} Maximum number of echoes permitted to M16_ID_CFG_SERIAL_PORT_MAX_ECHOES.
        M16_ID_DEFAULT_CFG_AUTO_ACQ_AVG_FRM             = M16_ID_COMPUTE( 0x00B6 ),   ///< (0x10B6) {LtUInt16} Default value of auto acq avg frame to set to M16_ID_CFG_AUTO_ACQ_AVG_FRM.
        M16_ID_CAL_LED_INTENSITY                        = M16_ID_COMPUTE( 0x00B7 ),   ///< (0x10B7) {LtFixedPoint}[17] Led intensity distance compensation
        M16_ID_CFG_BAYES_PRECISION                      = M16_ID_COMPUTE( 0x00B8 ),   ///< (0x10B8) {LtInt8} Bayes detector precision adjustment
        M16_ID_ACQUISITION_OPTION_MASK                  = M16_ID_COMPUTE( 0x00B9 ),   ///< (0x10B9) {LtUInt16} Acquisition option mask
        M16_ID_TRACE_TYPE                               = M16_ID_COMPUTE( 0x00BA ),   ///< (0x10BA) {LtUInt8} Select the trace type to send
        M16_ID_PREDICTED_TEMP                           = M16_ID_COMPUTE( 0x00BB ),   ///< (0x10BB) {LtInt32} Predicted temperature
        M16_ID_CFG_CAN_PORT_OPTIONS                     = M16_ID_COMPUTE( 0x00BC ),   ///< (0x10BC) {LtUInt16}[Number of CAN port] CAN port flag options: see \ref eLtCommPlatformM16CanOptions.
        M16_ID_CFG_CAN_PORT_MAILBOX_DELAY               = M16_ID_COMPUTE( 0x00BD ),   ///< (0x10BD) {LtUInt16}[Number of CAN port] Inter-mailbox(es) message delay in msec.
        M16_ID_CFG_CAN_PORT_ACQCYCLE_DELAY              = M16_ID_COMPUTE( 0x00BE ),   ///< (0x10BE) {LtUInt16}[Number of CAN port] Inter-cycle message block delay in msec.
        M16_ID_CFG_CAN_PORT_MAX_ECHOES                  = M16_ID_COMPUTE( 0x00BF ),   ///< (0x10BF) {LtUInt8}[Number of CAN port] Maximum number of echoes to send on the serial port.
        M16_ID_LIMIT_CFG_CAN_PORT_MAX_ECHOES            = M16_ID_COMPUTE( 0x00C0 ),   ///< (0x10C0) {LtUInt8} Maximum number of echoes permitted to M16_ID_CFG_CAN_PORT_MAX_ECHOES.
        M16_ID_CFG_LWECHOES_DIST_RES                    = M16_ID_COMPUTE( 0x00C1 ),   ///< (0x10C1) {LtUInt16} Lightweight echoes distance resolution.
        M16_ID_CFG_LWECHOES_CHANNEL_SELECT              = M16_ID_COMPUTE( 0x00C2 ),   ///< (0x10C2) {LtUInt16} Bits field of selected channel to send by lightweight echoes module.
        M16_ID_CAN_PORT_OPTIONS_MASK                    = M16_ID_COMPUTE( 0x00C3 ),   ///< (0x10C3) {LtUInt16} CAN port flag option mask of available options. see \ref eLtCommPlatformM16CanOptions
        M16_ID_SERIAL_PORT_BAUDRATE_OPTIONS_MASK        = M16_ID_COMPUTE( 0x00C4 ),   ///< (0x10C4) {LtUInt16}[Number of serial port] Bits field of supported baudrate. see \ref eLtCommPlatformM16SerialBaudrateOptionMask
        M16_ID_TEST_MODE                                = M16_ID_COMPUTE( 0x00C5 ),   ///< (0x10C5) {LtUInt16} Test mode.
        M16_ID_NB_CYCLE_PER_SCAN                        = M16_ID_COMPUTE( 0x00C6 ),   ///< (0x10C6) {LtUInt16} Number of cycle per scan in clock cycle.
        M16_ID_PWM_PERIOD                               = M16_ID_COMPUTE( 0x00C7 ),   ///< (0x10C7) {LtUInt16} PWM period in clock cycle.
        M16_ID_PWM_TABLE                                = M16_ID_COMPUTE( 0x00C8 ),   ///< (0x10C8) {LtUInt8}[16] PWM charge pulse in clock cycle.
        M16_ID_CFG_START_TRACE_INDEX                    = M16_ID_COMPUTE( 0x00C9 ),   ///< (0x10C9) {LtUInt32} Index where the peak detector begin.
        M16_ID_LIMIT_START_TRACE_INDEX                  = M16_ID_COMPUTE( 0x00CA ),   ///< (0x10CA) {LtUInt32}[2] Minimum and maximum index where the peak detector start to compute a pulse.
        M16_ID_DATA_LEVEL                               = M16_ID_COMPUTE( 0x00CB ),   ///< (0x10CB) {LtUInt32} See LT_COMM_DATA_LEVEL_...
        M16_ID_STATIC_THRESHOLD_DISTANCES               = M16_ID_COMPUTE( 0x00CC ),   ///< (0x00CC) {LtFixedPoint}[8] Distances for static threshold
        M16_ID_STATIC_THRESHOLD_AMPLITUDES              = M16_ID_COMPUTE( 0x00CD ),   ///< (0x00CD) {LtFixedPoint}[8] Amplitudes for static threshold
    } eLtCommPlatformM16ElementIdCodes;

    typedef enum eLtComPlatformIS16CfgSrvRequestCode
    {
        IS16_CFGSRV_REQUEST_TEACH = 0x8000, ///< (0x8000) Config by teach mode: need IS16_ID_LVLS_TEACH_STATE, IS16_ID_LVLS_CONFIG_ZONE and IS16_ID_LVLS_TEACH_FRAME
        IS16_CFGSRV_REQUEST_QUICK_MODE = 0x8001, ///< (0x8001) Config by quick mode: need IS16_ID_LVLS_QUICK_LIMIT_MODE, IS16_ID_LVLS_CONFIG_ZONE,
                                                 ///< IS16_ID_LVLS_QUICK_FAR_LIMIT and IS16_ID_LVLS_QUICK_NEAR_LIMIT

    } eLtComPlatformIS16CfgSrvRequestCode;

    typedef enum eLtCommPlatformIS16ElementIdCodes
    {
        IS16_ID_CFG_DISCRETE_OUTPUTS_RISING_DEBOUNCE    = IS16_ID_COMPUTE( 0x0000 ),    ///< (0x8000) {uint16_t}[IS16_DISCRETE_OUTPUTS_MAX] Rising debouncing value in number of samples (from deasserted to asserted).
        IS16_ID_CFG_DISCRETE_OUTPUTS_FALLING_DEBOUNCE   = IS16_ID_COMPUTE( 0x0001 ),    ///< (0x8001) {uint16_t}[IS16_DISCRETE_OUTPUTS_MAX] Falling debouncing value in number of samples (from asserted to deasserted).
        IS16_ID_CFG_DISCRETE_OUTPUTS_NPN_PNP            = IS16_ID_COMPUTE( 0x0002 ),    ///< (0x8002) {uint8_t} Bits field of electrical outputs configuration: 0=NPN, 1=PNP.
        IS16_ID_CFG_DISCRETE_OUTPUTS_INV                = IS16_ID_COMPUTE( 0x0003 ),    ///< (0x8003) {uint8_t} Bits field of inverted outputs configuration: 0=normal, 1=inverted.
        IS16_ID_CFG_LVLS_FAR_LIMIT                      = IS16_ID_COMPUTE( 0x0004 ),    ///< (0x8004) {LtFixedPoint}[IS16_ZONES_MAX][PGALAXY_NUMBER_CHANNELS] For advanced mode: Far distance limit per segment and per supported zone.
        IS16_ID_CFG_LVLS_NEAR_LIMIT                     = IS16_ID_COMPUTE( 0x0005 ),    ///< (0x8005) {LtFixedPoint}[IS16_ZONES_MAX][PGALAXY_NUMBER_CHANNELS] For advanced mode: Near distance limit per segment and per supported zone.
        IS16_ID_CFG_LVLS_SEGMENTS_ENABLE                = IS16_ID_COMPUTE( 0x0006 ),    ///< (0x8006) {uint16_t}[IS16_ZONES_MAX] For advanced mode: Bits field of enabled segment per zone.
        IS16_ID_CFG_LVLS_DETECT_ALGO_TYPE               = IS16_ID_COMPUTE( 0x0007 ),    ///< (0x8007) {uint8_t}[IS16_ZONES_MAX] Algorithm detection type per zone. See \ref eLtCommIS16DectectionAlgoType.
        IS16_ID_CFG_LVLS_ZONES_ENABLE                   = IS16_ID_COMPUTE( 0x0008 ),    ///< (0x8008) {uint8_t} Bits field of enabled zone.

        IS16_ID_LVLS_TEACH_STATE = IS16_ID_COMPUTE( 0x0009 ),   ///< (0x8009) {LtUInt8} Teach state: Beware, on LT_COMM_CFGSRV_REQUEST_GET, this element return an array with size of [IS16_ZONES_MAX].
        IS16_ID_LVLS_TEACH_FRAME = IS16_ID_COMPUTE( 0x000A ), ///< (0x800A) {LtUInt16} Number of frame to teach scene with sensor.
        //IS16_ID_LVLS_QUICK_LIMIT_MODE = IS16_ID_COMPUTE(0x000B),   /** (0x800B) {LtUInt8} Limit mode: see _eLtCommIS16QuickLimitMode. */
        //IS16_ID_LVLS_QUICK_FAR_LIMIT = IS16_ID_COMPUTE(0x000C),   /** (0x800C) {LtFixedPoint} Far distance limit for quick config. */
        //IS16_ID_LVLS_QUICK_NEAR_LIMIT = IS16_ID_COMPUTE(0x000D),   /** (0x800D) {LtFixedPoint} Near distance limit for quick config. */
        IS16_ID_CFG_LVLS_LAST_CONFIG_MODE  = IS16_ID_COMPUTE( 0x000E ), ///< (0x800E) {uint8_t}[IS16_ZONES_MAX] The last configuration method done: see \ref eLtCommIS16ConfigMode.
        IS16_ID_LVLS_CONFIG_ZONE           = IS16_ID_COMPUTE( 0x000F ), ///< (0x800F) {LtUInt8} The zone to configure in quick or teach mode.
        IS16_ID_CFG_LVLS_SECURITY_DISTANCE = IS16_ID_COMPUTE( 0x0010 ), ///< (0x8010) {LtFixedPoint} Security distance to add or remove from teach limit.
        IS16_ID_CFG_LCD_CONTRAST           = IS16_ID_COMPUTE( 0x0011 ), ///< (0x8011) {uint8_t} LCD contrast percent.
        IS16_ID_CFG_LCD_BACKLIGHT_BRIGHTNESS = IS16_ID_COMPUTE( 0x0012 ), ///< (0x8012) {uint8_t} LCD backlight brightness percent.
        IS16_ID_CFG_CONTROL_PANEL_ACCESS     = IS16_ID_COMPUTE( 0x0013 ), ///< (0x8013) {uint8_t} Control panel access. Locked on 1

    } eLtCommPlatformIS16ElementIdCodes;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eLtCommPlatformM16CfgSrvRequestCode
    ///
    /// \brief  Config server request code.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef enum eLtCommPlatformM16CfgSrvRequestCode
    {
        /// \brief  M16 platform specific configuration server request codes.
        M16_CFGSRV_REQUEST_MESUREMENT_RATE_TO_PARAMS = M16_CFG_REQUEST_COMPUTE( 0x0000 ),           ///< (0x1000) Convert measurement rate to accumulation and oversampling parameters.
        M16_CFGSRV_REQUEST_PARAMS_TO_MESUREMENT_RATE = M16_CFG_REQUEST_COMPUTE( 0x0001 ),           ///< (0x1001) Convert accumulation and oversampling parameters to measurement rate.
        M16_CFGSRV_REQUEST_LED_POWER_TO_PARAMS = M16_CFG_REQUEST_COMPUTE( 0x0002 ),                 ///< (0x1002) Convert led power to led intensity parameters.
        M16_CFGSRV_REQUEST_PARAMS_TO_LED_POWER = M16_CFG_REQUEST_COMPUTE( 0x0003 ),                 ///< (0x1003) Convert led intensity parameters to led power.
        M16_CFGSRV_REQUEST_PARAMS_TO_THRES_TABLE_OFFSET_MIN = M16_CFG_REQUEST_COMPUTE( 0x0004 ),    ///< (0x1004) Convert accumulation and oversampling parameters to the minimum permitted of table offset threshold.
        M16_CFGSRV_REQUEST_BASE_SAMPLE_COUNT_TO_BEAM_RANGE = M16_CFG_REQUEST_COMPUTE( 0x0005 ),     ///< (0x1005) Convert base sample count to beam range.
        M16_CFGSRV_REQUEST_DEFAULT_STATIC_THRESHOLD_TABLE = M16_CFG_REQUEST_COMPUTE( 0x0006 ),      ///< (0x1006) Get the default stat. threshold table from the device.
    } eLtCommPlatformM16CfgSrvRequestCode;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eLtCommDeviceOptions
    ///
    /// \brief  Mask for device options.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef enum eLtCommDeviceOptions
    {
        /// \brief  Device option bit field definitions with mask and specific option definitions.
        LT_COMM_DEVICE_OPTION_NONE = 0x00000000,   ///< Nothing option selected.
        LT_COMM_DEVICE_OPTION_ALL = 0xFFFFFFFF,   ///< All option selected.

        // ****************************
        // * Bit field definitions    *
        // * M16 families             *
        // ****************************
        LT_COMM_DEVICE_OPTION_LFOV_0 = 0x00000001,   ///< Leddar sensor field of view, see LT_COMM_DEVICE_OPTION_xx_DEG_LFOV.
        LT_COMM_DEVICE_OPTION_DEMO_KIT = 0x00000002,   ///< Demo unit: 0=normal, 1=demo kit
        LT_COMM_DEVICE_OPTION_PIN = 0x00000004,   ///< Sensor type: 0=APD, 1=PIN.
        LT_COMM_DEVICE_OPTION_NOPANTILT = 0x00000008,   ///< Pan & tilt option: 0=activated, 1=deactivated. See LT_COMM_DEVICE_OPTION_NOPANTILT_xx.
        LT_COMM_DEVICE_OPTION_LFOV_1 = 0x00000010,   ///< Leddar sensor field of view, see LT_COMM_DEVICE_OPTION_xx_DEG_LFOV.
        LT_COMM_DEVICE_OPTION_LFOV_2 = 0x00000020,   ///< Leddar sensor field of view, see LT_COMM_DEVICE_OPTION_xx_DEG_LFOV.
        LT_COMM_DEVICE_OPTION_TRIGGER = 0x00000040,   ///< Trigger option: 0=none, 1=present. See LT_COMM_DEVICE_OPTION_TRIGGER_xx.
        LT_COMM_DEVICE_OPTION_CFOV_0 = 0x00000080,   ///< Camera field of view, see LT_COMM_DEVICE_OPTION_xx_DEG_CFOV.
        LT_COMM_DEVICE_OPTION_CFOV_1 = 0x00000100,   ///< Camera field of view, see LT_COMM_DEVICE_OPTION_xx_DEG_CFOV.
        LT_COMM_DEVICE_OPTION_SENSOR_ORIENTATION = 0x00000200,   ///< Sensor orientation: see LT_COMM_DEVICE_OPTION_SENSOR_ORIENTATION_xx.
        LT_COMM_DEVICE_OPTION_GEN2 = 0x00000400,   ///< Product generation: see LT_COMM_DEVICE_OPTION_GEN_xx.
        LT_COMM_DEVICE_OPTION_CFOV_2 = 0x00000800,   ///< Camera field of view, see LT_COMM_DEVICE_OPTION_xx_DEG_CFOV.
        LT_COMM_DEVICE_OPTION_GHO_0 = 0x00001000,   ///< M16 hardware option field, see LT_COMM_DEVICE_OPTION_GHO_xx.
        LT_COMM_DEVICE_OPTION_GHO_1 = 0x00002000,   ///< M16 hardware option field, see LT_COMM_DEVICE_OPTION_GHO_xx.
        LT_COMM_DEVICE_OPTION_GHO_2 = 0x00004000,   ///< M16 hardware option field, see LT_COMM_DEVICE_OPTION_GHO_xx.
        LT_COMM_DEVICE_OPTION_GHO_3 = 0x00008000,   ///< M16 hardware option field, see LT_COMM_DEVICE_OPTION_GHO_xx.
        LT_COMM_DEVICE_OPTION_GHO_4 = 0x00010000,   ///< M16 hardware option field, see LT_COMM_DEVICE_OPTION_GHO_xx.


        // *******************************
        // * Mask and option definitions *
        // * M16 families                *
        // *******************************
        // - Leddar sensor optic field of view mask and option definitions.
        LT_COMM_DEVICE_OPTION_LFOV_MASK = LT_COMM_DEVICE_OPTION_LFOV_0 | LT_COMM_DEVICE_OPTION_LFOV_1 | LT_COMM_DEVICE_OPTION_LFOV_2,   ///< Leddar sensor optic mask.
        LT_COMM_DEVICE_OPTION_18_DEG_LFOV = LT_COMM_DEVICE_OPTION_NONE,                                   ///< 18 degrees Leddar sensor optic.
        LT_COMM_DEVICE_OPTION_34_DEG_LFOV = LT_COMM_DEVICE_OPTION_LFOV_0,                                 ///< 34 degrees Leddar sensor optic.
        LT_COMM_DEVICE_OPTION_26_DEG_LFOV = LT_COMM_DEVICE_OPTION_LFOV_1,                                 ///< 26 degrees Leddar sensor optic.
        LT_COMM_DEVICE_OPTION_60_DEG_LFOV = LT_COMM_DEVICE_OPTION_LFOV_0 | LT_COMM_DEVICE_OPTION_LFOV_1,  ///< 60 degrees Leddar sensor optic.
        LT_COMM_DEVICE_OPTION_45_DEG_LFOV = LT_COMM_DEVICE_OPTION_LFOV_2,                                 ///< 45 degrees Leddar sensor optic.
        LT_COMM_DEVICE_OPTION_10_DEG_LFOV = LT_COMM_DEVICE_OPTION_LFOV_2 | LT_COMM_DEVICE_OPTION_LFOV_0,  ///< 10 degrees Leddar sensor optic.
        LT_COMM_DEVICE_OPTION_100_DEG_LFOV = LT_COMM_DEVICE_OPTION_LFOV_2 | LT_COMM_DEVICE_OPTION_LFOV_1,  ///< 100 degrees Leddar sensor optic (currently available on M16 platform).

        // - Camera optic field of view mask and option definitions.
        LT_COMM_DEVICE_OPTION_CFOV_MASK = LT_COMM_DEVICE_OPTION_CFOV_0 | LT_COMM_DEVICE_OPTION_CFOV_1 | LT_COMM_DEVICE_OPTION_CFOV_2,   ///< Camera optic mask.
        LT_COMM_DEVICE_OPTION_37_DEG_CFOV = LT_COMM_DEVICE_OPTION_NONE,                                   ///< 37 degrees camera optic.
        LT_COMM_DEVICE_OPTION_58_DEG_CFOV = LT_COMM_DEVICE_OPTION_CFOV_0,                                 ///< 58 degrees camera optic.
        LT_COMM_DEVICE_OPTION_17_DEG_CFOV = LT_COMM_DEVICE_OPTION_CFOV_1,                                 ///< 17 degrees camera optic.
        LT_COMM_DEVICE_OPTION_28_DEG_CFOV = LT_COMM_DEVICE_OPTION_CFOV_0 | LT_COMM_DEVICE_OPTION_CFOV_1,  ///< 28 degrees camera optic.
        LT_COMM_DEVICE_OPTION_27_DEG_CFOV = LT_COMM_DEVICE_OPTION_CFOV_2,                                 ///< 27 degrees camera optic.

        // - Pan and tilt mask and option definitions.
        LT_COMM_DEVICE_OPTION_NOPANTILT_MASK = LT_COMM_DEVICE_OPTION_NOPANTILT,                              ///< Pan and tilt mask.
        LT_COMM_DEVICE_OPTION_NOPANTILT_PRESENT = LT_COMM_DEVICE_OPTION_NONE,                                   ///< Pan and tilt present and activated.
        LT_COMM_DEVICE_OPTION_NOPANTILT_NOT_PRESENT = LT_COMM_DEVICE_OPTION_NOPANTILT,                              ///< Pan and tilt no present or not activated.

        // - Physical trigger mask and option definitions.
        LT_COMM_DEVICE_OPTION_TRIGGER_MASK = LT_COMM_DEVICE_OPTION_TRIGGER,                                ///< Physical trigger mask.
        LT_COMM_DEVICE_OPTION_TRIGGER_NOT_PRESENT = LT_COMM_DEVICE_OPTION_NONE,                                   ///< No physical trigger.
        LT_COMM_DEVICE_OPTION_TRIGGER_PRESENT = LT_COMM_DEVICE_OPTION_TRIGGER,                                ///< Physical trigger present.

        // - Sensor orientation mask and option definitions.
        LT_COMM_DEVICE_OPTION_SENSOR_ORIENTATION_MASK = LT_COMM_DEVICE_OPTION_SENSOR_ORIENTATION,                     ///< Sensor orientation mask.
        LT_COMM_DEVICE_OPTION_SENSOR_ORIENTATION_HORIZ = LT_COMM_DEVICE_OPTION_NONE,                                   ///< Horizontal sensing (Ex. dtec).
        LT_COMM_DEVICE_OPTION_SENSOR_ORIENTATION_VERT = LT_COMM_DEVICE_OPTION_SENSOR_ORIENTATION,                     ///< Vertical sensing (Ex. side-tec).

        // - Device product generation mask and option definitions.
        LT_COMM_DEVICE_OPTION_GEN_MASK = LT_COMM_DEVICE_OPTION_GEN2,                                        ///< Product generation mask.
        LT_COMM_DEVICE_OPTION_GEN_FIRST = LT_COMM_DEVICE_OPTION_NONE,                                       ///< First product generation.
        LT_COMM_DEVICE_OPTION_GEN_SECOND = LT_COMM_DEVICE_OPTION_GEN2,                                      ///< Second product generation.

        // - M16 product hardware mask and option definitions.
        LT_COMM_DEVICE_OPTION_GHO_MASK = LT_COMM_DEVICE_OPTION_GHO_0 | LT_COMM_DEVICE_OPTION_GHO_1 | LT_COMM_DEVICE_OPTION_GHO_2 | LT_COMM_DEVICE_OPTION_GHO_3 | LT_COMM_DEVICE_OPTION_GHO_4,   ///< M16 hardware option mask.
        LT_COMM_DEVICE_OPTION_GHO_NO_DISPLAY = LT_COMM_DEVICE_OPTION_GHO_0,                                 ///< IS16 without display option.
        LT_COMM_DEVICE_OPTION_GHO_CAN = LT_COMM_DEVICE_OPTION_GHO_1,                                        ///< IS16 CAN port option (serial port disabled).
        LT_COMM_DEVICE_OPTION_GHO_RAW_DATA = LT_COMM_DEVICE_OPTION_GHO_2,                                   ///< IS16 with raw data output only.
        LT_COMM_DEVICE_OPTION_GHO_LASER_PRODUCT = LT_COMM_DEVICE_OPTION_GHO_3,                              ///< M16 product contain LASER source.
        LT_COMM_DEVICE_OPTION_GHO_FAILSAFE = LT_COMM_DEVICE_OPTION_GHO_4                                    ///< M16 product contain a fail safe option for test mode.

    } eLtCommDeviceOptions;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eLtCommPlatformM16AcqOptions
    ///
    /// \brief  Mask for acquisition options
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef enum eLtCommPlatformM16AcqOptions
    {
        /// \brief  Bits field acquisition options saved in configuration.
        M16_ACQ_OPTIONS_NONE = 0x0000,           ///< No acquisition options selected.

        M16_ACQ_OPTIONS_AUTO_LED_INTENSITY       = 0x0001, ///< Automatic led intensity.
        M16_ACQ_OPTIONS_AUTO_TRANSIMP_GAIN       = 0x0002, ///< Automatic transimpedance gain. This is available only if automatic led intensity is enabled.
        M16_ACQ_OPTIONS_DEMERGE_OBJECTS          = 0x0004, ///< Enable the two object demerge algorithm.
        M16_ACQ_OPTIONS_XTALK_REMOVAL_DISABLE    = 0x0008, ///< DISABLE xtalk removal algorithm (logical inverse : 1 = disabled).
        M16_ACQ_OPTIONS_TEMP_COMP_DISABLE        = 0x0010, ///< DISABLE temperature compensation (logical inverse : 1 = disabled).
        M16_ACQ_OPTIONS_XTALK_ECHO_REMOVAL       = 0x0020, ///< Enable the xtalk echo removal algorithm (located in peak detector).
        M16_ACQ_OPTIONS_OVERSHOOT                = 0x0040, ///< Select the overshoot method. 0=Standard, 1=Specialised (located in peak detector).
        M16_ACQ_OPTIONS_SATURATION               = 0x0080, ///< Enable the saturation compensation algorithm (located in peak detector).
        M16_ACQ_OPTIONS_AUTO_LED_INTENSITY2      = 0x0100, ///< Automatic led intensity v2 enabled.
        M16_ACQ_OPTIONS_STATIC_THRESHOLD_DISABLE = 0x0200, ///< DISABLE the Static detection threshold table usage (logical inverse : 1 = disabled).
        M16_ACQ_OPTIONS_INTERFERENCE             = 0x0400, ///< Enable the interference algorithm.

        M16_ACQ_OPTIONS_MASK                     = 0x07FF, ///< Supported option mask.
    } eLtCommPlatformM16AcqOptions;

    typedef enum eLtCommPlatformM16SerialBaudrateOptionMask
    {
        /// \brief  Serial of supported port baudrate.
        M16_SERIAL_PORT_OPTION_BAUDRATE_9600 = 0x0001,      ///< 9600 bps option is available
        M16_SERIAL_PORT_OPTION_BAUDRATE_19200 = 0x0002,     ///< 19200 bps option is available
        M16_SERIAL_PORT_OPTION_BAUDRATE_38400 = 0x0004,     ///< 38400 bps option is available
        M16_SERIAL_PORT_OPTION_BAUDRATE_57600 = 0x0008,     ///< 57600 bps option is available
        M16_SERIAL_PORT_OPTION_BAUDRATE_115200 = 0x0010,    ///< 115200 bps option is available
        M16_SERIAL_PORT_OPTION_BAUDRATE_230400 = 0x0020,    ///< 230400 bps option is available
        M16_SERIAL_PORT_OPTION_BAUDRATE_460800 = 0x0040,    ///< 460800 bps option is available
        M16_SERIAL_PORT_OPTION_BAUDRATE_921600 = 0x0080,    ///< 921600 bps option is available

        M16_SERIAL_PORT_OPTION_BAUDRATE_STANDARD = 0x001F
    } eLtCommPlatformM16SerialBaudrateOptionMask;

    typedef enum eLtCommPlatformM16SerialFlowControl
    {
        /// \brief  Serial port flow control enumeration.
        M16_SERIAL_PORT_CONTROL_NONE = 0,            ///< No flow control.
        M16_SERIAL_PORT_CONTROL_CTS_RTS = 1             ///< CTS-RTS flow control.
    } eLtCommPlatformM16SerialFlowControl;

    typedef enum eLtCommPlatformM16CanOptions
    {
        /// \brief  Bits field of CAN options.
        M16_CAN_PORT_OPTIONS_NONE = 0x0000,                 ///< No CAN option selected.

        M16_CAN_PORT_OPTIONS_MULT_MSG_ID_FORMAT = 0x0001,   ///< Send detection on multiple message ID.
        M16_CAN_PORT_OPTIONS_MSGBOX_DELAY = 0x0002,         ///< Delay in msec between a message sent in a message box(es).
        M16_CAN_PORT_OPTIONS_ACQCYCLE_DELAY = 0x0004,       ///< Delay in msec between cycle of acquisition message frame.
        M16_CAN_PORT_OPTIONS_FLAG_INFO = 0x0008,            ///< Use detection message with flag information.

        M16_CAN_PORT_OPTIONS_MASK = 0x000F                  ///< Supported option mask.
    } eLtCommPlatformM16CanOptions;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eLtCommIS16DectectionAlgoType
    ///
    /// \brief  Detection algo type definitions.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef enum eLtCommIS16DectectionAlgoType
    {
        IS16_ALGO_TYPE_BOOL_RAW = 3,       ///< Simple boolean mode. True (1) when something inside limits, else false (0). */
        IS16_ALGO_TYPE_BOOL_COUNTING = 5   ///< Boolean mode with frame counting. */
    } eLtCommIS16DectectionAlgoType;

    typedef enum eLtCommIS16ConfigMode
    {
        IS16_CONFIG_TEACH = 0,      ///< Last config done in teach mode.
        IS16_CONFIG_QUICK = 1,      ///< Last config done in quick mode: near and far distances are the same for all segments.
        IS16_CONFIG_ADVANCED = 2,   ///< Last config done in advanced mode: near and far distances can be configured for all segments, and segments can be disabled.
    } eLtCommIS16ConfigMode;

    /// \enum   eLtCommM16ZonesDetectorOptions
    ///
    /// \brief  Zones detector bits field options.
    typedef enum eLtCommM16ZonesDetectorOptions
    {
        M16_ZONESDET_OPTIONS_ENABLE             = 0x01,     ///< Enable the zones detector option.
        M16_ZONESDET_OPTIONS_RS485_TRIGGER      = 0x02,     ///< Enable the RS485 trigger. This disable the corresponding UART.
    } eLtCommM16ZonesDetectorOptions;


    /// \enum   eLtCommM16OperatorDefinitions
    ///
    /// \brief  Operator definitions for expression node used for zones detector.
    typedef enum eLtCommM16OperatorDefinitions
    {
        EVALKIT_OPERATOR_TYPE_MASK                  = 0xC000,   /** Mask to get operator type. */
        EVALKIT_OPERATOR_TYPE_CONDITIONAL           = 0x0000,   /** Conditional operator type. */
        EVALKIT_OPERATOR_TYPE_LOGIC                 = 0x4000,   /** Logic operator type. */

        EVALKIT_OPERATOR_OPTIONS_MASK               = 0x3F00,   /** Mask to get bit field operator options */
        EVALKIT_OPERATOR_OPTIONS_COND_ALL_SEGMENTS  = 0x0100,   /** Option used for conditional operator: all segments must encounter condition instead of "at least one". */

        EVALKIT_OPERATOR_VALUE_TYPE_MASK            = 0x00F0,   /** Mask to get value type in case of conditional operator. */
        EVALKIT_OPERATOR_VALUE_TYPE_DISTANCE        = 0x0000,   /** Distance value type. */
        EVALKIT_OPERATOR_VALUE_TYPE_AMPLITUDE       = 0x0010,   /** Amplitude value type. */

        EVALKIT_OPERATOR_MASK                       = 0x000F,   /** Mask to get the operator. */
        /* conditional operator list */
        EVALKIT_OPERATOR_COND_GREATER               = 0x0000,   /** "Greater than" conditional operator (>). */
        EVALKIT_OPERATOR_COND_GREATER_EQUAL         = 0x0001,   /** "Greater than or equal" conditional operator (>=). */
        EVALKIT_OPERATOR_COND_SMALLER               = 0x0002,   /** "Smaller than" conditional operator (<). */
        EVALKIT_OPERATOR_COND_SMALLER_EQUAL         = 0x0003,   /** "Smaller than or equal" conditional operator (<=). */
        EVALKIT_OPERATOR_COND_EQUAL                 = 0x0004,   /** "Equal" conditional operator (=). */
        EVALKIT_OPERATOR_COND_UNEQUAL               = 0x0005,   /** "Unequal" conditional operator (!=). */
        /* logic operator list */
        EVALKIT_OPERATOR_LOGIC_AND                  = 0x0000,   /** "AND" logic operator (&&). */
        EVALKIT_OPERATOR_LOGIC_OR                   = 0x0001,   /** "OR" logic operator (||). */
        EVALKIT_OPERATOR_LOGIC_XOR                  = 0x0002,   /** Exclusive "OR" logic operator (^). */
        EVALKIT_OPERATOR_LOGIC_NAND                 = 0x0003,   /** Inverted "AND" logic operator !(&&). */
        EVALKIT_OPERATOR_LOGIC_NOR                  = 0x0004,   /** Inverted "OR" logic operator !(||). */
        EVALKIT_OPERATOR_LOGIC_XNOR                 = 0x0005,   /** Inverted exclusive "OR" logic operator !(^). */
        EVALKIT_OPERATOR_LOGIC_NOT                  = 0x0006,   /** Inversion logic operator (!). */
    } eLtCommM16OperatorDefinitions;

    /// \brief Current acquisition parameters transmitted on state messages.
    typedef enum eLtCommPlateformGalaxyAcqCurrentParams
    {
        PGALAXY_ACQ_CURRENT_LED_INTENSITY_MASK  = 0x000000FF, /** Automatic led intensity mask. */
        PGALAXY_ACQ_CURRENT_TRANS_IMP_GAIN_MASK = 0x00000100, /** Automatic transimpedance gain mask (0=disable, 1=enable). */
        PGALAXY_ACQ_CURRENT_DEMERGE_OBJECT_MASK = 0x00000200, /** Demerge object status mask (1=all demerge, 0=otherwise). */
        PGALAXY_ACQ_CURRENT_TEST_MODE_MASK      = 0x00000400, /** Test mode (fail safe) status mask (1=test mode ON, 0=test mode OFF). */
        PGALAXY_ACQ_CURRENT_FLAGS_MASK = PGALAXY_ACQ_CURRENT_TRANS_IMP_GAIN_MASK | PGALAXY_ACQ_CURRENT_DEMERGE_OBJECT_MASK | PGALAXY_ACQ_CURRENT_TEST_MODE_MASK, /** Flag mask. */
    } eLtCommPlateformGalaxyAcqCurrentParams;

    /// \brief States of the teach command
    typedef enum eLtCommIS16TeachState : uint8_t
    {
        IS16_TEACH_STATE_START    = 0, ///< Start teaching mode.
        IS16_TEACH_STATE_CANCEL   = 1, ///< Cancel teaching mode.
        IS16_TEACH_STATE_TEACHING = 2, ///< Teaching state.
        IS16_TEACH_STATE_STOPPED  = 3, ///< Teaching stopped or successfully finished.
        IS16_TEACH_STATE_FAILED   = 4, ///< Last teaching finished with a failure or canceled by user.
    } eLtCommIS16TeachState;
}


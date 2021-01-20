////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorM16.cpp
///
/// \brief  Implements the LdSensorM16 class
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdSensorM16.h"
#if defined(BUILD_M16) && defined(BUILD_USB)

#include "LdPropertyIds.h"
#include "LdBitFieldProperty.h"
#include "LdBoolProperty.h"
#include "LdEnumProperty.h"
#include "LdFloatProperty.h"
#include "LdIntegerProperty.h"
#include "LdTextProperty.h"

#include "LtExceptions.h"
#include "LtStringUtils.h"
#include "LtTimeUtils.h"

#include "comm/LtComLeddarTechPublic.h"
#include "comm/Legacy/M16/LtComM16.h"
#include "comm/LtComUSBPublic.h"
#include "comm/Modbus/LtComModbus.h"

#include <cstring>

using namespace LeddarCore;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarDevice::LdSensorM16::LdSensorM16( LeddarConnection::LdConnection *aConnection )
///
/// \brief  Constructor
///
/// \param [in,out] aConnection If non-null, the connection.
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDevice::LdSensorM16::LdSensorM16( LeddarConnection::LdConnection *aConnection ):
    LdSensor( aConnection ),
    mProtocolConfig( nullptr ),
    mProtocolData( nullptr )
{
    if( aConnection )
    {
        mProtocolConfig = dynamic_cast<LeddarConnection::LdProtocolLeddartechUSB *>( aConnection );
        mProtocolData = new LeddarConnection::LdProtocolLeddartechUSB( mProtocolConfig->GetConnectionInfo(), mProtocolConfig, LeddarConnection::LdProtocolLeddartechUSB::EP_DATA );
    }

    mResultStatePropeties = GetResultStates()->GetProperties();
    InitProperties();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarDevice::LdSensorM16::~LdSensorM16( void )
///
/// \brief  Destructor
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDevice::LdSensorM16::~LdSensorM16( void )
{
    if( mProtocolData != nullptr )
    {
        delete mProtocolData;
        mProtocolData = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::InitProperties( void )
///
/// \brief  Initializes the properties for this sensor
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::InitProperties( void )
{
    //Constants
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FPGA_VERSION, 0, 2, "FPGA version" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FIRMWARE_VERSION_INT, 0, 4, "Firmware version" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_SERIAL_NUMBER, LtComLeddarTechPublic::LT_COMM_ID_SERIAL_NUMBER,
                              LT_COMM_SERIAL_NUMBER_LENGTH, LdTextProperty::TYPE_ASCII, "Serial Number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_SOFTWARE_PART_NUMBER, 0, LT_COMM_PART_NUMBER_LENGTH,
                              LdTextProperty::TYPE_ASCII, "Software part number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_PART_NUMBER, LtComLeddarTechPublic::LT_COMM_ID_HW_PART_NUMBER,
                              LT_COMM_PART_NUMBER_LENGTH, LdTextProperty::TYPE_ASCII, "Hardware part number" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_CRC32, 0, 4, "Firmware checksum" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_NONE, LdPropertyIds::ID_ACCUMULATION_LIMITS,
                              LtComLeddarTechPublic::LT_COMM_ID_LIMIT_CFG_ACCUMULATION_EXPONENT, 4, "Accumulation exponent limits" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_NONE, LdPropertyIds::ID_OVERSAMPLING_LIMITS,
                              LtComLeddarTechPublic::LT_COMM_ID_LIMIT_CFG_OVERSAMPLING_EXPONENT, 4, "Oversampling exponent limits" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_NONE, LdPropertyIds::ID_BASE_POINT_COUNT_LIMITS,
                              LtComLeddarTechPublic::LT_COMM_ID_LIMIT_CFG_BASE_SAMPLE_COUNT, 4, "Limits of base point count" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_OPTIONS,
                              LtComLeddarTechPublic::LT_COMM_ID_DEVICE_OPTIONS, 4, "Device options" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_NONE, LdPropertyIds::ID_CHANGE_DELAY_LIMITS,
                              LtComM16::M16_ID_LIMIT_CFG_AUTO_ACQ_AVG_FRM, 2, "Change delay (in frame) limits" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_DISTANCE_SCALE,
                              LtComLeddarTechPublic::LT_COMM_ID_DISTANCE_SCALE, 4, "Distance scaling between received value and distance in meter" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FILTERED_AMP_SCALE,
                              LtComLeddarTechPublic::LT_COMM_ID_FILTERED_SCALE, 4, "Amplitude scaling" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_DETECTION_LENGTH,
                              LtComM16::M16_ID_BEAM_RANGE, 4, 0, 1, "Theoretical maximum range" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_NONE, LdPropertyIds::ID_SENSIVITY_LIMITS,
                              LtComM16::M16_ID_LIMIT_CFG_THRESHOLD_TABLE_OFFSET, 4, 0, 1, "Threshold offset limits" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE_OPTIONS,
                              LtComM16::M16_ID_SERIAL_PORT_BAUDRATE_OPTIONS_MASK, 2, "Modbus available baud rates - 2 Values, one for each serial port. See \\ref eLtCommPlatformM16SerialBaudrateOptionMask" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE,
                              LdPropertyIds::ID_COM_CAN_PORT_OPTIONS_MASK, LtComM16::M16_ID_CAN_PORT_OPTIONS_MASK, 2, "CAN port options mask availability, see \\ref eLtCommPlatformM16CanOptions" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_REAL_DISTANCE_OFFSET,
                              LtComLeddarTechPublic::LT_COMM_ID_REAL_DIST_OFFSET, 4, 65536, 2, "Distance between trace start and actual 0" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_TRACE_POINT_STEP,
                              LtComLeddarTechPublic::LT_COMM_ID_TRACE_POINT_STEP, 4, 0, 3, "Distance between two points in the trace (ID_BASE_SAMPLE_DISTANCE*oversampling)" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_BASE_SAMPLE_DISTANCE,
                              LtComLeddarTechPublic::LT_COMM_ID_BASE_SAMPLE_DISTANCE, 4, 0, 3, "Distance between two base points" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_REFRESH_RATE,
                              LtComLeddarTechPublic::LT_COMM_ID_REFRESH_RATE, 4, 0, 2, "Theoretical refresh rate" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL, 0, 1, "Max Detection per Segment" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_ACQUISITION_OPTION_MASK,
                              LtComM16::M16_ID_ACQUISITION_OPTION_MASK, 2, "Mask of available bits of acquisition options" ) );


    //Config
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_DEVICE_NAME,
                              LtComLeddarTechPublic::LT_COMM_ID_DEVICE_NAME, LT_COMM_DEVICE_NAME_LENGTH, LdTextProperty::TYPE_UTF16, "Device name" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ACCUMULATION_EXP,
                              LtComLeddarTechPublic::LT_COMM_ID_CFG_ACCUMULATION_EXPONENT, 4, "Accumulation exponent" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_OVERSAMPLING_EXP,
                              LtComLeddarTechPublic::LT_COMM_ID_CFG_OVERSAMPLING_EXPONENT, 4, "Oversampling exponent" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_BASE_POINT_COUNT,
                              LtComLeddarTechPublic::LT_COMM_ID_CFG_BASE_SAMPLE_COUNT, 4, "Base point count, impact max detection distance" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_PRECISION,
                              LtComM16::M16_ID_CFG_BAYES_PRECISION, 1, "Smoothing", true ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_LED_INTENSITY,
                              LtComM16::M16_ID_CFG_LED_INTENSITY, 1, true, "Led power %, stored as index. Use GetStringValue and SetStringValue for easier use" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE, LdPropertyIds::ID_CHANGE_DELAY,
                              LtComM16::M16_ID_CFG_AUTO_ACQ_AVG_FRM, 2, "Change delay (in frame) for automatic led power" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ACQ_OPTIONS,
                              LtComM16::M16_ID_CFG_ACQ_OPTIONS, 2, "Bit field of acquisition options see \\ref eLtCommPlatformM16AcqOptions. Available bits defined in ID_ACQUISITION_OPTION_MASK" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_SEGMENT_ENABLE,
                              LtComLeddarTechPublic::LT_COMM_ID_DISABLED_CHANNELS, 4, "Enable / disable selected channels pair on the device (enable = 0)" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_GAIN_ENABLE,
                              LtComM16::M16_ID_CFG_TRANS_IMP_GAIN, "Enable transimpedance gain (internal use)" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_SENSIVITY_OLD,
                              LtComM16::M16_ID_CFG_THRESHOLD_TABLE_OFFSET, 4, 1000, 2, "Threshold offset" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_DISTANCE_RESOLUTION,
                              LtComM16::M16_ID_CFG_LWECHOES_DIST_RES, 2, true, "Distance resolution" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ORIGIN_X,
                              LtComM16::M16_ID_CFG_SENSOR_POSITION_X, 4, 2, 1, "Position of the sensor (X)" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ORIGIN_Y,
                              LtComM16::M16_ID_CFG_SENSOR_POSITION_Y, 4, 2, 1, "Position of the sensor (Y)" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ORIGIN_Z,
                              LtComM16::M16_ID_CFG_SENSOR_POSITION_Z, 4, 2, 1, "Position of the sensor (Z)" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_YAW,
                              LtComM16::M16_ID_CFG_SENSOR_ORIENTATION_YAW, 4, 0, 1, "Position of the sensor (Yaw)" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_PITCH,
                              LtComM16::M16_ID_CFG_SENSOR_ORIENTATION_PITCH, 4, 0, 1, "Position of the sensor (Pitch)" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ROLL,
                              LtComM16::M16_ID_CFG_SENSOR_ORIENTATION_ROLL, 4, 0, 1, "Position of the sensor (Roll)" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_STATIC_THRESHOLD_DISTANCES,
                              LtComM16::M16_ID_STATIC_THRESHOLD_DISTANCES, 4, 0, 2, "Static threshold distances" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_STATIC_THRESHOLD_AMPLITUDES,
                              LtComM16::M16_ID_STATIC_THRESHOLD_AMPLITUDES, 4, 0, 3, "Static threshold amplitudes" ) );

    //Config - detections zones
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CONDITION_COUNT,
                              LtComM16::M16_ID_CFG_ZONESDET_NB_VALID_NODES, 1, "Number of valid zones detector expression node. Must be <= EVALKIT_ZONESDET_NB_NODES_MAX." ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CONDITION_OPTIONS,
                              LtComM16::M16_ID_CFG_ZONESDET_OPTIONS, 1, "Zones detector bits field options. See \\ref eLtCommM16ZonesDetectorOptions." ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CONDITION_VALUE,
                              LtComM16::M16_ID_CFG_ZONESDET_CMP_VALUE, 4, "Value to compare" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CONDITION_OPERATION,
                              LtComM16::M16_ID_CFG_ZONESDET_OPERATOR, 2, "Operator. See \\ref eLtCommM16OperatorDefinitions" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CONDITION_INDEX1,
                              LtComM16::M16_ID_CFG_ZONESDET_OPERAND1, 1, "First operand:  cond = start segment index, logic = index of expression operator to get result." ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CONDITION_INDEX2,
                              LtComM16::M16_ID_CFG_ZONESDET_OPERAND2, 1, "Second operand: cond = stop segment index,  logic = index of expression operator to get result." ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CONDITION_RISING_DB,
                              LtComM16::M16_ID_CFG_DISCRETE_OUTPUTS_RISING_DEBOUNCE, 1, "Rising debouncing value in number of samples (from deasserted to asserted)." ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CONDITION_FALLING_DB,
                              LtComM16::M16_ID_CFG_DISCRETE_OUTPUTS_FALLING_DEBOUNCE, 1, "Falling debouncing value in number of samples (from asserted to deasserted)." ) );

    //Config - serial port
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE,
                              LtComM16::M16_ID_CFG_SERIAL_PORT_BAUDRATE, 4, true, "Modbus baudrate - Check availability with ID_COM_SERIAL_PORT_BAUDRATE_OPTIONS property" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_DATA_BITS,
                              LtComM16::M16_ID_CFG_SERIAL_PORT_DATA_BITS, 1, true, "Modbus data bits" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_PARITY,
                              LtComM16::M16_ID_CFG_SERIAL_PORT_PARITY, 1, true, "Modbus parity" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS,
                              LtComM16::M16_ID_CFG_SERIAL_PORT_STOP_BITS, 1, true, "Modbus stop bit" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS,
                              LtComM16::M16_ID_CFG_SERIAL_PORT_ADDRESS, 1, "Modbus address" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_MAX_ECHOES,
                              LtComM16::M16_ID_CFG_SERIAL_PORT_MAX_ECHOES, 1, "Modbus maximum detections returned by command 0x41" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_FLOW_CONTROL,
                              LtComM16::M16_ID_CFG_SERIAL_PORT_FLOW_CONTROL, 1, "Modbus flow control" ) );

    //Config - CANbus
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE,
                              LtComM16::M16_ID_CFG_CAN_PORT_BAUDRATE, 4, true,  "CAN port baudrate" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID,
                              LtComM16::M16_ID_CFG_CAN_PORT_TX_MSG_BASE_ID, 4, "CAN port transmission message base id" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID,
                              LtComM16::M16_ID_CFG_CAN_PORT_RX_MSG_BASE_ID, 4, "CAN port reception message base id" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT,
                              LtComM16::M16_ID_CFG_CAN_PORT_FRAME_FORMAT, "Frame format - false = standard" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_PORT_OPTIONS,
                              LtComM16::M16_ID_CFG_CAN_PORT_OPTIONS, 2, "CAN port options - See available option with property ID_COM_CAN_PORT_OPTIONS_MASK" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE,
                              LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY, LtComM16::M16_ID_CFG_CAN_PORT_MAILBOX_DELAY, 2, "CAN port mailbox delay" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE,
                              LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY, LtComM16::M16_ID_CFG_CAN_PORT_ACQCYCLE_DELAY, 2, "CAN Port acquisition delay" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE,
                              LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES, LtComM16::M16_ID_CFG_CAN_PORT_MAX_ECHOES, 1, "CAN port max echoes" ) );

    //License
    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_OTHER, LdProperty::F_EDITABLE, LdPropertyIds::ID_LICENSE,
                              LtComLeddarTechPublic::LT_COMM_ID_LICENSE, LT_COMM_LICENSE_KEY_LENGTH, "License key" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_OTHER, LdProperty::F_NONE, LdPropertyIds::ID_LICENSE_INFO,
                              LtComLeddarTechPublic::LT_COMM_ID_LICENSE_INFO, 4, "License type / subtype" ) );
    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_OTHER, LdProperty::F_EDITABLE, LdPropertyIds::ID_VOLATILE_LICENSE,
                              LtComLeddarTechPublic::LT_COMM_ID_VOLATILE_LICENSE, LT_COMM_LICENSE_KEY_LENGTH, "Temporary license key - internal use" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_OTHER, LdProperty::F_NONE, LdPropertyIds::ID_VOLATILE_LICENSE_INFO,
                              LtComLeddarTechPublic::LT_COMM_ID_VOLATILE_LICENSE_INFO, 4, "Volatile license type / subtype - internal use" ) );

    //Calib
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_TIMEBASE_DELAY,
                              LtComM16::M16_ID_CAL_CHAN_TIMEBASE_DELAY, 4, 65536, 2, "Timebase delay - Require integrator license to change" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_INTENSITY_COMPENSATIONS,
                              LtComM16::M16_ID_CAL_LED_INTENSITY, 4, 65536, 2, "Led power compensations - Require integrator license to change" ) );

    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS )->SetLimits( 1, MODBUS_MAX_ADDR );

    LdEnumProperty *lDistRes = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_RESOLUTION );
    lDistRes->AddEnumPair( 1000, "millimeter" );
    lDistRes->AddEnumPair( 100, "centimeter" );
    lDistRes->AddEnumPair( 10, "decimeter" );
    lDistRes->AddEnumPair( 1, "meter" );

    LdEnumProperty *lModbusBaudRates = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE );
    lModbusBaudRates->AddEnumPair( 9600, "9600" );
    lModbusBaudRates->AddEnumPair( 19200, "19200" );
    lModbusBaudRates->AddEnumPair( 38400, "38400" );
    lModbusBaudRates->AddEnumPair( 57600, "57600" );
    lModbusBaudRates->AddEnumPair( 115200, "115200" );
    lModbusBaudRates->AddEnumPair( 230400, "230400" );
    lModbusBaudRates->AddEnumPair( 460800, "460800" );
    lModbusBaudRates->AddEnumPair( 921600, "921600" );

    LdEnumProperty *lCANBaudRates = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE );
    lCANBaudRates->AddEnumPair( 10, "10 kbps" );
    lCANBaudRates->AddEnumPair( 20, "20 kbps" );
    lCANBaudRates->AddEnumPair( 50, "50 kbps" );
    lCANBaudRates->AddEnumPair( 100, "100 kbps" );
    lCANBaudRates->AddEnumPair( 125, "125 kbps" );
    lCANBaudRates->AddEnumPair( 250, "250 kbps" );
    lCANBaudRates->AddEnumPair( 500, "500 kbps" );
    lCANBaudRates->AddEnumPair( 1000, "1 Mbps" );

    LdEnumProperty *lSerialDataBits = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_DATA_BITS );
    lSerialDataBits->AddEnumPair( 8, "8 bits" );
    lSerialDataBits->AddEnumPair( 9, "9 bits" );

    LdEnumProperty *lSerialParity = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_PARITY );
    lSerialParity->AddEnumPair( 0, "None" );
    lSerialParity->AddEnumPair( 1, "Odd" );
    lSerialParity->AddEnumPair( 2, "Even" );

    LdEnumProperty *lSerialStopBits = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS );
    lSerialStopBits->AddEnumPair( 1, "1 bit" );
    lSerialStopBits->AddEnumPair( 2, "2 bits" );

    mProperties->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->SetDeviceId( LtComLeddarTechPublic::LT_COMM_ID_NUMBER_OF_SEGMENTS );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->ForceValue( 0, P_USB );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->SetClean();

    // Extra result state properties
    mResultStatePropeties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_RS_TIMESTAMP )->SetDeviceId( LtComLeddarTechPublic::LT_COMM_ID_TIMESTAMP );
    mResultStatePropeties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_SYSTEM_TEMP,
                                        LtComLeddarTechPublic::LT_COMM_ID_SYS_TEMP, 4, 0, 1, "System Temperature" ) );
    mResultStatePropeties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_PREDICT_TEMP,
                                        LtComM16::M16_ID_PREDICTED_TEMP, 4, 0, 1, "Predicted Temperature" ) );
    mResultStatePropeties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_DISCRETE_OUTPUTS,
                                        LtComM16::M16_ID_DISCRETE_OUTPUTS, 4, "Discrete Outputs" ) );
    mResultStatePropeties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_ACQ_CURRENT_PARAMS,
                                        LtComM16::M16_ID_ACQ_CURRENT_PARAMS, 4, "Acquisition Current Parameters" ) );
    mResultStatePropeties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_CPU_LOAD,
                                        LtComLeddarTechPublic::LT_COMM_ID_CPU_LOAD_V2, 4, 0, 2 ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::Connect( void )
///
/// \brief  Connect to device
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::Connect( void )
{
    LdDevice::Connect();
    mProtocolData->SetConnected( true );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::GetConstants( void )
///
/// \brief  Gets the constants from the device
///
/// \author Patrick Boulay
/// \date   February 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::GetConstants( void )
{
    // Get info from the USB identify package
    const LeddarConnection::LdConnectionInfoUsb *lUsbConnection = dynamic_cast<const LeddarConnection::LdConnectionInfoUsb *>
            ( GetConnection()->GetConnectionInfo() );
    GetProperties()->GetTextProperty( LdPropertyIds::ID_SERIAL_NUMBER )->ForceValue( 0, lUsbConnection->GetInfos().mSerialNumber );
    GetProperties()->GetTextProperty( LdPropertyIds::ID_PART_NUMBER )->ForceValue( 0, lUsbConnection->GetInfos().mHardwarePartNumber );
    GetProperties()->GetTextProperty( LdPropertyIds::ID_SOFTWARE_PART_NUMBER )->ForceValue( 0, lUsbConnection->GetInfos().mSoftwarePartNumber );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_FIRMWARE_VERSION_INT )->ForceValue( 0, lUsbConnection->GetInfos().mSoftwareVersion );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_FPGA_VERSION )->ForceValue( 0, lUsbConnection->GetInfos().mFpgaFirmwareVersion );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_CRC32 )->ForceValue( 0, lUsbConnection->GetInfos().mSoftwareCRC32 );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_DEVICE_TYPE )->ForceValue( 0, lUsbConnection->GetInfos().mDeviceType );

    GetListing();

    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET_DEVICE );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();
    mProtocolConfig->ReadElementToProperties( GetProperties() );

    uint16_t lIds[] =
    {
        LtComLeddarTechPublic::LT_COMM_ID_NUMBER_OF_SEGMENTS,
        LtComLeddarTechPublic::LT_COMM_ID_DISTANCE_SCALE,
        LtComLeddarTechPublic::LT_COMM_ID_FILTERED_SCALE,
        LtComLeddarTechPublic::LT_COMM_ID_AMPLITUDE_SCALE,
        LtComLeddarTechPublic::LT_COMM_ID_REAL_DIST_OFFSET,
        LtComLeddarTechPublic::LT_COMM_ID_TRACE_POINT_STEP,
        LtComLeddarTechPublic::LT_COMM_ID_BASE_SAMPLE_DISTANCE,
        LtComLeddarTechPublic::LT_COMM_ID_LIMIT_CFG_BASE_SAMPLE_COUNT,
        LtComLeddarTechPublic::LT_COMM_ID_LIMIT_CFG_ACCUMULATION_EXPONENT,
        LtComLeddarTechPublic::LT_COMM_ID_LIMIT_CFG_OVERSAMPLING_EXPONENT,
        LtComLeddarTechPublic::LT_COMM_ID_REFRESH_RATE,
        LtComM16::M16_ID_BEAM_RANGE,
        LtComM16::M16_ID_LIMIT_CFG_THRESHOLD_TABLE_OFFSET,
        LtComM16::M16_ID_LIMIT_CFG_AUTO_ACQ_AVG_FRM,
        LtComM16::M16_ID_ACQUISITION_OPTION_MASK,
        LtComM16::M16_ID_LIMIT_CFG_CAN_PORT_MAX_ECHOES,
        LtComM16::M16_ID_CAN_PORT_OPTIONS_MASK,
        LtComM16::M16_ID_SERIAL_PORT_BAUDRATE_OPTIONS_MASK,
        LtComM16::M16_ID_TEST_MODE
    };

    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_ELEMENT_LIST, LT_ALEN( lIds ), sizeof( lIds[ 0 ] ), lIds, sizeof( lIds[ 0 ] ) );
    mProtocolConfig->SendRequest();

    mProtocolConfig->ReadAnswer();
    mProtocolConfig->ReadElementToProperties( GetProperties() );

    uint32_t lDistanceScale = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE )->ValueT<uint32_t>();
    uint32_t lFilteredScale = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FILTERED_AMP_SCALE )->ValueT<uint32_t>();

    //CPU temp use the same scale as distance
    GetResultStates()->GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_SYSTEM_TEMP )->SetScale( lDistanceScale );
    GetResultStates()->GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_PREDICT_TEMP )->SetScale( 65536 );

    // Detection length is in fixed-point in sensor, but direct floating-point
    // in files so we must manipulate a little bit...
    LeddarCore::LdFloatProperty *lDetectionLength = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_DETECTION_LENGTH );

    if( lDetectionLength->Count() > 0 )
    {
        lDetectionLength->SetScale( lDistanceScale );
    }

    GetIntensityMappings();
    UpdateConstants();
    GetResultEchoes()->Init( lDistanceScale, lFilteredScale, M16_MAX_ECHOES );

    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_CONSTANT );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            ( *lIter )->SetClean();
        }
    }

    lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_INFO );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            ( *lIter )->SetClean();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::GetConfig( void )
///
/// \brief  Gets the device configuration properties
///
/// \author Patrick Boulay
/// \date   February 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::GetConfig( void )
{
    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET_CONFIG );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();
    mProtocolConfig->ReadElementToProperties( GetProperties() );

    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_CONFIGURATION );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            ( *lIter )->SetClean();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::SetConfig( void )
///
/// \brief  Sets the configuration to the device
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::SetConfig( void )
{
    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_SET_CONFIG );

    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_CONFIGURATION );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            mProtocolConfig->AddElement( ( *lIter )->GetDeviceId(), static_cast<uint16_t>( ( *lIter )->Count() ), ( *lIter )->UnitSize(), ( *lIter )->CStorage(),
                                         static_cast<uint32_t>( ( *lIter )->Stride() ) );
        }
    }

    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            ( *lIter )->SetClean();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::WriteConfig( void )
///
/// \brief  Writes the configuration on the device
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::WriteConfig( void )
{
    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_WRITE_CONFIG );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::RestoreConfig( void )
///
/// \brief  Restore configuration on the device. Call GetConfig() to update local (SDK) values
///         after doing a RestoreConfig
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::RestoreConfig( void )
{
    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_RESTORE_CONFIG );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::GetCalib()
///
/// \brief  Gets the calibration from the sensor
///
/// \author David Levy
/// \date   June 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::GetCalib()
{
    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET_CAL );
    mProtocolConfig->SendRequest();

    mProtocolConfig->ReadAnswer();
    mProtocolConfig->ReadElementToProperties( mProperties );

    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_CALIBRATION );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            ( *lIter )->SetClean();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::GetListing( void )
///
/// \brief  Read the listing of all supported commands and ids and store it so that it can be
///     queried.
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::GetListing( void )
{
    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_LISTING );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();

    uint16_t lElementCout = 0;
    LtComLeddarTechPublic::sLtCommElementRequestInfo *lElementReqInfo = nullptr;

    while( mProtocolConfig->ReadElement() )
    {
        if( mProtocolConfig->GetElementId() == LtComLeddarTechPublic::LT_COMM_ID_REQUEST_ELEMENT_LIST )
        {
            lElementReqInfo = static_cast<LtComLeddarTechPublic::sLtCommElementRequestInfo *>( mProtocolConfig->GetElementData() );
            lElementCout = mProtocolConfig->GetElementCount();
            break;
        }
    }

    bool lValidDataLevel = false;

    for( int i = 0; i < lElementCout; ++i )
    {
        if( lElementReqInfo[ i ].mElementId == LtComM16::M16_ID_DATA_LEVEL )
        {
            lValidDataLevel = true;
        }
    }

    if( !lValidDataLevel )
    {
        throw std::runtime_error( "Your firmware is incompatible with the SDK, please update your firmware." );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::GetIntensityMappings( void )
///
/// \brief  Get the intensity percentage for each value of LED intensity.
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::GetIntensityMappings( void )
{
    LdIntegerProperty lIntensityMapping( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE, LdPropertyIds::ID_LED_INTENSITY_LIST, 0, 1 );
    lIntensityMapping.SetCount( M16_LED_INTENSITY_MAX + 1 );
    uint8_t lCount = 0;
    uint8_t lOldValue = -1;

    for( uint8_t i = 0; i <= M16_LED_INTENSITY_MAX; ++i )
    {
        mProtocolConfig->StartRequest( LtComM16::M16_CFGSRV_REQUEST_PARAMS_TO_LED_POWER );
        mProtocolConfig->AddElement( LtComM16::M16_ID_CFG_LED_INTENSITY, 1, 1, &i, 1 );
        mProtocolConfig->SendRequest();

        mProtocolConfig->ReadAnswer();

        while( mProtocolConfig->ReadElement() )
        {
            if( mProtocolConfig->GetElementId() == LtComM16::M16_ID_LED_POWER )
            {
                uint8_t lValue = *( static_cast<uint8_t *>( mProtocolConfig->GetElementData() ) );
                lIntensityMapping.SetValue( i, lValue );

                if( lOldValue != lValue )
                {
                    ++lCount;
                    lOldValue = lValue;
                }
            }
        }
    }

    //Populate intensity list with it
    LdEnumProperty *lIntensity = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY );
    lIntensity->SetEnumSize( lCount );
    uint16_t lMax = 110;

    for( int i = M16_LED_INTENSITY_MAX; i >= 0; --i )
    {
        if( lIntensityMapping.Value( i ) != lMax )
        {
            lMax = lIntensityMapping.ValueT<uint16_t>( i );
            lIntensity->AddEnumPair( i, LeddarUtils::LtStringUtils::IntToString( lMax ) );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::UpdateConstants( void )
///
/// \brief  Updates the constants
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::UpdateConstants( void )
{
    mProperties->GetIntegerProperty( LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL )->ForceValue( 0, M16_MAX_ECHOES_BY_CHANNEL );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL )->SetClean();

    LdFloatProperty *lThresholdOffset = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY_OLD );
    lThresholdOffset->SetScale( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FILTERED_AMP_SCALE )->ValueT<uint32_t>() );
    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY_LIMITS )->SetScale( lThresholdOffset->Scale() );

    // For older devices that don't report it.
    LdIntegerProperty *lHChannelCount = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_HSEGMENT );

    if( lHChannelCount->Count() == 0 )
    {
        lHChannelCount->SetCount( 1 );
        lHChannelCount->ForceValue( 0, M16_NUMBER_CHANNELS );
    }

    GetProperties()->GetBitProperty( LdPropertyIds::ID_SEGMENT_ENABLE )->SetLimit( ( 1 << ( GetProperties()->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->Value() / 2 ) ) - 1 );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_VSEGMENT )->ForceValue( 0, 1 );

    // Adjust some properties limits.
    // Base sample count
    int64_t lMin = 2;
    int64_t lMax = 64;

    LdIntegerProperty *lPointCountLimits = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT_LIMITS );

    if( lPointCountLimits->Count() == 2 )
    {
        lMin = lPointCountLimits->Value( 0 );
        lMax = lPointCountLimits->Value( 1 );
    }

    LdIntegerProperty *lBaseSampleCount = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT );
    lBaseSampleCount->SetLimits( lMin, lMax );

    // Accumulation
    lMin = 0;
    lMax = 10;

    LdIntegerProperty *lAccumulationLimits = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_LIMITS );

    if( lAccumulationLimits->Count() == 2 )
    {
        lMin = lAccumulationLimits->Value( 0 );
        lMax = lAccumulationLimits->Value( 1 );
    }

    LdIntegerProperty *lAccumulationExp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP );
    lAccumulationExp->SetLimits( lMin, lMax );

    // Oversampling
    lMin = 0;
    lMax = 3;

    LdIntegerProperty *lOversamplingLimits = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_LIMITS );

    if( lOversamplingLimits->Count() == 2 )
    {
        lMin = lOversamplingLimits->Value( 0 );
        lMax = lOversamplingLimits->Value( 1 );
    }

    LdIntegerProperty *lOversamplingExp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP );
    lOversamplingExp->SetLimits( lMin, lMax );

    // Threshold offset
    float lMinf = -5;
    float lMaxf = 100;

    LdFloatProperty *lThresoldOffsetLimits = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY_LIMITS );

    if( lThresoldOffsetLimits->Count() == 2 )
    {
        lMinf = lThresoldOffsetLimits->Value( 0 );
        lMaxf = lThresoldOffsetLimits->Value( 1 );
    }

    LdFloatProperty *lThresoldOffset = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY_OLD );
    lThresoldOffset->SetLimits( lMinf, lMaxf );

    // Change delay
    lMin = 0;
    lMax = 32767;

    LdIntegerProperty *lChangeDelayLimits = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CHANGE_DELAY_LIMITS );

    if( lChangeDelayLimits->Count() == 2 )
    {
        lMin = lChangeDelayLimits->Value( 0 );
        lMax = lChangeDelayLimits->Value( 1 );
    }

    LdIntegerProperty *lChangeDelay = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CHANGE_DELAY );
    lChangeDelay->SetLimits( lMin, lMax );

    //Horizontal field of view
    LdFloatProperty *lHFOV = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_HFOV );
    float lValue = 0;

    switch( GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_OPTIONS )->Value( 0 ) & LtComM16::LT_COMM_DEVICE_OPTION_LFOV_MASK )
    {
        case LtComM16::LT_COMM_DEVICE_OPTION_18_DEG_LFOV:
            lValue = 19.4f;
            break;

        case LtComM16::LT_COMM_DEVICE_OPTION_34_DEG_LFOV:
            lValue = 36.4f;
            break;

        case LtComM16::LT_COMM_DEVICE_OPTION_26_DEG_LFOV:
            lValue = 26;
            break;

        case LtComM16::LT_COMM_DEVICE_OPTION_60_DEG_LFOV:
            lValue = 60;
            break;

        case LtComM16::LT_COMM_DEVICE_OPTION_45_DEG_LFOV:
            lValue = 48;
            break;

        case LtComM16::LT_COMM_DEVICE_OPTION_10_DEG_LFOV:
            lValue = 10;
            break;

        case LtComM16::LT_COMM_DEVICE_OPTION_100_DEG_LFOV:
            lValue = 100;
            break;
    }

    lHFOV->ForceValue( 0, lValue );
    lHFOV->SetClean();

    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION )->SetLimits( LtComM16::SMOOTHINGLIMITS[0], LtComM16::SMOOTHINGLIMITS[1] );

    uint32_t lDistanceScale = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE )->ValueT<uint32_t>();
    LeddarCore::LdFloatProperty *lDetectionLength = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_DETECTION_LENGTH );
    lDetectionLength->SetScale( lDistanceScale );

    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_STATIC_THRESHOLD_DISTANCES )->SetScale( lDistanceScale );
    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_STATIC_THRESHOLD_AMPLITUDES )->SetScale( GetProperties()->GetIntegerProperty(
                LeddarCore::LdPropertyIds::ID_FILTERED_AMP_SCALE )->ValueT<uint32_t>() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::SetDataMask( uint32_t aDataMask )
///
/// \brief  Set data mask for the data end point.
///
/// \param  aDataMask   The data mask.
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::SetDataMask( uint32_t aDataMask )
{
    mDataMask = aDataMask;
    uint32_t lDataMask = this->ConvertDataMaskToLTDataMask( aDataMask );

    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_SET );
    mProtocolConfig->AddElement( LtComM16::M16_ID_DATA_LEVEL, 1, sizeof( lDataMask ), &lDataMask, sizeof( lDataMask ) );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarDevice::LdSensorM16::GetData( void )
///
/// \brief  Gets the data from the device
///
/// \return True is new states (because they are received last and hold the timestamp information) were processed else false
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
LeddarDevice::LdSensorM16::GetData( void )
{
    // Verify if the data mask is set
    if( mDataMask == DM_NONE || ( ( mDataMask & DM_STATES ) == 0 ) ) //states are required for timestamp
    {
        SetDataMask( DM_ALL );
    }

    // Read available data on the data channel
    try
    {
        mProtocolData->ReadRequest();
    }
    catch( LeddarException::LtTimeoutException & )
    {
        return false;
    }

    uint16_t lRequestCode = mProtocolData->GetRequestCode();

    //Return true only on states because they are received last for a frame, and they hold the timestamp (trace and echo dont know the timestamp by themselves)
    if( lRequestCode == LtComLeddarTechPublic::LT_COMM_DATASRV_REQUEST_SEND_ECHOES )
    {
        ProcessEchoes();
        return false;
    }
    else if( lRequestCode == LtComLeddarTechPublic::LT_COMM_DATASRV_REQUEST_SEND_STATES )
    {
        return ProcessStates();
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarDevice::LdSensorM16::ProcessStates( void )
///
/// \brief  Process the states during a GetData and update timestamp on echoes
///
/// \return True if new states are received.
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
LeddarDevice::LdSensorM16::ProcessStates( void )
{
    uint32_t lPreviousTimeStamp = mStates.GetTimestamp();
    mProtocolData->ReadElementToProperties( mResultStatePropeties );

    if( lPreviousTimeStamp != mStates.GetTimestamp() )
    {
        //Only the states know the timestamp, so update echoes timestamp, swap buffers, and trigger the update finished
        if( mDataMask & DM_ECHOES )
        {
            mEchoes.SetTimestamp( mStates.GetTimestamp() );
            mEchoes.UnLock( LeddarConnection::B_SET );
            ComputeCartesianCoordinates();
            mEchoes.Swap();
            mEchoes.UpdateFinished();
        }

        //And Finally Get specific data from sensor
        RequestProperties( GetResultStates()->GetProperties(), std::vector<uint16_t>( 1, LtComLeddarTechPublic::LT_COMM_ID_CPU_LOAD_V2 ) );
        mStates.UpdateFinished();
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::ProcessEchoes( void )
///
/// \brief  Process the echoes read by GetData
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::ProcessEchoes( void )
{
    if( mProtocolData->GetMessageSize() == 0 )
    {
        mEchoes.SetEchoCount( 0 );
        return;
    }


    std::vector<LeddarConnection::LdEcho> &lEchoes = *mEchoes.GetEchoes( LeddarConnection::B_SET );
    mEchoes.Lock( LeddarConnection::B_SET );

    while( mProtocolData->ReadElement() )
    {
        switch( mProtocolData->GetElementId() )
        {
            case LtComLeddarTechPublic::LT_COMM_ID_ECHOES_AMPLITUDE:
                mEchoes.SetEchoCount( mProtocolData->GetElementCount() );
                mProtocolData->PushElementDataToBuffer( &lEchoes[ 0 ].mAmplitude,
                                                        mProtocolData->GetElementCount(),
                                                        sizeof( ( ( LeddarConnection::LdEcho * )0 )->mAmplitude ),
                                                        sizeof( lEchoes[ 0 ] ) );
                break;

            case LtComLeddarTechPublic::LT_COMM_ID_ECHOES_DISTANCE:
                mEchoes.SetEchoCount( mProtocolData->GetElementCount() );
                mProtocolData->PushElementDataToBuffer( &lEchoes[ 0 ].mDistance,
                                                        mProtocolData->GetElementCount(),
                                                        sizeof( ( ( LeddarConnection::LdEcho * )0 )->mDistance ),
                                                        sizeof( lEchoes[ 0 ] ) );
                break;

            case LtComLeddarTechPublic::LT_COMM_ID_ECHOES_BASE:
                mEchoes.SetEchoCount( mProtocolData->GetElementCount() );
                mProtocolData->PushElementDataToBuffer( &lEchoes[ 0 ].mBase,
                                                        mProtocolData->GetElementCount(),
                                                        sizeof( ( ( LeddarConnection::LdEcho * )0 )->mBase ),
                                                        sizeof( lEchoes[ 0 ] ) );
                break;

            case LtComLeddarTechPublic::LT_COMM_ID_ECHOES_CHANNEL_INDEX:
                mEchoes.SetEchoCount( mProtocolData->GetElementCount() );
                mProtocolData->PushElementDataToBuffer( &lEchoes[ 0 ].mChannelIndex,
                                                        mProtocolData->GetElementCount(),
                                                        sizeof( ( ( LeddarConnection::LdEcho * )0 )->mChannelIndex ),
                                                        sizeof( lEchoes[ 0 ] ) );
                break;

            case LtComLeddarTechPublic::LT_COMM_ID_ECHOES_VALID:
                mEchoes.SetEchoCount( mProtocolData->GetElementCount() );
                mProtocolData->PushElementDataToBuffer( &lEchoes[ 0 ].mFlag,
                                                        mProtocolData->GetElementCount(),
                                                        sizeof( ( ( LeddarConnection::LdEcho * )0 )->mFlag ),
                                                        sizeof( lEchoes[ 0 ] ) );
                break;
        }
    }

    //We do not swap nor send the UpdateFinished signal here, the timestamp is only in the states so we need to wait for the states
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions aOptions )
///
/// \brief  Resets the device
///
/// \exception  LeddarException::LtComException Thrown when a trying to do an unimplemented reset
///
/// \param  aType       Reset type. See \ref LeddarDefines::eResetType
/// \param  aOptions    Reset options. See \ref LeddarDefines::eResetOptions
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions aOptions, uint32_t )
{
    if( aType == LeddarDefines::RT_CONFIG_RESET )
    {
        mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_RESET_CONFIG );
        mProtocolConfig->SendRequest();
        mProtocolConfig->ReadAnswer();
    }
    else if( aType == LeddarDefines::RT_SOFT_RESET )
    {
        mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_RESET );
        uint8_t lSoftwareType = 0;

        if( aOptions == LeddarDefines::RO_MAIN )
        {
            lSoftwareType = LtComLeddarTechPublic::LT_COMM_SOFTWARE_TYPE_MAIN;
        }
        else if( aOptions == LeddarDefines::RO_FACTORY )
        {
            lSoftwareType = LtComLeddarTechPublic::LT_COMM_SOFTWARE_TYPE_FACTORY;
        }
        else
        {
            throw LeddarException::LtComException( "Reset option not valid: " + LeddarUtils::LtStringUtils::IntToString( aOptions ) + "." );
        }

        mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_SOFTWARE_TYPE, 1, sizeof( lSoftwareType ), &lSoftwareType, sizeof( lSoftwareType ) );
        mProtocolConfig->SendRequest();
        mProtocolConfig->ReadAnswer();

        LeddarUtils::LtTimeUtils::Wait( 1500 );

        Disconnect();
    }
    else
    {
        throw LeddarException::LtComException( "Reset type: " + LeddarUtils::LtStringUtils::IntToString( aType ) + " not implemented." );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::RequestProperties( LeddarCore::LdPropertiesContainer *aProperties, std::vector<uint16_t> aDeviceIds )
///
/// \brief  Update requested properties from the sensor
///
/// \param [in,out] aProperties Property container to store the update properties into.
/// \param          aDeviceIds  List of device ids for the properties.
///
/// \author David Levy
/// \date   June 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::RequestProperties( LeddarCore::LdPropertiesContainer *aProperties, std::vector<uint16_t> aDeviceIds )
{
    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_ELEMENT_LIST, static_cast<uint16_t>( aDeviceIds.size() ), sizeof( aDeviceIds[0] ), &aDeviceIds[0],
                                 sizeof( aDeviceIds[0] ) );
    mProtocolConfig->SendRequest();

    mProtocolConfig->ReadAnswer();
    mProtocolConfig->ReadElementToProperties( aProperties );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::SetProperties( LeddarCore::LdPropertiesContainer *aProperties, std::vector<uint16_t> aDeviceIds, unsigned int aRetryNbr )
///
/// \brief  Sets the properties
///
/// \exception  LeddarException::LtComException Thrown when a Lt Com error condition occurs.
///
/// \param [in,out] aProperties If non-null, the properties.
/// \param          aDeviceIds  List of identifiers for the devices.
/// \param          aRetryNbr   The retry number.
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::SetProperties( LeddarCore::LdPropertiesContainer *aProperties, std::vector<uint16_t> aDeviceIds, unsigned int aRetryNbr )
{
    for( size_t i = 0; i < aDeviceIds.size(); ++i )
    {
        LeddarCore::LdProperty *lProperty = aProperties->FindDeviceProperty( aDeviceIds[ i ] );

        if( lProperty != nullptr )
        {
            mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_SET );
            mProtocolConfig->AddElement( aDeviceIds[ i ], static_cast<uint16_t>( lProperty->Count() ), lProperty->UnitSize(), lProperty->CStorage(),
                                         static_cast<uint32_t>( lProperty->Stride() ) );
            mProtocolConfig->SendRequest();

            unsigned int lCount = aRetryNbr;
            bool lRetry = false;

            do
            {
                try
                {
                    lRetry = false;
                    mProtocolConfig->ReadAnswer();
                }
                catch( LeddarException::LtComException &e )
                {
                    if( e.GetDisconnect() == true )
                        throw;

                    ( lCount-- != 0 ) ? lRetry = true : throw;

                }
            }
            while( lRetry );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::RemoveLicense( const std::string &aLicense )
///
/// \brief  Removes a specific license from sensor
///
/// \exception  LeddarException::LtComException Thrown when the device answer code indicate an error.
///
/// \param  aLicense    The license.
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::RemoveLicense( const std::string &aLicense )
{
    std::string lCurrentLicense = GetProperties()->GetBufferProperty( LdPropertyIds::ID_LICENSE )->GetStringValue();
    std::transform( lCurrentLicense.begin(), lCurrentLicense.end(), lCurrentLicense.begin(), ::toupper );

    std::string lToRemove = aLicense;
    std::transform( lToRemove.begin(), lToRemove.end(), lToRemove.begin(), ::toupper );

    if( lToRemove == lCurrentLicense )
    {
        uint8_t lEmptyLicense[ LT_COMM_LICENSE_KEY_LENGTH ];
        memset( lEmptyLicense, 0, LT_COMM_LICENSE_KEY_LENGTH );

        try
        {
            SendLicense( lEmptyLicense, false );
        }
        catch( std::runtime_error &e )
        {
            //Invalid license sent on purpose to remove the real license
            if( strcmp( e.what(), "Invalid license." ) != 0 )
                throw;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::RemoveAllLicenses( void )
///
/// \brief  Removes all licenses from the sensor
///
/// \exception  LeddarException::LtComException Thrown when the device answer code indicate an error.
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16::RemoveAllLicenses( void )
{
    uint8_t lEmptyLicense[ LT_COMM_LICENSE_KEY_LENGTH ];
    memset( lEmptyLicense, 0, LT_COMM_LICENSE_KEY_LENGTH );

    try
    {
        SendLicense( lEmptyLicense, false );
    }
    catch( std::runtime_error &e )
    {
        //Invalid license sent on purpose to remove the real license
        if( strcmp( e.what(), "Invalid license." ) != 0 )
            throw;
    }

    try
    {
        SendLicense( lEmptyLicense, true );
    }
    catch( std::runtime_error &e )
    {
        //Invalid license sent on purpose to remove the real license
        if( strcmp( e.what(), "Invalid license." ) != 0 )
            throw;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarDevice::sLicense LeddarDevice::LdSensorM16::SendLicense( const std::string &aLicense, bool aVolatile )
///
/// \brief  Sends a license to the sensor
///
/// \exception  std::length_error               Raised when license length is invalid.
/// \exception  LeddarException::LtComException Thrown when the device answer code indicate an error.
/// \exception  std::runtime_error              If the license sent is invalid.
///
/// \param  aLicense    The license to send.
/// \param  aVolatile   True if license is volatile / temporary. [LeddarTech internal use]
///
/// \return A LeddarDevice::sLicense containing the license string and the authorisation level.
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDefines::sLicense
LeddarDevice::LdSensorM16::SendLicense( const std::string &aLicense, bool aVolatile )
{
    if( aLicense.length() != LT_COMM_LICENSE_KEY_LENGTH * 2 && aLicense.length() != 0 )
        throw std::length_error( "Invalid license length." );

    // Convert the user string to 16 bytes license
    uint8_t lBuffer[LT_COMM_LICENSE_KEY_LENGTH];

    for( size_t i = 0; i < aLicense.length(); i += 2 )
    {
        lBuffer[i / 2] = ( uint8_t )strtoul( aLicense.substr( i, 2 ).c_str(), nullptr, 16 );
    }

    return SendLicense( lBuffer, aVolatile );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarDevice::sLicense LeddarDevice::LdSensorM16::SendLicense( const uint8_t *aLicense, bool aVolatile )
///
/// \brief  Sends a license to the sensor
///
/// \exception  LeddarException::LtComException Thrown when the device answer code indicate an error.
/// \exception  std::runtime_error              If the license sent is invalid.
///
/// \param  aLicense    The license to send.
/// \param  aVolatile   True if license is volatile / temporary. [LeddarTech internal use]
///
/// \return A LeddarDevice::sLicense containing the license string and the authorisation level.
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDefines::sLicense
LeddarDevice::LdSensorM16::SendLicense( const uint8_t *aLicense, bool aVolatile )
{
    //Store the license in the property, send it to the sensor, and read back from the sensor license info
    LdBufferProperty *lLicenseProp = GetProperties()->GetBufferProperty( aVolatile ? LdPropertyIds::ID_VOLATILE_LICENSE : LdPropertyIds::ID_LICENSE );

    if( lLicenseProp->Count() == 0 )
        lLicenseProp->SetCount( 1 );

    lLicenseProp->SetValue( 0, aLicense, LT_COMM_LICENSE_KEY_LENGTH );
    lLicenseProp->SetClean();

    std::vector<uint16_t> lDeviceIds;
    lDeviceIds.push_back( aVolatile ? LtComLeddarTechPublic::LT_COMM_ID_VOLATILE_LICENSE : LtComLeddarTechPublic::LT_COMM_ID_LICENSE );
    SetProperties( GetProperties(), lDeviceIds );

    LeddarDefines::sLicense lResultLicense;

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        throw LeddarException::LtComException( "Wrong answer code : " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ) );
    }

    lDeviceIds.push_back( aVolatile ? LtComLeddarTechPublic::LT_COMM_ID_VOLATILE_LICENSE_INFO : LtComLeddarTechPublic::LT_COMM_ID_LICENSE_INFO );
    RequestProperties( GetProperties(), lDeviceIds );

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        throw LeddarException::LtComException( "Wrong answer code : " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ) );
    }

    LdIntegerProperty *lLicenseInfo = GetProperties()->GetIntegerProperty( aVolatile ?  LdPropertyIds::ID_VOLATILE_LICENSE_INFO : LdPropertyIds::ID_LICENSE_INFO );
    lResultLicense.mLicense = lLicenseProp->GetStringValue( 0 );
    lResultLicense.mType = lLicenseInfo->Value() & 0xFFFF;
    lResultLicense.mSubType = static_cast<uint8_t>( lLicenseInfo->ValueT<uint32_t>() >> 16 );

    if( lResultLicense.mType == 0 )
    {
        throw std::runtime_error( "Invalid license." );
    }

    return lResultLicense;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::vector<LeddarDevice::sLicense> LeddarDevice::LdSensorM16::GetLicenses( void )
///
/// \brief  Gets the licenses on the sensor
///
/// \return The permanent licenses on the sensor.
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<LeddarDefines::sLicense>
LeddarDevice::LdSensorM16::GetLicenses( void )
{
    std::vector<LeddarDefines::sLicense> lLicenses;
    std::vector<uint16_t> lDeviceIds;
    lDeviceIds.push_back( LtComLeddarTechPublic::LT_COMM_ID_LICENSE );
    lDeviceIds.push_back( LtComLeddarTechPublic::LT_COMM_ID_LICENSE_INFO );
    lDeviceIds.push_back( LtComLeddarTechPublic::LT_COMM_ID_VOLATILE_LICENSE );
    lDeviceIds.push_back( LtComLeddarTechPublic::LT_COMM_ID_VOLATILE_LICENSE_INFO );
    RequestProperties( GetProperties(), lDeviceIds );

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        return lLicenses;
    }

    LdIntegerProperty *lLicenseInfo = GetProperties()->GetIntegerProperty( LdPropertyIds::ID_LICENSE_INFO );
    LdBufferProperty *lLicenseProp = GetProperties()->GetBufferProperty( LdPropertyIds::ID_LICENSE );

    for( size_t i = 0; i < lLicenseProp->Count(); ++i )
    {
        LeddarDefines::sLicense lResultLicense;
        lResultLicense.mLicense = lLicenseProp->GetStringValue( i );
        lResultLicense.mType = lLicenseInfo->Value( i ) & 0xFFFF;
        lResultLicense.mSubType = static_cast<uint8_t>( lLicenseInfo->ValueT<uint32_t>( i ) >> 16 );

        lLicenses.push_back( lResultLicense );
    }

    return lLicenses;
}

#endif
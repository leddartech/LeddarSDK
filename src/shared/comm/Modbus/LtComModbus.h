/// ****************************************************************************
///
/// \file      comm/Modbus/LtComModbus.h
///
/// \brief     Shared data for modbus communication
///                 Note on modbus usage :
///                 If you use SendRawRequest and ReceiveRawConfirmation, you need to use MODBUS_DATA_OFFSET and convert data to BigEndian
///                 If you use libmodbus (via LdLibModbusSerial) you don't need to use the offset nor to convert data to bugendian
///
/// \author    David Levy
///
/// \since     September 2017
///
/// \copyright (c) 2017 LeddarTech Inc. All rights reserved.
///
/// ***************************************************************************

#pragma once

constexpr uint8_t MODBUS_MAX_ADDR              = 247;
constexpr uint16_t LTMODBUS_TCP_MAX_ADU_LENGTH = 260;
constexpr uint16_t LTMODBUS_RTU_MAX_ADU_LENGTH = 256;
constexpr uint8_t MODBUS_CRC_SIZE              = 2;
constexpr uint8_t MODBUS_DATA_OFFSET           = 2; // First byte is modbus address, second byte is  function code, then actual data
// For several commands (see official modbus documentation) the next byte is the byte count, then the actual data


cmake_minimum_required(VERSION 3.10.0)
include(CMakeDependentOption)

project (LeddarConfigurator4)

if ( MSVC )
    if(${CMAKE_CXX_FLAGS} MATCHES "/W[0-9]")
        string(REGEX REPLACE "/W[0-9]" "/W4" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    else()
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
    endif() 
    add_compile_options(/we4996)
else(MSVC)
    add_compile_options(-Wall -Wextra -Wsuggest-override -Werror=suggest-override)
endif(MSVC)

set(Leddar_Src
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdBitFieldProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdBoolProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdBufferProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdCanKomodo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdCarrierEnhancedModbus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnectionFactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnectionInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnectionUniversal.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnectionUniversalCan.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnectionUniversalModbus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnectionUniversalSpi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdDevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdDeviceFactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdDoubleBuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdEnumProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdEthernet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdFloatProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdIntegerProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdInterfaceCan.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdLibModbusSerial.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdLibUsb.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdLjrRecordReader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdLjrRecorder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdObject.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdPropertiesContainer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdProtocolCan.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdProtocolLeddarEngineRTP.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdProtocolLeddarTech.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdProtocolLeddarTechHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdProtocolLeddartechEthernet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdProtocolLeddartechEthernetPixell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdProtocolLeddartechEthernetUDP.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdProtocolLeddartechUSB.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdRecordPlayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdResultEchoes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdResultProvider.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdResultStates.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdRtpPacket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensorDTec.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensorIS16.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensorLeddarAuto.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensorM16.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensorM16Can.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensorM16Modbus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensorOneModbus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensorPixell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensorVu.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensorVu8.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensorVu8Can.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensorVu8Modbus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSpiBCM2835.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSpiFTDI.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdTextProperty.cpp

    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnectionDefines.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnectionInfoCan.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnectionInfoEthernet.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnectionInfoModbus.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnectionInfoSpi.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnectionInfoUsb.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdConnectionModbusStructures.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdDefines.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdDetectionPacketReceiver.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdDetectionPacket.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdInterfaceEthernet.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdInterfaceModbus.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdInterfaceSpi.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdInterfaceUsb.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdLjrDefines.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdPropertyIds.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdRecorder.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdRecordReader.h
    
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdRtpPacketReceiver.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdWaveformPacket.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdWaveformPacketReceiver.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/LdSensorVuDefines.h

    #M16 FPGA Update
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/M16_FPGA/M16_FPGA_core.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/M16_FPGA/M16_FPGA_hardware.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/M16_FPGA/M16_FPGA_intrface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/M16_FPGA/M16_FPGA_SSPIEm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/M16_FPGA/M16_FPGA_util.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/M16_FPGA/M16_FPGA_debug.h
    ${CMAKE_CURRENT_LIST_DIR}/Leddar/M16_FPGA/M16_FPGA_SSPIEm.h
)

set(LeddarTech_Src
    ${CMAKE_CURRENT_LIST_DIR}/LeddarTech/LtCRCUtils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LeddarTech/LtFileUtils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LeddarTech/LtKeyboardUtils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LeddarTech/LtMathUtils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LeddarTech/LtStringUtils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LeddarTech/LtSystemUtils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LeddarTech/LtTimeUtils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LeddarTech/LtIntelHex.cpp

    ${CMAKE_CURRENT_LIST_DIR}/LeddarTech/LtDefines.h
    ${CMAKE_CURRENT_LIST_DIR}/LeddarTech/LtExceptions.h
    ${CMAKE_CURRENT_LIST_DIR}/LeddarTech/LtIntUtilities.h
    ${CMAKE_CURRENT_LIST_DIR}/LeddarTech/LtScope.h
)

set(LeddarPy_Src
    ${CMAKE_CURRENT_LIST_DIR}/LeddarPy/Connecters.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LeddarPy/LeddarPy.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LeddarPy/LeddarPyDevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LeddarPy/PythonHelper.h
)

set(Libs_Src
    ${CMAKE_CURRENT_LIST_DIR}/../libs/libmodbus/src/modbus.c
    ${CMAKE_CURRENT_LIST_DIR}/../libs/libmodbus/src/modbus-data.c
    ${CMAKE_CURRENT_LIST_DIR}/../libs/libmodbus/src/modbus-LT.c
    ${CMAKE_CURRENT_LIST_DIR}/../libs/libmodbus/src/modbus-rtu.c
    ${CMAKE_CURRENT_LIST_DIR}/../libs/libmodbus/src/modbus-tcp.c

    ${CMAKE_CURRENT_LIST_DIR}/../libs/libmodbus/src/modbus-private.h
    ${CMAKE_CURRENT_LIST_DIR}/../libs/libmodbus/src/modbus-rtu-private.h
    ${CMAKE_CURRENT_LIST_DIR}/../libs/libmodbus/src/modbus-tcp-private.h
    ${CMAKE_CURRENT_LIST_DIR}/../libs/libmodbus/src/modbus-version.h

    ${CMAKE_CURRENT_LIST_DIR}/../libs/Komodo/komodo.c
)

if(WIN32)
    list(APPEND Libs_Src
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/core.c
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/descriptor.c
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/hotplug.c
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/io.c
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/strerror.c
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/sync.c
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/os/poll_windows.c
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/os/threads_windows.c
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/os/windows_nt_common.c
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/os/windows_winusb.c

        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/libusb.h
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/libusbi.h
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/msvc/config.h
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/os/windows_common.h
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/version.h
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb/version_nano.h
    )
endif()

add_library(LC4 STATIC ${Leddar_Src} ${LeddarTech_Src} ${Libs_Src})
target_include_directories(LC4 PUBLIC
    #Our headers
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${CMAKE_CURRENT_LIST_DIR}/Leddar
    ${CMAKE_CURRENT_LIST_DIR}/LeddarTech
    ${CMAKE_CURRENT_LIST_DIR}/shared
    ${CMAKE_CURRENT_LIST_DIR}/shared/comm

    #External libs
    ${CMAKE_CURRENT_LIST_DIR}/../libs
    ${CMAKE_CURRENT_LIST_DIR}/../libs/boost
    ${CMAKE_CURRENT_LIST_DIR}/../libs/RapidJson
    ${CMAKE_CURRENT_LIST_DIR}/../libs/Komodo
    ${CMAKE_CURRENT_LIST_DIR}/../libs/libmodbus/src
    ${CMAKE_CURRENT_LIST_DIR}/../libs/MPSSE
)

if(WIN32)
    target_include_directories(LC4 PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../libs/libusb
        ${CMAKE_CURRENT_LIST_DIR}/../libs/FTDI/windows
    )
else()
    target_include_directories(LC4 PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../libs/FTDI/linux
    )
endif()

if(MSVC)
    target_compile_definitions(LC4 PRIVATE _SCL_SECURE_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)
else(MSVC)
    target_link_libraries(LC4 PUBLIC dl)
endif(MSVC)

#Configuration of the defines
option(BUILD_MODBUS "Enable modbus build (includes Vu8 serial" ON)
option(BUILD_SPI "Enable generic SPI (for hardware independent SPI)" ON)
CMAKE_DEPENDENT_OPTION(BUILD_SPI_FTDI "Enable SPI using FTDI hardware" ON "BUILD_SPI" OFF)
CMAKE_DEPENDENT_OPTION(BUILD_SPI_BCM2835 "Enable SPI using BCM2835 (raspberry pi)" OFF "BUILD_SPI" OFF)
option(BUILD_CANBUS "Enable Generic CANBus (for hardware independent CAN)" ON)
CMAKE_DEPENDENT_OPTION(BUILD_CANBUS_KOMODO "Enable CANBus using Komodo hardware" ON "BUILD_CANBUS" OFF)
option(BUILD_USB "Enable USB build" ON)
option(BUILD_ETHERNET "Enable ethernet build" ON)

CMAKE_DEPENDENT_OPTION(BUILD_ONE "Enable LeddarOne build" ON "BUILD_MODBUS" OFF)
option(BUILD_VU "Enable Vu8 build" ON)
option(BUILD_M16 "Enable M16 family build" ON)
CMAKE_DEPENDENT_OPTION(BUILD_AUTO "Enable LeddarAuto build" ON "BUILD_ETHERNET" OFF)
CMAKE_DEPENDENT_OPTION(BUILD_DTEC  "Enable Dtec build" ON "BUILD_ETHERNET" OFF)
CMAKE_DEPENDENT_OPTION(BUILD_LEDDARENGINE  "Enable Leddar Engine build" ON "BUILD_ETHERNET" OFF)

if(BUILD_ONE)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_ONE)
endif(BUILD_ONE)
if(BUILD_VU)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_VU)
endif(BUILD_VU)
if(BUILD_M16)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_M16)
endif(BUILD_M16)
if(BUILD_AUTO)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_AUTO)
endif(BUILD_AUTO)
if(BUILD_DTEC)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_DTEC)
endif(BUILD_DTEC)
if(BUILD_MODBUS)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_MODBUS)
endif(BUILD_MODBUS)
if(BUILD_SPI)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_SPI)
endif(BUILD_SPI)
if(BUILD_SPI_FTDI)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_SPI_FTDI)
endif(BUILD_SPI_FTDI)
if(BUILD_SPI_BCM2835)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_SPI_BCM2835)
endif(BUILD_SPI_BCM2835)
if(BUILD_CANBUS)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_CANBUS)
endif(BUILD_CANBUS)
if(BUILD_CANBUS_KOMODO)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_CANBUS_KOMODO)
endif(BUILD_CANBUS_KOMODO)
if(BUILD_USB)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_USB)
    if(NOT WIN32)
        target_link_libraries(LC4 PUBLIC usb-1.0)
    endif(NOT WIN32)
endif(BUILD_USB)
if(BUILD_ETHERNET)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_ETHERNET)
endif(BUILD_ETHERNET)
if(BUILD_LEDDARENGINE)
    set(BUILD_OPTIONS ${BUILD_OPTIONS} BUILD_LEDDARENGINE)
endif(BUILD_LEDDARENGINE)
target_compile_definitions(LC4 PUBLIC ${BUILD_OPTIONS})

if(${CMAKE_VERSION} VERSION_LESS "3.14.0") 
    message(STATUS "Update to cmake >= 3.14 for python package generation")
else()    
    find_package(Python COMPONENTS Interpreter Development NumPy)

    if(${Python_FOUND} AND ${Python_NumPy_FOUND})
        message(STATUS "Python " ${Python_VERSION} " found. Numpy " ${Python_NumPy_VERSION} " found")
        add_library(LeddarPy SHARED ${LeddarPy_Src})
        target_link_libraries(LeddarPy LC4 ${Python_LIBRARIES}) 
        target_include_directories(LeddarPy PRIVATE ${Python_INCLUDE_DIRS} ${Python_NumPy_INCLUDE_DIRS}) 
    elseif(${Python_FOUND})
        message(WARNING "Numpy not found")
    else()
        message(WARNING "Python not found")
    endif()
endif()

add_executable(LeddarExample ${CMAKE_CURRENT_LIST_DIR}/LeddarExample/LeddarExample.cpp)
target_link_libraries(LeddarExample LC4) 
set_property(TARGET LC4 PROPERTY POSITION_INDEPENDENT_CODE ON) #Force PIC option for python build


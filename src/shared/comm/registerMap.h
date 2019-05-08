// ****************************************************************************
/*!
Module:    Register map.

Platform:  Independant

\file      comm/registerMap.h

\brief     The register map contains the definition of the registers as well
           as the mapping between the logical address and the physical address
           of the register.

\author    Vincent Simard Bilodeau
\author    Frédéric Parent
\author    Samuel Gidel

\since     November 27, 2015

\copyright (c) 2015 LeddarTech Inc. All rights reserved.
*/
// ***************************************************************************

#ifndef  _REGISTER_MAP_
#define  _REGISTER_MAP_


//*****************************************************************************
//******************** Header includes ****************************************
//*****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _LT
  // Operating system includes files
  #include <os.h>
#endif

// Standard library includes files
//#include <inttypes.h>
#include <stdint.h>

// Platform include files
#include "comm/registerMapProduct.h"
#ifdef _LT
  #include "drivers/spiSlave.h"
#endif


//*****************************************************************************
//*************** Constants and Macros ****************************************
//*****************************************************************************

// Configuration data version
#define CFG_DATA_VERSION_0       0
#define CFG_DATA_VERSION         CFG_DATA_VERSION_0

// Advanced configuration data version
#define ADV_CFG_DATA_VERSION_0    0
#define ADV_CFG_DATA_VERSION_1    1
#define ADV_CFG_DATA_VERSION      ADV_CFG_DATA_VERSION_1

// License key list version
#define LICENCE_KEY_VERSION_0    0
#define LICENSE_KEY_VERSION      LICENCE_KEY_VERSION_0


// Device information version
#define DEV_INFO_VERSION_0       0
#define DEV_INFO_VERSION         DEV_INFO_VERSION_0

// Device information version
#define ADV_DEV_INFO_VERSION_0   0
#define ADV_DEV_INFO_VERSION     ADV_DEV_INFO_VERSION_0

// Static noise version
#define STATIC_NOISE_VERSION_0   0
#define STATIC_NOISE_VERSION     STATIC_NOISE_VERSION_0

// Backup version
#define BACKUP_VERSION_0         0
#define BACKUP_VERSION           BACKUP_VERSION_0

// Number of bank
#define REGMAP_NBBANK            22

// Number of command
#define REGMAP_NBCMD             6

// Register definitions
#define REGMAP_PRODUCT_ID_LENGTH           (32)
#define REGMAP_PRODUCT_SERIAL_LENGTH       (32)
#define REGMAP_MFG_NAME_LENGTH             (32)
#define REGMAP_SERIAL_NUMBER_LENGTH        (32)
#define REGMAP_GROUP_ID_LENGTH             (32)
#define REGMAP_BUILD_DATE                  (32)
#define REGMAP_FIRMWATE_VERSION_LENGTH     (32)
#define REGMAP_BOOTLOADER_VERSION_LENGTH   (32)
#define REGMAP_ASIC_VERSION_LENGTH         (32)
#define REGMAP_FPGA_VERSION_LENGTH         (32)
#define REGMAP_PRODUCT_NAME_LENGTH         (32)
#define REGMAP_HIGH_SPEED_READ_ADDR_LENGTH (8)

// Product-specific register definitions
#define REGMAP_SEGMENT_COUNT               M7_NB_CHANNELS
#define REGMAP_MAX_ECHOES_PER_CHANNEL      M7_MAX_ECHOES_PER_CHANNEL
#define REGMAP_USR_LED_POWER_COUNT_MAX     M7_NB_USER_LED_POWER_MAX
#define REGMAP_FILTER_COEF_MAX_LENGTH      M7_NB_COEFF_FILTER_MAX
#define REGMAP_FILTER_COEF_COUNT_LENGTH    M7_ACCUMULATION_EXPONENT_MAX
#define REGMAP_AMP_SAT_LUT_LENGTH_MAX      M7_SATURATION_AMP_LUT_SIZE_MAX
#define REGMAP_DIS_SAT_LUT_LENGTH_MAX      M7_SATURATION_DIS_LUT_SIZE_MAX
#define REGMAP_XTALK_NB_TYPE_MAX           M7_XTALK_NB_TYPE_MAX
#define REGMAP_ACCUMULATION_EXPONENT_MAX   M7_ACCUMULATION_EXPONENT_MAX
#define REGMAP_OVERSAMPLING_EXPONENT_MAX   M7_OVERSAMPLING_EXPONENT_MAX
#define REGMAP_DEM_REF_PULSE_MAX_PTS       M7_DEM_REF_PULSE_MAX_PTS
#define REGMAP_DEM_LUT_MAX_PTS             M7_DEM_LUT_MAX_PTS
#define REGMAP_MAX_SAMPLE_PER_CHANNEL      M7_NB_SAMPLES_PER_CHANNEL_MAX
#define REGMAP_PRECISION_LUT_SIZE_MAX      M7_BAYES_STD_LUT_SIZE_MAX
#define REGMAP_MAG_CORR_LUT_SIZE           M7_MAG_CORRECTION_LUT_SIZE_MAX
#define REGMAP_NB_BASE_SAMPLE_MAX_CALIB    M7_NB_BASE_SAMPLE_MAX_CALIB
#define REGMAP_OVERSAMPLING_CALIB          M7_OVERSAMPLING_CALIB
#define REGMAP_NB_USER_LED_POWER_MAX       M7_NB_USER_LED_POWER_MAX
#define REGMAP_NB_USER_FIELD_MAX           M7_NB_USER_FIELD_MAX
#define REGMAP_KEY_LENGTH                  16                  ///< Key length in bytes.

// Register map
//  Bank #,                      Bank Name,                                           Bank Start Address, Bank Size (KB), Data Size (B),                Secure transaction support   Read Access,  Write Access,
#define REGMAP(P, T, I, A, NO)                                                                                                                                                                                 \
{                                                                                                                                                                                                              \
  { REGMAP_CFG_DATA,             "Configuration Data",                                0x00000000,         1024,           sizeof(sCfgData),             1,                           (P | I | A),  (P | I | A)}, \
  { REGMAP_ADV_CFG_DATA,         "Advanced Configuration Data",                       0x00100000,         1024,           sizeof(sAdvCfgData),          1,                           (P | I | A),  (    I | A)}, \
  { REGMAP_PRD_CFG_DATA,         "Product Configuration Data",                        0x00200000,         512,            sizeof(sProductCfgData),      1,                           (P | I | A),  (P | I | A)}, \
  { REGMAP_PRD_ADV_CFG_DATA,     "Product Advanced Config Data",                      0x00280000,         512,            sizeof(sProductAdvCfgData),   1,                           (P | I | A),  (    I | A)}, \
  { REGMAP_LICENSE_KEYS,         "License Key List",                                  0x00300000,         512,            sizeof(sLicenseKeys),         1,                           (P | I | A),  (P | I | A)}, \
  { REGMAP_VOLATILE_LICENSE_KEYS,"Volatile License Key List",                         0x00380000,         512,            sizeof(sVolatileLicenseKeys), 1,                           (P | I | A),  (P | I | A)}, \
  { REGMAP_DEV_INFO,             "Device Information and Constants",                  0x00400000,         128,            sizeof(sDevInfo),             1,                           (P | I | A),  (        A)}, \
  { REGMAP_ADV_DEV_INFO,         "Advanced Device Information and Constants",         0x00420000,         128,            sizeof(sAdvDevInfo),          1,                           (    I | A),  (        A)}, \
  { REGMAP_PRD_DEV_INFO,         "Product Device Information and Constants",          0x00440000,         128,            sizeof(sProductDevInfo),      1,                           (P | I | A),  (        A)}, \
  { REGMAP_PRD_ADV_DEV_INFO,     "Product Advanced Device Information and Constants", 0x00460000,         128,            sizeof(sProductAdvDevInfo),   1,                           (    I | A),  (        A)}, \
  { REGMAP_CMD_LIST,             "Commands and Status",                               0x00480000,         128,            sizeof(sCmdList),             1,                           (P | I | A),  (P | I | A)}, \
  { REGMAP_ADV_CMD_LIST,         "Advanced Commands and Status",                      0x004A0000,         128,            sizeof(sAdvCmdList),          1,                           (T | I | A),  (    I | A)}, \
  { REGMAP_PRD_CMD_LIST,         "Product Commands and Status",                       0x004C0000,         128,            sizeof(sProductCmdList),      1,                           (P | I | A),  (P | I | A)}, \
  { REGMAP_PRD_ADV_CMD_LIST,     "Product Advanced Commands and Status",              0x004E0000,         128,            sizeof(sProductAdvCmdList),   1,                           (    I | A),  (    I | A)}, \
  { REGMAP_DETECTIONS,           "Detection List",                                    0x00500000,         1024,           sizeof(sDetections),          1,                           (P | I | A),  (    NO   )}, \
  { REGMAP_ADV_ALGO,             "Advanced Algorithm Results",                        0x00600000,         1024,           sizeof(sAdvAlgo),             1,                           (T | I | A),  (    NO   )}, \
  { REGMAP_ADV_CFG_DATA_NO_INIT, "Advanced Configuration Data No Init",               0x00700000,         1024,           sizeof(sAdvCfgDataNoInit),    1,                           (    I | A),  (    I | A)}, \
  { REGMAP_TRACES,               "Leddar Trace",                                      0x00800000,         2048,           sizeof(sTraces),              1,                           (T | I | A),  (    NO   )}, \
  { REGMAP_FPGA,                 "Fpga",                                              0x00A00000,         512,            sizeof(sFpga),                1,                           (    I | A),  (    I | A)}, \
  { REGMAP_ASIC,                 "Asic patch",                                        0x00A80000,         512,            sizeof(sAsic),                1,                           (P | I | A),  (P | I | A)}, \
  { REGMAP_BACKUP,               "Backup",                                            0x00B00000,         128,            sizeof(sBackupData),          1,                           (    I | A),  (    NO   )}, \
  { REGMAP_TRN_CFG,              "Transaction Configuration",                         0x00FFFB00,         1,              sizeof(sTransactionCfg),      0,                           (P | I | A),  (P | I | A)}, \
}

//  Command #,     Command Name,        Access rights    Write enable 
//                                                       prerequisite 
#define CMDMAP(P, I, A)                                         \
{                                                               \
  { REGMAP_WRDIS,  "Write Disable",     P | I | A,       0},    \
  { REGMAP_WREN ,  "Write Enable",      P | I | A,       0},    \
  { REGMAP_CBAK ,  "Create Backup",     P | I | A,       1},    \
  { REGMAP_DBAK ,  "Delete Backup",     P | I | A,       1},    \
  { REGMAP_SWRST,  "Software Reset",    P | I | A,       0},    \
  { REGMAP_CE   ,  "Chip Erase",        P | I | A,       1},    \
}

//*****************************************************************************
//*************** Data Type Definition ****************************************
//*****************************************************************************

/// \struct sRegisterDes
/// \brief  Register description structure.
typedef struct
{
  uint32_t mLogicalAddr;
  uint32_t mValue;
  uint8_t  mModified;
} sRegisterDes;

/// \enum   eRegisterAccess 
/// \brief  Register access right enumeration.
typedef enum
{
  REGMAP_PRIMARY_KEY_NO         = 0x0000,  //< No access right access specified.
  REGMAP_PRIMARY_KEY_ADMIN      = 0x0001,  //< Administrator read right access.
  REGMAP_PRIMARY_KEY_INTEGRATOR = 0x0002,  //< Integrator read right access.
  REGMAP_PRIMARY_KEY_PUBLIC     = 0x0004,  //< Public read right access.
  REGMAP_PRIMARY_KEY_TRACE      = 0x0008,  //< Trace read right access
  REGMAP_PRIMARY_KEY_COUNT      = 3,
} eRegisterAccess;

/// \enum eLicenseType
/// \brief License type enumeration.
typedef enum
{
  REGMAP_SECONDARY_KEY_SERIAL_NUMBER = 0,
  REGMAP_SECONDARY_KEY_GROUP_ID      = 1,
  REGMAP_SECONDARY_KEY_COUNT         = 2,
} eLicenseType;

///\ enum eTrnInfo
///\brief Command information tag.
typedef enum
{
  REGMAP_NO_ERR                 = 0,
  REGMAP_ACCESS_RIGHT_VIOLATION = 1 << 0,
  REGMAP_INVALID_ADDR           = 1 << 1,
  REGMAP_CMD_NOT_FOUND          = 1 << 2,
  REGMAP_WRITE_DISABLE          = 1 << 3,
  REGMAP_CRC_FAILED             = 1 << 4,
  REGMAP_CMD_EXEC_ERROR         = 1 << 5,
  REGMAP_INVALID_PACKET         = 1 << 6,
} eTrnInfo;

///\ enum eCmd
///\brief List of commands.
typedef enum
{
  REGMAP_WRDIS = 0x04,
  REGMAP_WREN  = 0x06,
  REGMAP_CBAK  = 0x57,
  REGMAP_DBAK  = 0x5E,
  REGMAP_SWRST = 0x99,
  REGMAP_CE    = 0xC7,
} eCmd;

///\ enum
typedef enum
{
  REGMAP_CFG_DATA = 0,
  REGMAP_ADV_CFG_DATA,
  REGMAP_PRD_CFG_DATA,
  REGMAP_PRD_ADV_CFG_DATA,
  REGMAP_LICENSE_KEYS,
  REGMAP_VOLATILE_LICENSE_KEYS,
  REGMAP_DEV_INFO,
  REGMAP_ADV_DEV_INFO,
  REGMAP_PRD_DEV_INFO,
  REGMAP_PRD_ADV_DEV_INFO,
  REGMAP_CMD_LIST,
  REGMAP_ADV_CMD_LIST,
  REGMAP_PRD_CMD_LIST,
  REGMAP_PRD_ADV_CMD_LIST,
  REGMAP_DETECTIONS,
  REGMAP_ADV_ALGO,
  REGMAP_ADV_CFG_DATA_NO_INIT,
  REGMAP_TRACES,
  REGMAP_FPGA,
  REGMAP_ASIC,
  REGMAP_BACKUP,
  REGMAP_TRN_CFG,
} eRegisterIdx;

/// \enum  eDeviceStatus
/// \brief Status register flag enumeration
typedef enum
{
  REGMAP_STATUS_NONE           = 0x00,  //< No flag selected.
  REGMAP_STATUS_DEV_BUSY       = 0x00,  //< Device ready: 1 = busy, 0 = ready.
  REGMAP_STATUS_WRITE_ENABLE   = 0x01,  //< Write enable: 1 = write enabled, 0 = write disabled.
  REGMAP_STATUS_NB             = 2,     //< Number of available status bit.
  REGMAP_STATUS_ALL            = 0xFF,  //< All flag selected.
} eDeviceStatus;

/// \enum  eTransactionMode
/// \brief Transaction mode.
typedef enum
{
  REGMAP_FREERUN = 0,
  REGMAP_BLOCKING,
  REGMAP_PARTIAL_BLOCKING,
} eTransactionMode;

/// \enum  eTransactionReadyDeassertingBank
/// \brief Bank that could deassert the mcu ready pin.
typedef enum
{
  REGMAP_READY_DEASSETED_ON_TRACE = 0,
  REGMAP_READY_DEASSETED_ON_DETECTION,
} eTransactionReadyDeassertingBank;

// Callback function pointers
typedef  volatile void  *(*fRegAddCallBack)              (void);
typedef  void            (*fTransactionCompleteCallBack) (void);
typedef  uint8_t         (*fCommandCallBack)             (void);
typedef  void            (*fUnlockCallBack)              (uint32_t aStartAddr, uint32_t aSize);
typedef  uint8_t         (*fLockCallBack)                (uint8_t aFromIsr);

/// \struct sRegMap
/// \brief  Register map structure.
typedef struct
{
  uint32_t         mBank;              //< Bank index
  char             mName[64];          //< Name of the bank
  uint32_t         mStartAddr;         //< Bank logical address
  uint32_t         mSize;              //< Maximum size of the bank
  uint32_t         mDataSize;          //< Size of the data in the bank
  uint32_t         mSecureTrn;         //< Secure transaction support of the bank.
  uint32_t         mReadAccess;        //< Bank read access.
  uint32_t         mWriteAccess;       //< Bank right access.
} sRegMap;

/// \struct sCmdMap
/// \brief  Command map structure.
typedef struct
{
  uint32_t         mCmd;                    //< Command identification number
  char             mName[64];               //< Name of the bank
  uint8_t          mRightAccess;            //< Command right access.
  uint8_t          mWriteEnprerequisite;    //< Write enable prerequisite (write must be enable to execute the command)
} sCmdMap;

#ifdef _LT
/// \struct sREGMAPrivate
/// \brief  Private register map structure.
typedef struct
{
  fRegAddCallBack               mRegAddCallBack;               //< Physical address callback function.
  fTransactionCompleteCallBack  mTransactionCompleteCallBack;  //< Transaction completed callback function.
  fMemcpy                       mMemcpyCallBack;               //< Memory copy callback.
  fUnlockCallBack               mUnlockCallBack;               //< Register unlock callback function.
  fLockCallBack                 mLockCallBack;                 //< Register lock callback function.
} sRegMapPrivate;

/// \struct sCmdMapPrivate
/// \brief  Private command map structure.
typedef struct
{
  fCommandCallBack  mCommandCallBack;  ///< Transaction completed callback function
} sCmdMapPrivate;

#endif

#pragma pack(push, 1) 
/// \struct sDevInfo
/// \brief  Device information data structure.
typedef struct
{
  // ============== Dev Info rev0. =========================
  char      mPartNumber[REGMAP_PRODUCT_ID_LENGTH];              //< Hardware part number
  char      mSoftPartNumber[REGMAP_PRODUCT_ID_LENGTH];          //< Software part number
  char      mSerialNumber[REGMAP_PRODUCT_SERIAL_LENGTH];        //< Serial number
  char      mMfgName[REGMAP_MFG_NAME_LENGTH];                   //< Manufacturer name
  char      mGroupIdenficationNumber[REGMAP_GROUP_ID_LENGTH];   //< Group identification number.
  char      mBuildDate[REGMAP_BUILD_DATE];                      //< Build date
  char      mFirmwareVersion[REGMAP_FIRMWATE_VERSION_LENGTH];   //< Firmware version
  char      mBootldVersion[REGMAP_BOOTLOADER_VERSION_LENGTH];   //< Bootloader version
  char      mASICVersion[REGMAP_ASIC_VERSION_LENGTH];           //< ASIC version
  char      mFPGAVersion[REGMAP_FPGA_VERSION_LENGTH];           //< FPGA version
  uint16_t  mDeviceType;                                        //< Device type.
  uint32_t  mOptions;                                           //< Option bits

  // Grabber constants
  uint8_t   mAccExpMin;                                         //< Minimum accumulation exponent
  uint8_t   mAccExpMax;                                         //< Maximum accumulation exponent
  uint8_t   mOvrExpMin;                                         //< Minimum oversampling exponent
  uint8_t   mOvrExpMax;                                         //< Maximum oversampling exponent
  uint8_t   mBasePointMin;                                      //< Minimum base point count
  uint8_t   mBasePointMax;                                      //< Maximum base point count
  uint16_t  mNbVerticalSegment;                                 //< Number of segments along the vertical
  uint16_t  mNbHonrizontalSegment;                              //< Number of segments along the horizontal
  uint16_t  mNbRefSegment;                                      //< Number of reference segments  
  uint32_t  mBaseSplDist;                                       //< Distance between two base sample points in meters 
  uint32_t  mRefSegMask;                                        //< Bit field indicating which segments are used as reference.
  uint16_t  mNbSampleMax;                                       //< Maximum number of samples
  uint8_t   mRefreshRateScale;                                  //< Refresh rate scale
  uint32_t  mGrabClockFreq;                                     //< clock frequency of the grabber

  // Peak detector                                              
  uint8_t  mDetectionPerSegmentCountMax;                        //< Number of detection per segment
  uint32_t mDistanceScale;                                      //< Distance scale
  uint8_t  mRawAmplitudeScaleBits;                              //< Raw Amplitude scale bits
  uint32_t mRawAmplitudeScale;                                  //< Amplitude scale
  int16_t  mPrecisionMin;                                       //< Minimum precision setting
  int16_t  mPrecisionMax;                                       //< Maximum precision setting
  int32_t  mSensitivitytMin;                                    //< Minimum threshold offset
  int32_t  mSensitivitytMax;                                    //< Maximum threshold offset
                                                                
  // LED power                                                  
  uint8_t   mUsrLedPowerCountMax;                               //< Maximum number of user LED power  
  uint16_t  mLedUserAutoFrameAvgMin;                            //< Minimum number of frame to average for automatic led control.
  uint16_t  mLedUserAutoFrameAvgMax;                            //< Maximum number of frame to average for automatic led control.
  uint8_t   mLedUserPowerPercentMin;                            //< Minimum percent authorized.
  uint8_t   mLedUserPowerPercentMax;                            //< Maximum percent authorized.
  uint8_t   mLedUserAutoEchoAvgMin;                             //< Maximum number of echo to average for automatic led control.
  uint8_t   mLedUserAutoEchoAvgMax;                             //< Minimum number of echo to average for automatic led control.

  // Static noise removal
  uint8_t mStNoiseRmvCalibBy;                                   //< Indicate if the static noise has been calibrate the client or not (0 = client, 1 = production), is in error or never calibrate .

  // State
  uint32_t mCpuLoadScale;                                        //< Scale factor of the cpu load.
  uint32_t mTempScale;                                           //< Scale fadctor of the temperature
  // ============== Dev Info revX. =========================
  // Insert new data up of this line and increment DEV_INFO_VERSION

} sDevInfo;

/// \struct sLicenseKeys
/// \brief  List of license key
typedef struct
{

  // ============== Licence Keys rev0 =========================
  uint8_t mKey1[REGMAP_KEY_LENGTH];        //< Administrator key.
  uint8_t mKey2[REGMAP_KEY_LENGTH];        //< Integrator key.
  uint8_t mKey3[REGMAP_KEY_LENGTH];        //< Trace key.

  // ============== Licence Keys revX =========================
  // Insert new data up of this line and increment LICENCE_KEY_VERSION

} sLicenseKeys;

/// \struct sVolatileLicenseKeys
/// \brief  This copy is necessary during the compilation of Leddar Configurator
typedef struct
{

  // ============== Licence Keys rev0 =========================
  uint8_t mKey1[REGMAP_KEY_LENGTH];        //< Administrator key.
  uint8_t mKey2[REGMAP_KEY_LENGTH];        //< Integrator key.
  uint8_t mKey3[REGMAP_KEY_LENGTH];        //< Trace key.

  // ============== Licence Keys revX =========================
  // Insert new data up of this line and increment LICENCE_KEY_VERSION

} sVolatileLicenseKeys;

/// \struct sCmdList
/// \brief  Command list structure.
typedef struct
{
  // ============== Cmd List rev0. =========================
  // Algorithm state and control
  uint8_t  mDetectionReady;
  uint32_t mStaticNoiseRmvErrorFlag;        //< Static noise removal initialization flag.
  uint64_t mPeakDetectorErrorFlag;          //< Peak detector initialization flag.
  uint32_t mLedPowerCtrlErrorFlag;          //< Led power control initialization flag.
  uint32_t mXtalkRmvErrorFlag;              //< xtalk traces removal initialization flag.
  uint32_t mDemergedErrorFlag;              //< demerged initialization flag

  // Thread states
  uint32_t mAcquistionErrorCount;          //< Acquisition error counter.
  uint16_t mAcquisitionState;              //< Acquisition state.
  uint16_t mProcessingState;               //< Processing state.
  uint32_t mAsicTransactionCount;          //< Asic transaction counter.

  // System information
  uint32_t mCpuUsage;                      //< CPU usage.
  uint32_t mWdtTriggered;                  //< number of watchdog trigger.
  uint16_t mBankAccessRights;              //< Bank access right.
  uint32_t mLicenceInfo[3];                //< License information (bit 0-15: Access rights of the license, bit 16-17: License type).
  uint32_t mLicenceInfoVolatile[3];        //< Volatile License information (bit 0-15: Access rights of the license, bit 16-17: License type).
  uint32_t mRefreshRate;                   //< Current refresh rate.
  
  // ============== Cmd List rev1. =========================
  uint32_t mBinaryCrc32;                   //< CRC32 of application firmware binary calculated at boot-up.
  uint16_t mBankVolatileAccessRights;      //< Bank access right.
  
  // ============== Cmd List rev2. =========================
  uint32_t mBackupStatus;                  //< Backup status: 0=invalid; 1=factory backup; 2=user backup
  uint32_t mBankReadError;                 //< Bank read error counter during bank update
  uint32_t mBankWriteError;                //< Bank write error counter during bank update
  
  // ============== Cmd List rev3. =========================
  uint32_t mStaticNoiseRmvCalibErrorFlag;  //< Static noise removal calibration error flag.
  
  // ============== Cmd List revX. =========================
  // Insert new data up of this line.

} sCmdList;

/// \struct sTransactionCfg
/// \brief  Transaction configuration
typedef struct
{
  // ============== Transaction Cfg rev0. =========================
  uint8_t  mSecureTransferEnableFlag; //< Secure transfer enable flag.
  uint8_t  mTransferMode;             //< Transfer mode (0 = free run, 1 = blocking read, 2 = partial blocking read)
  uint16_t mTransactionCrc;           //< crc of the last transaction.
  uint16_t mTransactionInfo;          //< Information about the last transaction.
  uint8_t  mReadyDeassertingData;      //< register that deasserts the ready pin (0 = trace, 1 = detection).
  
  // ============== Transaction Cfg revX. =========================
  // Insert new data up of this line.
  
} sTransactionCfg;

/// \struct sAdvCmdList
/// \brief  Command list structure.
typedef struct
{
  // ============== Adv Cmd List rev0. =========================
  uint8_t  mStaticNoiseRmvCalibTrigger;    //< Trigger sent when a calibration is asked by an user.
  uint8_t  mStaticNoiseRmvStatus;          //< Calibration status giving an information on the channel currently in learning.
  uint8_t  mRefPulseCalibTrigger;          //< Trigger to send to perform a reference pulse calibration.
  int8_t   mRefPulseCalibStatus;           //< Status of the reference pulse calibration.
  uint8_t  mTraceReady;
  uint8_t  mAlgoReady;
  
  // ============== Adv Cmd List rev1. =========================
  uint32_t mBackupCmd;                     //< Backup command or argument
  
  // ============== Adv Cmd List rev2. =========================
  uint8_t  mTempCompCalibTrigger;          //< Trigger to send to perform a temperature compensation calibration.
  int8_t   mTempCompCalibStatus;           //< Status of the temperature compensation calibration.
  
  // ============== Adv Cmd List revX. =========================
  // Insert new data up of this line.
  
} sAdvCmdList;

/// \struct sCfgData
/// \brief  Configuration data structure
typedef struct
{
  // ============== Config Data rev0. =========================
  // Device
  uint8_t  mDeviceName[REGMAP_PRODUCT_NAME_LENGTH]; //< Device name

  // Grabber
  uint8_t   mAccumulationExp;                       //< Number of accumulation.
  uint8_t   mOversamplingExp;                       //< Number of oversampling.
  uint8_t   mBasePointCount;                        //< Number of base points.
  uint32_t  mSegmentEnable;                         //< Segment enable.
  uint32_t  mRefPulseRate;                          //< Acquisition rate of the reference pulse.
  
  // Sensor characteristics
  float     mYaw;               //< Yaw angle of the sensor.
  float     mPitch;             //< Pitch angle of the sensor.
  float     mRoll;              //< Roll angle of the sensor.
  float     mX;                 //< x position of the sensor.
  float     mY;                 //< y position of the sensor.
  float     mZ;                 //< z position of the sensor.

  // Peak detector
  int8_t    mPrecision;                  //< Precision of the detector. If the parameter is < -16 or > 16, the algorithm is disabled, otherwise a small number means less stability and a high number means more stability.
  uint8_t   mPrecisionEnable;            //< Enable or disable the precision algorithm. 
  uint8_t   mSatCompEnable;              //< Saturation compensation enable
  uint8_t   mOvershootManagementEnable;  //< Overshoot management enable
  int32_t   mSensitivity;                //< Sensitivity setting.

  // Led Power
  uint8_t   mLedUserCurrentPowerPercent; //< Current User led power level [in percent]
  uint8_t   mLedUserAutoPowerEnable;     //< Enable the auto LED power algorithm.
  uint16_t  mLedUserAutoFrameAvg;        //< Modifies the responsivity of the auto LED power algorithm according to the number of frames.
  uint8_t   mLedUserAutoEchoAvg;         //< Modifies the responsivity of the auto LED power algorithm according to the number of echoes.

  // Demerge
  uint8_t   mDemEnable;                  //< Object demerging enable.

 // Static noise removal
  uint8_t   mStNoiseRmvEnable;           //< Static noise removal enable
 
  // ============== Config Data revX. =========================
  // Insert new data up of this line and increment CFG_DATA_VERSION
  
} sCfgData;

/// \struct sAdvDevInfo
/// \brief  Advanced device information data structure
typedef struct
{
  // ============== Adv Dev Info rev0. =========================
  // Grabber
  uint8_t   mGrbSamplingOffsetMin;        //< Minimum sampling delay.
  uint8_t   mGrbSamplingOffsetMax;        //< Maximum sampling delay.
  uint32_t  mGrbGainMin;                  //< Minimum gain value.
  uint32_t  mGrbGainMax;                  //< Maximum gain value.
  uint8_t   mGrbPwmPulseCountMax;         //< Number of pwm pulses.
  uint8_t   mGrbPWMWidthsMin;             //< Minimum number of clock cycles for each PWM pulse.
  uint8_t   mGrbPWMWidthsMax;             //< Maximum number of clock cycles for each PWM pulse.
  uint16_t  mGrbPWMPeriodMin;             //< Minimum time for one PWM cycle.
  uint16_t  mGrbPWMPeriodMax;             //< Maximum time for one PWM cycle.
  uint32_t  mGrbTriggerLengthMin;         //< Allows to adjust the length of the trigger by steps of 1/8 of sampling period.
  uint32_t  mGrbTriggerLengthMax;         //< Allows to adjust the length of the trigger by steps of 1/8 of sampling period.
  uint16_t  mGrbScanTimeMin;              //< Duration of one scan.
  uint16_t  mGrbScanTimeMax;              //< Duration of one scan.

  // Peak detector
  uint8_t mPeakFilterCoefsCountMax;       //< Maximum number of filter coefficients.
  uint8_t mPeakRefFactorMax;              //< factor scale max for reference pulse
  uint8_t mPeakRefFactorMin;              //< factor scale min for reference pulse
  // Saturation
  uint8_t mSatLutLengthMax;               //< Maximum length of the saturation luts.

  // Crosstalk echoes removal             
  uint8_t mXtalkTypeCountMax;             //< Maximum number of crosstalk type.

  // Demerge
  uint16_t mDemRefPulsePtsCountMax;       ///< Maximum number of point of reference pulse template.
  uint16_t mDemLutLengthMax;              ///< Maximum length of the demerge lut.
  uint16_t mDemMaxSplInMergePulse;        ///< Maximum number of sample in the merged pulse
  uint16_t mDemRefPulseMaxNbPts;          ///< Maximum number of coefficients in the reference pulse.
  uint8_t  mDemLutMaxNbPts;               ///< Maximum number of coefficients in the LUT.

  // Led Power
  uint8_t mLedUserNbFieldMin;             //< Minimum number of field used to configure the Led Power controller (see Grabber)
  uint8_t mLedUserNbFieldMax;             //< Maximum number of field used to configure the Led Power controller (see Grabber)
  uint16_t mLedUserLowAmplitudeMin;       //< Minimum low amplitude authorized where led power automatic snaps
  uint16_t mLedUserLowAmplitudeMax;       //< Maximum low amplitude authorized where led power automatic snaps
  uint16_t mLedUserHighAmplitudeMin;      //< Minimum high amplitude authorized where led power automatic snaps
  uint16_t mLedUserHighAmplitudeMax;      //< Maximum high amplitude authorized where led power automatic snaps
  
  // Static noise removal
  uint16_t mStNoiseRmvNbBaseSplCountMin;  //< Minimum base sample count authorized during calibration
  uint16_t mStNoiseRmvNbBaseSplCountMax;  //< Maximum base sample count authorized during calibration
  uint16_t mStNoiseRmvNbFrmMin;           //< Minimum number of frames authorized during calibration
  uint16_t mStNoiseRmvNbFrmMax;           //< Maximum number of frames authorized during calibration
  uint16_t mStNoiseRmvThresholdMeanMin;   //< Minimum threshold mean authorized during trace update
  uint16_t mStNoiseRmvThresholdMeanMax;   //< Maximum threshold mean authorized during trace update
  uint16_t mStNoiseRmvDetThrMin;          //< Minimum Threshold for update
  uint16_t mStNoiseRmvDetThrMax;          //< Maximum Threshold for update
  
  // ============== Adv Dev Info revX. =========================
  // Insert new data up of this line and increment ADV_DEV_INFO_VERSION
  
} sAdvDevInfo; 

/// \struct sAdvCfgData
/// \brief  Configuration data structure
typedef struct
{
  // ============== Adv Cfg Data rev0. =========================
  // Trace buffer type
  uint8_t   mTraceBufferType;            //< Select the contain of the trace buffer.

  // Sensor characteristics
  uint32_t  mFieldOfView;                                    //< Field of view of the sensor

  // Static Noise removal
  uint16_t mStNoiseRmvNbBaseSplCount;                        //< Number of base sample used during a learning
  uint16_t mStNoiseRmvThresholdMean;                         //< Factor of averaging during an update
  uint16_t mStNoiseRmvWinShift;                              //< Shift the window where the static noise is compensated
  uint16_t mStNoiseRmvNbFrm;                                 //< Number of frame used for learn the static noise removal
  uint8_t  mStNoiseRmvSideBaseNb;                            //< Number of sample used to compute the right and left level of the base
  uint8_t  mStNoiseRmvUpdateEnable;                          //< Update function of static noise removal enable
  uint16_t mStNoiseRmvDetThr;                                //< Threshold for update
  uint8_t  mStNoiseRmvEnableRefPulse;                        //< Static noise removal enable for reference pulse

  // Peak detector
  uint8_t  mPeakHalfPulseWidth[(REGMAP_OVERSAMPLING_EXPONENT_MAX+1)];                                                   //< Half pulse width.
  uint32_t mPeakPulseWidthLimit[2];                                                                                     //< Pulse width limit.
  uint8_t  mPeakCheckPulseWidthEnable;                                                                                  //< Check pulse width enable.                               
  uint8_t  mPeakFilterSumBits;                                                                                          //< Log2 of the sum of the filter coefficients
  int16_t  mPeakFilterCoefs[(REGMAP_OVERSAMPLING_EXPONENT_MAX+1)][REGMAP_FILTER_COEF_MAX_LENGTH];                       //< Smoothing filter coefficients as a function of the oversampling.
  uint8_t  mPeakFilterNbCoefs[(REGMAP_OVERSAMPLING_EXPONENT_MAX+1)];                                                    //< Number of coefficients of the smoothing filter as a function of the oversampling.
  uint8_t  mPeakFilterOffsets[(REGMAP_OVERSAMPLING_EXPONENT_MAX+1)];                                                    //< Filter offset
  uint16_t mPeakStaticNoiseDetectionThr[REGMAP_MAX_SAMPLE_PER_CHANNEL];                                                 //< Static detection threshold as a function of the sample index (maximum oversampling).
  uint16_t mPeakRandomNoiseDetectionThr[(REGMAP_OVERSAMPLING_EXPONENT_MAX+1)][(REGMAP_ACCUMULATION_EXPONENT_MAX+1)];    //< Random detection threshold.
  uint16_t mPeakNoiseLevel;                                                                                             //< Trace noise level used in local maximum detection algorithm.
  int32_t  mPeakNegDistanceThr;                                                                                         //< Threshold under which the distance is set to zero.
  int32_t  mPeakCalibrationOffset[REGMAP_SEGMENT_COUNT];                                                                //< Calibration offsets for each segment.
  int32_t  mPeakCalibrationLed[REGMAP_USR_LED_POWER_COUNT_MAX];                                                         //< Calibration offsets for each user LED power.
  int32_t  mPeakRealDistanceOffset;                                                                                     //< Real distance offset  
  uint8_t  mPeakNbSampleForBaseLevEst;                                                                                  //< Number of sample used to compute the base level
  uint32_t mRefPulseThr;                                                                                                //< Reference pulse Threshold
  int32_t  mPeakCalibrationRefDist[REGMAP_USR_LED_POWER_COUNT_MAX];                                                     //< Calibration Reference distance 
  uint8_t  mPeakRefDistEnable;                                                                                          //< Reference pulse enable
  uint8_t  mPeakTempEnable;                                                                                             //< Temperature enable
  uint8_t  mPeakOutputRefEchoes;                                                                                        //< Provide echoes detected in the reference trace.
  uint8_t  mPeakRefFactor;                                                                                              //< factor for reference pulse
  
  // Saturation                                                                              
  uint16_t mSatValue;                                                                        //< Value at which the trace is saturated.
  uint32_t mSatAmpLutLength;                                                                 //< Length of the saturation amplitude lookup table.
  int32_t  mSatAmpLut[REGMAP_AMP_SAT_LUT_LENGTH_MAX][3];                                     //< Amplitude versus saturation width lookup table.
  uint8_t  mSatDisLutLength;                                                                 //< Length of the saturation distance lookup table.
  int32_t  mSatDisLut[REGMAP_USR_LED_POWER_COUNT_MAX][REGMAP_DIS_SAT_LUT_LENGTH_MAX][3];     //< Distance offset versus saturation width lookup table.
  uint32_t mSatWidthSwitch;                                                                  //< Saturation width above which the saturation compensation is used.
  uint8_t  mSatOversamExpSwitch;                                                             //< Oversampling exponent above which the saturation compensation is used.
  uint8_t  mSatSubSampAccuracyLimit;                                                         //< Accuracy limit of the saturation width sub pixel estimation.
  
  // Overshoot and undershoot
  uint16_t mOvershootThr[2];          //< Minimum amplitude at which a pulse can have  overshoot.
  uint8_t  mOvershootNegRatio;        //< Overshoot negative ratio.
  uint8_t  mOvershootAmpRatio;        //< Amplitude ratio between overshoot peaks and the pulse.
  uint32_t mOvershootRange[2];        //< Maximum distance between the pulse and the overshoot peaks.
  uint16_t mOvershootAmp;             //< Amplitude of the overshoot pulse
  uint8_t  mOvershootElectronicRatio; //< Amplitude ratio of the overshoot pulse cause by electronics
  uint16_t mOvershootElectronicAmp;   //< Amplitude of the overshoot pulse cause by electronics

  // Crosstalk echoes removal
  uint32_t mXtalkScanLimits[2];                                         //< Scale limit around the aggressor pulse.
  uint8_t  mXtalkTypeCount;                                             //< Number of crosstalk types.
  uint16_t mXtalkTypeLut[REGMAP_SEGMENT_COUNT][REGMAP_SEGMENT_COUNT];   //< Crosstalk type as a function of the segment index lookup table.
  uint16_t mXtalkGains[2][REGMAP_XTALK_NB_TYPE_MAX];                    //< Gain as a function of the crosstalk type lookup table.
  uint8_t  mXtalkAmpPenalty;                                            //< Amplitude penalty applied on pulses that have the shape of a crosstalk signal.
  uint16_t mXtalkShapeCriteriaThr;                                      //< Crosstalk shape criteria threshold.
  uint8_t  mXtalkShapeCriteriaFac;                                      //< Crosstalk shape criteria factor.
  uint32_t mXtalkAmplitudeThrAgg;                                       //< Minimum amplitude of an aggressor pulse to generate crosstalk.
  uint32_t mXtalkAmplitudeThrVic;                                       //< Maximum amplitude amplitude of a pulse to be considered as a crosstalk.
  
  // Precision
  uint8_t  mPrecisionNoiseLutSize;                                                                      //< Noise lut size
  int32_t  mPrecisionNoiseLut[(REGMAP_ACCUMULATION_EXPONENT_MAX+1)][(3*REGMAP_PRECISION_LUT_SIZE_MAX)]; //< Distance variance as a function of the oversampling and the accumulation.
  uint16_t mPrecisionMaxAmp;          //< Saturate the echo amplitude at this value before lookup.
  uint32_t mPrecisionDisVarMin;       //< Minimum distance state variance.
  uint32_t mPrecisionMesVarMin;       //< Minimum distance measurement variance.
  uint32_t mPrecisionOutlierThr;      //< Outlier rejection threshold.
  uint8_t  mPrecisionIniDisStdFac;    //< Distance state initial standard deviation factor (this factor times the measurement std. dev. gives the initial state covariance).
  uint16_t mPrecisionAmpSat;          //< Amplitude saturation for measurement standard deviation lookup.
  
  // Temperature compensation
  int32_t mTempComRef;                                                               //< Temperature set point
  int32_t mTempComSlope;                                                             //< Temperature compensation slope

  // Magnitude compensation
  uint8_t   mAmpCorrEnable;                                                          //< Enable distance compensation based on amplitude
  uint8_t   mAmpCorrLutSize;                                                         //< Size of the amplitude compensation lookup table
  int32_t   mAmpCorrLut[REGMAP_MAG_CORR_LUT_SIZE][3];                                //< Amplitude versus distance correction lookup table
                                                                                     
  // LED power                                                                       
  uint8_t  mLedUserPowerEnable;                                                      //< Use or not the user led power levels.
  uint8_t  mLedUserNbField;                                                          //< Number of field currently used to configure the Led Power controller (see Grabber)
  uint8_t  mLedUsrPowerCount;                                                        //< Number of user LED power.
  uint8_t  mLedUserPowerLut[REGMAP_NB_USER_LED_POWER_MAX][REGMAP_NB_USER_FIELD_MAX]; //< User LED configuration asic lookup table.
  uint32_t mLedUserLowAmplitude;                                                     //< Amplitude minimum before activation of auto led power
  uint32_t mLedUserHighAmplitude;                                                    //< Amplitude maximum before activation of auto led power
  uint32_t mLedUserPowerLutDescription[REGMAP_NB_USER_FIELD_MAX];                    //< Description of the field used in the LUT
  uint8_t  mLedUserPowerLutDescriptionNb[REGMAP_NB_USER_FIELD_MAX];                  //< Description of the number of field used in the LUT
  uint8_t  mLedUserPercentLut[REGMAP_NB_USER_LED_POWER_MAX];                         //< User LED percent power lookup table.
                
  // Object demerging
  int32_t   mDemAmpThrMin;                                                                               //< Minimum amplitude threshold
  uint32_t  mDemAmpPulseThrNearest;                                                                      //< Amplitude threshold of the nearest pulse
  uint32_t  mDemAmpPulseThr;                                                                             //< Amplitude threshold of the pulse
  int32_t   mDemResidueThreshold;                                                                        //< Residue threshold
  int32_t   mDemLvmLambda;                                                                               //< Lambda parameter used by Levenberg Marquardt method
  int32_t   mDemObservabilityThreshold;                                                                  //< Decide if a demerge is possible or not
  int32_t   mDemOptiCompIdxThr;                                                                          //< Remove nonlinear part of a pulse
  uint8_t   mDemMode;                                                                                    //< Mode used (free for all, punctual or any background)
  int32_t   mDemThrMode2;                                                                                //< Threshold used by mode 2
  int32_t   mDemCorrectionRawVsFilteredTemplate[REGMAP_USR_LED_POWER_COUNT_MAX];                         //< Correction of the central point between filtered and raw template in a reference pulse
  uint16_t  mRefPulseMaxPts;                                                                             //< Number of samples located in the templates
  uint16_t  mDemRefPulseNbPts           [REGMAP_USR_LED_POWER_COUNT_MAX];                                //< Number of coefficients in a reference pulse
  uint16_t  mDemRefPulseMaxIdx          [REGMAP_USR_LED_POWER_COUNT_MAX];                                //< Central point (peak) in a reference pulse
  uint32_t  mDemRefPulseMaxInterpIdx    [REGMAP_USR_LED_POWER_COUNT_MAX];                                //< Interpolation of the central point (peak) in a reference pulse
  uint16_t  mDemRefFilteredPulseNbPts   [REGMAP_USR_LED_POWER_COUNT_MAX];                                //< Number of coefficient in a filtered reference pulse
  uint16_t  mDemRefFilteredPulseMaxIdx  [REGMAP_USR_LED_POWER_COUNT_MAX];                                //< Central point (peak) in a reference filtered pulse
  uint32_t  mDemRefFilteredPulseMaxInterpolIdx[REGMAP_USR_LED_POWER_COUNT_MAX];                          //< Interpolation of the central point (peak) in a reference filtered pulse.
  int8_t    mDemRefGapIdx               [REGMAP_USR_LED_POWER_COUNT_MAX];                                //< Gap between central point in a reference filtered pulse and reference raw pulse
  int32_t   mDemBgPosition              [REGMAP_SEGMENT_COUNT];                                          //< Position of each background known.
  uint16_t  mDemTtLutSize;                                                                               //< Number of coefficients in the LUT.
  uint8_t   mDemRightThr;                                                                                //< amplitude threshold used on the right of a pulse
  uint8_t   mDemThrIdx;                                                                                  //< threshold index to cancel or not a far away demerged
  uint16_t  mDemThrPulseWidth;                                                                           //< threshold index on the width of a pulse
  uint8_t   mDemObsOvThr;                                                                                //< threshold on the observability of the problem
  
  // Reference pulse calibration
  uint16_t mCalrNumFrameTempSet;                                                                         //< Number of frame to consider that the temperature is stabilized.
  uint16_t mCalrNumFrameTempUnSet;                                                                       //< Number of frame before failing if the temperature never reach steady state.
  int32_t  mCalrRefTempMin;                                                                              //< Minimum temperature set point that can be used during the reference pulse calibration.
  uint8_t  mCalrTempFeedbackGain;                                                                        //< Temperature feedback controller gain.
  uint8_t  mCalrNumFrameAcc;                                                                             //< Number of frame used for reference pulse distance averaging.

  // ============== Adv Cfg Data rev1. =========================
  // Peak detector
  int32_t  mPeakCalibTempCompOffset[REGMAP_SEGMENT_COUNT];                                               //< Calibration offsets for each segment for temperature Compensation.
  
  // ============== Adv Cfg Data revX. =========================
  // Insert new data up of this line and increment ADV_CFG_DATA_VERSION

} sAdvCfgData;

/// \struct sAdvCfgDataNoInit
/// \brief  Template, look up table without initialization
typedef struct
{

  // ============== Adv Cfg Data No init rev0. =========================
  int16_t   mStaticNoiseRmv     [REGMAP_SEGMENT_COUNT*REGMAP_NB_USER_LED_POWER_MAX*REGMAP_NB_BASE_SAMPLE_MAX_CALIB*REGMAP_OVERSAMPLING_CALIB];
  uint16_t  mDemRefPulse        [REGMAP_USR_LED_POWER_COUNT_MAX][REGMAP_DEM_REF_PULSE_MAX_PTS]; //< Coefficient of the reference pulse as a function of the LED power
  int16_t   mDemRefPulseDiff    [REGMAP_USR_LED_POWER_COUNT_MAX][REGMAP_DEM_REF_PULSE_MAX_PTS]; //< Coefficient of the derivative reference pulse as a function of the LED power
  uint16_t  mDemRefFilteredPulse[REGMAP_USR_LED_POWER_COUNT_MAX][REGMAP_DEM_REF_PULSE_MAX_PTS]; //< Coefficient of the filtered pulse as a function of LED power
  float     mDemTTLut           [REGMAP_USR_LED_POWER_COUNT_MAX][REGMAP_DEM_LUT_MAX_PTS][4];
  float     mDemTDTLut          [REGMAP_USR_LED_POWER_COUNT_MAX][REGMAP_DEM_LUT_MAX_PTS*2][4];
  float     mDemDTDTLut         [REGMAP_USR_LED_POWER_COUNT_MAX][REGMAP_DEM_LUT_MAX_PTS][4];

  // ============== Adv Cfg Data No init revX. =========================
  // Insert new data up of this line

} sAdvCfgDataNoInit;

/// \struct sDetections
/// \brief  List of detection
typedef struct
{

   // ============== Detections rev0. =========================
  uint32_t    mTimestamp;                                                      //< Detection timestamp.
  uint16_t    mNbDetection;                                                    //< Number of detections in the scan.
  uint16_t    mCurrentUsrLedPower;                                             //< Current LED power.
  uint32_t    mAcquistionOptions;                                              //< Acquisition options
  sEchoLigth  mEchoes[REGMAP_MAX_ECHOES_PER_CHANNEL*REGMAP_SEGMENT_COUNT];     //< Detection data (distance, segment index, ...)

  // ============== Detections revX. =========================
  // Insert new data up of this line and increment DETECTION_VERSION

} sDetections;

/// \struct sTraces
/// \brief  Leddar traces
typedef struct
{

 // ============== Traces rev0. =========================
  uint32_t    mTimestamp;                                                       //< Trace timestamp.
  uint16_t    mTraceCount;                                                      //< Number of traces.
  uint16_t    mTraceLength;                                                     //< Length of the trace data.
  uint16_t    mAccExp;                                                          //< number of accumulation (2^mAccExp)
  uint8_t     mOvrExp;                                                          //< number of oversampling (2^mOvrExp)
  uint8_t     mLedPowIdx;                                                       //< index of Led Power in user table
  uint8_t     mCurrentUserLedPower;                                             //< percent of Led Power in user table.
  uint8_t     mIsRefTrace;                                                      //< Indicate if the last trace is used has reference.
  uint32_t    mSegEnable;                                                       //< Indicate which segment are enabled.
  uint16_t    mAcqReg;                                                          //< Acquisition register.
  uint16_t    mTraces[REGMAP_MAX_SAMPLE_PER_CHANNEL*REGMAP_SEGMENT_COUNT];      //< Trace data.
  
  // ============== Traces revX. =========================
  // Insert new data up of this line and increment TRACE_VERSION

} sTraces;

/// \struct sAdvAlgo
/// \brief  Leddar traces
typedef struct
{

  // ============== Adv Algo rev0. =========================
  uint32_t    mTimestamp;                                                      //< Trace timestamp.
  uint16_t    mTraceCount;                                                     //< Number of traces.
  uint16_t    mTraceLength;                                                    //< Length of the trace data.
  uint16_t    mAccExp;                                                         //< number of accumulation (2^mAccExp).
  uint8_t     mOvrExp;                                                         //< number of oversampling (2^mOvrExp).
  uint8_t     mLedPowIdx;                                                      //< index of Led Power in user table.
  uint8_t     mCurrentUserLedPower;                                            //< percent of Led Power in user table.
  uint8_t     mIsRefTrace;                                                     //< Indicate if the last trace is used has reference.
  uint32_t    mSegEnable;                                                      //< Indicate which segment are enabled.
  uint16_t    mAcqReg;                                                         //< Acquisition register.
  uint16_t    mTraces[REGMAP_MAX_SAMPLE_PER_CHANNEL*REGMAP_SEGMENT_COUNT];     //< Trace data.
  
  // ============== Adv Algo revX. =========================
  // Insert new data up of this line 

} sAdvAlgo;

/// \struct sStNoiRefTrc
/// \brief Static Noise reference trace
typedef struct 
{
  // ============== Static Noise Ref Trc rev0. =========================
  uint16_t    mTraces[REGMAP_SEGMENT_COUNT*REGMAP_USR_LED_POWER_COUNT_MAX*REGMAP_NB_BASE_SAMPLE_MAX_CALIB*REGMAP_OVERSAMPLING_CALIB];
  // ============== Static Noise Ref Trc revX. =========================
  // Insert new data up of this line
} sStNoiRefTrc;


/// \struct sBackupBankInfo
/// \brief Bank information data into backup.
typedef struct
{
    uint16_t    mBankId;        //< Bank ID corresponding to bank backup stored into data section
    uint16_t    mVersion;       //< Version of bank backup
    size_t      mOffset;        //< Offset of bank backup stored into data section
    size_t      mSize;          //< Size of bank backup stored into data section
} sBackupBankInfo;

/// \struct sBackupData
/// \brief Backup bank for data recovery on failure.
typedef struct
{
    // ============== Backup Data rev0. =========================
    char            mFirmwareVersion[REGMAP_FIRMWATE_VERSION_LENGTH];   //< Firmware version used for backup
    uint8_t         mBackupType;                                        //< Backup type, see \ref eBackupType
    uint8_t         mNumberOfBank;                                      //< Number of backup bank stored
    sBackupBankInfo mBackupBankInfo[16];                                //< Information of stored backup of bank into data section
    uint8_t         mReserved[286];                                     //< Reserved section for future use.

    uint8_t         mData[0x8000];                                      //< Data section where are stored backup of banks in the order of mBackupBankInfo
} sBackupData;

typedef enum
{
    REGMAP_BACKUP_INVALID = 0,      //< Invalid backup
    REGMAG_BACKUP_VALID_FACTORY,    //< Valid factory backup
    REGMAP_BACKUP_VALID_USER,       //< Valid user backup
} eBackupType;

#pragma pack(pop)


//*****************************************************************************
//*************** Public Variable Declarations ********************************
//*****************************************************************************

extern volatile const sRegMap gRegMap[REGMAP_NBBANK];
extern volatile const sCmdMap gCmdMap[REGMAP_NBCMD];

#ifdef _LT
extern volatile const sRegMapPrivate gRegMapPrvt[REGMAP_NBBANK];
extern volatile const sCmdMapPrivate gCmdMapPrvt[REGMAP_NBCMD];
#endif

#ifdef __cplusplus
}
#endif

#endif // _REGISTER_MAP_


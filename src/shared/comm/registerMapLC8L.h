// ****************************************************************************
/*!
Module:   Register map.

Platform: Independant

\file     registerMapLC8L.h

\brief    The register map contains the definition of the registers as well
          as the mapping between the logical address and the physical address
          of the register.

\author   Vincent Simard Bilodeau
\author   Frédéric Parent
\author   Samuel Gidel

\since    November 27, 2015

\copyright (c) 2015 LeddarTech Inc. All rights reserved.
*/
// ***************************************************************************

#ifndef  _REGISTER_MAP_PRODUCT_
#define  _REGISTER_MAP_PRODUCT_

#ifdef __GNUC__
  #define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#else
  #define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif

// Standard library includes files
#include <stdint.h>

// Platform include files
#include "PlatformLC08l32DefinitionsShared.h"

//*****************************************************************************
//*************** Constants and Macros ****************************************
//*****************************************************************************

// Product-specific configuration data version
#define PRD_CFG_DATA_VERSION_0          0
#define PRD_CFG_DATA_VERSION            PRD_CFG_DATA_VERSION_0

// Product-specific advanced configuration data version
#define PRD_ADV_CFG_DATA_VERSION_0      0
#define PRD_ADV_CFG_DATA_VERSION_1      1
#define PRD_ADV_CFG_DATA_VERSION        PRD_ADV_CFG_DATA_VERSION_1

// Product-specific device information.
#define PRD_DEV_INFO_VERSION_0          0
#define PRD_DEV_INFO_VERSION            PRD_DEV_INFO_VERSION_0

// Product-specific advanced device information.
#define PRD_ADV_DEV_INFO_VERSION_0      0
#define PRD_ADV_DEV_INFO_VERSION        PRD_ADV_DEV_INFO_VERSION_0

// Asic patch version
#define  ASIC_PATCH_VERSION_0           0
#define  ASIC_PATCH_VERSION             ASIC_PATCH_VERSION_0

// Product-specific register definitions
#define REGMAPP_DIFF_EQ_MAX                 LC08L32_DIFF_EQ_MAX
#define REGMAPP_TEMPLATE_MAX                LC08L32_TEMPLATE_MAX
#define REGMAPP_DIFF_EQ_COEFF_MAX           LC08L32_DIFF_EQ_COEFF_MAX
#define REGMAPP_TEMPLATE_COUNT_MAX          LC08L32_TEMPLATE_COUNT_MAX
#define REGMAP_PWM_WIDTH_LENGTH             LC08L32_NB_PWM_PULSES


//*****************************************************************************
//*************** Data Type Definition ****************************************
//*****************************************************************************

#pragma pack(push, 1)

/// \struct sProductCfgData
/// \brief  Product configuration data structure
typedef struct
{
  // ============== Prod Cfg Data rev0. =========================
  // Echo due to crosstalk removal.
  uint8_t   mXtalkEchoRemovalEnable;     ///< Crosstalk echo removal enable

  // Xtalk trace removal
  uint8_t   mXtalkRmvEnable;            ///< Xtalk trace removal enable
  
  // ============== Prod Cfg Data revX. =========================
  // Insert new data up of this line and increment PRD_CFG_DATA_VERSION

} sProductCfgData;

/// \struct sProductDevInfo
/// \brief  Product configuration data register.
typedef struct
{
  // ============== Prod Dev Info rev0. =========================
  // Grabber info
  uint8_t mTempSensorScaleBits;

  // ============== Prod Dev Info revX. =========================
  // Insert new data up of this line and increment PRD_DEV_INFO_VERSION

} sProductDevInfo;

/// \struct sProductAdvDevInfo
/// \brief  Product advanced configuration data register.
typedef struct
{
  // ============== Prod Adv Dev Info rev0. =========================
  // Crosstalk traces removal
  uint16_t mXtalkRmvTraceNoiseLevelMin;   ///< Minimum trace noise level used by peak detector basic
  uint16_t mXtalkRmvTraceNoiseLevelMax;   ///< Maximum trace noise level used by peak detector basic
  uint32_t mXtalkRmvResolMin;             ///< Minimum between two consecutive samples used by peak detector basic
  uint32_t mXtalkRmvResolMax;             ///< Maximum between two consecutive samples used by peak detector basic
  uint8_t  mXtalkRmvDiffEqCoeffCountMin;  ///< Minimum number of coeff for each difference equation
  uint8_t  mXtalkRmvDiffEqCoeffCountMax;  ///< Maximum number of coeff for each difference equation
  uint16_t mXtalkRmvTemplateGainMin;      ///< Minimum Gain of the template
  uint16_t mXtalkRmvTemplateGainMax;      ///< Maximum Gain of the template
  
  // ============== Prod Adv Dev Info revX. =========================
  // Insert new data up of this line and increment PRD_ADV_DEV_INFO_VERSION

} sProductAdvDevInfo;

/// \struct sProductAdvCfgData
/// \brief  Product advanced configuration data structure
typedef struct
{
  // ============== Prod Adv Cfg Data rev0. =========================
  uint8_t   mGrbAdc;                                 ///< Adc control (see device control).
  uint8_t   mGrbTimeBaseDelay;                       ///< Delay between the beginning of an acquisition and the trigger (see timing control 0).
  uint16_t  mGrbTiaPowerUpDelay;                     ///< Delay in clock count before starting a new frame (see timing control 1).
  uint8_t   mGrbTriggerControl;                      ///< Trigger enable  (see source control 0).
  uint16_t  mGrbTriggerWidth;                        ///< Duration of the trigger pulses 1/8 system clock (see source control 1).
  uint8_t   mGrbRandomCtrl;                          ///< Enable random acquisition to reduce acquisition (see random control).
  uint16_t  mGrbScanDuration;                        ///< Duration in clock count of one acquisition (see random control).
  uint8_t   mGrbTiaCtrl;                             ///< TIA gain and control (see tia control).
  uint8_t   mGrbPwmWidths[REGMAP_PWM_WIDTH_LENGTH];  ///< Number of clock cycles for each PWM pulse (see PWM pulse width 1 to 16).
  uint8_t   mGrbPwmPeriod;                           ///< Time required for one PWM cycle (see PWM period).
  uint8_t   mGrbPwmEnable;                           ///< Adjusts the LED power by enabling or disabling some PWM charge pulses (PWM pulse count).

  // Crosstalk traces removal
  uint16_t mXtalkRmvThr;                                                                                                              ///< Detection threshold used by peak detector basic
  uint16_t mXtalkRmvTraceNoiseLevel;                                                                                                  ///< Trace noise level used by peak detector basic
  uint8_t  mXtalkRmvDiffEqCoeffCount        [(LC08L32_DIFF_EQ_MAX*(LC08L32_OVERSAMPLING_EXPONENT_MAX+1))];                            ///< Number of coeff for each difference equation
  uint16_t mXtalkRmvTypeLut                 [LC08L32_NB_CHANNELS][LC08L32_NB_CHANNELS];                                               ///<  List of crosstalk type
  float    mXtalkRmvEqDiffLut               [(LC08L32_DIFF_EQ_MAX*(LC08L32_OVERSAMPLING_EXPONENT_MAX+1))][LC08L32_DIFF_EQ_COEFF_MAX]; ///< Coefficients used by difference equations
  uint8_t  mXtalkRmvTemplateCount           [LC08L32_TEMPLATE_MAX];                                                                   ///< Number of samples belong to the template
  uint8_t  mXtalkRmvTemplateCenter          [LC08L32_TEMPLATE_MAX];                                                                   ///< Center of the template
  uint16_t mXtalkRmvTemplateGain            [LC08L32_TEMPLATE_MAX];                                                                   ///< Gain of the template
  uint16_t mXtalkRmvTemplateDelay           [LC08L32_TEMPLATE_MAX];                                                                   ///< Delay of the template
  int32_t  mXtalkRmvTemplate                [LC08L32_TEMPLATE_MAX][LC08L32_TEMPLATE_COUNT_MAX];                                       ///< amplitude of each sample located in the normalize template
    
  // ============== Prod Adv Cfg Data rev1. =========================
  // Crosstalk traces removal
  uint8_t  mXtalkRmvDiffEqCoeffCountFir     [(LC08L32_DIFF_EQ_MAX*(LC08L32_OVERSAMPLING_EXPONENT_MAX+1))];                            ///< Number of coeff for each difference equation
  int32_t  mXtalkRmvEqDiffLutFir            [(LC08L32_DIFF_EQ_MAX*(LC08L32_OVERSAMPLING_EXPONENT_MAX+1))][LC08L32_DIFF_EQ_COEFF_MAX]; ///< Coefficients used by difference equations
    
  
  // ============== Prod Adv Cfg Data revX. =========================
  // Insert new data up of this line and increment PRD_ADV_CFG_DATA_VERSION

} sProductAdvCfgData;

/// \struct sProductCmdList
/// \brief  Product commands and status data structure.
typedef struct
{
  // ============== Prod Cmd List rev0. =========================
  // States
  uint32_t mSensorTemp;

  // Algorithm initialization error flag.
  uint32_t mCalibRefErrFlag;

  // ============== Prod Cmd List rev1. =========================
  // States
  uint32_t mSensorTempPred;
  // ============== Prod Cmd List revX. =========================
  // Insert new data up of this line.

} sProductCmdList;

/// \struct sProductAdvCmdList
/// \brief  Product advanced commands and status data structure.
typedef struct
{
  // ============== Prod Adv Cmd List rev0. =========================
  uint8_t mReserved;

  // ============== Prod Adv Cmd List revX. =========================
  // Insert new data up of this line.

} sProductAdvCmdList;

/// \struct sFpga
/// \brief  FPGA write and write registers
typedef struct
{
  // ============== FPGA rev0. =========================
  uint8_t  mCmd;
  uint16_t mAdd;
  uint8_t  mLength;
  uint8_t  mData[256];
  
  // ============== FPGA revX. =========================
  // Insert new data up of this line

} sFpga;

/// \struct sAsic
/// \brief  FPGA write and write registers
typedef struct
{
  // ============== Asic Patch rev0. =========================
  uint16_t  mSize;
  uint16_t  mStartAddr;
  uint8_t   mPatch[0x7FF4];
  
  // ============== Asic Patch revX. =========================
  // Insert new data up of this line and increment ASIC_PATCH_VERSION

} sAsic;

#pragma pack(pop)

#endif  // _REGISTER_MAP_PRODUCT_
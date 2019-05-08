// ****************************************************************************
/*!
Module:   Register map.

Platform: Independant

\file     registerMapLC16IL.h

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
#include "PlatformLC16il32DefinitionsShared.h"

//*****************************************************************************
//*************** Constants and Macros ****************************************
//*****************************************************************************

// Product-specific configuration data version
#define PRD_CFGD_ATA_VERSION_0          0
#define PRD_CFG_DATA_VERSION            PRD_CFGD_ATA_VERSION_0

// Product-specific advanced configuration data version
#define PRD_ADV_CFG_DATA_VERSION_0      0
#define PRD_ADV_CFG_DATA_VERSION        PRD_ADV_CFG_DATA_VERSION_0

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
#define REGMAPP_DIFF_EQ_MAX             LC16IL32_DIFF_EQ_MAX
#define REGMAPP_TEMPLATE_MAX            LC16IL32_TEMPLATE_MAX
#define REGMAPP_DIFF_EQ_COEFF_MAX       LC16IL32_DIFF_EQ_COEFF_MAX
#define REGMAPP_TEMPLATE_COUNT_MAX      LC16IL32_TEMPLATE_COUNT_MAX
#define REGMAP_PWM_WIDTH_LENGTH         LC16IL32_NB_PWM_PULSES

//*****************************************************************************
//*************** Data Type Definition ****************************************
//*****************************************************************************

#pragma pack(push, 1)

/// \struct sProductCfgData
/// \brief  Product configuration data structure
typedef struct
{

  // Echo due to crosstalk removal.
  uint8_t   mXtalkEchoRemovalEnable;     ///< Crosstalk echo removal enable

  // Xtalk trace removal
  uint8_t   mXtalkRmvEnable;            ///< Xtalk trace removal enable

} sProductCfgData;

/// \struct sProductDevInfo
/// \brief  Product configuration data register.
typedef struct
{
  uint8_t mReserved;
} sProductDevInfo;

/// \struct sProductAdvDevInfo
/// \brief  Product advanced configuration data register.
typedef struct
{

  // Crosstalk traces removal
  uint16_t mXtalkRmvTraceNoiseLevelMin;   ///< Minimum trace noise level used by peak detector basic
  uint16_t mXtalkRmvTraceNoiseLevelMax;   ///< Maximum trace noise level used by peak detector basic
  uint32_t mXtalkRmvResolMin;             ///< Minimum between two consecutive samples used by peak detector basic
  uint32_t mXtalkRmvResolMax;             ///< Maximum between two consecutive samples used by peak detector basic
  uint8_t  mXtalkRmvDiffEqCoeffCountMin;  ///< Minimum number of coeff for each difference equation
  uint8_t  mXtalkRmvDiffEqCoeffCountMax;  ///< Maximum number of coeff for each difference equation
  uint16_t mXtalkRmvTemplateGainMin;      ///< Minimum Gain of the template
  uint16_t mXtalkRmvTemplateGainMax;      ///< Maximum Gain of the template

} sProductAdvDevInfo;

/// \struct sProductAdvCfgData
/// \brief  Product advanced configuration data structure
typedef struct
{
  // Grabber
  uint32_t mGrbGain[LC16IL32_NB_CHANNELS];
  uint16_t mGrbPwmPeriod;
  uint8_t  mGrbPwmWidths[LC16IL32_NB_PWM_PULSES];
  uint16_t mGrbPwmEnable;
  uint8_t  mGrbScanTime;
  uint8_t  mGrbTriggerEnable;
  uint8_t  mGrbTriggerPeriod;
  uint16_t mGrbSamplingOffset;

  // Crosstalk traces removal
  uint16_t mXtalkRmvThr;                                                                                                                 ///< Detection threshold used by peak detector basic
  uint16_t mXtalkRmvTraceNoiseLevel;                                                                                                     ///< Trace noise level used by peak detector basic
  uint32_t mXtalkRmvResol;                                                                                                               ///< Resolution between two consecutive samples used by peak detector basic
  uint8_t  mXtalkRmvDiffEqCoeffCount        [(LC16IL32_DIFF_EQ_MAX*(LC16IL32_OVERSAMPLING_EXPONENT_MAX+1))];                             ///< Number of coeff for each difference equation
  uint16_t mXtalkRmvTypeLut                 [LC16IL32_NB_CHANNELS][LC16IL32_NB_CHANNELS];                                                ///<  List of crosstalk type
  float    mXtalkRmvEqDiffLut               [(LC16IL32_DIFF_EQ_MAX*(LC16IL32_OVERSAMPLING_EXPONENT_MAX+1))][LC16IL32_DIFF_EQ_COEFF_MAX]; ///< Coefficients used by difference equations
  uint8_t  mXtalkRmvTemplateCount           [LC16IL32_TEMPLATE_MAX];                                                                     ///< Number of samples belong to the template
  uint8_t  mXtalkRmvTemplateCenter          [LC16IL32_TEMPLATE_MAX];                                                                     ///< Center of the template
  uint16_t mXtalkRmvTemplateGain            [LC16IL32_TEMPLATE_MAX];                                                                     ///< Gain of the template
  uint16_t mXtalkRmvTemplateDelay          [LC16IL32_TEMPLATE_MAX];                                                                      ///< Delay of the template
  int32_t  mXtalkRmvTemplate                [LC16IL32_TEMPLATE_MAX][LC16IL32_TEMPLATE_COUNT_MAX];                                        ///< amplitude of each sample located in the normalize template
} sProductAdvCfgData;

/// \struct sProductCmdList
/// \brief  Product commands and status data structure.
typedef struct
{
  uint8_t mReserved;
} sProductCmdList;

/// \struct sProductAdvCmdList
/// \brief  Product advanced commands and status data structure.
typedef struct
{
  uint8_t mReserved;
} sProductAdvCmdList;

/// \struct sFpga
/// \brief  FPGA write and write registers
typedef struct
{
  uint8_t  mCmd;
  uint16_t mAdd;
  uint8_t  mLength;
  uint16_t mData[128];
} sFpga;

/// \struct sAsic
/// \brief  FPGA write and write registers
typedef struct
{
  uint16_t  mSize;
  uint16_t  mStartAddr;
  uint8_t   mPatch[0x7FF4];
} sAsic;

#pragma pack(pop)

//*****************************************************************************
//************** Public Variable Declarations ********************************
//*****************************************************************************

#endif
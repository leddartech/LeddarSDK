// ****************************************************************************
/*!
Module:     Shared definitions relative to Galaxy platform.

Platform:   Independent

\file       PlatformM7DefinitionsShared.h

\brief      Some Galaxy platform definitions can be shared between different
            system environment (LeddarSoftware, LeddarExplorer, embedded firmware).

\author     Frédéric Parent
\since      Mar 25, 2013

\copyright (c) 2013 LeddarTech Inc. All rights reserved.
*/
// ***************************************************************************

#ifndef __PLATFORMM7DEFINITIONSSHARED_H__
#define __PLATFORMM7DEFINITIONSSHARED_H__

//*****************************************************************************
//*************** Header Includes *********************************************
//*****************************************************************************

#include <stdint.h>

//*****************************************************************************
//*************** Constants and Macros ****************************************
//*****************************************************************************

#ifdef GCC
  #define PACKED    __attribute__((packed))
#else
  #define PACKED
#endif

#define  M7_NB_HON_CHANNELS               16
#define  M7_NB_VER_CHANNELS               1
#define  M7_NB_REF_CHANNELS               1
#define  M7_NB_CHANNELS                   ( (M7_NB_HON_CHANNELS*M7_NB_VER_CHANNELS) + M7_NB_REF_CHANNELS )
#define  M7_MAX_ECHOES_PER_CHANNEL        6
#define  M7_NB_USER_LED_POWER_MAX         16
#define  M7_NB_COEFF_FILTER_MAX           68
#define  M7_ACCUMULATION_EXPONENT_MAX     12
#define  M7_OVERSAMPLING_EXPONENT_MAX     5
#define  M7_SATURATION_AMP_LUT_SIZE_MAX   16
#define  M7_SATURATION_DIS_LUT_SIZE_MAX   16
#define  M7_XTALK_NB_TYPE_MAX             10
#define  M7_DEM_REF_PULSE_MAX_PTS         400
#define  M7_DEM_LUT_MAX_PTS               200
#define  M7_NB_SAMPLES_PER_CHANNEL_MAX    1184
#define  M7_BAYES_STD_LUT_SIZE_MAX        5
#define  M7_MAG_CORRECTION_LUT_SIZE_MAX   50
#define  M7_NB_BASE_SAMPLE_MAX_CALIB      15
#define  M7_OVERSAMPLING_EXPONENT_CALIB   3
#define  M7_OVERSAMPLING_CALIB            (1 << M7_OVERSAMPLING_EXPONENT_CALIB)
#define  M7_NB_USER_LED_POWER_MAX         16
#define  M7_NB_USER_FIELD_MAX             22
#define  M7_CPU_LOAD_SCALE                10


//*****************************************************************************
//*************** Data Type Definition ****************************************
//*****************************************************************************

/// \struct sEchoLigth
/// \brief  Light echo structure
typedef struct
{
  int32_t  mDistance;
  uint32_t mAmplitude;
  uint16_t mSegment;
  uint16_t mFlag;
} PACKED sEchoLigth;

/// \struct SEchoFP
/// \brief Element of the echoes list
/// \warning  The size of the structure is used in USB communication.
///           Does not change the structure without changing the
///           definition of gEchoElemStruct in processingData.c. This
///           size dependency should be addressed.
typedef struct
{
  int32_t      mDistance;          ///< Echo distance.
  uint32_t     mAmplitude;         ///< Echo amplitude.
  uint32_t     mBase;              ///< Base value to
  uint16_t     mMaxIndex;          ///< Index of the maximum amplitude.
  uint8_t      mChannelIndex;      ///< Channel index of the echo.
  uint8_t      mValid;             ///< Bit fields validation flags.
  uint32_t     mAmplitudeLowScale; ///< Echo amplitude in low scale factor.
  uint32_t     mSaturationWidth;   ///< Width of the saturation.
} sEcho;

/// \struct SEchoesFP
/// \brief List of Echoes
typedef struct
{
  uint32_t  mEchoCount;                                         ///< Number of echoes in the list.
  sEcho     mEchoes[M7_MAX_ECHOES_PER_CHANNEL*M7_NB_CHANNELS];  ///< Echoes structure list.
} sEchoes;

/// \struct SPulse
/// \brief Element of the pulses list. A peak in a trace, not yet an echo.
typedef struct
{
  uint16_t     index;         //<  Index in the trace.
  uint16_t     maxi;          //<  Index of maximum amplitude.
  int32_t      distIntpl;     //<  Distance obtained by interpolation.
  uint16_t     minLefti;      //<  Index of minimum amplitude located on the Left of the pulse.
  uint16_t     minRighti;     //<  Index of minimum amplitude located on the Right of the pulse.
  uint32_t     max;           //<  Maximum amplitude.
  uint32_t     amp;           //<  Real amplitude of the pulse.
  uint32_t     base;          //<  Base level.
  uint32_t     minLeft;       //<  Minimum amplitude on the Left of the pulse.
  uint32_t     minRight;      //<  Minimum amplitude on the Right of the pulse.
  uint32_t     satWidth;      //<  Saturation width.
  uint32_t     ampLowScale;   //<  Real amplitude of the pulse with a small fixed point.
  uint32_t     satLefti;      //<  First point of the pulse saturated.
  uint32_t     satRighti;     //<  Last point of the pulse saturated.
  int32_t      inflec;        //<  Inflection point.
  uint8_t      ch;            //<  Channel Index.
  uint8_t      valid;         //<  Pulse validation.
  uint8_t      satFlag;       //<  Saturation flag.
  uint8_t      bayesFlag;     //<  Bayes detector flag.
} sPulse;

#endif  /* __PLATFORMM7DEFINITIONSSHARED_H__ */
// End of PlatformM7DefinitionShared.h

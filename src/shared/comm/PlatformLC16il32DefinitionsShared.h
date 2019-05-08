// ****************************************************************************
/*!
Module:     Shared definitions relative to Galaxy platform.

Platform:   Independent

\file       PlatformLC16il32DefinitionsShared.h

\brief      Some LC16il32 platform definitions can be shared between different
            system environment.

\author     Samuel Gidel
\author     Vincent Simard Bilodeau
\author     Frédéric Parent
\since      May 16, 2016

\copyright (c) 2016 LeddarTech Inc. All rights reserved.
*/
// ***************************************************************************

#ifndef __PLATFORMLC16IL32DEFINITIONSSHARED_H__
#define __PLATFORMLC16IL32DEFINITIONSSHARED_H__

//*****************************************************************************
//*************** Header Includes *********************************************
//*****************************************************************************

#include <stdint.h>
#include "PlatformM7DefinitionsShared.h"

//*****************************************************************************
//*************** Constants and Macros ****************************************
//*****************************************************************************

#ifdef GCC
  #define PACKED			__attribute__((packed))
#else
  #define PACKED
#endif

// Peak detector function selection
#define _USE_CUSTOM_PULSE
#undef  _USE_MAG_CORRECTION
#define _USE_BAYES_DETECTOR
#define _USE_AUTO_LED_POWER
#define _USE_XTALK_REJECTION
#undef  _USE_TIME_PROFILING
#define _USE_TEMP_SLOPE
#undef _NO_64BIT_OPS

// Device
#define LC16IL32_DEFAULT_PART_NAME                       "DEFAULT NAME"                 ///< Default device name.
#define LC16IL32_DEFAULT_PART_NUMBER                     "UNKNOWN"                      ///< Default device part number.
#define LC16IL32_DEFAULT_SOFT_PART_NUMBER                VERSION_FIRMWARE_PART_NUMBER   ///< Default software part number.
#define LC16IL32_DEFAULT_SERIAL_NUMBER                   "LEDDAR12345678-PO"            ///< Default device serial number.
#define LC16IL32_DEFAULT_GROUP_ID                        "LEDDARTECH"                   ///< Default group id.
#define LC16IL32_DEFAULT_ASIC_VERSION                    "1234"                         ///< Default asic version.
#define LC16IL32_DEFAULT_FPGA_VERSION                    "32912782-3"                   ///< Default FPGA version.
#define LC16IL32_DEFAULT_MFG_NAME                        "LeddarTech"                   ///< Default manufacturer name.
#define LC16IL32_DEFAULT_BUILD_DATE                      "12 January 2016"              ///< Default build data
#define LC16IL32_DEFAULT_BOOTLD_VERSION                  "123dd4"                       ///< Bootloader version
#define LC16IL32_DEFAULT_OPTIONS                         0x2C                           ///< Default device options.

// Platform
#define LC16IL32_DISTANCE_SCALE_BITS                     16
#define LC16IL32_DISTANCE_SCALE                          (1<<LC16IL32_DISTANCE_SCALE_BITS)                   ///< Distance scale factor to a value expressed in fixed-point.
#define LC16IL32_ADC_SCALE_BITS                          10                                                  ///< ADC scale bits.
#define LC16IL32_ADC_SCALE                               (1 << LC16IL32_ADC_SCALE_BITS )                     ///< ADC scale
#define LC16IL32_RAW_AMPLITUDE_SCALE_BITS                6u                                                  ///< Raw amplitude is expressed on 10 bits and the fractional part is on 6 bits.
#define LC16IL32_RAW_AMPLITUDE_SCALE                     (1 << LC16IL32_RAW_AMPLITUDE_SCALE_BITS)            ///< Raw amplitude scale factor to be expressed in fixed-point.
#define LC16IL32_TEMP_SCALE_BITS                         LC16IL32_DISTANCE_SCALE_BITS                        ///< Temperature scale factor to a value expressed in fixed-point. Beware,
#define LC16IL32_TEMP_SCALE                              LC16IL32_DISTANCE_SCALE
#define LC16IL32_MEASUREMENT_RATE_SCALE_BITS             16
#define LC16IL32_MEASUREMENT_RATE_SCALE                  (1 << LC16IL32_MEASUREMENT_RATE_SCALE_BITS)         ///< Measurement rate scale factor to a value expressed in fixed-point Hz. Must be same as distance scale for temperature compensation of distance function.
#define LC16IL32_SRC_LIGHT_FREQ                          102400.0F                                           ///< Infrared source light frequency in Hz.
#define LC16IL32_DEFAULT_SCAN_TIME                       10                                                  ///< Default duration of one scan.
#define LC16IL32_REFRESH_RATE_SCALE                      64
#define LC16IL32_ASIC_FREQUENCY                          100000000
#define LC16L32_DEFAULT_SRC_LIGHT_FREQ                   102400                                              ///< Infrared source light frequency in Hz.
#define LC16IL32_BASE_SAMPLE_DISTANCE                    1.49896229F                                         ///< Distance between two base sample points in meters.
#define LC16IL32_BEAM_RANGE_GAP                          (49 * LC16IL32_DISTANCE_SCALE / 10)                 ///< (4.9m) Distance between the beam range and the selected base sample distance.
#define LC16IL32_NB_VERTICAL_CHANNELS                    1                                                   ///< Number of Leddar channels along the vertical.
#define LC16IL32_NB_HONRIZONTAL_CHANNELS                 16                                                  ///< Number of Leddar channels along the horizontal.
#define LC16IL32_NB_REF_CHANNELS                         0                                                   ///< Number of reference channels.
#define LC16IL32_NB_CHANNELS                             (LC16IL32_NB_VERTICAL_CHANNELS * LC16IL32_NB_HONRIZONTAL_CHANNELS) ///< Total number of channel.
#define LC16IL32_DEFAULT_BASE_SAMPLE_COUNT               18                                                                 ///< Default base sample count value.
#define LC16IL32_NB_BASE_SAMPLE_MIN                      2                                                                  ///< Minimum authorized base sample count value.
#define LC16IL32_NB_BASE_SAMPLE_MAX                      74                                                                 ///< Maximum authorized base sample count value.
#define LC16IL32_DEFAULT_ACCUMULATION_EXPONENT           8                                                                  ///< Default accumulation exponent value.
#define LC16IL32_ACCUMULATION_EXPONENT_MIN               0                                                                  ///< Minimum authorized accumulation exponent value.
#define LC16IL32_ACCUMULATION_EXPONENT_MAX               10                                                                 ///< Maximum authorized accumulation exponent value.
#define LC16IL32_DEFAULT_OVERSAMPLING_EXPONENT           3                                                                  ///< Default oversampling exponent value.
#define LC16IL32_OVERSAMPLING_EXPONENT_MIN               0                                                                  ///< Minimum authorized oversampling exponent value.
#define LC16IL32_OVERSAMPLING_EXPONENT_MAX               5                                                                  ///< Maximum authorized oversampling exponent value.
#define LC16IL32_DEFAULT_SEGMENT_ENABLE                  0xFFFF                                                             ///< Default segment enable flag.
#define LC16IL32_NB_SAMPLES_PER_CHANNEL_MAX              1184                                                               ///< Maximum number of samples per channel (limited by RAM size --> only half of distance for 32 oversampling).
#define LC16IL32_DEFAULT_SAMPLING_OFFSET                 0x3                                                                ///< Default sampling offset
#define LC16IL32_DEFAULT_TRIGGER_PERIOD                  (0x1 | 0x0 << 3)                                                   ///< Default period of the trigger by steps of 1/8 of sampling period.
#define LC16IL32_DEFAULT_TRIGGER_ENABLE                  0x1                                                                ///< Default trigger enable setting
#define LC16IL32_DEFAULT_TRACE_BUFFER_TYPE               0                                                                  ///< Default trace buffer state.
#ifdef _USE_CVI_GUI
#define LC16IL32_MAX_ECHOES_PER_CHANNEL                  60
#else
#define LC16IL32_MAX_ECHOES_PER_CHANNEL                  6
#endif
#define LC16IL32_MAX_ECHOES                              LC16IL32_NB_VERTICAL_CHANNELS*LC16IL32_NB_HONRIZONTAL_CHANNELS*LC16IL32_MAX_ECHOES_PER_CHANNEL
#define LC16IL32_DEFAULT_FIELD_OF_VIEW                   (45*LC16IL32_DISTANCE_SCALE)
#define LC16IL32_DEFAULT_YAW                             0
#define LC16IL32_DEFAULT_PITCH                           0
#define LC16IL32_DEFAULT_ROLL                            0
#define LC16IL32_DEFAULT_POSITION_X                      0
#define LC16IL32_DEFAULT_POSITION_Y                      0
#define LC16IL32_DEFAULT_POSITION_Z                      0

// Led power
#define LC16IL32_DEFAULT_LEDPOW_ENABLE                   1
#define LC16IL32_DEFAULT_TRANS_IMP_GAIN                  { 2, 2, 2, 2,  \
                                                           2, 2, 2, 2,  \
                                                           2, 2, 2, 2,  \
                                                           2, 2, 2, 2 }         ///< Default transimpedance gain.
#define LC16IL32_TRANS_IMP_GAIN_MIN                     0                       ///< Minimum authorized transimpedance gain.
#define LC16IL32_TRANS_IMP_GAIN_MAX                     7                       ///< Maximum authorized transimpedance gain.
#define LC16IL32_DEFAULT_PWM_WIDTH_1                    0x5
#define LC16IL32_DEFAULT_PWM_WIDTH_2                    0x5
#define LC16IL32_DEFAULT_PWM_WIDTH_3                    0xa
#define LC16IL32_DEFAULT_PWM_WIDTH_4                    0xa
#define LC16IL32_DEFAULT_PWM_WIDTH_5                    0xf
#define LC16IL32_DEFAULT_PWM_WIDTH_6                    0xf
#define LC16IL32_DEFAULT_PWM_WIDTH_7                    0x14
#define LC16IL32_DEFAULT_PWM_WIDTH_8                    0x14
#define LC16IL32_DEFAULT_PWM_WIDTH_9                    0x19
#define LC16IL32_DEFAULT_PWM_WIDTH_10                   0x19
#define LC16IL32_DEFAULT_PWM_WIDTH_11                   0x1e
#define LC16IL32_DEFAULT_PWM_WIDTH_12                   0x1e
#define LC16IL32_DEFAULT_PWM_WIDTH_13                   0x23
#define LC16IL32_DEFAULT_PWM_WIDTH_14                   0x23
#define LC16IL32_DEFAULT_PWM_WIDTH_15                   0x28
#define LC16IL32_DEFAULT_PWM_WIDTH_16                   0x28
#define LC16IL32_DEFAULT_PWM_WIDTH                      { LC16IL32_DEFAULT_PWM_WIDTH_1, LC16IL32_DEFAULT_PWM_WIDTH_2,  LC16IL32_DEFAULT_PWM_WIDTH_3,  LC16IL32_DEFAULT_PWM_WIDTH_4,  \
                                                          LC16IL32_DEFAULT_PWM_WIDTH_5, LC16IL32_DEFAULT_PWM_WIDTH_6,  LC16IL32_DEFAULT_PWM_WIDTH_7,  LC16IL32_DEFAULT_PWM_WIDTH_8,  \
                                                          LC16IL32_DEFAULT_PWM_WIDTH_9, LC16IL32_DEFAULT_PWM_WIDTH_10, LC16IL32_DEFAULT_PWM_WIDTH_11, LC16IL32_DEFAULT_PWM_WIDTH_12,  \
                                                          LC16IL32_DEFAULT_PWM_WIDTH_13,LC16IL32_DEFAULT_PWM_WIDTH_14, LC16IL32_DEFAULT_PWM_WIDTH_15, LC16IL32_DEFAULT_PWM_WIDTH_16 }               ///< Default pwm width.                                                                           ///< Default number of clock cycles for each PWM pulse.
#define LC16IL32_PWM_WIDTH_MIN                          0                                        ///< Minimum authorized transimpedance gain.
#define LC16IL32_PWM_WIDTH_MAX                          0xFF
#define LC16IL32_DEFAULT_PWM_PERIOD                     0x31                                     ///< Default time of one PWM cycle.
#define LC16IL32_PWM_PERIOD_MIN                         0                                        ///< Default time of one PWM cycle.
#define LC16IL32_PWM_PERIOD_MAX                         0xFF                                     ///< Default time of one PWM cycle.
#define LC16IL32_NB_PWM_PULSES                          16                                      ///< Number of pwm pulse.
#define LC16IL32_PWM_ENABLE_MIN                         0                                        ///< PWM enable min.
#define LC16IL32_PWM_ENABLE_MAX                         16

#define LC16IL32_DEFAULT_NB_USER_LED_POWER              10
#define LC16IL32_DEFAULT_CURRENT_USER_LED_POWER         LC16IL32_DEFAULT_NB_USER_LED_POWER-1        ///< Current User led power level [in index]
#define LC16IL32_NB_USER_LED_POWER_MAX                  16                                          ///< Maximum authorized user led power.
#define LC16IL32_NB_USER_FIELD_MAX                      22                                          ///< Maximum authorized field led power.
#define LC16IL32_DEFAULT_REF_PULSE_RATE                 1
#define LC16IL32_REF_SEG_MASK                           0x0
#define LC16IL32_DEFAULT_REF_SEGMENT_ENABLE             LC16IL32_REF_SEG_MASK                   ///< Enable or not the reference pulse by default

// Demerge
#define LC16IL32_DEFAULT_DEM_ENABLE                     0         ///< Default demerge enable state.
#define LC16IL32_DEM_REF_PULSE_MAX_PTS                  400
#define LC16IL32_DEM_LUT_MAX_PTS                        200
#define LC16IL32_OBJECT_DEMERGING_MAX                   2

// Peak detector
#define LC16IL32_DEFAULT_FILTER_SUM_BITS                13                                                             ///< Number of bits added by the trace filter
#define LC16IL32_FILTER_SUM_BITS_MIN                    1
#define LC16IL32_FILTER_SUM_BITS_MAX                    16
#define LC16IL32_DEFAULT_FILTER_SUM                     (1 <<LC16IL32_DEFAULT_FILTER_SUM_BITS )                        ///< The sum of all element in the filter matrix.
#define LC16IL32_DEFAULT_FILTERED_SCALE                 (LC16IL32_RAW_AMPLITUDE_SCALE * LC16IL32_DEFAULT_FILTER_SUM)   ///< Filter scale factor to be expressed in fixed-point.
#define LC16IL32_DEFAULT_FILTERED_SCALE_BITS            (LC16IL32_DEFAULT_FILTER_SUM_BITS+LC16IL32_RAW_AMPLITUDE_SCALE_BITS)
#define LC16IL32_DEFAULT_RAW_AMPLITUDE_MAX              (1020*LC16IL32_RAW_AMPLITUDE_SCALE)
#define LC16IL32_DEFAULT_REAL_DISTANCE_OFFSET           ((int32_t)(2.6*LC16IL32_DISTANCE_SCALE))                       ///< Difference in distance between the signal peak and the real distance: 2.6 m
#define LC16IL32_REAL_DISTANCE_OFFSET_MIN               ((int32_t)(0*LC16IL32_DISTANCE_SCALE))
#define LC16IL32_REAL_DISTANCE_OFFSET_MAX               ((int32_t)(10*LC16IL32_DISTANCE_SCALE))
#define LC16IL32_NB_COEFF_FILTER_MAX                    68                                                             ///< Maximum authorized number of coefficients in a Gaussian filter. BEWARE: must be an even value and a multiple of 4.
#define LC16IL32_BAYES_STD_LUT_SIZE_MAX                 5
#define LC16IL32_MAG_CORRECTION_LUT_SIZE_MAX            50
#define LC16IL32_XTALK_NB_TYPE_MAX                      10
#define LC16IL32_SATURATION_DIS_LUT_SIZE_MAX            16
#define LC16IL32_SATURATION_AMP_LUT_SIZE_MAX            16
#define LC16IL32_ACCUMULATION_DEM_EXPONENT_MIN          6
#define LC16IL32_OVERSAMPLING_DEM_EXPONENT_MIN          3

// Crosstalk removal
#define LC16IL32_DIFF_EQ_MAX                             8
#define LC16IL32_TEMPLATE_MAX                            8
#define LC16IL32_DIFF_EQ_COEFF_MAX                       21
#define LC16IL32_TEMPLATE_COUNT_MAX                      200

// Static noise removal
#define LC16IL32_DEFAULT_STNOIRMV_ENABLE                 1
#define LC16IL32_OVERSAMPLING_EXPONENT_CALIB             3                                                               ///< Maximum oversampling exponent authorized during a learning.
#define LC16IL32_OVERSAMPLING_CALIB                      (1 << LC16IL32_OVERSAMPLING_EXPONENT_CALIB)                     ///< Maximum oversampling authorized during a learning.
#define LC16IL32_NB_BASE_SAMPLE_MIN_CALIB                1                                                               ///< Minimum authorized number base sample.
#define LC16IL32_NB_BASE_SAMPLE_MAX_CALIB                15                                                              ///< Maximum authorized number base sample.

// Mathematics
#define LC16IL32_DELAY_SCALE_BITS                        16


//*****************************************************************************
//*************** Public Function Declarations ********************************
//*****************************************************************************


#endif  /* __PLATFORMLC16IL32DEFINITIONSSHARED_H__ */

// End of PlatformLC16IL32DefinitionShared.h

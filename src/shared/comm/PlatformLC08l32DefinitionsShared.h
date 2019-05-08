// ****************************************************************************
/*!
Module:     Shared definitions relative to Galaxy platform.

Platform:   Independent

\file       PlatformLC08l32DefinitionsShared.h

\brief      Some LC08l32 platform definitions can be shared between different
            system environment.

\author     Samuel Gidel
\author     Vincent Simard Bilodeau
\author     Frédéric Parent
\since      May 16, 2016

\copyright (c) 2016 LeddarTech Inc. All rights reserved.
*/
// ***************************************************************************

#ifndef __PLATFORMLC08L32DEFINITIONSSHARED_H__
#define __PLATFORMLC08L32DEFINITIONSSHARED_H__


// Function prefix
#undef PFX

#ifdef _USE_CVI_GUI
#define PFX(_name)                                      _name
#else
#define PFX(_name)                                      _name
#endif // ifdef __USE_CVI_GUI

#ifdef GCC
#define PACKED    __attribute__((packed))
#else
#define PACKED
#endif

//*****************************************************************************
//*************** Header Includes *********************************************
//*****************************************************************************

#include <stdint.h>
#include "PlatformM7DefinitionsShared.h"

//*****************************************************************************
//*************** Constants and Macros ****************************************
//*****************************************************************************

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
#define LC08L32_DEFAULT_PART_NAME                       "DEFAULT NAME"                  ///< Default device name.
#define LC08L32_DEFAULT_PART_NUMBER                     "UNKNOWN"                       ///< Default device part number.
#define LC08L32_DEFAULT_SOFT_PART_NUMBER                VERSION_FIRMWARE_PART_NUMBER    ///< Default software part number.
#define LC08L32_DEFAULT_SERIAL_NUMBER                   "LEDDAR12345678-PO"             ///< Default device serial number.
#define LC08L32_DEFAULT_GROUP_ID                        "LEDDARTECH"                    ///< Default group id.
#define LC08L32_DEFAULT_ASIC_VERSION                    "1234"                          ///< Default asic version.
#define LC08L32_DEFAULT_FPGA_VERSION                    "32912782-3"                    ///< Default FPGA version.
#define LC08L32_DEFAULT_MFG_NAME                        "LeddarTech"                    ///< Default manufacturer name.
#define LC08L32_DEFAULT_BUILD_DATE                      "12 January 2016"               ///< Default build data
#define LC08L32_DEFAULT_BOOTLD_VERSION                  "123dd4"                        ///< Bootloard version
#define LC08L32_DEFAULT_OPTIONS                         0x2C                            ///< Default device options.

// Platform
#define LC08L32_DISTANCE_SCALE_BITS                     16
#define LC08L32_DISTANCE_SCALE                          (1<<LC08L32_DISTANCE_SCALE_BITS)                                 ///< Distance scale factor to a value expressed in fixed-point.
#define LC08L32_ADC_SCALE_BITS                          10                                                               ///< ADC scale bits.
#define LC08L32_ADC_SCALE                               (1 << LC08L32_ADC_SCALE_BITS )                                   ///< ADC scale
#define LC08L32_RAW_AMPLITUDE_SCALE_BITS                6u                                                               ///< Raw amplitude is expressed on 10 bits and the fractional part is on 6 bits.
#define LC08L32_RAW_AMPLITUDE_SCALE                     (1 << LC08L32_RAW_AMPLITUDE_SCALE_BITS)                          ///< Raw amplitude scale factor to be expressed in fixed-point.
#define LC08L32_TEMP_SENSOR_SCALE_BITS                  8                                                                ///< Temperature scale factor
#define LC08L32_TEMP_SENSOR_SCALE                       LC08L32_TEMP_SENSOR_SCALE_BITS                                   ///< Temperature scale factor bits.
#define LC08L32_TEMP_SCALE_BITS                         LC08L32_DISTANCE_SCALE_BITS                                      ///< Temperature scale factor to a value expressed in fixed-point. Beware,
#define LC08L32_TEMP_SCALE                              LC08L32_DISTANCE_SCALE
#define LC08L32_MEASUREMENT_RATE_SCALE_BITS             16
#define LC08L32_MEASUREMENT_RATE_SCALE                  (1 << LC08L32_MEASUREMENT_RATE_SCALE_BITS)                       ///< Measurement rate scale factor to a value expressed in fixed-point Hz. Must be same as distance scale for temperature compensation of distance function.
#define LC08L32_DEFAULT_SCAN_TIME                       0x2710                                                            ///< Default duration of one scan in 100 Mhz clock cycles.
#define LC08L32_REFRESH_RATE_SCALE                      64
#define LCO8L32_FPGA_FREQUENCY                          100000000
#define LC08L32_DEFAULT_SRC_LIGHT_FREQ                  (LCO8L32_FPGA_FREQUENCY/LC08L32_DEFAULT_SCAN_TIME)               ///< Infrared source light frequency in Hz.
#define LC08L32_BASE_SAMPLE_DISTANCE                    1.49896229F                                                      ///< Distance between two base sample points in meters.
#define LC08L32_BEAM_RANGE_GAP                          (49 * LC08L32_DISTANCE_SCALE / 10)                               ///< (4.9m) Distance between the beam range and the selected base sample distance.
#define LC08L32_NB_VERTICAL_CHANNELS                    1                                                                ///< Number of Leddar channels along the vertical.
#define LC08L32_NB_REF_CHANNELS                         1                                                                ///< Number of reference channels.
#define LC08L32_NB_HONRIZONTAL_CHANNELS                 8                                                                ///< Number of Leddar channels along the horizontal.
#define LC08L32_NB_CHANNELS                             (LC08L32_NB_VERTICAL_CHANNELS * LC08L32_NB_HONRIZONTAL_CHANNELS+LC08L32_NB_REF_CHANNELS) ///< Total number of channel.
#define LC08L32_DEFAULT_BASE_SAMPLE_COUNT               18                                                               ///< Default base sample count value.
#define LC08L32_NB_BASE_SAMPLE_MIN                      2                                                                ///< Minimum authorized base sample count value.
#define LC08L32_NB_BASE_SAMPLE_MAX                      128                                                               ///< Maximum authorized base sample count value.
#define LC08L32_DEFAULT_ACCUMULATION_EXPONENT           8                                                                ///< Default accumulation exponent value.
#define LC08L32_ACCUMULATION_EXPONENT_MIN               0                                                                ///< Minimum authorized accumulation exponent value.
#define LC08L32_ACCUMULATION_EXPONENT_MAX               10                                                               ///< Maximum authorized accumulation exponent value.
#define LC08L32_DEFAULT_OVERSAMPLING_EXPONENT           3                                                                ///< Default oversampling exponent value.
#define LC08L32_OVERSAMPLING_EXPONENT_MIN               0                                                                ///< Minimum authorized oversampling exponent value.
#define LC08L32_OVERSAMPLING_EXPONENT_MAX               5                                                                ///< Maximum authorized oversampling exponent value.
#define LC08L32_DEFAULT_SEGMENT_ENABLE                  0x00FF                                                           ///< Default segment enable flag.
#define LC08L32_NB_SAMPLES_PER_CHANNEL_MAX              1024                                                             ///< Maximum number of samples per channel (limited by RAM size --> only half of distance for 32 oversampling).
#define LC08L32_DEFAULT_START_SCAN_OFFSET               0x0                                                              ///< Default sampling offset
#define LC08L32_DEFAULT_TRIGGER_PERIOD                  0xffff                                                           ///< Default period of the trigger by steps of 1/8 of sampling period.
#define LC08L32_DEFAULT_TRIGGER_CFG                     0x8                                                              ///< Default trigger enable setting
#define LC08L32_DEFAULT_TRACE_BUFFER_TYPE               0                                                                ///< Default trace buffer state.
#ifdef _USE_CVI_GUI
  #define LC08L32_MAX_ECHOES_PER_CHANNEL                  60
#else
  #define LC08L32_MAX_ECHOES_PER_CHANNEL                  6
#endif
#define LC08L32_MAX_ECHOES                              (LC08L32_NB_CHANNELS*LC08L32_MAX_ECHOES_PER_CHANNEL)
#define LC08L32_DEFAULT_FIELD_OF_VIEW                   (45*LC08L32_DISTANCE_SCALE)
#define LC08L32_DEFAULT_YAW                             0.0F
#define LC08L32_DEFAULT_PITCH                           0.0F
#define LC08L32_DEFAULT_ROLL                            0.0F
#define LC08L32_DEFAULT_POSITION_X                      0.0F
#define LC08L32_DEFAULT_POSITION_Y                      0.0F
#define LC08L32_DEFAULT_POSITION_Z                      0.0F

// Grabber and led power
#define LC08L32_DEFAULT_LEDPOW_ENABLE                  1
#define LC08L32_DEFAULT_TRANS_IMP_GAIN                 2         ///< Default transimpedance gain.
#define LC08L32_TRANS_IMP_GAIN_MIN                     0         ///< Minimum authorized transimpedance gain.
#define LC08L32_TRANS_IMP_GAIN_MAX                     7         ///< Maximum authorized transimpedance gain.
#define LC08L32_DEFAULT_PWM_WIDTH_1                    0x5
#define LC08L32_DEFAULT_PWM_WIDTH_2                    0x5
#define LC08L32_DEFAULT_PWM_WIDTH_3                    0x5
#define LC08L32_DEFAULT_PWM_WIDTH_4                    0x5
#define LC08L32_DEFAULT_PWM_WIDTH_5                    0x5
#define LC08L32_DEFAULT_PWM_WIDTH_6                    0x5
#define LC08L32_DEFAULT_PWM_WIDTH_7                    0x5
#define LC08L32_DEFAULT_PWM_WIDTH_8                    0x5
#define LC08L32_DEFAULT_PWM_WIDTH_9                    0x5
#define LC08L32_DEFAULT_PWM_WIDTH_10                   0x6
#define LC08L32_DEFAULT_PWM_WIDTH_11                   0x7
#define LC08L32_DEFAULT_PWM_WIDTH_12                   0x9
#define LC08L32_DEFAULT_PWM_WIDTH_13                   0xb
#define LC08L32_DEFAULT_PWM_WIDTH_14                   0xe
#define LC08L32_DEFAULT_PWM_WIDTH_15                   0x14
#define LC08L32_DEFAULT_PWM_WIDTH_16                   0x20
#define LC08L32_DEFAULT_PWM_WIDTH                      { LC08L32_DEFAULT_PWM_WIDTH_1, LC08L32_DEFAULT_PWM_WIDTH_2,  LC08L32_DEFAULT_PWM_WIDTH_3,  LC08L32_DEFAULT_PWM_WIDTH_4,  \
                                                         LC08L32_DEFAULT_PWM_WIDTH_5, LC08L32_DEFAULT_PWM_WIDTH_6,  LC08L32_DEFAULT_PWM_WIDTH_7,  LC08L32_DEFAULT_PWM_WIDTH_8,  \
                                                         LC08L32_DEFAULT_PWM_WIDTH_9, LC08L32_DEFAULT_PWM_WIDTH_10, LC08L32_DEFAULT_PWM_WIDTH_11, LC08L32_DEFAULT_PWM_WIDTH_12,  \
                                                         LC08L32_DEFAULT_PWM_WIDTH_13,LC08L32_DEFAULT_PWM_WIDTH_14, LC08L32_DEFAULT_PWM_WIDTH_15, LC08L32_DEFAULT_PWM_WIDTH_16 }               ///< Default pwm width.                                                                           ///< Default number of clock cycles for each PWM pulse.
#define LC08L32_PWM_WIDTH_MIN                          0                                        ///< Minimum authorized transimpedance gain.
#define LC08L32_PWM_WIDTH_MAX                          0xFF
#define LC08L32_DEFAULT_PWM_PERIOD                     0x32                                     ///< Default time of one PWM cycle.
#define LC08L32_PWM_PERIOD_MIN                         0                                        ///< Default time of one PWM cycle.
#define LC08L32_PWM_PERIOD_MAX                         0xFF                                     ///< Default time of one PWM cycle.
#define LC08L32_NB_PWM_PULSES                          16                                       ///< Number of pwm pulse.
#define LC08L32_PWM_ENABLE_MIN                         0                                        ///< PWM enable min.
#define LC08L32_PWM_ENABLE_MAX                         16

#define LC08L32_DEFAULT_NB_USER_LED_POWER              5
#define LC08L32_DEFAULT_CURRENT_USER_LED_POWER         (LC08L32_DEFAULT_NB_USER_LED_POWER-1)       ///< Current User led power level [in index]
#define LC08L32_NB_USER_LED_POWER_MAX                  16                                          ///< Maximum authorized user led power.
#define LC08L32_NB_USER_FIELD_MAX                      22                                          ///< Maximum authorized field led power.
#define LC08L32_NB_PWM_PULSES                          16
#define LC08L32_DEFAULT_TIME_BASE_DELAY                0xc
#define LC08L32_DEFAULT_RANDOM_CTRL                    0
#define LC08L32_DEFAULT_ADC                            0
#define LC08L32_DEFAULT_REF_PULSE_RATE                 1
#define LC08L32_REF_SEG_MASK                           0x100
#define LC08L32_DEFAULT_REF_SEGMENT_ENABLE             LC08L32_REF_SEG_MASK                     ///< Enable or not the reference pulse by default

// Demerge
#define LC08L32_DEFAULT_DEM_ENABLE                     0         ///< Default demerge enable state.
#define LC08L32_DEM_REF_PULSE_MAX_PTS                  400
#define LC08L32_DEM_LUT_MAX_PTS                        200
#define LC08L32_OBJECT_DEMERGING_MAX                   2

// Peak detector
// Bits field used in pulse struct (valid field)
#define LC08L32_PULSE_INVALID               0     // Reset "valid" field in Pulse struct
#define LC08L32_PULSE_VALID                 0
#define LC08L32_PULSE_MULTOBJ               1
#define LC08L32_PULSE_OVSHOOT               2
#define LC08L32_PULSE_SATURAT               3
#define LC08L32_PULSE_LOWAMP                4
#define LC08L32_PULSE_MULTOBJ2              5
#define LC08L32_PULSE_XTALK                 6
#define LC08L32_PULSE_WPB                   7
#define LC08L32_PULSE_REF                   8

// Peak detector
#define LC08L32_DEFAULT_FILTER_SUM_BITS                13                                                             ///< Number of bits added by the trace filter
#define LC08L32_FILTER_SUM_BITS_MIN                    1
#define LC08L32_FILTER_SUM_BITS_MAX                    16
#define LC08L32_DEFAULT_FILTER_SUM                     (1 <<LC08L32_DEFAULT_FILTER_SUM_BITS )                        ///< The sum of all element in the filter matrix.
#define LC08L32_DEFAULT_FILTERED_SCALE                 (LC08L32_RAW_AMPLITUDE_SCALE * LC08L32_DEFAULT_FILTER_SUM)   ///< Filter scale factor to be expressed in fixed-point.
#define LC08L32_DEFAULT_FILTERED_SCALE_BITS            (LC08L32_DEFAULT_FILTER_SUM_BITS+LC08L32_RAW_AMPLITUDE_SCALE_BITS)
#define LC08L32_DEFAULT_RAW_AMPLITUDE_MAX              (1020*LC08L32_RAW_AMPLITUDE_SCALE)
#define LC08L32_DEFAULT_REAL_DISTANCE_OFFSET           ((int32_t)(2.6*LC08L32_DISTANCE_SCALE))                       ///< Difference in distance between the signal peak and the real distance: 2.6 m
#define LC08L32_REAL_DISTANCE_OFFSET_MIN               ((int32_t)(0*LC08L32_DISTANCE_SCALE))
#define LC08L32_REAL_DISTANCE_OFFSET_MAX               ((int32_t)(10*LC08L32_DISTANCE_SCALE))
#define LC08L32_NB_COEFF_FILTER_MAX                    68                                                             ///< Maximum authorized number of coefficients in a Gaussian filter. BEWARE: must be an even value and a multiple of 4.
#define LC08L32_BAYES_STD_LUT_SIZE_MAX                 5
#define LC08L32_MAG_CORRECTION_LUT_SIZE_MAX            50
#define LC08L32_XTALK_NB_TYPE_MAX                      10
#define LC08L32_SATURATION_DIS_LUT_SIZE_MAX            16
#define LC08L32_SATURATION_AMP_LUT_SIZE_MAX            16
#define LC08L32_ACCUMULATION_DEM_EXPONENT_MIN           6
#define LC08L32_OVERSAMPLING_DEM_EXPONENT_MIN           3
#define LC08L32_REF_PULSE_FACTOR_SCALE_BITS             6

// Crosstalk removal
#define LC08L32_DIFF_EQ_MAX                            8
#define LC08L32_TEMPLATE_MAX                           8
#define LC08L32_DIFF_EQ_COEFF_MAX                      21
#define LC08L32_TEMPLATE_COUNT_MAX                     200

// Static noise removal
#define LC08L32_DEFAULT_STNOIRMV_ENABLE                1                                                               ///< Static noise removal for standard segment
#define LC08L32_DEFAULT_STNOIRMV_ENABLE_REFPULSE       1                                                               ///< Static noise removal for reference segment
#define LC08L32_OVERSAMPLING_EXPONENT_CALIB            3                                                               ///< Maximum oversampling exponent authorized during a learning.
#define LC08L32_OVERSAMPLING_CALIB                     (1 << LC08L32_OVERSAMPLING_EXPONENT_CALIB)                      ///< Maximum oversampling authorized during a learning.
#define LC08L32_NB_BASE_SAMPLE_MIN_CALIB               1                                                               ///< Minimum authorized number base sample.
#define LC08L32_NB_BASE_SAMPLE_MAX_CALIB               15                                                              ///< Maximum authorized number base sample.

// Mathematics
#define LC08L32_DELAY_SCALE_BITS                       16

// Refresh Rate
#define LC08L32_REFRESH_RATE_NUM_SCALE                 (1<<16)
#define LC08L32_REFRESH_RATE_DEN_SCALE                 (1<<10)

#endif  /* __PLATFORMLC08L32DEFINITIONSSHARED_H__ */

// End of PlatformLC08l32DefinitionShared.h

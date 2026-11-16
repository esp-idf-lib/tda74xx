/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ruslan V. Uss <unclerus@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file tda74xx.h
 * @defgroup tda74xx tda74xx
 * @{
 *
 * ESP-IDF driver for TDA7439/TDA7439DS/TDA7440 audioprocessors
 *
 * Copyright (c) 2018 Ruslan V. Uss <unclerus@gmail.com>
 *
 * MIT Licensed as described in the file LICENSE
 */
#ifndef __TDA74XX_H__
#define __TDA74XX_H__

#include <stdbool.h>
#include <i2cdev.h>
#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TDA74XX_ADDR 0x44 //!< I2C address

#define TDA74XX_MAX_INPUT_GAIN 30   //!< Maximum input gain, dB

#define TDA74XX_MIN_VOLUME     -47  //!< Minimum volume level, dB
#define TDA74XX_MAX_VOLUME     0    //!< Maximum volume level, dB

#define TDA74XX_MIN_EQ_GAIN    -14  //!< Minimum equalizer gain, dB
#define TDA74XX_MAX_EQ_GAIN    14   //!< Maximum equalizer gain, dB

#define TDA74XX_MAX_ATTEN      79   //!< Maximum speaker attenuation level, dB

/**
 * Input of the input selector
 *
 * The values are the ones the chip expects, so the numbering runs backwards:
 * ::TDA74XX_INPUT_1 is 3 and ::TDA74XX_INPUT_4 is 0.
 */
typedef enum
{
    TDA74XX_INPUT_4 = 0,
    TDA74XX_INPUT_3,
    TDA74XX_INPUT_2,
    TDA74XX_INPUT_1
} tda74xx_input_t;

/**
 * Audio channel
 */
typedef enum
{
    TDA74XX_CHANNEL_LEFT = 0,
    TDA74XX_CHANNEL_RIGHT
} tda74xx_channel_t;

/**
 * Equalizer band
 */
typedef enum
{
    TDA74XX_BAND_BASS = 0,
    TDA74XX_BAND_MIDDLE,    //!< Not supported on TDA7440
    TDA74XX_BAND_TREBLE,
} tda74xx_band_t;

/**
 * @brief Initialize device descriptor
 *
 * @param dev Device descriptor
 * @param port I2C port number
 * @param sda_gpio GPIO pin number for SDA
 * @param scl_gpio GPIO pin number for SCL
 * @return `ESP_OK` on success, `ESP_ERR_INVALID_ARG` if a port or a pin is
 *         invalid or the pins are the same, `ESP_ERR_NO_MEM` if the mutex
 *         cannot be created
 */
esp_err_t tda74xx_init_desc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio);

/**
 * @brief Free device descriptor
 *
 * @param dev Device descriptor
 * @return `ESP_OK` on success, `ESP_ERR_INVALID_ARG` if the descriptor is NULL
 */
esp_err_t tda74xx_free_desc(i2c_dev_t *dev);

/**
 * @brief Switch input
 *
 * @param dev Device descriptor
 * @param input Input to select
 * @return `ESP_OK` on success, `ESP_ERR_INVALID_ARG` if the input is unknown,
 *         an error code of the I2C bus otherwise
 */
esp_err_t tda74xx_set_input(i2c_dev_t *dev, tda74xx_input_t input);

/**
 * @brief Set input gain
 *
 * @param dev Device descriptor
 * @param gain_db Gain, 0..30 dB in 2 dB steps
 * @return `ESP_OK` on success, `ESP_ERR_INVALID_ARG` if the gain is out of
 *         range or not a multiple of 2 dB, an error code of the I2C bus
 *         otherwise
 */
esp_err_t tda74xx_set_input_gain(i2c_dev_t *dev, uint8_t gain_db);

/**
 * @brief Set master volume
 *
 * The volume control covers 0 to -47 dB in 1 dB steps. The register also has
 * mute codes, and the codes between the last level and the mute codes are not
 * defined by the datasheet: the driver rejects them, so the only way to mute
 * the output is ::tda74xx_set_volume_mute().
 *
 * @param dev Device descriptor
 * @param volume_db Volume, -47..0 dB in 1 dB steps
 * @return `ESP_OK` on success, `ESP_ERR_INVALID_ARG` if the volume is out of
 *         range, an error code of the I2C bus otherwise
 */
esp_err_t tda74xx_set_volume(i2c_dev_t *dev, int8_t volume_db);

/**
 * @brief Mute or unmute the master volume
 *
 * Unmuting sets the volume to 0 dB: the previous level is not restored, call
 * ::tda74xx_set_volume() afterwards if it is needed.
 *
 * @param dev Device descriptor
 * @param mute `true` to mute the output, `false` to unmute it
 * @return `ESP_OK` on success, `ESP_ERR_INVALID_ARG` if the descriptor is NULL,
 *         an error code of the I2C bus otherwise
 */
esp_err_t tda74xx_set_volume_mute(i2c_dev_t *dev, bool mute);

/**
 * @brief Set equalizer gain
 *
 * @param dev Device descriptor
 * @param band Band
 * @param gain_db Gain, -14..14 dB in 2 dB steps
 * @return `ESP_OK` on success, `ESP_ERR_INVALID_ARG` if the band is unknown,
 *         the gain is out of range or not a multiple of 2 dB, an error code
 *         of the I2C bus otherwise
 */
esp_err_t tda74xx_set_equalizer_gain(i2c_dev_t *dev, tda74xx_band_t band, int8_t gain_db);

/**
 * @brief Attenuate speaker
 *
 * @param dev Device descriptor
 * @param channel Audio channel
 * @param atten_db Attenuation, 0..79 dB in 1 dB steps
 * @return `ESP_OK` on success, `ESP_ERR_INVALID_ARG` if the channel is unknown
 *         or the attenuation is out of range, an error code of the I2C bus
 *         otherwise
 */
esp_err_t tda74xx_set_speaker_attenuation(i2c_dev_t *dev, tda74xx_channel_t channel, uint8_t atten_db);

/**
 * @brief Mute or unmute speaker
 *
 * Unmuting sets the attenuation to 0 dB: the previous level is not restored,
 * call ::tda74xx_set_speaker_attenuation() afterwards if it is needed.
 *
 * @param dev Device descriptor
 * @param channel Audio channel
 * @param mute `true` to mute the channel, `false` to unmute it
 * @return `ESP_OK` on success, `ESP_ERR_INVALID_ARG` if the channel is
 *         unknown, an error code of the I2C bus otherwise
 */
esp_err_t tda74xx_set_speaker_mute(i2c_dev_t *dev, tda74xx_channel_t channel, bool mute);

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* __TDA74XX_H__ */

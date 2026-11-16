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
 * @file tda74xx.c
 *
 * ESP-IDF driver for TDA7439/TDA7439DS/TDA7440 audioprocessors
 *
 * Copyright (c) 2018, 2020 Ruslan V. Uss <unclerus@gmail.com>
 *
 * MIT Licensed as described in the file LICENSE
 */

#include <esp_log.h>
#include <esp_idf_lib_helpers.h>
#include "tda74xx.h"

#define I2C_FREQ_HZ 100000 // 100kHz

static const char *TAG = "tda74xx";

#define REG_INPUT_SELECTOR 0x00
#define REG_INPUT_GAIN     0x01
#define REG_VOLUME         0x02
#define REG_BASS_GAIN      0x03
#define REG_MID_GAIN       0x04
#define REG_TREBLE_GAIN    0x05
#define REG_ATTEN_R        0x06
#define REG_ATTEN_L        0x07

#define VOLUME_MUTE 0x38
#define ATTEN_MUTE 0x78

#define INPUT_MAX 0x03

#define EQ_STEP 2
#define EQ_SIGN 0x08
#define EQ_MASK 0x07

#define CHECK(x) do { esp_err_t __; if ((__ = x) != ESP_OK) return __; } while (0)
#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)

static bool input_valid(tda74xx_input_t input)
{
    return input == TDA74XX_INPUT_1 || input == TDA74XX_INPUT_2
           || input == TDA74XX_INPUT_3 || input == TDA74XX_INPUT_4;
}

static esp_err_t band_reg(tda74xx_band_t band, uint8_t *reg)
{
    switch (band)
    {
        case TDA74XX_BAND_BASS:
            *reg = REG_BASS_GAIN;
            return ESP_OK;
        case TDA74XX_BAND_MIDDLE:
            *reg = REG_MID_GAIN;
            return ESP_OK;
        case TDA74XX_BAND_TREBLE:
            *reg = REG_TREBLE_GAIN;
            return ESP_OK;
        default:
            return ESP_ERR_INVALID_ARG;
    }
}

static esp_err_t channel_reg(tda74xx_channel_t channel, uint8_t *reg)
{
    switch (channel)
    {
        case TDA74XX_CHANNEL_LEFT:
            *reg = REG_ATTEN_L;
            return ESP_OK;
        case TDA74XX_CHANNEL_RIGHT:
            *reg = REG_ATTEN_R;
            return ESP_OK;
        default:
            return ESP_ERR_INVALID_ARG;
    }
}

// The tone controls do not use a plain offset: below zero dB the register
// counts up from 0x00 (0 = -14 dB, 0x07 = 0 dB), above zero dB it counts down
// from 0x08 (0x08 = +14 dB, 0x0e = +2 dB, 0x0f = 0 dB again).
static uint8_t eq_value(int8_t gain_db)
{
    return gain_db < 0
           ? (uint8_t)((gain_db - TDA74XX_MIN_EQ_GAIN) / EQ_STEP)
           : (uint8_t)(EQ_SIGN | ((TDA74XX_MAX_EQ_GAIN - gain_db) / EQ_STEP));
}

static esp_err_t write_reg(i2c_dev_t *dev, uint8_t reg, uint8_t val)
{
    I2C_DEV_TAKE_MUTEX(dev);
    I2C_DEV_CHECK(dev, i2c_dev_write_reg(dev, reg, &val, 1));
    I2C_DEV_GIVE_MUTEX(dev);

    ESP_LOGD(TAG, "[%02x] = %02x", reg, val);

    return ESP_OK;
}

esp_err_t tda74xx_init_desc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    CHECK_ARG(dev && port >= 0 && sda_gpio >= 0 && scl_gpio >= 0 && sda_gpio != scl_gpio);

    dev->port = port;
    dev->addr = TDA74XX_ADDR;
    dev->cfg.sda_io_num = sda_gpio;
    dev->cfg.scl_io_num = scl_gpio;
    dev->cfg.master.clk_speed = I2C_FREQ_HZ;

    return i2c_dev_create_mutex(dev);
}

esp_err_t tda74xx_free_desc(i2c_dev_t *dev)
{
    CHECK_ARG(dev);

    return i2c_dev_delete_mutex(dev);
}

esp_err_t tda74xx_set_input(i2c_dev_t *dev, tda74xx_input_t input)
{
    CHECK_ARG(dev && input_valid(input));

    return write_reg(dev, REG_INPUT_SELECTOR, (uint8_t)input);
}

esp_err_t tda74xx_set_input_gain(i2c_dev_t *dev, uint8_t gain_db)
{
    CHECK_ARG(dev && gain_db <= TDA74XX_MAX_INPUT_GAIN && gain_db % EQ_STEP == 0);

    return write_reg(dev, REG_INPUT_GAIN, gain_db / EQ_STEP);
}

esp_err_t tda74xx_set_volume(i2c_dev_t *dev, int8_t volume_db)
{
    CHECK_ARG(dev && volume_db <= TDA74XX_MAX_VOLUME && volume_db >= TDA74XX_MIN_VOLUME);

    return write_reg(dev, REG_VOLUME, (uint8_t) - volume_db);
}

esp_err_t tda74xx_set_volume_mute(i2c_dev_t *dev, bool mute)
{
    CHECK_ARG(dev);

    return write_reg(dev, REG_VOLUME, mute ? VOLUME_MUTE : 0);
}

esp_err_t tda74xx_set_equalizer_gain(i2c_dev_t *dev, tda74xx_band_t band, int8_t gain_db)
{
    CHECK_ARG(dev && gain_db >= TDA74XX_MIN_EQ_GAIN && gain_db <= TDA74XX_MAX_EQ_GAIN
              && gain_db % EQ_STEP == 0);

    uint8_t reg;
    CHECK(band_reg(band, &reg));

    return write_reg(dev, reg, eq_value(gain_db));
}

esp_err_t tda74xx_set_speaker_attenuation(i2c_dev_t *dev, tda74xx_channel_t channel, uint8_t atten_db)
{
    CHECK_ARG(dev && atten_db <= TDA74XX_MAX_ATTEN);

    uint8_t reg;
    CHECK(channel_reg(channel, &reg));

    return write_reg(dev, reg, atten_db);
}

esp_err_t tda74xx_set_speaker_mute(i2c_dev_t *dev, tda74xx_channel_t channel, bool mute)
{
    CHECK_ARG(dev);

    uint8_t reg;
    CHECK(channel_reg(channel, &reg));

    return write_reg(dev, reg, mute ? ATTEN_MUTE : 0);
}


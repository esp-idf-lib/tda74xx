#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <tda74xx.h>

void tda74xx_test(void *pvParameters)
{
    i2c_dev_t dev;
    memset(&dev, 0, sizeof(i2c_dev_t));

    // init device descriptor
    ESP_ERROR_CHECK(tda74xx_init_desc(&dev, 0, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));

    // Disable attenuation
    ESP_ERROR_CHECK(tda74xx_set_speaker_attenuation(&dev, TDA74XX_CHANNEL_LEFT, 0));
    ESP_ERROR_CHECK(tda74xx_set_speaker_attenuation(&dev, TDA74XX_CHANNEL_RIGHT, 0));

    // Set input gain
    ESP_ERROR_CHECK(tda74xx_set_input_gain(&dev, CONFIG_EXAMPLE_INPUT_GAIN));

    // Switch input: the configuration holds the input number, 1..4
    ESP_ERROR_CHECK(tda74xx_set_input(&dev, (tda74xx_input_t)(4 - CONFIG_EXAMPLE_INPUT_CHANNEL)));

    // Set volume & eq
    ESP_ERROR_CHECK(tda74xx_set_volume(&dev, CONFIG_EXAMPLE_VOLUME));
    ESP_ERROR_CHECK(tda74xx_set_equalizer_gain(&dev, TDA74XX_BAND_BASS, CONFIG_EXAMPLE_BASS));
    ESP_ERROR_CHECK(tda74xx_set_equalizer_gain(&dev, TDA74XX_BAND_TREBLE, CONFIG_EXAMPLE_TREBLE));
#ifdef CONFIG_EXAMPLE_USING_MIDDLE_RANGE_CONTROL
    ESP_ERROR_CHECK(tda74xx_set_equalizer_gain(&dev, TDA74XX_BAND_MIDDLE, CONFIG_EXAMPLE_MIDDLE));
#endif

    int8_t vol = TDA74XX_MIN_VOLUME;
    while (1)
    {
        ESP_ERROR_CHECK(tda74xx_set_volume(&dev, vol));
        if (++vol > 0)
            vol = TDA74XX_MIN_VOLUME;
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

void app_main()
{
    // Init i2cdev library
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreate(tda74xx_test, "tda74xx_test", 8192, NULL, 5, NULL);
}


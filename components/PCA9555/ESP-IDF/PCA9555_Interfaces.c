///
/// PCA9555 driver
///

#include <stdarg.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

#include "PCA9555_Interfaces.h"

/// @brief Start I2C bus with the given config
/// @param config
/// @return 0 if successful, -1 if failed
int pca9555_i2c_start(const pca9555_config_t *config)
{
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = config->i2c_sda,
        .scl_io_num = config->i2c_scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = config->i2c_freq_hz,
    };
    esp_err_t err = i2c_param_config(config->i2c_iface_num, &i2c_config);
    if (err != ESP_OK)
    {
        // ESP_LOGE(TAG, "I2C param config failed: %s", esp_err_to_name(err));
        return -1;
    }
    err = i2c_driver_install(config->i2c_iface_num, I2C_MODE_MASTER, 0, 0, 0);
    if (err != ESP_OK)
    {
        // ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(err));
        return -1;
    }
    return 0;
}

/// @brief Send I2C command
/// @param i2c_num I2C interface number
/// @param i2c_addr I2C address of the device
/// @param data Pointer to the data to send
/// @param len Length of the data
/// @return 0 if successful, -1 if failed
int pca9555_i2c_send(int i2c_num, int i2c_addr, const void *data, size_t len)
{
    // Split the data into chunks if necessary
    esp_err_t err = ESP_OK;
    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();
    err = i2c_master_start(cmd);
    err |= i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_WRITE, true);

    err |= i2c_master_write(cmd, (const uint8_t *)data, len, true);

    err |= i2c_master_stop(cmd);
    err |= i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(2));
    i2c_cmd_link_delete(cmd);

    if (err != ESP_OK)
    {
        ESP_LOGE("PCA9555", "I2C send failed: %s", esp_err_to_name(err));
        return -1;
    }
    return 0;
}

/// @brief Receive I2C command
/// @param i2c_addr I2C address of the device
/// @param data_buffer Pointer to the data buffer to receive data
/// @param len Length of the data to receive
/// @return 0 if successful, -1 if failed
int pca9555_i2c_receive(int i2c_addr, void *data_buffer, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(cmd);
    err |= i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_READ, true);
    if (len > 1)
    {
        err |= i2c_master_read(cmd, (uint8_t *)data_buffer, len - 1, I2C_MASTER_ACK);
    }
    err |= i2c_master_read_byte(cmd, ((uint8_t *)data_buffer) + len - 1, I2C_MASTER_NACK);
    err |= i2c_master_stop(cmd);
    err |= i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(5));
    i2c_cmd_link_delete(cmd);

    if (err != ESP_OK)
    {
        // ESP_LOGE(TAG, "I2C receive failed: %s", esp_err_to_name(err));
        return -1;
    }
    return 0;
}

int pca9555_log(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    return 0;
}
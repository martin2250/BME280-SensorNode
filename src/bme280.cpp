#include "Arduino.h"
#include "lib/bme280.h"
#include <Wire.h>

void user_delay_ms(uint32_t period)
{
	delay(period);
}

int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

	Wire.beginTransmission(dev_id);
	Wire.write(reg_addr);
	rslt = Wire.endTransmission();

	if (rslt)
		return rslt;

	rslt = (Wire.requestFrom(dev_id, (uint8_t)len) != len);

	if (rslt)
		return rslt;

	while (len--)
		*reg_data++ = Wire.read();

	return rslt;
}

int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

	Wire.beginTransmission(dev_id);
	Wire.write(reg_addr);
	while (len--)
		Wire.write(*reg_data++);
	rslt = Wire.endTransmission();

	return rslt;
}

bool setup_bme280(struct bme280_dev &dev)
{
	int8_t rslt = BME280_OK;

	dev.dev_id = BME280_I2C_ADDR_PRIM;
	dev.intf = BME280_I2C_INTF;
	dev.read = user_i2c_read;
	dev.write = user_i2c_write;
	dev.delay_ms = user_delay_ms;

	rslt = bme280_init(&dev);
	if (rslt)
		return false;

	dev.settings.osr_h = BME280_OVERSAMPLING_1X;
	dev.settings.osr_p = BME280_OVERSAMPLING_1X;
	dev.settings.osr_t = BME280_OVERSAMPLING_1X;
	dev.settings.filter = BME280_FILTER_COEFF_OFF;

	uint8_t settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;
	rslt = bme280_set_sensor_settings(settings_sel, &dev);
	if (rslt)
		return false;

	rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
	if (rslt)
		return false;

	return true;
}

bool read_bme280(struct bme280_dev &dev, struct bme280_data &data)
{
	return !bme280_get_sensor_data(BME280_TEMP | BME280_HUM, &data, &dev);
}

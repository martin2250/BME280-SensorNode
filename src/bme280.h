#ifndef BME280_INTERNAL_H_
#define BME280_INTERNAL_H_

#include "lib/bme280.h"

bool setup_bme280(struct bme280_dev &dev);
bool read_bme280(struct bme280_dev &dev, struct bme280_data &data);

#endif

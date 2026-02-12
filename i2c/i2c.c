#include "i2c.h"

static int __init i2c_init(void);
// module_init(i2c_init);

int i2c_write(struct i2c_driver *client, uint8_t reg_addr, uint16_t data);

int i2c_read(struct i2c_driver *client, uint8_t reg_addr);

int i2c_probe(struct i2c_driver *client); // method to bind the driver to the i2c device, should be called in init

static void i2c_remove(struct i2c_driver *client);

static int __exit i2c_close();
// module_exit(i2c_close);
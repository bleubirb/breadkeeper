/** C code to attempt to make an i2c driver for the raspi zero w 2 from scratch just to learn
 * raspi zero w 2 uses linux server LTS 24.04
*/

#ifndef I2C_H
#define I2C_H

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/module.h>
#include <stdlib.h>
#include <stdint.h>

// getting information on i2c driver from linux kernel docs
// https://docs.kernel.org/i2c/writing-clients.html
static const struct i2c_device_id pi_idtable[] = {
    {"pi_i2c", 0},
    { } // null terminator for the list
};

MODULE_DEVICE_TABLE(i2c, pi_idtable);

static struct i2c_driver pi_driver = {
    .driver = {
        .name = "pi_driver",
        .pm = NULL // setting to null for now, power management shouldn't be needed
    },
    .id_table = pi_idtable,
    .probe = i2c_probe, // call on init to bind driver to device
    .remove = i2c_remove, // call on exit to unbind driver from device
    .shutdown = NULL, // optionally implement shutdown function, not sure if needed
};

static int __init i2c_init(void);
// module_init(i2c_init);

int i2c_write(struct i2c_driver *client, uint8_t reg_addr, uint16_t data);

int i2c_read(struct i2c_driver *client, uint8_t reg_addr);

int i2c_probe(struct i2c_driver *client); // method to bind the driver to the i2c device, should be called in init

static void i2c_remove(struct i2c_driver *client);

static int __exit i2c_close();
// module_exit(i2c_close);

#endif /* I2C_H */
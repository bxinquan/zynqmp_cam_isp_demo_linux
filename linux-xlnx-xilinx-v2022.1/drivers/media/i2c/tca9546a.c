#include <linux/i2c.h>
#include <linux/delay.h>

static int tca9546a_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	int ret;

#define CHANNEL_ID  2
	unsigned char data[1] = { CHANNEL_ID };
	ret = i2c_master_send(client, data, 1);
	if (ret < 0) {
		dev_dbg(dev, "%s: i2c write error, reg: %x\n",
				__func__, data[0]);
		return ret;
	}

	msleep(10);

	dev_info(dev, "tca9546a i2c switcher driver probed\n");

	return 0;
}

static int tca9546a_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id tca9546a_id[] = {
	{ "tca9546a", 0 },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(i2c, tca9546a_id);

#if IS_ENABLED(CONFIG_OF)
static const struct of_device_id tca9546a_of_match[] = {
	{ .compatible = "tca9546a" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, tca9546a_of_match);
#endif

static struct i2c_driver tca9546a_driver = {
	.driver = {
		.of_match_table = of_match_ptr(tca9546a_of_match),
		.name	= "tca9546a",
	},
	.probe_new	= tca9546a_probe,
	.remove		= tca9546a_remove,
	.id_table	= tca9546a_id,
};

module_i2c_driver(tca9546a_driver);

MODULE_AUTHOR("bxinquan");
MODULE_DESCRIPTION("tca9546a i2c switcher");
MODULE_LICENSE("GPL v2");

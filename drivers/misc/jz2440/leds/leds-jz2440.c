/* drivers/leds/leds-s3c24xx.c
 *
 * (c) 2006 Simtec Electronics
 *	http://armlinux.simtec.co.uk/
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * S3C24XX - LEDs GPIO driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_data/leds-s3c24xx.h>

#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/of.h>

#define LED_NUM 	3

static struct pinctrl *pinctrl1;
static struct pinctrl_state *led1_on_state, *led1_off_state, 
					   *led2_on_state, *led2_off_state,
					   *led3_on_state, *led3_off_state; 

/* our context */

struct s3c24xx_gpio_led {
	struct led_classdev		 cdev;
	struct s3c24xx_led_platdata	*pdata;
	int led_index;
};

static void s3c24xx_led_set(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
	int index;
	struct s3c24xx_gpio_led *s3c24xx_led = (struct s3c24xx_gpio_led *)led_cdev;

	index = s3c24xx_led->led_index;
	pr_info("get led index = %d\n", index);

	if(value > 0)
	{
		switch(index){
			case 1:
				pinctrl_select_state(pinctrl1, led1_on_state);
				break;
			case 2:
				pinctrl_select_state(pinctrl1, led2_on_state);
				break;
			case 3:
				pinctrl_select_state(pinctrl1, led3_on_state);
				break;
			default:
				pr_err("invalue led index!\n");
				break;
		}
	}
	else
	{
		switch(index){
			case 1:
				pinctrl_select_state(pinctrl1, led1_off_state);
				break;
			case 2:
				pinctrl_select_state(pinctrl1, led2_off_state);
				break;
			case 3:
				pinctrl_select_state(pinctrl1, led3_off_state);
				break;
			default:
				pr_err("invalue led index!\n");
				break;
		}
	}
	
}

static void s3c24xx_led_gpio_get(struct device *dev)
{
	int ret;

	pinctrl1 = devm_pinctrl_get(dev);
	if (IS_ERR_OR_NULL(pinctrl1)) {
		dev_err(dev, "s3c24xx led gpio get failed!\n");
        goto out;
	}

	led1_on_state = pinctrl_lookup_state(pinctrl1, "led1_on");
	if (IS_ERR_OR_NULL(led1_on_state)) {
		PTR_ERR(led1_on_state);
		goto out;
	}
	led1_off_state = pinctrl_lookup_state(pinctrl1, "led1_off");
	if (IS_ERR_OR_NULL(led1_off_state)) {
		PTR_ERR(led1_off_state);
		goto out;
	}
	led2_on_state = pinctrl_lookup_state(pinctrl1, "led2_on");
	if (IS_ERR_OR_NULL(led2_on_state)) {
		PTR_ERR(led2_on_state);
		goto out;
	}
	led2_off_state = pinctrl_lookup_state(pinctrl1, "led2_off");
	if (IS_ERR_OR_NULL(led2_off_state)) {
		PTR_ERR(led2_off_state);
		goto out;
	}
	led3_on_state = pinctrl_lookup_state(pinctrl1, "led3_on");
	if (IS_ERR_OR_NULL(led3_on_state)) {
		PTR_ERR(led3_on_state);
		goto out;
	}
	led3_off_state = pinctrl_lookup_state(pinctrl1, "led3_off");
	if (IS_ERR_OR_NULL(led3_off_state)) {
		PTR_ERR(led3_off_state);
		goto out;
	}

	return;

out:
	pr_err("s3c24xx get led gpio info faile!\n");
}

static struct s3c24xx_gpio_led jz2440_leds[LED_NUM] = {
		{
				.cdev = {
						.brightness_set = s3c24xx_led_set,
						.flags = LED_CORE_SUSPENDRESUME,
						.name = "led1",
				},
				.led_index = 1,
		},
		{
				.cdev = {
						.brightness_set = s3c24xx_led_set,
						.flags = LED_CORE_SUSPENDRESUME,
						.name = "led2",
				},
				.led_index = 2,
		},
		{
				.cdev = {
						.brightness_set = s3c24xx_led_set,
						.flags = LED_CORE_SUSPENDRESUME,
						.name = "led3",
				},
				.led_index = 3,
		},
};

static int s3c24xx_led_probe(struct platform_device *dev)
{
	int ret;
	int i;

	pr_info("enter %s\n", __func__);


#if CONFIG_OF
	s3c24xx_led_gpio_get(&dev->dev);
#else
	ret = devm_gpio_request(&dev->dev, pdata->gpio, "S3C24XX_LED");
	if (ret < 0)
		return ret;
	/* no point in having a pull-up if we are always driving */

	s3c_gpio_setpull(pdata->gpio, S3C_GPIO_PULL_NONE);

	if (pdata->flags & S3C24XX_LEDF_TRISTATE)
		gpio_direction_input(pdata->gpio);
	else
		gpio_direction_output(pdata->gpio,
			pdata->flags & S3C24XX_LEDF_ACTLOW ? 1 : 0);
#endif

	/* register our new led device */

	for(i=0;i<LED_NUM;i++)
	{
		ret = devm_led_classdev_register(&dev->dev, &jz2440_leds[i].cdev);
		if (ret < 0)
			dev_err(&dev->dev, "led_classdev_register failed\n");
	}

	return ret;
}

static struct of_device_id s3c24xx_led_ids[] = {
	{.compatible = "jz2440-led"},
	{},
};

static struct platform_driver s3c24xx_led_driver = {
	.probe		= s3c24xx_led_probe,
	.driver		= {
		.name		= "s3c24xx_led",
#if CONFIG_OF
		.of_match_table = s3c24xx_led_ids,
#endif
	},
};

module_platform_driver(s3c24xx_led_driver);

MODULE_AUTHOR("fitch <fitchwa@163.com>");
MODULE_DESCRIPTION("JZ2440 LED driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:s3c24xx_led");

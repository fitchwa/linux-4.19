/* 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#define pr_fmt(fmt) KBUILD_MODNAME ": "fmt 

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>


enum button_index {
    BUTTON_0 = 0,
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_MAX,
};

struct jz2440_button_info {
	int pin;
    int irq;
};
static struct jz2440_button_info g_info[BUTTON_MAX];

static irqreturn_t button_irq_handler(int irq, void *unsed);

static int jz2440_get_button_info(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
    struct resource *res;
	struct pinctrl *pinctrl1;
    int i;

	pinctrl1 = devm_pinctrl_get_select(dev, "gpio_as_button");
    if(IS_ERR_OR_NULL(pinctrl1)){
        pr_err("get pinctrl failed!\n");
        return -ENOMEM;
    }       

    for(i=0;i<BUTTON_MAX;i++){
        res = platform_get_resource(pdev, IORESOURCE_IRQ, i);
        if(res){
            g_info[i].irq = res->start;
            pr_info("get irq %d\n", g_info[i].irq);
        }
        else{
            pr_err("could't get irq res!\n");
            return -ENOMEM;
        }

        g_info[i].pin = of_get_named_gpio(np, "eint-pins", i);
        pr_info("get pin %d\n", g_info[i].pin);
    }
            
    return 0;
}

static irqreturn_t button_irq_handler(int irq, void *unused)
{
    struct jz2440_button_info *info = (struct jz2440_button_info *)unused;

    pr_info("irqhandler:%d\n", info->irq);

    return IRQ_HANDLED;
}

static int jz2440_set_button_irq(void)
{
    int ret;

    ret = request_irq(g_info[0].irq, button_irq_handler, IRQF_TRIGGER_FALLING, "S2", &g_info[0]);
    if(ret){
        pr_err("request_irq %d err!\n", g_info[0].irq);
        return ret;
    }
    ret = request_irq(g_info[1].irq, button_irq_handler, IRQF_TRIGGER_FALLING, "S3", &g_info[1]);
    if(ret){
        pr_err("request_irq %d err!\n", g_info[1].irq);
        return ret;
    }
    ret = request_irq(g_info[2].irq, button_irq_handler, IRQF_TRIGGER_FALLING, "S4", &g_info[2]);
    if(ret){
        pr_err("request_irq %d err!\n", g_info[2].irq);
        return ret;
    }
    ret = request_irq(g_info[3].irq, button_irq_handler, IRQF_TRIGGER_FALLING, "S5", &g_info[3]);
    if(ret){
        pr_err("request_irq %d err!\n", g_info[3].irq);
        return ret;
    }

    return 0;
}

static int jz2440_button_probe(struct platform_device *pdev)
{
    int ret;

    ret = jz2440_get_button_info(pdev);
    if(ret){
        goto out;
    }

    ret = jz2440_set_button_irq();
    if(ret){
        goto out;
    }

	return 0;

out:
    return ret;
}

static struct of_device_id jz2440_button_ids[] = {
	{.compatible = "jz2440-button"},
	{},
};

static struct platform_driver jz2440_button_driver = {
	.probe		= jz2440_button_probe,
	.driver		= {
		.name		= "jz2440_button",
#if CONFIG_OF
		.of_match_table = jz2440_button_ids,
#endif
	},
};

module_platform_driver(jz2440_button_driver);

MODULE_AUTHOR("fitch <fitchwa@163.com>");
MODULE_DESCRIPTION("JZ2440 LED driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:s3c24xx_button");

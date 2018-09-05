/*
 * pwm-mockup driver
 *
 * Copyright (C) Paulo Neves
 *
 * Author: Paulo Neves <ptsneves@gmail.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/clk.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/slab.h>

#define MODULE_NAME "gpio-fake"
#define pwm_mockup_err(...) pr_err(MODULE_NAME ": " __VA_ARGS__)
#define pwm_mockup_info(...) pr_info(MODULE_NAME ": " __VA_ARGS__)
#define pwm_mockup_MAX_DEVICES 1

struct pwm_mockup_pwm_chip {
  struct pwm_chip chip;
};

static inline struct pwm_mockup_pwm_chip *to_pwm_mockup_pwm_chip(struct pwm_chip *chip)
{
  return container_of(chip, struct pwm_mockup_pwm_chip, chip);
}

static int pwm_mockup_pwm_request(struct pwm_chip *chip, struct pwm_device *pwm)
{
  return 0;
}

static void pwm_mockup_pwm_free(struct pwm_chip *chip, struct pwm_device *pwm)
{
  pwm_set_chip_data(pwm, NULL);
}

static int pwm_mockup_pwm_config(struct pwm_chip *chip, struct pwm_device *pwm_dev,
           int duty_ns, int period_ns)
{
  pwm_mockup_info("pin:%d;duty_ns:%d,period_ns:%d", pwm_dev->hwpwm, duty_ns, period_ns);
  return 0;
}

static int pwm_mockup_pwm_set_polarity(struct pwm_chip *chip,
           struct pwm_device *pwm_dev,
           enum pwm_polarity polarity)
{
  pwm_mockup_info("pin:%d;polarity:1", pwm_dev->hwpwm);
  return 0;
}

static int pwm_mockup_pwm_enable(struct pwm_chip *chip, struct pwm_device *pwm_dev)
{
  pwm_mockup_info("pin:%d;enable:1", pwm_dev->hwpwm);
  return 0;
}

static void pwm_mockup_pwm_disable(struct pwm_chip *chip,
             struct pwm_device *pwm_dev)
{
  pwm_mockup_info("pin:%d;enable:0", pwm_dev->hwpwm);
}

static const struct pwm_ops pwm_mockup_pwm_ops = {
  .request = pwm_mockup_pwm_request,
  .free = pwm_mockup_pwm_free,
  .config = pwm_mockup_pwm_config,
  .set_polarity = pwm_mockup_pwm_set_polarity,
  .enable = pwm_mockup_pwm_enable,
  .disable = pwm_mockup_pwm_disable,
  .owner = THIS_MODULE,
};

static int pwm_mockup_pwm_probe(struct platform_device *pdev)
{
  struct pwm_mockup_pwm_chip *pwm;
  int ret;

  pwm = devm_kzalloc(&pdev->dev, sizeof(*pwm), GFP_KERNEL);
  if (!pwm)
    return -ENOMEM;


  pwm->chip.base = -1;
  pwm->chip.dev = &pdev->dev;
  pwm->chip.ops = &pwm_mockup_pwm_ops;
  pwm->chip.npwm = 4;
  pwm->chip.of_pwm_n_cells = 3;

  ret = pwmchip_add(&pwm->chip);
  if (ret < 0) {
    pwm_mockup_err("failed to add PWM chip: %d\n", ret);
    return ret;
  }

  platform_set_drvdata(pdev, pwm);

  pwm_mockup_info("Probed");
  return 0;
}

static int pwm_mockup_pwm_remove(struct platform_device *pdev)
{
  struct pwm_mockup_pwm_chip *pwm = platform_get_drvdata(pdev);
  int ret;

  ret = pwmchip_remove(&pwm->chip);
  return ret;
}

static struct platform_driver pwm_mockup_pwm_driver = {
  .probe = pwm_mockup_pwm_probe,
  .remove = pwm_mockup_pwm_remove,
  .driver = {
    .name = MODULE_NAME,
  },
};

static struct platform_device *pwm_mockup_pdevs[pwm_mockup_MAX_DEVICES];

static void pwm_mockup_unregister_pdevs(void)
{
  struct platform_device *pdev;
  int i;

  for (i = 0; i < pwm_mockup_MAX_DEVICES; i++) {
    pdev = pwm_mockup_pdevs[i];

    if (pdev)
      platform_device_unregister(pdev);
  }
}

static int __init pwm_mockup_init(void) {
  int err = 0;
  int i = 0;
  struct platform_device *pdev = NULL;

  err = platform_driver_register(&pwm_mockup_pwm_driver);
  if (err) {
    pwm_mockup_err("pwm_mockup: could not register driver");
    return err;
  }

  for (; i < pwm_mockup_MAX_DEVICES; i++) {
    pdev = platform_device_register_simple(MODULE_NAME, i, NULL, 0);
    if (IS_ERR(pdev)) {
      pwm_mockup_err("pwm_mockup: error registring device");
      platform_driver_unregister(&pwm_mockup_pwm_driver);
      if (pdev) {
        platform_device_unregister(pdev);
      }
      return PTR_ERR(pdev);
    }
    pwm_mockup_pdevs[i] = pdev;
  }

  return 0;
}

static void __exit pwm_mockup_exit(void)
{
  platform_driver_unregister(&pwm_mockup_pwm_driver);
  pwm_mockup_unregister_pdevs();
}

module_init(pwm_mockup_init);
module_exit(pwm_mockup_exit);

MODULE_AUTHOR("Paulo Neves <ptsneves@gmail.com>");
MODULE_DESCRIPTION("Mockup PWM driver");
MODULE_LICENSE("GPL v2");

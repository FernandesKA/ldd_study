#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/err.h>

#define GPIO_NUM 17

static struct gpio_desc *desc;
static unsigned int irq_number;
static int irq_counter = 0;

static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    irq_counter++;
    pr_info("GPIO IRQ: Interrupt occurred %d times\n", irq_counter);
    return IRQ_HANDLED;
}

static int __init gpio_irq_init(void)
{
    int result;

    desc = gpiod_get_index(NULL, NULL, GPIO_NUM, GPIOD_IN);
    if (IS_ERR(desc)) {
        pr_err("Failed to get gpio descriptor\n");
        return PTR_ERR(desc);
    }

    // irq number из дескриптора
    irq_number = gpiod_to_irq(desc);
    pr_info("GPIO to IRQ mapping: GPIO %d -> IRQ %d\n", GPIO_NUM, irq_number);

    result = request_irq(irq_number, gpio_irq_handler,
                         IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                         "gpio_irq_handler", NULL);
    if (result) {
        pr_err("Unable to request IRQ: %d\n", result);
        gpiod_put(desc);
        return result;
    }

    pr_info("GPIO IRQ driver loaded\n");
    return 0;
}

static void __exit gpio_irq_exit(void)
{
    free_irq(irq_number, NULL);
    gpiod_put(desc);
    pr_info("GPIO IRQ driver unloaded\n");
}

module_init(gpio_irq_init);
module_exit(gpio_irq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FernandesKA");
MODULE_DESCRIPTION("GPIO IRQ driver");

#include "gpio_reaction.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define GPIO_REACTION_NODE DT_PATH(zephyr_user)
#define GPIO_REACTION_DELAY_MS DT_PROP(GPIO_REACTION_NODE, reaction_delay_ms)

BUILD_ASSERT(0 < GPIO_REACTION_DELAY_MS, "GPIO reaction delay must be positive");

static const struct gpio_dt_spec reaction_input =
    GPIO_DT_SPEC_GET(GPIO_REACTION_NODE, reaction_input_gpios);
static const struct gpio_dt_spec reaction_output =
    GPIO_DT_SPEC_GET(GPIO_REACTION_NODE, reaction_output_gpios);
static struct gpio_callback reaction_input_callback;
static struct k_work_delayable reaction_output_work;

static void toggle_gpio_output(struct k_work *work)
{
    (void)work;

    int toggle_result = gpio_pin_toggle_dt(&reaction_output);

    if (0 != toggle_result)
    {
        printk("GPIO output failed: %d\n", toggle_result);
    }
}



static void schedule_gpio_output_change(const struct device *port,
                                       struct gpio_callback *callback,
                                       uint32_t changed_pins)
{
    (void)port;
    (void)callback;
    (void)changed_pins;

    k_work_reschedule(&reaction_output_work, K_MSEC(GPIO_REACTION_DELAY_MS));
}



int initialize_gpio_reaction(void)
{
    if (false == gpio_is_ready_dt(&reaction_input) ||
        false == gpio_is_ready_dt(&reaction_output))
    {
        return -ENODEV;
    }

    int configuration_result =
        gpio_pin_configure_dt(&reaction_output, GPIO_OUTPUT_INACTIVE);

    if (0 != configuration_result)
    {
        return configuration_result;
    }

    configuration_result = gpio_pin_configure_dt(&reaction_input, GPIO_INPUT);

    if (0 != configuration_result)
    {
        return configuration_result;
    }

    k_work_init_delayable(&reaction_output_work, toggle_gpio_output);
    gpio_init_callback(&reaction_input_callback, schedule_gpio_output_change,
                       BIT(reaction_input.pin));

    configuration_result =
        gpio_add_callback(reaction_input.port, &reaction_input_callback);

    if (0 != configuration_result)
    {
        return configuration_result;
    }

    configuration_result =
        gpio_pin_interrupt_configure_dt(&reaction_input, GPIO_INT_EDGE_BOTH);

    if (0 != configuration_result)
    {
        gpio_remove_callback(reaction_input.port, &reaction_input_callback);
    }

    return configuration_result;
}

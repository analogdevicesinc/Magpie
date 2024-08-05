/**
 * @file  gpio_helpers.h
 * @brief A software module with a few GPIO convenience functions is represented here.
 */

#ifndef GPIO_HELPERS_H_
#define GPIO_HELPERS_H_

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include "mxc_device.h"
#include <stdbool.h>

/* Public enumerations -----------------------------------------------------------------------------------------------*/

/**
 * These pins are available on J14 of the FTHR2
 */
typedef enum
{
    GPIO_PROFILING_PIN_P1_8 = MXC_GPIO_PIN_8,
    GPIO_PROFILING_PIN_P1_9 = MXC_GPIO_PIN_9,
    GPIO_PROFILING_PIN_P1_10 = MXC_GPIO_PIN_10,
    GPIO_PROFILING_PIN_P1_11 = MXC_GPIO_PIN_11,
} GPIO_Profiling_Pin_t;

/* Public function declarations --------------------------------------------------------------------------------------*/

void gpio_write_pin(const mxc_gpio_cfg_t *pin, bool state);

void gpio_toggle_pin(const mxc_gpio_cfg_t *pin);

bool gpio_read_pin(const mxc_gpio_cfg_t *pin);

void gpio_profiling_pin_init();

void gpio_profiling_pin_write(GPIO_Profiling_Pin_t pin, bool state);

#endif /* GPIO_HELPERS_H_ */

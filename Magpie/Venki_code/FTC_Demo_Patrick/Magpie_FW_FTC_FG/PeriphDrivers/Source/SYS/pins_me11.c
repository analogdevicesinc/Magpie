/**
 * @file mxc_pins.c
 * @brief      This file contains constant pin configurations for the peripherals.
 */

/******************************************************************************
 *
 * Copyright (C) 2022-2023 Maxim Integrated Products, Inc. All Rights Reserved.
 * (now owned by Analog Devices, Inc.),
 * Copyright (C) 2023 Analog Devices, Inc. All Rights Reserved. This software
 * is proprietary to Analog Devices, Inc. and its licensors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

#include "gpio.h"
#include "mxc_device.h"

/***** Definitions *****/

/***** Global Variables *****/

// clang-format off
const mxc_gpio_cfg_t gpio_cfg_swda = { MXC_GPIO0, (MXC_GPIO_PIN_0 | MXC_GPIO_PIN_1), MXC_GPIO_FUNC_ALT1,
                                       MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_swdb = { MXC_GPIO0, (MXC_GPIO_PIN_8 | MXC_GPIO_PIN_9), MXC_GPIO_FUNC_ALT2,
                                       MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };

const mxc_gpio_cfg_t gpio_cfg_i2c0 = { MXC_GPIO0, (MXC_GPIO_PIN_8 | MXC_GPIO_PIN_9), MXC_GPIO_FUNC_ALT1,
                                       MXC_GPIO_PAD_PULL_UP, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_i2c1 = { MXC_GPIO0, (MXC_GPIO_PIN_2 | MXC_GPIO_PIN_3), MXC_GPIO_FUNC_ALT1,
                                       MXC_GPIO_PAD_PULL_UP, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };

const mxc_gpio_cfg_t gpio_cfg_uart0 = { MXC_GPIO0, (MXC_GPIO_PIN_4 | MXC_GPIO_PIN_5), MXC_GPIO_FUNC_ALT2,
                                        MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_uart0_flow = { MXC_GPIO0, (MXC_GPIO_PIN_6 | MXC_GPIO_PIN_7), MXC_GPIO_FUNC_ALT2,
                                             MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_uart1a = { MXC_GPIO1, (MXC_GPIO_PIN_12 | MXC_GPIO_PIN_13), MXC_GPIO_FUNC_ALT2,
                                         MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_uart1b = { MXC_GPIO1, (MXC_GPIO_PIN_12 | MXC_GPIO_PIN_13), MXC_GPIO_FUNC_ALT3,
                                         MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_uart1c = { MXC_GPIO0, (MXC_GPIO_PIN_6 | MXC_GPIO_PIN_7), MXC_GPIO_FUNC_ALT3,
                                         MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_uart1_flow = { MXC_GPIO0, (MXC_GPIO_PIN_12 | MXC_GPIO_PIN_13), MXC_GPIO_FUNC_ALT2,
                                             MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };

const mxc_gpio_cfg_t gpio_cfg_spi0 = { MXC_GPIO0, (MXC_GPIO_PIN_4 | MXC_GPIO_PIN_5 | MXC_GPIO_PIN_6 | MXC_GPIO_PIN_7),
                                       MXC_GPIO_FUNC_ALT1, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_spi1a = { MXC_GPIO0, (MXC_GPIO_PIN_10 | MXC_GPIO_PIN_11 | MXC_GPIO_PIN_12 | MXC_GPIO_PIN_13),
                                        MXC_GPIO_FUNC_ALT1, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_spi1b = { MXC_GPIO0, (MXC_GPIO_PIN_0 | MXC_GPIO_PIN_1 | MXC_GPIO_PIN_2 | MXC_GPIO_PIN_3),
                                        MXC_GPIO_FUNC_ALT2, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };

// Timers are only defined once, depending on package, each timer could be mapped to other pins
const mxc_gpio_cfg_t gpio_cfg_tmr0 = { MXC_GPIO0, MXC_GPIO_PIN_3, MXC_GPIO_FUNC_ALT3,
                                       MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_32kcal = { MXC_GPIO0, MXC_GPIO_PIN_2, MXC_GPIO_FUNC_ALT3,
                                         MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };

const mxc_gpio_cfg_t gpio_cfg_i2s0a = { MXC_GPIO0, (MXC_GPIO_PIN_10 | MXC_GPIO_PIN_11 | MXC_GPIO_PIN_12 | MXC_GPIO_PIN_13),
                                        MXC_GPIO_FUNC_ALT1, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_i2s0b = { MXC_GPIO0, (MXC_GPIO_PIN_0 | MXC_GPIO_PIN_1 | MXC_GPIO_PIN_2 | MXC_GPIO_PIN_3),
                                        MXC_GPIO_FUNC_ALT2, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };

// SPI v2 Pin Definitions
const mxc_gpio_cfg_t gpio_cfg_spi0_standard = { MXC_GPIO0,
                                                (MXC_GPIO_PIN_4 | MXC_GPIO_PIN_5 | MXC_GPIO_PIN_6),
                                                MXC_GPIO_FUNC_ALT1, MXC_GPIO_PAD_NONE,
                                                MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_spi0_3wire = { MXC_GPIO0, (MXC_GPIO_PIN_5 | MXC_GPIO_PIN_6),
                                             MXC_GPIO_FUNC_ALT1, MXC_GPIO_PAD_NONE,
                                             MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
// MXC_SPI0 does not support Dual or Quad mode.

const mxc_gpio_cfg_t gpio_cfg_spi1a_standard = {
    MXC_GPIO0, (MXC_GPIO_PIN_10 | MXC_GPIO_PIN_11 | MXC_GPIO_PIN_12), MXC_GPIO_FUNC_ALT1,
    MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0
};
const mxc_gpio_cfg_t gpio_cfg_spi1a_3wire = { MXC_GPIO0, (MXC_GPIO_PIN_11 | MXC_GPIO_PIN_12),
                                              MXC_GPIO_FUNC_ALT1, MXC_GPIO_PAD_NONE,
                                              MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_spi1b_standard = { MXC_GPIO0,
                                                 (MXC_GPIO_PIN_0 | MXC_GPIO_PIN_1 | MXC_GPIO_PIN_2),
                                                 MXC_GPIO_FUNC_ALT2, MXC_GPIO_PAD_NONE,
                                                 MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_spi1b_3wire = { MXC_GPIO0, (MXC_GPIO_PIN_1 | MXC_GPIO_PIN_2),
                                              MXC_GPIO_FUNC_ALT2, MXC_GPIO_PAD_NONE,
                                              MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
// MXC_SPI1 does not support Dual or Quad mode.

// SPI v2 Target Selects Pin Definitions
const mxc_gpio_cfg_t gpio_cfg_spi0_ts0 = { MXC_GPIO0, MXC_GPIO_PIN_7, MXC_GPIO_FUNC_ALT1,
                                           MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_spi1a_ts0 = { MXC_GPIO0, MXC_GPIO_PIN_13, MXC_GPIO_FUNC_ALT1,
                                            MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };
const mxc_gpio_cfg_t gpio_cfg_spi1b_ts0 = { MXC_GPIO0, MXC_GPIO_PIN_3, MXC_GPIO_FUNC_ALT2,
                                            MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO, MXC_GPIO_DRVSTR_0 };

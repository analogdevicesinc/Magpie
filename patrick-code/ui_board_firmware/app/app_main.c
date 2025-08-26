/* Private includes --------------------------------------------------------------------------------------------------*/

#include "main.h"
#include "gpio.h"

/* Private variables -------------------------------------------------------------------------------------------------*/

const uint32_t NUM_MSEC_OF_HOLDING_PWR_BUTTON_TO_INITIATE_SHUTDOWN = 3 * 1000;

const uint32_t POLLING_CYCLE_DURATION_MSEC = 10;

const uint32_t NUM_POLLING_CYCLES_OF_HOLDING_PWR_BUTTON_TO_INITIATE_SHUTDOWN = NUM_MSEC_OF_HOLDING_PWR_BUTTON_TO_INITIATE_SHUTDOWN / POLLING_CYCLE_DURATION_MSEC;

/* Public function definitions ---------------------------------------------------------------------------------------*/

void app_main()
{
    // we only need PORT-B to remain active so we can check the PWR button state
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();
    __HAL_RCC_GPIOF_CLK_DISABLE();

    /*
      delay at startup so that if the user holds down the PWR button for a long
      time while powering up the unit they don't inadvertantly turn it off again
    */
    HAL_Delay(5000);

    while (1)
    {
        int pwr_button_press_count = 0;

        /*
          Immediately enter low power mode and wait for a PWR button press event (WFE)
        */
        HAL_SuspendTick();
        HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFE);

        /*
          Now we're waiting for a PWR button press to wake us back up.
          The PWR button generates an EVENT, and we're sleeping with a WFE condition.
          If we get past this block, it means someone pressed the PWR button.
         */

        HAL_ResumeTick();

        /*
          Now we check if the user will hold down the PWR button for long enough to
          trigger a power-down. If they do hold the button for long enough, we need
          to enable PORT-A so that the PWR ASSERT pin can change state, and then
          drive that pin LOW. Doing so will immediately shut down power to the whole
          system, including this microcontroller.

          If the user stops holding down the button before they trigger a power-down,
          then we just go back to sleep until we detect another PWR button press.
        */
        while (!HAL_GPIO_ReadPin(PWR_PB_GPIO_Port, PWR_PB_Pin))
        {
            pwr_button_press_count += 1;

            if (pwr_button_press_count >= NUM_POLLING_CYCLES_OF_HOLDING_PWR_BUTTON_TO_INITIATE_SHUTDOWN)
            {
                __HAL_RCC_GPIOA_CLK_ENABLE();
                HAL_GPIO_WritePin(PWR_ASSERT_GPIO_Port, PWR_ASSERT_Pin, GPIO_PIN_RESET);

                /*
                  If we get here power will immediately be cut to the whole system.
                */
            }

            HAL_Delay(POLLING_CYCLE_DURATION_MSEC);
        }
    }
}

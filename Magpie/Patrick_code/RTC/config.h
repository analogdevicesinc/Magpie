
//#define FIRST_SET_RTC 1    //********** comment this out so you don't set DS3231 RTC time

#define OUTPUT_MSG_BUFFER_SIZE       128U

#define MXC_GPIO_PORT_INTERRUPT_IN MXC_GPIO0
#define MXC_GPIO_PIN_INTERRUPT_IN MXC_GPIO_PIN_5

// I2C config for connecting to DS3231
#define MAX32666_I2C_BUS_3V3_PULLUPS (MXC_I2C2_BUS0)    //***Change the I2C Bus here
#define MAX32666_I2C_CFG_MASTER_MODE (1)
#define MAX32666_I2C_CLK_SPEED (MXC_I2C_STD_MODE)

#define FTHR2_LED_RED 0
#define FTHR2_LED_BLUE 2
#define FTHR2_LED_GREEN 1

#define ALARM_SYNC_DELAY_S 2   //set how long in seconds before the DS3231 alarm trigger from the current time

/****************** TypeDef ******************/
/**
 * @brief Enumerated DS3231 I2C error mode.
 */
typedef enum{
    DS3231_I2C_NO_ERROR,
	MCU_MASTER_I2C_INIT_ERROR,
    DS3231_I2C_INIT_ERROR,
    DS3231_I2C_FREQSET_ERROR
} DS3231_i2cError;


//Function Prototypes
DS3231_i2cError DS3231_I2C_init();
static void enable_DS3231_Interrupt(void);

static void ds3231_ISR(void);
static void Sync_RTC_to_DS3231(void);

static void reset_MAX_RTC(int hour, int minute, int sec);

void RTC_IRQHandler(void);
void buttonHandler();
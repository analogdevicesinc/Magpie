/***** Includes *****/

#include <stdio.h>
#include <string.h>
#include "mxc_device.h"
#include "board.h"
#include "gpio.h"
#include "nvic_table.h"
#include "spi.h"
#include "i2c.h"

//#include "spi_reva2.h"

#include "mxc_delay.h"
#include "icc.h"
#include "dma.h"
#include "arm_math.h"
#include "SEGGER_RTT.h"




//#include "rtc.h"
//#include "tmr.h"
#define SPI1_BASE_addr  0x40046000
#define DMA0_BASE_addr 0x40028000


/***** Definitions *****/

// *** set 24 bit or 16 bit,
//#define bits24
#define bits16
//Pin 0.20 for ADC clock enable or disable

#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT20 MXC_GPIO_PIN_20 // A4 on feather

//Pin 0.12 for ADC clock enable or disable

#define BUSY_PORT MXC_GPIO0
#define BUSY_PIN MXC_GPIO_PIN_10 //Rx on feather, this triggers the interrupt, drive with sq wave




// SPI PIns on feather for alternate 2, sclk = A3, MOSI/SDIO0 = A1, MISO/SDIO1=A2, SDIO2 = A4, SDIO3 = A5
// SS0 (slave Sel) = A0, SS1 and SS2 not avail on feather
// note, SPI1 is on APB; SPI0 is on AHB, which is faster, but not avail on feather (oops)

//****************************************************************
//SPI defines for SPI1 master
//****************************************************************
/***** Definitions *****/
#define DATA_LEN_SPI1 3 // number of bytes per spi transaction
//#define SPI_SPEED_SPI1 16000000 // Bit Rate
#define SPI_SPEED_SPI1 20000000 // Bit Rate
#define DATA_SIZE_SPI1 8  //Data bits per character
#define SPI_MASTER1 MXC_SPI1 //Using SPI2 as Master to configure ADC
#define SPI_MASTER1_SSIDX 0




// I2C defines
#define I2C_MASTER MXC_I2C0_BUS0
#define I2C_SLAVE_ADDR (0x98) // for a write, read is 99
#define I2C_BYTES 2
#define I2C_FREQ 100000


// bare-metal spi registers for direct spi control
#define SPI1_DATA0_direct  (*( ( volatile unsigned int * )SPI1_DATA0_addr ) )
#define SPI1_CTRL0_direct  (*( ( volatile unsigned int * )SPI1_CTRL0_addr ) )



// params for input buffering direct from spi
#define spi_rx_mem_len 1024



//#define spi_rx_mem_len_deci2 96000 // inputs arrive in chunks of 128, get offset and assembled
#define longMemLen 96000

/***** Globals *****/
static uint32_t countISR = 0;


// bare metal spi control
static uint32_t SPI1_DATA0_addr =  SPI1_BASE_addr + 0x00000000;
static uint32_t SPI1_CTRL0_addr =  SPI1_BASE_addr + 0x00000004;

static q15_t outBuffLong[longMemLen] = {0}; // 96K samples at 192k


static uint32_t wrPtr=0;


static q15_t spi_data_q15; //the raw spi input in 2's comp form
static uint32_t offsetAdr=0;





// ping-pong state, '0' means "writing to bankA while reading bankkB", 1 means "writing to bankB while reading bankkA"
static uint8_t lowByte=0, midByte=0,highByte=0, switchBankFlag=0;
static uint32_t pingPongState=0u;
//static q31_t spi_data_q31; // use for 24 bit


static uint32_t temp32u=0;
//static signed long int ozone_scope_var=0;



volatile uint8_t ADC_BUSY = 0;
uint8_t SPI1_rx[DATA_LEN_SPI1];
uint8_t SPI1_tx[DATA_LEN_SPI1];
int mychannel = -1;


mxc_spi_req_t SPI1_req; // struct for SPI int


//static q15_t spi_data_q15; // use for 16-bit

// I2C variables
static uint8_t i2cErr=0;
static int I2C_FLAG=0;

static uint8_t i2c_txdata[I2C_BYTES] = {0x1};
static uint8_t i2c_rxdata[I2C_BYTES] = {0x1};


// debug variables
//static uint32_t debug0=0,debug1=0,debug2=0,debug3=0;
//static uint32_t temp0=0,temp1=0,temp2=0;




// *********** start functions *******************
//I2C callback function
void I2C_Callback(mxc_i2c_req_t *req, int error)
{
    I2C_FLAG = error;
}




void BUSY_ISR(void) // gpio adc busy input interrupt handler

{

	uint32_t stat;


	stat = MXC_GPIO_GetFlags(MXC_GPIO0);
	MXC_GPIO_ClearFlags(MXC_GPIO0, stat);



	SPI1_CTRL0_direct |= 0x00000020; // bare-metal spi start

	//note, the next bare-metal read from SPI1_DATA0_direct is happening while the spi read is still active
	// since the following 4 lines only take about 1us, the spi read is not finished yet
	// so we are really getting data from the previous read, stored in the fifo

	temp32u = SPI1_DATA0_direct; // read spi fifo, result has byte order in wrong order, need to fix


	lowByte = (temp32u >> 8) & 0xff;
	highByte = temp32u & 0xff;
	spi_data_q15 = lowByte | (highByte << 8); // 16-but 2's comp, need to sign-extend to 32 bits
	outBuffLong[countISR] = spi_data_q15;

	countISR++;

}





int main(void)

{
	int retVal, fail = 0;

	//MXC_ICC_Enable(); // enable cache (ON by default)
	//MXC_ICC_Disable(); // disable cache


	// set up green led gpio out for timing tests
	#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
	#define MXC_GPIO_PIN_OUT5 MXC_GPIO_PIN_5



	mxc_gpio_cfg_t gpio_out5;

	gpio_out5.port =MXC_GPIO_PORT_OUT0;
	gpio_out5.mask= MXC_GPIO_PIN_OUT5;
	gpio_out5.pad = MXC_GPIO_PAD_NONE;
	gpio_out5.func = MXC_GPIO_FUNC_OUT;
	gpio_out5.vssel = MXC_GPIO_VSSEL_VDDIO;
	MXC_GPIO_Config(&gpio_out5);

	MXC_GPIO_OutClr(gpio_out5.port,gpio_out5.mask); // set LOW (Led on)


	mxc_gpio_cfg_t BUSY;

	// set up the gpio interupt input pin which will be driven by the adc busy signal (falling edge trigger)
	BUSY.port = BUSY_PORT;
	BUSY.mask = BUSY_PIN;
	BUSY.pad = MXC_GPIO_PAD_PULL_UP;
	BUSY.func = MXC_GPIO_FUNC_IN;
	BUSY.vssel = MXC_GPIO_VSSEL_VDDIO;





	// Initialize the SPI
	// MXC_SPI_Init(mxc_spi_regs_t *spi, int masterMode, int quadModeUsed, int numSlaves,...
    // ... unsigned ssPolarity, unsigned int hz, sys_map_t map);
	retVal = MXC_SPI_Init(SPI_MASTER1, 1, 0, 1, 0, SPI_SPEED_SPI1, MAP_A);

	//Setting the data size
	retVal = MXC_SPI_SetDataSize(SPI_MASTER1, DATA_SIZE_SPI1); // in units of #bits/character, note that '16' writes 0 to the reg (special case)
	//Setting width of the SPI in this case 3- wire SPI for SPI1 master
	retVal = MXC_SPI_SetWidth(SPI_MASTER1, SPI_WIDTH_3WIRE);
	//Setting the SPI mode
	retVal = MXC_SPI_SetMode(SPI_MASTER1, SPI_MODE_0); // clock phase = 0, clock polarity = 0


	// we call mxc_spi_mastertransaction only once to complete the initialization (MXC_SPI_Init only does part of the initialization; weird!)
	SPI1_req.spi = SPI_MASTER1;
	SPI1_req.txData = (uint8_t *)SPI1_tx;
	SPI1_req.rxData = (uint8_t *)SPI1_rx;
	SPI1_req.txLen = 0;
	SPI1_req.rxLen = 3; // bytes/char (3)
	SPI1_req.ssIdx = 0;
	SPI1_req.ssDeassert = 1;
	SPI1_req.txCnt = 0;
	SPI1_req.rxCnt = 0;
	SPI1_req.completeCB = NULL;

	memset(SPI1_rx, 0x0, 3); //Filling a block of memory
	// call this function to execute the reg init part of the function, the data is not used
	MXC_SPI_MasterTransaction(&SPI1_req); // complete the init; don't use the data!


	// I2C not used in this project
	i2cErr = MXC_I2C_Init(I2C_MASTER, 1, 0);
	 mxc_i2c_req_t reqMaster;
	reqMaster.i2c = I2C_MASTER;
	reqMaster.addr = I2C_SLAVE_ADDR;
	reqMaster.tx_buf = i2c_txdata;
	reqMaster.tx_len = I2C_BYTES;
	reqMaster.rx_buf = i2c_rxdata;
	reqMaster.rx_len = 0;
	reqMaster.restart = 0;
	reqMaster.callback = NULL;
	reqMaster.callback = I2C_Callback;
	I2C_FLAG = 1;

    MXC_I2C_SetFrequency(I2C_MASTER, I2C_FREQ);

	i2cErr = MXC_I2C_MasterTransaction(&reqMaster);


	// start interupts; this should be tied to the pushbutton
	MXC_GPIO_IntConfig(&BUSY, MXC_GPIO_INT_FALLING);
	MXC_GPIO_EnableInt(BUSY.port, BUSY.mask);
	MXC_NVIC_SetVector(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(BUSY_PORT)), BUSY_ISR);
	NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(BUSY_PORT))); //move this to just before the while() loop so that the 1st interupt is aligned with the block memory pointer==0

	while (1) 
	{
		if(countISR >= longMemLen) 
		{
				MXC_GPIO_DisableInt(BUSY.port, BUSY.mask); // stop interupts when reach max

				// insert your uart transfer code here
		}
		int location = 0;
		location = countISR;
		if (location == countISR) // Checks if the buffer reached 150000 locations
		{

			while (location--) // Starts moving data to the PC
			{

				printf("0X%x ", outBuffLong[location]); // stack[location]);
			}
			printf("\n"); // matlab must be set for \n as seperator
		}

	} // end of while()



	return 0;

}


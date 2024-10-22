// address of peripherals for direct bare-metal register access
#define SPI1_BASE_addr  0x40046000
#define SPI2_BASE_addr  0x40047000
#define DMA0_BASE_addr  0x40028000
#define TMR0_BASE_addr  0x40010000 

#define DMA0_CTRL_direct  (*( (volatile unsigned int * )DMA0_CTRL_addr ) )
#define DMA0_INTR_direct  (*( ( volatile unsigned int * )DMA0_INTR_addr ) )
#define DMA0_CH0_CFG_direct  (*( ( volatile unsigned int * )DMA0_CH0_CFG_addr ) )
#define DMA0_CH0_ST_direct  (*( ( volatile unsigned int * )DMA0_CH0_ST_addr ) )
#define DMA0_CH0_SRC_direct  (*( ( volatile unsigned int *)DMA0_CH0_SRC_addr ) )
#define DMA0_CH0_DST_direct  (*( ( volatile unsigned int *)DMA0_CH0_DST_addr ) )
#define DMA0_CH0_CNT_direct  (*( ( volatile unsigned int * )DMA0_CH0_CNT_addr ) )
#define DMA0_CH0_SRC_RLD_direct  (*( (volatile unsigned int * )DMA0_CH0_SRC_RLD_addr ) )
#define DMA0_CH0_DST_RLD_direct  (*( ( volatile unsigned int *)DMA0_CH0_DST_RLD_addr ) )
#define DMA0_CH0_CNT_RLD_direct  (*( ( volatile unsigned int *)DMA0_CH0_CNT_RLD_addr ) )


#define SPI1_DATA0_direct  (*( ( volatile unsigned int * )SPI1_DATA0_addr ) )
#define SPI1_DATA0_char_direct  (*( ( volatile unsigned char * )SPI1_DATA0_addr ) )
#define SPI1_CTRL0_direct  (*( (  unsigned int * )SPI1_CTRL0_addr ) )
#define SPI1_CTRL1_direct  (*( ( volatile unsigned int * )SPI1_CTRL1_addr ) )
#define SPI1_CTRL2_direct  (*( ( volatile unsigned int * )SPI1_CTRL2_addr ) )
#define SPI1_SS_TIME_direct  (*( ( volatile unsigned int * )SPI1_SS_TIME_addr ) )
#define SPI1_CLK_CFG_direct  (*( ( volatile unsigned int * )SPI1_CLK_CFG_addr ) )
#define SPI1_DMA_direct  (*( ( volatile unsigned int * )SPI1_DMA_addr ) )
#define SPI1_INT_FL_direct  (*( ( volatile unsigned int * )SPI1_INT_FL_addr ) )
#define SPI1_INT_EN_direct  (*( ( volatile unsigned int * )SPI1_INT_EN_addr ) )
#define SPI1_WAKE_FL_direct  (*( ( volatile unsigned int * )SPI1_WAKE_FL_addr ) )
#define SPI1_WAKE_EN_direct  (*( ( volatile unsigned int * )SPI1_WAKE_EN_addr ) )
#define SPI1_STAT_direct  (*( ( volatile unsigned int * )SPI1_STAT_addr ) )

#define SPI2_DATA0_direct  (*( ( volatile unsigned int * )SPI2_DATA0_addr ) )
#define SPI2_CTRL0_direct  (*( (  unsigned int * )SPI2_CTRL0_addr ) )
#define SPI2_CTRL1_direct  (*( ( volatile unsigned int * )SPI2_CTRL1_addr ) )
#define SPI2_CTRL2_direct  (*( ( volatile unsigned int * )SPI2_CTRL2_addr ) )
#define SPI2_SS_TIME_direct  (*( ( volatile unsigned int * )SPI2_SS_TIME_addr ) )
#define SPI2_CLK_CFG_direct  (*( ( volatile unsigned int * )SPI2_CLK_CFG_addr ) )
#define SPI2_DMA_direct  (*( ( volatile unsigned int * )SPI2_DMA_addr ) )
#define SPI2_INT_FL_direct  (*( ( volatile unsigned int * )SPI2_INT_FL_addr ) )
#define SPI2_INT_EN_direct  (*( ( volatile unsigned int * )SPI2_INT_EN_addr ) )
#define SPI2_WAKE_FL_direct  (*( ( volatile unsigned int * )SPI2_WAKE_FL_addr ) )
#define SPI2_WAKE_EN_direct  (*( ( volatile unsigned int * )SPI2_WAKE_EN_addr ) )
#define SPI2_STAT_direct  (*( ( volatile unsigned int * )SPI2_STAT_addr ) )


#define TMR0_CNT_direct  (*( ( volatile unsigned int * )TMR0_CNT_addr ) )
#define TMR0_CMP_direct  (*( ( volatile unsigned int * )TMR0_CMP_addr ) )
#define TMR0_PWM_direct  (*( ( volatile unsigned int * )TMR0_PWM_addr ) )
#define TMR0_INTR_direct  (*( ( volatile unsigned int * )TMR0_INTR_addr ) )
#define TMR0_CN_direct  (*( ( volatile unsigned int * )TMR0_CN_addr ) )
#define TMR0_NOLCMP_direct  (*( ( volatile unsigned int * )TMR0_NOLCMP_addr ) )





// *****bare-metal direct-register access to SPI1, SPI2, and DMA0 peripheral registers**
static uint32_t SPI1_DATA0_addr =  SPI1_BASE_addr;
static uint32_t SPI1_CTRL0_addr =  (SPI1_BASE_addr + 0x00000004);
static uint32_t SPI1_CTRL1_addr =  (SPI1_BASE_addr + 0x00000008);
static uint32_t SPI1_CTRL2_addr =  (SPI1_BASE_addr + 0x0000000c);
static uint32_t SPI1_SS_TIME_addr =  (SPI1_BASE_addr + 0x00000010);
static uint32_t SPI1_CLK_CFG_addr =  (SPI1_BASE_addr + 0x00000014);
static uint32_t SPI1_DMA_addr =  (SPI1_BASE_addr + 0x0000001c);
static uint32_t SPI1_INT_FL_addr =  (SPI1_BASE_addr + 0x00000020);
static uint32_t SPI1_INT_EN_addr =  (SPI1_BASE_addr + 0x00000024);
static uint32_t SPI1_WAKE_FL_addr =  (SPI1_BASE_addr + 0x00000028);
static uint32_t SPI1_WAKE_EN_addr =  (SPI1_BASE_addr + 0x0000002c);
static uint32_t SPI1_STAT_addr =  (SPI1_BASE_addr + 0x00000030);

static uint32_t SPI2_DATA0_addr =  SPI2_BASE_addr;
static uint32_t SPI2_CTRL0_addr =  (SPI2_BASE_addr + 0x00000004);
static uint32_t SPI2_CTRL1_addr =  (SPI2_BASE_addr + 0x00000008);
static uint32_t SPI2_CTRL2_addr =  (SPI2_BASE_addr + 0x0000000c);
static uint32_t SPI2_SS_TIME_addr =  (SPI2_BASE_addr + 0x00000010);
static uint32_t SPI2_CLK_CFG_addr =  (SPI2_BASE_addr + 0x00000014);
static uint32_t SPI2_DMA_addr =  (SPI2_BASE_addr + 0x0000001c);
static uint32_t SPI2_INT_FL_addr =  (SPI2_BASE_addr + 0x00000020);
static uint32_t SPI2_INT_EN_addr =  (SPI2_BASE_addr + 0x00000024);
static uint32_t SPI2_WAKE_FL_addr =  (SPI2_BASE_addr + 0x00000028);
static uint32_t SPI2_WAKE_EN_addr =  (SPI2_BASE_addr + 0x0000002c);
static uint32_t SPI2_STAT_addr =  (SPI2_BASE_addr + 0x00000030);

static uint32_t DMA0_CTRL_addr =  DMA0_BASE_addr;
static uint32_t DMA0_INTR_addr =  (DMA0_BASE_addr + 0x00000004);
static uint32_t DMA0_CH0_CFG_addr =  (DMA0_BASE_addr + 0x00000100);
static uint32_t DMA0_CH0_ST_addr =  (DMA0_BASE_addr + 0x00000104);
static uint32_t DMA0_CH0_SRC_addr =  (DMA0_BASE_addr + 0x00000108);
static uint32_t DMA0_CH0_DST_addr =  (DMA0_BASE_addr + 0x0000010c);
static uint32_t DMA0_CH0_CNT_addr =  (DMA0_BASE_addr + 0x00000110);
static uint32_t DMA0_CH0_SRC_RLD_addr =  (DMA0_BASE_addr + 0x00000114);
static uint32_t DMA0_CH0_DST_RLD_addr =  (DMA0_BASE_addr + 0x00000118);
static uint32_t DMA0_CH0_CNT_RLD_addr =  (DMA0_BASE_addr + 0x0000011c);


static uint32_t TMR0_CNT_addr = TMR0_BASE_addr;
static uint32_t TMR0_CMP_addr = (TMR0_BASE_addr + 0x00000004);
static uint32_t TMR0_PWM_addr = (TMR0_BASE_addr + 0x00000008);
static uint32_t TMR0_INTR_addr = (TMR0_BASE_addr + 0x0000000C);
static uint32_t TMR0_CN_addr = (TMR0_BASE_addr + 0x00000010);
static uint32_t TMR0_NOLCMP_addr = (TMR0_BASE_addr + 0x00000014);





static uint32_t DMA0_CTRL_direct_probe = 0x00000000;
static uint32_t DMA0_INTR_direct_probe  =0x00000000;
static uint32_t DMA0_CH0_CFG_direct_probe = 0x00000000;
static uint32_t DMA0_CH0_ST_direct_probe =0x00000000;
static uint32_t DMA0_CH0_SRC_direct_probe =0x00000000;
static uint32_t DMA0_CH0_DST_direct_probe =0x00000000;
static uint32_t DMA0_CH0_CNT_direct_probe = 0x00000000;
static uint32_t DMA0_CH0_SRC_RLD_direct_probe =0x00000000;
static uint32_t DMA0_CH0_DST_RLD_direct_probe=0x00000000;
static uint32_t DMA0_CH0_CNT_RLD_direct_probe =0x00000000;

static uint32_t SPI1_DATA0_direct_probe =  0x00000000;
static uint32_t SPI1_CTRL0_direct_probe =  0x00000000;
static uint32_t SPI1_CTRL1_direct_probe =  0x00000000;
static uint32_t SPI1_CTRL2_direct_probe =  0x00000000;
static uint32_t SPI1_SS_TIME_direct_probe =  0x00000000;
static uint32_t SPI1_CLK_CFG_direct_probe =  0x00000000;
static uint32_t SPI1_DMA_direct_probe =  0x00000000;
static uint32_t SPI1_INT_FL_direct_probe =  0x00000000;
static uint32_t SPI1_INT_EN_direct_probe =  0x00000000;
static uint32_t SPI1_WAKE_FL_direct_probe =  0x00000000;
static uint32_t SPI1_WAKE_EN_direct_probe =  0x00000000;
static uint32_t SPI1_STAT_direct_probe =  0x00000000;

static uint32_t SPI2_DATA0_direct_probe =  0x00000000;
static uint32_t SPI2_CTRL0_direct_probe =  0x00000000;
static uint32_t SPI2_CTRL1_direct_probe =  0x00000000;
static uint32_t SPI2_CTRL2_direct_probe =  0x00000000;
static uint32_t SPI2_SS_TIME_direct_probe =  0x00000000;
static uint32_t SPI2_CLK_CFG_direct_probe =  0x00000000;
static uint32_t SPI2_DMA_direct_probe =  0x00000000;
static uint32_t SPI2_INT_FL_direct_probe =  0x00000000;
static uint32_t SPI2_INT_EN_direct_probe =  0x00000000;
static uint32_t SPI2_WAKE_FL_direct_probe =  0x00000000;
static uint32_t SPI2_WAKE_EN_direct_probe =  0x00000000;
static uint32_t SPI2_STAT_direct_probe =  0x00000000;


volatile uint32_t TMR0_CNT_direct_probe =  0x00000000;
volatile uint32_t TMR0_CMP_direct_probe =  0x00000000;
volatile uint32_t TMR0_PWM_direct_probe =  0x00000000;
volatile uint32_t TMR0_INTR_direct_probe =  0x00000000;
volatile uint32_t TMR0_CN_direct_probe =  0x00000000;
volatile uint32_t TMR0_NOLCMP_direct_probe =  0x00000000;



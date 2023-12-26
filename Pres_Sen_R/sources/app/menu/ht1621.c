/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     ht1621.c
  * @author   ZouZH
  * @version  V1.00
  * @date     17-June-2019
  * @brief    ht1621 lcd driver for stm32f10x.
  ******************************************************************************
  */

/* INCLUDES ------------------------------------------------------------------- */
#include "ht1621.h"
#include "includes.h"

/* TYPEDEFS ------------------------------------------------------------------- */

enum HT1621_CMD
{
  SYS_DIS          = 0x0,
  SYS_EN           = 0x1,
  LCD_OFF          = 0x2,
  LCD_ON           = 0x3,
  TIMER_DIS        = 0x4,
  WDT_DIS          = 0x5,
  TIMER_EN         = 0x6,
  WDT_EN           = 0x7,
  TONE_OFF         = 0x8,
  TONE_ON          = 0x9,

  // Set bias and commons: 1/2 or 1/3 bias, 2,3 or 4 commons
  BIAS_HALF_2_COM  = 0x20,
  BIAS_HALF_3_COM  = 0x24,
  BIAS_HALF_4_COM  = 0x28,
  BIAS_THIRD_2_COM = 0x21,
  BIAS_THIRD_3_COM = 0x25,
  BIAS_THIRD_4_COM = 0x29,

  // Buzzer tones
  TONE_4K          = 0x80,
  TONE_2K          = 0xC0,

  // For factory testing
  TEST_ON          = 0xE0,
  TEST_OFF         = 0xE3
};

enum HT1621_MODE
{
  MODE_CMD         = 0x4,
  MODE_WR          = 0x5,
  MODE_RD          = 0x6
};


/* MACROS  -------------------------------------------------------------------- */

#define GPIO_PORT_CS         GPIOB
#define GPIO_PORT_RD         GPIOC
#define GPIO_PORT_WR         GPIOC
#define GPIO_PORT_DATA       GPIOA
#define GPIO_PORT_IRQ        GPIOA

#define GPIO_PIN_CS          GPIO_Pin_1
#define GPIO_PIN_RD          GPIO_Pin_5
#define GPIO_PIN_WR          GPIO_Pin_4
#define GPIO_PIN_DATA        GPIO_Pin_7
#define GPIO_PIN_IRQ         GPIO_Pin_6

#define GPIO_CLK_CS          RCC_APB2Periph_GPIOB
#define GPIO_CLK_RD          RCC_APB2Periph_GPIOC
#define GPIO_CLK_WR          RCC_APB2Periph_GPIOC
#define GPIO_CLK_DATA        RCC_APB2Periph_GPIOA
#define GPIO_CLK_IRQ         RCC_APB2Periph_GPIOA

#define HT1621_CS_H()        GPIO_SetBits(GPIO_PORT_CS, GPIO_PIN_CS)
#define HT1621_RD_H()        GPIO_SetBits(GPIO_PORT_RD, GPIO_PIN_RD)
#define HT1621_WR_H()        GPIO_SetBits(GPIO_PORT_WR, GPIO_PIN_WR)
#define HT1621_DATA_H()      GPIO_SetBits(GPIO_PORT_DATA, GPIO_PIN_DATA)

#define HT1621_CS_L()        GPIO_ResetBits(GPIO_PORT_CS, GPIO_PIN_CS)
#define HT1621_RD_L()        GPIO_ResetBits(GPIO_PORT_RD, GPIO_PIN_RD)
#define HT1621_WR_L()        GPIO_ResetBits(GPIO_PORT_WR, GPIO_PIN_WR)
#define HT1621_DATA_L()      GPIO_ResetBits(GPIO_PORT_DATA, GPIO_PIN_DATA)

#define HT1621_DATA_READ()   GPIO_ReadInputDataBit(GPIO_PORT_DATA, GPIO_PIN_DATA)


/* CONSTANTS  ----------------------------------------------------------------- */

#define HT1621_DELAY_NUM     1

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */

/**
 * display buffer
 */
static uint8_t s_ucDispBuf[HT1621_SEG];
static uint8_t s_ucDispBufBak[HT1621_SEG];


/* LOCAL FUNCTIONS ------------------------------------------------------------ */

static uint8_t ht1621_get_disp_code(char ch);


static void    ht1621_lowlevel_init(void);
static void    ht1621_write_bits(uint8_t data, uint8_t cnt);
static uint8_t ht1621_read_bits(uint8_t cnt);
static void    ht1621_write_cmd(uint8_t cmd);
static void    ht1621_write_seg(uint8_t addr, uint8_t dat);
static void    ht1621_set_segs(uint8_t addr, uint8_t dat, uint8_t cnt);
static void    ht1621_write_segs(uint8_t addr, const uint8_t *pdat, uint8_t cnt);
static uint8_t ht1621_read_seg(uint8_t addr);
static void    ht1621_read_segs(uint8_t addr, uint8_t *pdat, uint8_t cnt);
static void    ht1621_delay(uint16_t cnt);


/**
 * @brief  Init
 * @param  None
 * @retval 0 success
 */
uint8_t HT1621_Init(void)
{
  uint8_t dat = 10;

  ht1621_lowlevel_init();

  ht1621_write_seg(0x5A, dat);

  if ((ht1621_read_seg(0x5A) != dat) && !HT1621_IRQ_READ())
    return 1;

  ht1621_set_segs(0, 0xF, HT1621_SEG);
  ht1621_write_cmd(SYS_EN);
  ht1621_write_cmd(TIMER_DIS);
  ht1621_write_cmd(WDT_DIS);
  ht1621_write_cmd(TONE_OFF);
  ht1621_write_cmd(BIAS_THIRD_4_COM);
  ht1621_write_cmd(LCD_ON);

  ht1621_read_segs(0, s_ucDispBuf, HT1621_SEG);

  return 0;
}


/**
 * @brief  Clear all
 * @param  dat: 0 all off, 1 all on
 * @retval None
 */
void HT1621_Clear(uint8_t dat)
{
  if (!dat)
    lib_memset(s_ucDispBuf, 0, sizeof(s_ucDispBuf));
  else
    lib_memset(s_ucDispBuf, 0xFF, sizeof(s_ucDispBuf));

  //ht1621_set_segs(0, dat, HT1621_SEG);
}

/**
 * @brief  Backup all display buffer
 * @param  None
 * @retval None
 */
void HT1621_Backup(void)
{
  lib_memcpy(s_ucDispBufBak, s_ucDispBuf, sizeof(s_ucDispBufBak));
}


/**
 * @brief  Restore all display buffer
 * @param  None
 * @retval None
 */
void HT1621_Restore(void)
{
  lib_memcpy(s_ucDispBuf, s_ucDispBufBak, sizeof(s_ucDispBuf));

  ht1621_write_segs(0, s_ucDispBuf, HT1621_SEG);
}

/**
 * @brief  Refresh changed display area
 * @param  None
 * @retval None
 */
void HT1621_Refresh(void)
{
  if (!lib_memcmp(s_ucDispBuf, s_ucDispBufBak, sizeof(s_ucDispBufBak)))
    return ;

  ht1621_write_segs(0, s_ucDispBuf, HT1621_SEG);

  lib_memcpy(s_ucDispBufBak, s_ucDispBuf, sizeof(s_ucDispBufBak));
}

/**
 * @brief  Refresh changed display area, but don't backup
 * @param  None
 * @retval None
 */
void HT1621_RefreshNoSave(void)
{
  ht1621_write_segs(0, s_ucDispBuf, HT1621_SEG);
}


/**
 * @brief  Trun-off display
 * @param  None
 * @retval None
 */
void HT1621_DisplayOff(void)
{
  ht1621_write_cmd(LCD_OFF);
  ht1621_write_cmd(SYS_DIS);
}


/**
 * @brief  Trun-on display
 * @param  None
 * @retval None
 */
void HT1621_DisplayOn(void)
{
  ht1621_write_cmd(SYS_EN);
  ht1621_write_cmd(LCD_ON);
}

/**
 * @brief  Write pixel
 * @param  seg: Segments(<HT1621_SEG)
 * @param  com: Common(<HT1621_COM)
 * @param  dat: 0(off)or1(on)
 * @retval None
 */
void HT1621_WritePixel(uint8_t seg, uint8_t com, uint8_t dat)
{
  if ((seg < HT1621_SEG) && (com < HT1621_COM))
  {
    if (dat & 0x001)
      s_ucDispBuf[seg] |= 1 << (com % HT1621_COM);
    else
      s_ucDispBuf[seg] &= ~(1 << (com % HT1621_COM));
  }
}


/**
 * @brief  Read pixel
 * @param  seg: Segments(<HT1621_SEG)
 * @param  com: Common(<HT1621_COM)
 * @retval 0(off)or1(on)
 */
uint8_t HT1621_ReadPixel(uint8_t seg, uint8_t com)
{
  if ((seg < HT1621_SEG) && (com < HT1621_COM))
  {
    if (s_ucDispBuf[seg] & (1 << (com % HT1621_COM)))
      return 1;
    else
      return 0;
  }

  return 0;
}


/**
 * @brief  Display a char on the specified digit.
 * @param  pos: Display position (0-7)
 * @param  ch: Char to display (0-9, A-Z)
 * @param  dot: Is display dot
 * @retval None
 */
void HT1621_DispChar(uint8_t pos, char ch, uint8_t dot)
{
  uint8_t code;

  if (pos >= 8)
    return;

  code = ht1621_get_disp_code(ch) | ((dot & 0x01) << 7);

  s_ucDispBuf[pos * 2] = UINT8_LO(code);
  s_ucDispBuf[(pos * 2) + 1] = UINT8_HI(code);
}


/**
 * @brief  Display string.
 * @param  string: String to display (0-9, A-Z), length <= 8
 * @retval None
 */
void HT1621_DispString(const char *string)
{
  char ch = ' ';
  uint8_t pos = 0;

  while ('\0' != *string)
  {
    if (pos >= 8)
      break;

    ch = *string++;

    if ('.' == *string)
    {
      HT1621_DispChar(pos, ch, 1);
      string++;
    }
    else
      HT1621_DispChar(pos, ch, 0);

    pos += 1;
  }
}


/**
 * @brief  Convert an alphanumeric ch to the 7-segment code.
 * @param  ch: Display  char
 * @retval segment code
 */
static uint8_t ht1621_get_disp_code(char ch)
{
  /**
   *  LED Segments:         a
   *                     ----
   *                   f|    |b
   *                    |  g |
   *                     ----
   *                   e|    |c
   *                    |    |
   *                     ----  o dp
   *                       d
   *      bit:  7  6  5  4  3  2  1  0
   *           dp  c  b  a  d  e  g  f
   */
  static const struct
  {
    char    ascii;
    uint8_t code;
  } CHAR_CODE[] =
  {
    {' ', 0x00},
    {'-', 0x02},
    {'0', 0x7D},
    {'1', 0x60},
    {'2', 0x3E},
    {'3', 0x7A},
    {'4', 0x63},
    {'5', 0x5B},
    {'6', 0x5F},
    {'7', 0x70},
    {'8', 0x7F},
    {'9', 0x7B},
    {'A', 0x77},
    {'B', 0x4F},
    {'C', 0x1D},
    {'D', 0x6E},
    {'E', 0x1F},
    {'F', 0x17},
    {'G', 0x5D},
    {'H', 0x67},
    {'I', 0x1C},
    {'J', 0x6C},
    {'K', 0x27},
    {'L', 0x0D},
    {'M', 0x56},
    {'N', 0x46},
    {'O', 0x5E},
    {'P', 0x37},
    {'Q', 0x73},
    {'R', 0x06},
    {'S', 0x53},
    {'T', 0x0F},
    {'U', 0x6D},
    {'V', 0x4C},
    {'W', 0x2B},
    {'X', 0x5C},
    {'Y', 0x6B},
    {'Z', 0x1A},
    {0x00, 0x00}
  };

  for (char i = 0; CHAR_CODE[i].ascii; i++)
  {
    if (ch == CHAR_CODE[i].ascii)
      return CHAR_CODE[i].code;
  }

  return 0;
}



/* ================== Low level hardware driver ===================== */

/**
 * @brief  Low level hardware init
 * @param  None
 * @retval None
 */
static void ht1621_lowlevel_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(GPIO_CLK_CS | GPIO_CLK_RD | GPIO_CLK_WR | GPIO_CLK_DATA | GPIO_CLK_IRQ, ENABLE);

  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_CS;
  GPIO_Init(GPIO_PORT_CS, &GPIO_InitStructure);
  GPIO_SetBits(GPIO_PORT_CS, GPIO_PIN_CS);

  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_RD;
  GPIO_Init(GPIO_PORT_RD, &GPIO_InitStructure);
  GPIO_SetBits(GPIO_PORT_RD, GPIO_PIN_RD);

  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_WR;
  GPIO_Init(GPIO_PORT_WR, &GPIO_InitStructure);
  GPIO_ResetBits(GPIO_PORT_WR, GPIO_PIN_WR);

  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_DATA;
  GPIO_Init(GPIO_PORT_DATA, &GPIO_InitStructure);
  GPIO_SetBits(GPIO_PORT_DATA, GPIO_PIN_DATA);

  GPIO_SetBits(GPIO_PORT_IRQ, GPIO_PIN_IRQ);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IRQ;
  GPIO_Init(GPIO_PORT_IRQ, &GPIO_InitStructure);
}


/**
 * @brief  Write bits
 * @param  data: Byte data, high bit fist
 * @param  cnt: bit count[1~8]
 * @retval None
 */
static void ht1621_write_bits(uint8_t data, uint8_t cnt)
{
  while (cnt)
  {
    HT1621_WR_L();

    if ((data & (1 << (cnt - 1))))
      HT1621_DATA_H();
    else
      HT1621_DATA_L();

    ht1621_delay(HT1621_DELAY_NUM);

    HT1621_WR_H();

    ht1621_delay(HT1621_DELAY_NUM);

    cnt--;
  }
}


/**
 * @brief  Read bits
 * @param  cnt: bit count[1~8]
 * @retval Read data, high bit fist
 */
static uint8_t ht1621_read_bits(uint8_t cnt)
{
  uint8_t data = 0;

  // float input dir(CRL=0~7,CRH=8-15)
  GPIO_PORT_DATA->CRL &= (0x0FFFFFFFul);
  GPIO_PORT_DATA->CRL |= (0x40000000ul);

  while (cnt)
  {
    ht1621_delay(HT1621_DELAY_NUM);

    HT1621_RD_L();

    ht1621_delay(HT1621_DELAY_NUM);

    if (HT1621_DATA_READ())
      data |= (1 << (cnt - 1));

    HT1621_RD_H();
    cnt--;
  }

  // push-pull output dir
  GPIO_PORT_DATA->CRL &= (0x0FFFFFFFul);
  GPIO_PORT_DATA->CRL |= (0x30000000ul);

  return data;
}



/**
 * @brief  Write command
 * @param  cmd: Command
 * @retval None
 */
static void ht1621_write_cmd(uint8_t cmd)
{
  HT1621_CS_L();

  ht1621_delay(HT1621_DELAY_NUM);

  ht1621_write_bits(MODE_CMD, 3);
  ht1621_write_bits(cmd, 8);

  // Last bit - don't care
  ht1621_write_bits(0, 1);

  HT1621_CS_H();

  ht1621_delay(HT1621_DELAY_NUM);
}


/**
 * @brief  Write segment
 * @param  addr: Address( < HT1621_SEG)
 * @param  dat: Bit[0~3]
 * @retval None
 */
static void ht1621_write_seg(uint8_t addr, uint8_t dat)
{
  HT1621_CS_L();

  ht1621_delay(HT1621_DELAY_NUM);

  ht1621_write_bits(MODE_WR, 3);
  ht1621_write_bits(addr, 6);
  ht1621_write_bits(dat, 4);

  HT1621_CS_H();

  ht1621_delay(HT1621_DELAY_NUM);
}


/**
 * @brief  Write segments data
 * @param  addr: Address( < HT1621_SEG)
 * @param  dat: Data Bit[0~3]
 * @param  cnt: Segment count
 * @retval None
 */
static void ht1621_set_segs(uint8_t addr, uint8_t dat, uint8_t cnt)
{
  if (!cnt || addr + cnt > HT1621_SEG)
    return;

  HT1621_CS_L();

  ht1621_delay(HT1621_DELAY_NUM);

  ht1621_write_bits(MODE_WR, 3);
  ht1621_write_bits(addr, 6);

  for (uint8_t i = 0; i < cnt; i++)
    ht1621_write_bits(dat, 4);

  HT1621_CS_H();

  ht1621_delay(HT1621_DELAY_NUM);
}


/**
 * @brief  Write segments data
 * @param  addr: Address( < HT1621_SEG)
 * @param  pdat: Data buffer Bit[0~3]
 * @param  cnt: Segment count
 * @retval None
 */
static void ht1621_write_segs(uint8_t addr, const uint8_t *pdat, uint8_t cnt)
{
  if ((NULL == pdat) || !cnt || addr + cnt > HT1621_SEG)
    return;

  HT1621_CS_L();

  ht1621_delay(HT1621_DELAY_NUM);

  ht1621_write_bits(MODE_WR, 3);
  ht1621_write_bits(addr, 6);

  for (uint8_t i = 0; i < cnt; i++)
    ht1621_write_bits(pdat[i], 4);

  HT1621_CS_H();

  ht1621_delay(HT1621_DELAY_NUM);
}


/**
 * @brief  Read segment
 * @param  addr: Address( < HT1621_SEG)
 * @retval Read data Bit[0~3]
 */
static uint8_t ht1621_read_seg(uint8_t addr)
{
  uint8_t data;

  HT1621_CS_L();

  ht1621_delay(HT1621_DELAY_NUM);

  ht1621_write_bits(MODE_RD, 3);
  ht1621_write_bits(addr, 6);
  data = ht1621_read_bits(4);

  HT1621_CS_H();

  ht1621_delay(HT1621_DELAY_NUM);

  return data;
}


/**
 * @brief  Read segments data
 * @brief  addr: Address( < HT1621_SEG)
 * @param  pdat: Data buffer Bit[0~3]
 * @param  cnt: Segment count
 * @retval None
 */
static void ht1621_read_segs(uint8_t addr, uint8_t *pdat, uint8_t cnt)
{
  if ((NULL == pdat) || !cnt || addr + cnt > HT1621_SEG)
    return;

  HT1621_CS_L();

  ht1621_delay(HT1621_DELAY_NUM);

  ht1621_write_bits(MODE_RD, 3);
  ht1621_write_bits(addr, 6);

  for (uint8_t i = 0; i < cnt; i++)
    pdat[i] = ht1621_read_bits(4);

  HT1621_CS_H();

  ht1621_delay(HT1621_DELAY_NUM);
}


/**
 * @brief  Serial line delay
 * @param  cnt: 200Khz/72Mhz
 * @retval
 */
static void ht1621_delay(uint16_t cnt)
{
  volatile uint32_t i = 4 * cnt;

  while (i--)
    __NOP();
}


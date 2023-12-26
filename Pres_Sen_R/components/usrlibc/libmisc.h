/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     libmisc.h
  * @author   ZouZH
  * @version  V1.09
  * @date     05-Dec.-2019
  * @brief    .
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __LIBMISC_H
#define __LIBMISC_H

#ifdef __cplusplus
 extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ------------------------------------------------------------------- */
#include <stdint.h>

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

uint32_t lib_pow(uint8_t base, uint8_t times);
uint64_t lib_pow64(uint8_t base, uint8_t times);

uint8_t  isbcds(const void *pbuf, uint8_t len);
uint8_t  bcd2dec(uint8_t bcd);
uint8_t  dec2bcd(uint8_t dec);
uint32_t bcds2dec(const void *pbuf, uint8_t len);
uint64_t bcds2dec_ex(const void *pbuf, uint8_t len);
uint32_t bcds2dec_invert(const void *pbuf, uint8_t len);
uint64_t bcds2dec_ex_invert(const void *pbuf, uint8_t len);

void     dec2bcds(uint32_t dec, void *pbuf, uint8_t len);
void     dec2bcds_invert(uint32_t dec, void *pbuf, uint8_t len);
void     dec2bcds_ex(uint64_t dec, void *pbuf, uint8_t len);
void     dec2bcds_ex_invert(uint64_t dec, void *pbuf, uint8_t len);

int      lib_atob(uint32_t *vp, char *p, int base);
int32_t  lib_atoi(const char *p);
long     lib_atol(const char *p);
void lib_atoint8_t(const char *p,uint8_t value[]);
double   lib_atof(char *p);


uint16_t lib_find_frame_header(uint8_t **ppbuf, uint16_t len, uint8_t header);
uint16_t lib_find_frame_tail(uint8_t **ppbuf, uint16_t len, uint8_t tail);

uint16_t lib_findframe188(uint8_t **ppbuf, uint16_t len);
uint16_t lib_findframegdky(uint8_t **ppbuf, uint16_t len);
uint16_t lib_findframejgsh(uint8_t **ppbuf, uint16_t len);

uint16_t lib_findframedlsd(uint8_t **ppbuf, uint16_t len);
uint8_t  lib_encrypt_frame_dlsd(uint8_t *pbuf, uint16_t len);
uint8_t  lib_decrypt_frame_dlsd(uint8_t *pbuf, uint16_t len);

void     lib_encode_tc_gdky(uint8_t *pbuf, uint8_t len);
void     lib_decode_tc_gdky(uint8_t *pbuf, uint8_t len);

void sort_bubble(uint16_t *pbuf, uint8_t len);    //冒泡法排序
uint16_t temp_avg(uint16_t *pbuf, uint8_t len);    //取平均值



/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __LIBMISC_H */


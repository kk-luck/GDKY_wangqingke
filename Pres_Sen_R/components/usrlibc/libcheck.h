/**
  ******************************************************************************
  *               Copyright(C) 2018-2028 GDKY All Rights Reserved
  *
  * @file     libcheck.h
  * @author   ZouZH
  * @version  V1.04
  * @date     29-Aug-2018
  * @brief    Data check library.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __LIBCHECK_H
#define __LIBCHECK_H

#ifdef __cplusplus
 extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ------------------------------------------------------------------- */
#include <stdint.h>

/**
 * @addtogroup Check
 * @{
 */

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */


#ifndef CHECK_SUM8
  #define CHECK_SUM8 1
#endif  /* CHECK_SUM8 */

#ifndef CHECK_SUM16
  #define CHECK_SUM16 0
#endif  /* CHECK_SUM16 */

#ifndef CHECK_SUM32
  #define CHECK_SUM32 1
#endif  /* CHECK_SUM32 */

#ifndef CHECK_SUMINTELHEX
  #define CHECK_SUMINTELHEX 0
#endif  /* CHECK_SUMINTELHEX */

#ifndef CHECK_SUMNET
  #define CHECK_SUMNET 0
#endif  /* CHECK_SUMNET */

#ifndef CHECK_BCC
  #define CHECK_BCC 1
#endif  /* CHECK_BCC */

#ifndef CHECK_ODDEVEN
  #define CHECK_ODDEVEN 0
#endif  /* CHECK_ODDEVEN */

#ifndef CHECK_CRC8
  #define CHECK_CRC8 0
#endif  /* CHECK_CRC8 */

#ifndef CHECK_CRC16
  #define CHECK_CRC16 0
#endif  /* CHECK_CRC16 */

#ifndef CHECK_CRC16CCITT
  #define CHECK_CRC16CCITT 1
#endif  /* CHECK_CRC16CCITT */

#ifndef CHECK_CRC16RTU
  #define CHECK_CRC16RTU 0
#endif  /* CHECK_CRC16RTU */

#ifndef CHECK_CRC16MB
  #define CHECK_CRC16MB 1
#endif  /* CHECK_CRC16MB */

#ifndef CHECK_CRC32
  #define CHECK_CRC32 0
#endif  /* CHECK_CRC32 */


/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

#if (CHECK_SUM8 == 1)
  uint8_t chk_sum8_dec(const void *pbuf, uint16_t len);
  uint8_t chk_sum8_hex(const void *pbuf, uint16_t len);
  uint8_t chk_sum8_hex_xor(const void *pbuf, uint16_t len);
#endif  /* CHECK_SUM8 */


#if (CHECK_SUM16 == 1)
  uint16_t chk_sum16(const void *buf, uint16_t len);
#endif  /* CHECK_SUM16 */


#if (CHECK_SUM32 == 1)
  uint32_t chk_sum32(const void *pbuf, uint32_t len);
#endif  /* CHECK_SUM32 */


#if (CHECK_SUMINTELHEX == 1)
  uint8_t chk_sum8_hex_intel(const void *pbuf, uint32_t len);
#endif  /* CHECK_SUMINTELHEX */


#if (CHECK_SUMNET == 1)
  uint16_t chk_sum16_net(const void *pbuf, uint16_t len);
#endif  /* CHECK_SUMNET */


#if (CHECK_BCC == 1)
  uint8_t chk_bcc(const void *pbuf, uint32_t len);
#endif  /* CHECK_BCC */


#if (CHECK_ODDEVEN == 1)
  uint8_t chk_odd(const void *pbuf, uint32_t len);
  uint8_t chk_even(const void *pbuf, uint32_t len);
#endif  /* CHECK_ODDEVEN */


#if (CHECK_CRC8 == 1)
  uint8_t chk_crc8(const void *pbuf, uint32_t len);
#endif  /* CHECK_CRC8 */


#if (CHECK_CRC16 == 1)
  uint16_t chk_crc16(const void  *pbuf, uint32_t len);
#endif  /* CHECK_CRC16 */


#if (CHECK_CRC16CCITT == 1)
  uint16_t chk_crc16_CCITT(const void * pbuf, uint32_t len);
  uint16_t chk_crc16_X25(const void * pbuf, uint32_t len);
#endif  /* CHECK_CRC16CCITT */


#if (CHECK_CRC16RTU == 1)
  uint16_t chk_crc16_RTU( const void * pbuf, uint32_t len);
#endif  /* CHECK_CRC16RTU */


#if (CHECK_CRC16MB == 1)
  uint16_t chk_crc16_MB( const void * pbuf, uint16_t len );
#endif  /* CHECK_CRC16MB */


#if (CHECK_CRC32 == 1)
  uint32_t chk_crc32(const void *pbuf, uint32_t len);
#endif  /* CHECK_CRC32 */

/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __LIBCHECK_H */

/**
 * @}
 */


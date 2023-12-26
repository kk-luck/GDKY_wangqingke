/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     cmdproc.c
  * @author   ZouZH
  * @version  V1.00
  * @date     16-May-2019
  * @brief    command process routine for Valve.
  ******************************************************************************
  */
#define LOG_TAG    "CMD"

/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"
#include "cmdproc.h"

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

#if  defined(STM32G030xx) && defined(USE_FULL_ASSERT)
#define CMD_AT_ENABLE     0
#else
#define CMD_AT_ENABLE     1
#endif

#define CMD_MODBUS_ENABLE 0

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */
CMD_ERROR_t CmdRxPollRs485(void);
CMD_ERROR_t CmdRxPollBle(void);
CMD_ERROR_t CmdRxPollDebug(void);

uint8_t     CmdRxProcPoll(void);

/* LOCAL VARIABLES ------------------------------------------------------------ */

CmdRunObj_t s_tCmd =
{
  .error     = CMD_OK,
  .dnQueue   = NULL,
  .upQueue   = NULL,
};

CmdQBuff_t  s_tCmdQRx =
{
  .port      = CMD_PORT_RS485,
  .len       = 0,
  .flag      = 0,
  .pbuf      = NULL,
};

/* LOCAL FUNCTIONS ------------------------------------------------------------ */

/**
 * @brief  Init
 * @param  None
 * @retval 0 Success, other Failed
 */
CMD_ERROR_t CmdInit(void)
{
  s_tCmd.error = CMD_OK;

  if (NULL == s_tCmd.dnQueue)
    s_tCmd.dnQueue = xQueueCreate(CMD_RX_Q_MAX_NUM, sizeof(CmdQBuff_t));

  if (NULL == s_tCmd.upQueue)
    s_tCmd.upQueue = xQueueCreate(CMD_RX_Q_MAX_NUM, sizeof(CmdQBuff_t));

  if ((NULL == s_tCmd.dnQueue) || (NULL == s_tCmd.upQueue))
    s_tCmd.error = CMD_ERR_Malloc;

  BusInit(BUS_NUM);
  BusSetParam(BUS_NUM, REC_BAUD, 0);

  return s_tCmd.error;
}


/**
 * @brief  Check and Save receive frame
 * @param  None
 * @retval 0 Success, other Failed
 */
CMD_ERROR_t CmdRxPoll(void)
{
  // 查询缓存指令
  CmdRxPollRs485();
  CmdRxPollBle();
  CmdRxPollDebug();

  // 查询处理指令
  CmdRxProcPoll();

  return s_tCmd.error;
}


/**
 * @brief  Check and Save receive frame from RS485 port
 * @param  None
 * @retval 0 Success, other Failed
 */
CMD_ERROR_t CmdRxPollRs485(void)
{
  CmdQBuff_t cmdBuff;

  uint16_t len = 0;
  uint8_t *pbuf = NULL;

  if (!BusCheckRxPoll(BUS_RS485))
    return CMD_OK;

  do
  {
    s_tCmd.error = CMD_OK;

    if (NULL == s_tCmd.dnQueue)
    {
      CmdInit();
      s_tCmd.error = CMD_ERR_Malloc;
      break;
    }

    if (!uxQueueSpacesAvailable(s_tCmd.dnQueue))
    {
      log_w("RS485 receive queue is full!!!");
      break;
    }

    len  = BusGetRxLen(BUS_RS485);
    pbuf = BusGetRxBuffer(BUS_RS485);

    cmdBuff.port = CMD_PORT_RS485;
    cmdBuff.len = len;
    cmdBuff.flag = 0;

    if (!cmdBuff.len || (cmdBuff.len > BusMaxRxLen(BUS_RS485)))
    {
      s_tCmd.error = CMD_ERR_Receive;
      break;
    }

    cmdBuff.pbuf = SYSMalloc(cmdBuff.len);

    if (NULL == cmdBuff.pbuf)
    {
      s_tCmd.error = CMD_ERR_Malloc;
      break;
    }

    lib_memcpy(cmdBuff.pbuf, pbuf, cmdBuff.len);

    if (pdTRUE != xQueueSend(s_tCmd.dnQueue, &cmdBuff, 0))
    {
      s_tCmd.error = CMD_ERR_QFull;
      SYSFree(cmdBuff.pbuf);
    }
  } while (0);

  BusEnableRx(BUS_RS485);

  return s_tCmd.error;
}



/**
 * @brief  Read and Process one command from receive queue
 * @param  None
 * @retval 1 Process one, 0 None command.
 */
uint8_t CmdRxProcPoll(void)
{
  CmdQBuff_t qrecv;

  if (xQueueReceive(s_tCmd.dnQueue, &s_tCmdQRx, 0) != pdTRUE)
  {
    if (xQueueReceive(s_tCmd.upQueue, &s_tCmdQRx, 0) != pdTRUE)
      return 0;
  }

  if ((NULL == s_tCmdQRx.pbuf) || (s_tCmdQRx.len < 4) || (s_tCmdQRx.port >= CMD_PORT_NUM))
  {
    // 释放内存
    SYSFree(s_tCmdQRx.pbuf);
    s_tCmdQRx.pbuf = NULL;
    s_tCmdQRx.len = 0;
    return 0;
  }

  lib_memcpy(&qrecv, &s_tCmdQRx, sizeof(CmdQBuff_t));

#ifdef USE_FULL_ASSERT

  if (qrecv.port != CMD_PORT_RS485)
  {
    log_i("[Cmd Recv]port=%d, len=%d", qrecv.port, qrecv.len);

    if (!strncasecmp((char *)qrecv.pbuf, "AT", 2) || !strncasecmp((char *)qrecv.pbuf, "CO", 2))
    {
      for (uint16_t i = 0; i < qrecv.len; i++)
        xprintf("%c", qrecv.pbuf[i]);
    }
    else
    {
      for (uint16_t i = 0; i < qrecv.len; i++)
      {
        xprintf("%02X ", qrecv.pbuf[i]);

        if (!((i + 1) % 32))
          xprintf("\r\n");
      }

      xprintf("\r\n");
    }
  }

#endif /* USE_FULL_ASSERT */

  // GDKY协议
  s_tCmdQRx.len = lib_findframegdky(&s_tCmdQRx.pbuf, s_tCmdQRx.len);

  if (s_tCmdQRx.len >= 9)
  {
    extern CMD_ERROR_t CmdRxMsgGdky(void);

    if (!CmdRxMsgGdky())
      goto EXIT_PROC;
  }

  // 恢复原始帧
  lib_memcpy(&s_tCmdQRx, &qrecv, sizeof(CmdQBuff_t));


// Modbus-RTU协议
#if CMD_MODBUS_ENABLE
  //s_tCmdQRx.len = lib_find_frame_header(&s_tCmdQRx.pbuf, s_tCmdQRx.len, g_tSysParam.comAddr);

  if (((g_tSysParam.comAddr == s_tCmdQRx.pbuf[0]) || !s_tCmdQRx.pbuf[0]) && !chk_crc16_MB(s_tCmdQRx.pbuf, s_tCmdQRx.len))
  {
    extern CMD_ERROR_t CmdRxMsgModbus(void);

    if (!CmdRxMsgModbus())
      goto EXIT_PROC;
  }

  // 恢复原始帧
  lib_memcpy(&s_tCmdQRx, &qrecv, sizeof(CmdQBuff_t));

#endif /* CMD_MODBUS_ENABLE */

  // AT指令
#if CMD_AT_ENABLE
  s_tCmdQRx.len = lib_find_frame_header(&s_tCmdQRx.pbuf, s_tCmdQRx.len, 'A');

  if (s_tCmdQRx.len < 4)
    s_tCmdQRx.len = lib_find_frame_header(&s_tCmdQRx.pbuf, s_tCmdQRx.len, 'C');

  s_tCmdQRx.len = lib_find_frame_tail(&s_tCmdQRx.pbuf, s_tCmdQRx.len, '\n');

  if ((!strncasecmp((char *)s_tCmdQRx.pbuf, "AT", 2) || !strncasecmp((char *)s_tCmdQRx.pbuf, "CO", 2)))
  {
    s_tCmdQRx.pbuf[s_tCmdQRx.len - 1] = '\0';

    if (NULL != strpbrk((char *)s_tCmdQRx.pbuf, "\r\n"))
    {
      extern uint8_t CmdParseAT(char *str, uint16_t len);
      CmdParseAT((char *)s_tCmdQRx.pbuf, s_tCmdQRx.len);
    }
  }

  // 恢复原始帧
  lib_memcpy(&s_tCmdQRx, &qrecv, sizeof(CmdQBuff_t));
#endif /* CMD_AT_ENABLE */

EXIT_PROC:

  // 释放内存
  SYSFree(qrecv.pbuf);
  s_tCmdQRx.pbuf = NULL;
  s_tCmdQRx.len = 0;

  return 1;
}

/**
 * @brief  Command receive queue is empty?
 * @param  None
 * @retval 1 Empty, 0 Not empty
 */
uint8_t CmdRxIsEmpty(void)
{
  if (uxQueueMessagesWaiting(s_tCmd.dnQueue) || uxQueueMessagesWaiting(s_tCmd.upQueue))
    return 0;
  else
    return 1;
}



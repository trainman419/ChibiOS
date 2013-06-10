/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    FlexCAN_v1/can_lld.c
 * @brief   SPC5xx CAN subsystem low level driver source.
 *
 * @addtogroup CAN
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_CAN || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief CAN1 driver identifier.*/
#if SPC5_CAN_USE_FLEXCAN0 || defined(__DOXYGEN__)
CANDriver CAND1;
#endif

/** @brief CAN2 driver identifier.*/
#if SPC5_CAN_USE_FLEXCAN1 || defined(__DOXYGEN__)
CANDriver CAND2;
#endif

/** @brief CAN3 driver identifier.*/
#if SPC5_CAN_USE_FLEXCAN2 || defined(__DOXYGEN__)
CANDriver CAND3;
#endif

/** @brief CAN4 driver identifier.*/
#if SPC5_CAN_USE_FLEXCAN3 || defined(__DOXYGEN__)
CANDriver CAND4;
#endif

/** @brief CAN5 driver identifier.*/
#if SPC5_CAN_USE_FLEXCAN4 || defined(__DOXYGEN__)
CANDriver CAND5;
#endif

/** @brief CAN6 driver identifier.*/
#if SPC5_CAN_USE_FLEXCAN5 || defined(__DOXYGEN__)
CANDriver CAND6;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Common TX ISR handler.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
static void can_lld_tx_handler(CANDriver *canp) {
  uint32_t iflag1;

  iflag1 = canp->flexcan->IFRL.R;
  /* No more events until a message is transmitted.*/
  canp->flexcan->IFRL.R |= iflag1 & 0xFFFFFF00;
  canp->flexcan->IFRH.R |= canp->flexcan->IFRH.R & 0xFFFFFFFF;
  chSysLockFromIsr();
  while (chSemGetCounterI(&canp->txsem) < 0)
    chSemSignalI(&canp->txsem);
  chEvtBroadcastFlagsI(&canp->txempty_event, iflag1 & 0xFFFFFF00);
  chSysUnlockFromIsr();
}

/**
 * @brief   Common RX ISR handler.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
static void can_lld_rx_handler(CANDriver *canp) {
  uint32_t iflag1;

  iflag1 = canp->flexcan->IFRL.R;
  if ((iflag1 & 0x000000FF) != 0) {
    chSysLockFromIsr();
    while (chSemGetCounterI(&canp->rxsem) < 0)
      chSemSignalI(&canp->rxsem);
    chEvtBroadcastFlagsI(&canp->rxfull_event, iflag1 & 0x000000FF);
    chSysUnlockFromIsr();

    /* Release the mailbox.*/
    canp->flexcan->IFRL.R |= iflag1 & 0x000000FF;
  }
}

/**
 * @brief   Common error ISR handler.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
static void can_lld_err_handler(CANDriver *canp) {

  uint32_t esr = canp->flexcan->ESR.R;
  flagsmask_t flags = 0;

  /* Error event.*/
  if ((esr & CAN_ESR_TWRN_INT) || (esr & CAN_ESR_RWRN_INT)) {
    canp->flexcan->ESR.B.TXWRN = 1U;
    canp->flexcan->ESR.B.RXWRN = 1U;
    flags |= CAN_LIMIT_WARNING;
  }

  if (esr & CAN_ESR_BOFF_INT) {
    canp->flexcan->ESR.B.BOFFINT = 1U;
    flags |= CAN_BUS_OFF_ERROR;
  }

  if (esr & CAN_ESR_ERR_INT) {
    canp->flexcan->ESR.B.ERRINT = 1U;
    flags |= CAN_FRAMING_ERROR;
  }
  chSysLockFromIsr();
  chEvtBroadcastFlagsI(&canp->error_event, flags);
  chSysUnlockFromIsr();
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if SPC5_CAN_USE_FLEXCAN0 || defined(__DOXYGEN__)
/**
 * @brief   CAN1 TX interrupt handler for MB 8-11.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_08_11_HANDLER) {


  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 TX interrupt handler for MB 12-15.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_12_15_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_16_31_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  CH_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN0_MB == 64)
/**
 * @brief   CAN1 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_32_63_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  CH_IRQ_EPILOGUE();
}
#endif

/*
 * @brief   CAN1 RX interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_00_03_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND1);

  CH_IRQ_EPILOGUE();
}

/*
 * @brief   CAN1 RX interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_04_07_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND1);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_ESR_ERR_INT_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND1);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 ESR_BOFF interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_ESR_BOFF_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND1);

  CH_IRQ_EPILOGUE();
}
#endif /* SPC5_CAN_USE_FLEXCAN0 */

#if SPC5_CAN_USE_FLEXCAN1 || defined(__DOXYGEN__)
/**
 * @brief   CAN2 TX interrupt handler for MB 8-11.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_08_11_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 TX interrupt handler for MB 12-15.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_12_15_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_16_31_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  CH_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN1_MB == 64)
/**
 * @brief   CAN2 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_32_63_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  CH_IRQ_EPILOGUE();
}
#endif

/*
 * @brief   CAN2 RX interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_00_03_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND2);

  CH_IRQ_EPILOGUE();
}

/*
 * @brief   CAN2 RX interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_04_07_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND2);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_ESR_ERR_INT_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND2);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 ESR_BOFF interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_ESR_BOFF_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND2);

  CH_IRQ_EPILOGUE();
}
#endif /* SPC5_CAN_USE_FLEXCAN1 */

#if SPC5_CAN_USE_FLEXCAN2 || defined(__DOXYGEN__)
/**
 * @brief   CAN3 TX interrupt handler for MB 8-11.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_08_11_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 TX interrupt handler for MB 12-15.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_12_15_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_16_31_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  CH_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN2_MB == 64)
/**
 * @brief   CAN3 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_32_63_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  CH_IRQ_EPILOGUE();
}
#endif

/*
 * @brief   CAN3 RX interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_00_03_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND3);

  CH_IRQ_EPILOGUE();
}

/*
 * @brief   CAN3 RX interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_04_07_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND3);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_ESR_ERR_INT_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND3);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 ESR_BOFF interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_ESR_BOFF_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND3);

  CH_IRQ_EPILOGUE();
}
#endif /* SPC5_CAN_USE_FLEXCAN2 */

#if SPC5_CAN_USE_FLEXCAN3 || defined(__DOXYGEN__)
/**
 * @brief   CAN4 TX interrupt handler for MB 8-11.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_08_11_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 TX interrupt handler for MB 12-15.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_12_15_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_16_31_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  CH_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN3_MB == 64)
/**
 * @brief   CAN4 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_32_63_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  CH_IRQ_EPILOGUE();
}
#endif

/*
 * @brief   CAN4 RX interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_00_03_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND4);

  CH_IRQ_EPILOGUE();
}

/*
 * @brief   CAN4 RX interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_04_07_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND4);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_ESR_ERR_INT_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND4);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 ESR_BOFF interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_ESR_BOFF_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND4);

  CH_IRQ_EPILOGUE();
}
#endif /* SPC5_CAN_USE_FLEXCAN3 */

#if SPC5_CAN_USE_FLEXCAN4 || defined(__DOXYGEN__)
/**
 * @brief   CAN5 TX interrupt handler for MB 8-11.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_08_11_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 TX interrupt handler for MB 12-15.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_12_15_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_16_31_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  CH_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN4_MB == 64)
/**
 * @brief   CAN5 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_32_63_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  CH_IRQ_EPILOGUE();
}
#endif

/*
 * @brief   CAN5 RX interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_00_03_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND5);

  CH_IRQ_EPILOGUE();
}

/*
 * @brief   CAN5 RX interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_04_07_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND5);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_ESR_ERR_INT_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND5);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 ESR_BOFF interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_ESR_BOFF_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND5);

  CH_IRQ_EPILOGUE();
}
#endif /* SPC5_CAN_USE_FLEXCAN4 */

#if SPC5_CAN_USE_FLEXCAN5 || defined(__DOXYGEN__)
/**
 * @brief   CAN6 TX interrupt handler for MB 8-11.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_08_11_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 TX interrupt handler for MB 12-15.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_12_15_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_16_31_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  CH_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN5_MB == 64)
/**
 * @brief   CAN6 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_32_63_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  CH_IRQ_EPILOGUE();
}
#endif

/*
 * @brief   CAN6 RX interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_00_03_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND6);

  CH_IRQ_EPILOGUE();
}

/*
 * @brief   CAN6 RX interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_04_07_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND6);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_ESR_ERR_INT_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND6);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 ESR_BOFF interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_ESR_BOFF_HANDLER) {

  CH_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND6);

  CH_IRQ_EPILOGUE();
}
#endif /* SPC5_CAN_USE_FLEXCAN5 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level CAN driver initialization.
 *
 * @notapi
 */
void can_lld_init(void) {

#if SPC5_CAN_USE_FLEXCAN0
  /* Driver initialization.*/
  canObjectInit(&CAND1);
  CAND1.flexcan = &SPC5_FLEXCAN_0;
  INTC.PSR[SPC5_FLEXCAN0_FLEXCAN_ESR_ERR_INT_NUMBER].R =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN0_FLEXCAN_ESR_BOFF_NUMBER].R =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN0_FLEXCAN_BUF_00_03_NUMBER].R =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN0_FLEXCAN_BUF_04_07_NUMBER].R =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN0_FLEXCAN_BUF_08_11_NUMBER].R =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN0_FLEXCAN_BUF_12_15_NUMBER].R =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN0_FLEXCAN_BUF_16_31_NUMBER].R =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
#endif

#if SPC5_CAN_USE_FLEXCAN1
  /* Driver initialization.*/
  canObjectInit(&CAND2);
  CAND2.flexcan = &SPC5_FLEXCAN_1;
  INTC.PSR[SPC5_FLEXCAN1_FLEXCAN_ESR_ERR_INT_NUMBER].R =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN1_FLEXCAN_ESR_BOFF_NUMBER].R =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN1_FLEXCAN_BUF_00_03_NUMBER].R =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN1_FLEXCAN_BUF_04_07_NUMBER].R =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN1_FLEXCAN_BUF_08_11_NUMBER].R =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN1_FLEXCAN_BUF_12_15_NUMBER].R =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN1_FLEXCAN_BUF_16_31_NUMBER].R =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
#endif

#if SPC5_CAN_USE_FLEXCAN2
  /* Driver initialization.*/
  canObjectInit(&CAND3);
  CAND3.flexcan = &SPC5_FLEXCAN_2;
  INTC.PSR[SPC5_FLEXCAN2_FLEXCAN_ESR_ERR_INT_NUMBER].R =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN2_FLEXCAN_ESR_BOFF_NUMBER].R =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN2_FLEXCAN_BUF_00_03_NUMBER].R =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN2_FLEXCAN_BUF_04_07_NUMBER].R =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN2_FLEXCAN_BUF_08_11_NUMBER].R =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN2_FLEXCAN_BUF_12_15_NUMBER].R =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN2_FLEXCAN_BUF_16_31_NUMBER].R =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
#endif

#if SPC5_CAN_USE_FLEXCAN3
  /* Driver initialization.*/
  canObjectInit(&CAND4);
  CAND4.flexcan = &SPC5_FLEXCAN_3;
  INTC.PSR[SPC5_FLEXCAN3_FLEXCAN_ESR_ERR_INT_NUMBER].R =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN3_FLEXCAN_ESR_BOFF_NUMBER].R =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN3_FLEXCAN_BUF_00_03_NUMBER].R =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN3_FLEXCAN_BUF_04_07_NUMBER].R =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN3_FLEXCAN_BUF_08_11_NUMBER].R =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN3_FLEXCAN_BUF_12_15_NUMBER].R =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN3_FLEXCAN_BUF_16_31_NUMBER].R =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
#endif

#if SPC5_CAN_USE_FLEXCAN4
  /* Driver initialization.*/
  canObjectInit(&CAND5);
  CAND5.flexcan = &SPC5_FLEXCAN_4;
  INTC.PSR[SPC5_FLEXCAN4_FLEXCAN_ESR_ERR_INT_NUMBER].R =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN4_FLEXCAN_ESR_BOFF_NUMBER].R =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN4_FLEXCAN_BUF_00_03_NUMBER].R =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN4_FLEXCAN_BUF_04_07_NUMBER].R =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN4_FLEXCAN_BUF_08_11_NUMBER].R =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN4_FLEXCAN_BUF_12_15_NUMBER].R =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN4_FLEXCAN_BUF_16_31_NUMBER].R =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
#endif

#if SPC5_CAN_USE_FLEXCAN5
  /* Driver initialization.*/
  canObjectInit(&CAND6);
  CAND6.flexcan = &SPC5_FLEXCAN_5;
  INTC.PSR[SPC5_FLEXCAN5_FLEXCAN_ESR_ERR_INT_NUMBER].R =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN5_FLEXCAN_ESR_BOFF_NUMBER].R =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN5_FLEXCAN_BUF_00_03_NUMBER].R =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN5_FLEXCAN_BUF_04_07_NUMBER].R =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN5_FLEXCAN_BUF_08_11_NUMBER].R =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN5_FLEXCAN_BUF_12_15_NUMBER].R =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC.PSR[SPC5_FLEXCAN5_FLEXCAN_BUF_16_31_NUMBER].R =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
#endif
}

/**
 * @brief   Configures and activates the CAN peripheral.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
void can_lld_start(CANDriver *canp) {

  uint8_t mb_index = 0, id = 0;

  /* Clock activation.*/
#if SPC5_CAN_USE_FLEXCAN0
  /* Set peripheral clock mode.*/
  if(&CAND1 == canp)
    SPC5_FLEXCAN0_ENABLE_CLOCK();
#endif

#if SPC5_CAN_USE_FLEXCAN1
  /* Set peripheral clock mode.*/
  if(&CAND2 == canp)
    SPC5_FLEXCAN1_ENABLE_CLOCK();
#endif

#if SPC5_CAN_USE_FLEXCAN2
  /* Set peripheral clock mode.*/
  if(&CAND3 == canp)
    SPC5_FLEXCAN2_ENABLE_CLOCK();
#endif

#if SPC5_CAN_USE_FLEXCAN3
  /* Set peripheral clock mode.*/
  if(&CAND4 == canp)
    SPC5_FLEXCAN3_ENABLE_CLOCK();
#endif

#if SPC5_CAN_USE_FLEXCAN4
  /* Set peripheral clock mode.*/
  if(&CAND5 == canp)
    SPC5_FLEXCAN4_ENABLE_CLOCK();
#endif

#if SPC5_CAN_USE_FLEXCAN5
  /* Set peripheral clock mode.*/
  if(&CAND6 == canp)
    SPC5_FLEXCAN5_ENABLE_CLOCK();
#endif

  /* Entering initialization mode. */
  canp->state = CAN_STARTING;
  canp->flexcan->CR.R |= CAN_CTRL_CLK_SRC;
  canp->flexcan->MCR.R &= ~CAN_MCR_MDIS;

  /*
   * Individual filtering per MB, disable frame self reception,
   * disable the FIFO, enable SuperVisor mode.
   */
#if SPC5_CAN_USE_FLEXCAN0
  if(&CAND1 == canp)
    canp->flexcan->MCR.R |= CAN_MCR_SUPV | CAN_MCR_MAXMB(SPC5_FLEXCAN0_MB - 1);
#endif

#if SPC5_CAN_USE_FLEXCAN1
  if(&CAND2 == canp)
    canp->flexcan->MCR.R |= CAN_MCR_SUPV | CAN_MCR_MAXMB(SPC5_FLEXCAN1_MB - 1);
#endif

#if SPC5_CAN_USE_FLEXCAN2
  if(&CAND3 == canp)
    canp->flexcan->MCR.R |= CAN_MCR_SUPV | CAN_MCR_MAXMB(SPC5_FLEXCAN2_MB - 1);
#endif

#if SPC5_CAN_USE_FLEXCAN3
  if(&CAND4 == canp)
    canp->flexcan->MCR.R |= CAN_MCR_SUPV | CAN_MCR_MAXMB(SPC5_FLEXCAN3_MB - 1);
#endif

#if SPC5_CAN_USE_FLEXCAN4
  if(&CAND5 == canp)
    canp->flexcan->MCR.R |= CAN_MCR_SUPV | CAN_MCR_MAXMB(SPC5_FLEXCAN4_MB - 1);
#endif

#if SPC5_CAN_USE_FLEXCAN5
  if(&CAND6 == canp)
    canp->flexcan->MCR.R |= CAN_MCR_SUPV | CAN_MCR_MAXMB(SPC5_FLEXCAN5_MB - 1);
#endif

  canp->flexcan->CR.R |= CAN_CTRL_TSYN | CAN_CTRL_RJW(3);

  /* TX MB initialization.*/
#if SPC5_CAN_USE_FLEXCAN0
  if(&CAND1 == canp) {
    for(mb_index = 0; mb_index < (SPC5_FLEXCAN0_MB - CAN_RX_MAILBOXES);
        mb_index++) {
      canp->flexcan->BUF[mb_index + CAN_RX_MAILBOXES].CS.B.CODE = 8U;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN1
  if(&CAND2 == canp) {
    for(mb_index = 0; mb_index < (SPC5_FLEXCAN1_MB - CAN_RX_MAILBOXES);
        mb_index++) {
      canp->flexcan->BUF[mb_index + CAN_RX_MAILBOXES].CS.B.CODE = 8U;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN2
  if(&CAND3 == canp) {
    for(mb_index = 0; mb_index < (SPC5_FLEXCAN2_MB - CAN_RX_MAILBOXES);
        mb_index++) {
      canp->flexcan->BUF[mb_index + CAN_RX_MAILBOXES].CS.B.CODE = 8U;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN3
  if(&CAND4 == canp) {
    for(mb_index = 0; mb_index < (SPC5_FLEXCAN3_MB - CAN_RX_MAILBOXES);
        mb_index++) {
      canp->flexcan->BUF[mb_index + CAN_RX_MAILBOXES].CS.B.CODE = 8U;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN4
  if(&CAND5 == canp) {
    for(mb_index = 0; mb_index < (SPC5_FLEXCAN4_MB - CAN_RX_MAILBOXES);
        mb_index++) {
      canp->flexcan->BUF[mb_index + CAN_RX_MAILBOXES].CS.B.CODE = 8U;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN5
  if(&CAND6 == canp) {
    for(mb_index = 0; mb_index < (SPC5_FLEXCAN5_MB - CAN_RX_MAILBOXES);
        mb_index++) {
      canp->flexcan->BUF[mb_index + CAN_RX_MAILBOXES].CS.B.CODE = 8U;
    }
  }
#endif

  /* Unlock Message buffers.*/
  (void) canp->flexcan->TIMER.R;

  /* MCR initialization.*/
  canp->flexcan->MCR.R |= canp->config->mcr;

  /* CTRL initialization.*/
  canp->flexcan->CR.R |= canp->config->ctrl;

  /* Interrupt sources initialization.*/
  canp->flexcan->MCR.R |= CAN_MCR_WRN_EN;

  canp->flexcan->CR.R |= CAN_CTRL_BOFF_MSK | CAN_CTRL_ERR_MSK  |
                         CAN_CTRL_TWRN_MSK | CAN_CTRL_RWRN_MSK;

#if !SPC5_CAN_USE_FILTERS
  /* RX MB initialization.*/
  for(mb_index = 0; mb_index < CAN_RX_MAILBOXES; mb_index++) {
    canp->flexcan->BUF[mb_index].CS.B.CODE = 0U;
    canp->flexcan->BUF[mb_index].ID.R = 0U;
    canp->flexcan->BUF[mb_index].CS.B.CODE = 4U;
  }

  /* Receive all.*/
  canp->flexcan->RXGMASK.R = 0x00000000;
#else
  for (id = 0; id < CAN_RX_MAILBOXES; id++) {
    canp->flexcan->BUF[id].CS.B.CODE = 0U;
    if (canp->config->RxFilter[id].scale) {
      canp->flexcan->BUF[id].CS.B.IDE = 1U;
      canp->flexcan->BUF[id].ID.R = canp->config->RxFilter[id].register1;
    }
    else {
      canp->flexcan->BUF[id].CS.B.IDE = 0U;
      canp->flexcan->BUF[id].ID.B.STD_ID = canp->config->RxFilter[id].register1;
      canp->flexcan->BUF[id].ID.B.EXT_ID = 0U;
    }
    /* RX MB initialization.*/
    canp->flexcan->BUF[id].CS.B.CODE = 4U;
  }
  canp->flexcan->RXGMASK.R = 0x0FFFFFFF;
#endif

  /* Enable MBs interrupts.*/
#if SPC5_CAN_USE_FLEXCAN0
  if(&CAND1 == canp) {
    if(SPC5_FLEXCAN0_MB == 32) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
    }
    else if(SPC5_FLEXCAN0_MB == 64) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
      canp->flexcan->IMRH.R = 0xFFFFFFFF;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN1
  if(&CAND2 == canp) {
    if(SPC5_FLEXCAN1_MB == 32) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
    }
    else if(SPC5_FLEXCAN1_MB == 64) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
      canp->flexcan->IMRH.R = 0xFFFFFFFF;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN2
  if(&CAND3 == canp) {
    if(SPC5_FLEXCAN2_MB == 32) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
    }
    else if(SPC5_FLEXCAN2_MB == 64) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
      canp->flexcan->IMRH.R = 0xFFFFFFFF;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN3
  if(&CAND4 == canp) {
    if(SPC5_FLEXCAN3_MB == 32) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
    }
    else if(SPC5_FLEXCAN3_MB == 64) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
      canp->flexcan->IMRH.R = 0xFFFFFFFF;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN4
  if(&CAND5 == canp) {
    if(SPC5_FLEXCAN4_MB == 32) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
    }
    else if(SPC5_FLEXCAN4_MB == 64) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
      canp->flexcan->IMRH.R = 0xFFFFFFFF;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN5
  if(&CAND6 == canp) {
    if(SPC5_FLEXCAN5_MB == 32) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
    }
    else if(SPC5_FLEXCAN5_MB == 64) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
      canp->flexcan->IMRH.R = 0xFFFFFFFF;
    }
  }
#endif

  /* CAN BUS synchronization.*/
  canp->flexcan->MCR.B.HALT = 0;
}

/**
 * @brief   Deactivates the CAN peripheral.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
void can_lld_stop(CANDriver *canp) {

  /* If in ready state then disables the CAN peripheral.*/
  if (canp->state == CAN_READY) {

    /* Disable Interrupt sources.*/
    canp->flexcan->MCR.R &= ~CAN_MCR_WRN_EN;
    canp->flexcan->CR.R &= ~(CAN_CTRL_BOFF_MSK | CAN_CTRL_ERR_MSK  |
                             CAN_CTRL_TWRN_MSK | CAN_CTRL_RWRN_MSK);
    canp->flexcan->IMRL.R = 0x00000000;

    canp->flexcan->MCR.R &= ~CAN_MCR_MDIS;

#if SPC5_CAN_USE_FLEXCAN0
  /* Set peripheral clock mode.*/
  if(&CAND1 == canp)
    SPC5_FLEXCAN0_DISABLE_CLOCK();

#endif
#if SPC5_CAN_USE_FLEXCAN1
  /* Set peripheral clock mode.*/
  if(&CAND2 == canp)
    SPC5_FLEXCAN1_DISABLE_CLOCK();
#endif
#if SPC5_CAN_USE_FLEXCAN2
  /* Set peripheral clock mode.*/
  if(&CAND3 == canp)
    SPC5_FLEXCAN2_DISABLE_CLOCK();
#endif
#if SPC5_CAN_USE_FLEXCAN3
  /* Set peripheral clock mode.*/
  if(&CAND4 == canp)
    SPC5_FLEXCAN3_DISABLE_CLOCK();
#endif
#if SPC5_CAN_USE_FLEXCAN4
  /* Set peripheral clock mode.*/
  if(&CAND5 == canp)
    SPC5_FLEXCAN4_DISABLE_CLOCK();
#endif
#if SPC5_CAN_USE_FLEXCAN5
  /* Set peripheral clock mode.*/
  if(&CAND6 == canp)
    SPC5_FLEXCAN5_DISABLE_CLOCK();
#endif
  }
}

/**
 * @brief   Determines whether a frame can be transmitted.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] mailbox   mailbox number, @p CAN_ANY_MAILBOX for any mailbox
 *
 * @return              The queue space availability.
 * @retval FALSE        no space in the transmit queue.
 * @retval TRUE         transmit slot available.
 *
 * @notapi
 */
bool_t can_lld_is_tx_empty(CANDriver *canp, canmbx_t mailbox) {

  uint8_t mbid = 0;

  if(mailbox == CAN_ANY_MAILBOX) {
#if SPC5_CAN_USE_FLEXCAN0
    if(&CAND1 == canp) {
      for (mbid = 8; mbid < SPC5_FLEXCAN0_MB; mbid++) {
        if (canp->flexcan->BUF[mbid].CS.B.CODE == 0x08) {
          return TRUE;
        }
      }
      return FALSE;
    }
#endif
#if SPC5_CAN_USE_FLEXCAN1
    if(&CAND2 == canp) {
      for (mbid = 8; mbid < SPC5_FLEXCAN1_MB; mbid++) {
        if (canp->flexcan->BUF[mbid].CS.B.CODE == 0x08) {
          return TRUE;
        }
      }
      return FALSE;
    }
#endif
#if SPC5_CAN_USE_FLEXCAN2
    if(&CAND3 == canp) {
      for (mbid = 8; mbid < SPC5_FLEXCAN2_MB; mbid++) {
        if (canp->flexcan->BUF[mbid].CS.B.CODE == 0x08) {
          return TRUE;
        }
      }
      return FALSE;
    }
#endif
#if SPC5_CAN_USE_FLEXCAN3
    if(&CAND4 == canp) {
      for (mbid = 8; mbid < SPC5_FLEXCAN3_MB; mbid++) {
        if (canp->flexcan->BUF[mbid].CS.B.CODE == 0x08) {
          return TRUE;
        }
      }
      return FALSE;
    }
#endif
#if SPC5_CAN_USE_FLEXCAN4
    if(&CAND5 == canp) {
      for (mbid = 8; mbid < SPC5_FLEXCAN4_MB; mbid++) {
        if (canp->flexcan->BUF[mbid].CS.B.CODE == 0x08) {
          return TRUE;
        }
      }
      return FALSE;
    }
#endif
#if SPC5_CAN_USE_FLEXCAN5
    if(&CAND6 == canp) {
      for (mbid = 8; mbid < SPC5_FLEXCAN5_MB; mbid++) {
        if (canp->flexcan->BUF[mbid].CS.B.CODE == 0x08) {
          return TRUE;
        }
      }
      return FALSE;
    }
#endif
  }
  else {
    return canp->flexcan->BUF[mailbox + 7].CS.B.CODE == 0x08;
  }
  return FALSE;
}

/**
 * @brief   Inserts a frame into the transmit queue.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] ctfp      pointer to the CAN frame to be transmitted
 * @param[in] mailbox   mailbox number,  @p CAN_ANY_MAILBOX for any mailbox
 *
 * @notapi
 */
void can_lld_transmit(CANDriver *canp,
                      canmbx_t mailbox,
                      const CANTxFrame *ctfp) {

  CAN_TxMailBox_TypeDef *tmbp = NULL;
  uint8_t mbid = 0;

  /* Pointer to a free transmission mailbox.*/
  if (mailbox == CAN_ANY_MAILBOX) {
#if SPC5_CAN_USE_FLEXCAN0
    if(&CAND1 == canp) {
      for (mbid = 8; mbid < SPC5_FLEXCAN0_MB; mbid++) {
        if ((canp->flexcan->BUF[mbid].CS.B.CODE & 8U) == 1) {
          tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mbid];
          break;
        }
      }
    }
#endif
#if SPC5_CAN_USE_FLEXCAN1
    if(&CAND2 == canp) {
      for (mbid = 8; mbid < SPC5_FLEXCAN1_MB; mbid++) {
        if ((canp->flexcan->BUF[mbid].CS.B.CODE & 8U) == 1) {
          tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mbid];
          break;
        }
      }
    }
#endif
#if SPC5_CAN_USE_FLEXCAN2
    if(&CAND3 == canp) {
      for (mbid = 8; mbid < SPC5_FLEXCAN2_MB; mbid++) {
        if ((canp->flexcan->BUF[mbid].CS.B.CODE & 8U) == 1) {
          tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mbid];
          break;
        }
      }
    }
#endif
#if SPC5_CAN_USE_FLEXCAN3
    if(&CAND4 == canp) {
      for (mbid = 8; mbid < SPC5_FLEXCAN3_MB; mbid++) {
        if ((canp->flexcan->BUF[mbid].CS.B.CODE & 8U) == 1) {
          tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mbid];
          break;
        }
      }
    }
#endif
#if SPC5_CAN_USE_FLEXCAN4
    if(&CAND5 == canp) {
      for (mbid = 8; mbid < SPC5_FLEXCAN4_MB; mbid++) {
        if ((canp->flexcan->BUF[mbid].CS.B.CODE & 8U) == 1) {
          tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mbid];
          break;
        }
      }
    }
#endif
#if SPC5_CAN_USE_FLEXCAN5
    if(&CAND6 == canp) {
      for (mbid = 8; mbid < SPC5_FLEXCAN5_MB; mbid++) {
        if ((canp->flexcan->BUF[mbid].CS.B.CODE & 8U) == 1) {
          tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mbid];
          break;
        }
      }
    }
#endif
  }
  else {
    tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mailbox + 7];
  }

  /* Preparing the message.*/
  if (ctfp->IDE) {
    tmbp->CS.B.IDE = 1U;
    tmbp->CS.B.RTR = 0U;
    tmbp->ID.R = ctfp->EID;
  }
  else {
    tmbp->CS.B.IDE = 0U;
    tmbp->CS.B.RTR = 0U;
    tmbp->ID.R = ctfp->SID << 18;
  }
  tmbp->CS.B.LENGTH = ctfp->LENGTH;
  tmbp->DATA[0] = ctfp->data32[0];
  tmbp->DATA[1] = ctfp->data32[1];
  tmbp->CS.B.CODE = 0x0C;
}

/**
 *
 * @brief   Determines whether a frame has been received.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] mailbox   mailbox number, @p CAN_ANY_MAILBOX for any mailbox
 *
 * @return              The queue space availability.
 * @retval FALSE        no space in the transmit queue.
 * @retval TRUE         transmit slot available.
 *
 * @notapi
 */

bool_t can_lld_is_rx_nonempty(CANDriver *canp, canmbx_t mailbox) {

  uint8_t mbid = 0;
  bool_t mb_status = FALSE;

  switch (mailbox) {
  case CAN_ANY_MAILBOX:
    for (mbid = 0; mbid < CAN_RX_MAILBOXES; mbid++) {
      if(canp->flexcan->BUF[mbid].CS.B.CODE == 2U) {
        mb_status = TRUE;
      }
    }
    return mb_status;
  case 1:
    return (canp->flexcan->BUF[0].CS.B.CODE == 2U);
  case 2:
    return (canp->flexcan->BUF[1].CS.B.CODE == 2U);
  case 3:
    return (canp->flexcan->BUF[2].CS.B.CODE == 2U);
  case 4:
    return (canp->flexcan->BUF[3].CS.B.CODE == 2U);
  case 5:
    return (canp->flexcan->BUF[4].CS.B.CODE == 2U);
  case 6:
    return (canp->flexcan->BUF[5].CS.B.CODE == 2U);
  case 7:
    return (canp->flexcan->BUF[6].CS.B.CODE == 2U);
  case 8:
    return (canp->flexcan->BUF[7].CS.B.CODE == 2U);
  default:
    return FALSE;
  }
}

/**
 * @brief   Receives a frame from the input queue.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] mailbox   mailbox number, @p CAN_ANY_MAILBOX for any mailbox
 * @param[out] crfp     pointer to the buffer where the CAN frame is copied
 *
 * @notapi
 */
void can_lld_receive(CANDriver *canp,
                     canmbx_t mailbox,
                     CANRxFrame *crfp) {

  uint32_t mbid = 0, index = 0;

  if(mailbox != CAN_ANY_MAILBOX) {
    mbid = mailbox;
  }
  else {
    for (index = 0; index < CAN_RX_MAILBOXES; index++) {
      if(canp->flexcan->BUF[index].CS.B.CODE == 2U) {
        mbid = index;
        break;
      }
    }
  }

  /* Lock the RX MB.*/
  (void) canp->flexcan->BUF[mbid].CS.B.CODE;

  /* Fetches the message.*/
  crfp->data32[0] = canp->flexcan->BUF[mbid].DATA.W[0];
  crfp->data32[1] = canp->flexcan->BUF[mbid].DATA.W[1];

  /* Decodes the various fields in the RX frame.*/
  crfp->RTR = canp->flexcan->BUF[mbid].CS.B.RTR;
  crfp->IDE = canp->flexcan->BUF[mbid].CS.B.IDE;
  if (crfp->IDE)
    crfp->EID = canp->flexcan->BUF[mbid].ID.B.EXT_ID;
  else
    crfp->SID = canp->flexcan->BUF[mbid].ID.B.STD_ID;
  crfp->LENGTH = canp->flexcan->BUF[mbid].CS.B.LENGTH;
  crfp->TIME = canp->flexcan->BUF[mbid].CS.B.TIMESTAMP;

  /* Unlock the RX MB.*/
  (void) canp->flexcan->TIMER.R;

  /* Reconfigure the RX MB in empty status.*/
  canp->flexcan->BUF[mbid].CS.B.CODE = 4U;
}

#if CAN_USE_SLEEP_MODE || defined(__DOXYGEN__)
/**
 * @brief   Enters the sleep mode.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
void can_lld_sleep(CANDriver *canp) {

  /*canp->can->MCR |= CAN_MCR_SLEEP;*/
}

/**
 * @brief   Enforces leaving the sleep mode.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
void can_lld_wakeup(CANDriver *canp) {

  /*canp->can->MCR &= ~CAN_MCR_SLEEP;*/
}
#endif /* CAN_USE_SLEEP_MODE */

#endif /* HAL_USE_CAN */

/** @} */
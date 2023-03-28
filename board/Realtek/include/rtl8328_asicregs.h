#ifndef __RTL8328_ASIC_REGS_H__
#define __RTL8328_ASIC_REGS_H__

#include <rtl8328_types.h>

/* Register access macro (REG*()) */
#ifndef REG32
#define REG32(reg)          (*((volatile uint32 *)(reg)))
#endif
#ifndef REG16
#define REG16(reg)          (*((volatile uint16 *)(reg)))
#endif
#ifndef REG8
#define REG8(reg)           (*((volatile uint8  *)(reg)))
#endif

/* Register access macro (READ_MEM32() and WRITE_MEM32()). */
#if defined(__MODEL_USER__)
/*
 * big_endian() is a macro to make endian consistence between 8389 board and x86 PC.
 * All the ASIC registers stored in memory will be stored at big-endian order.
 * Therefore, we will see the same result when memory dump ASIC registers.
 */
#define big_endian32(x) ((((x)&0xff000000)>>24)|(((x)&0x00ff0000)>>8)|(((x)&0x0000ff00)<<8)|(((x)&0x000000ff)<<24))
#define big_endian16(x) (((x)>>8)|(((x)&0x000000ff)<<8))
#define big_endian(x) big_endian32(x) /* backward-compatible */
#else
#define big_endian32(x) (x)
#define big_endian16(x) (x)
#define big_endian(x) big_endian32(x) /* backward-compatible */
#endif

#define MEM32_WRITE(reg,val) REG32(reg)=big_endian(val)
#define MEM16_WRITE(reg,val) REG16(reg)=big_endian16(val)
#define MEM8_WRITE(reg,val) REG8(reg)=(val)
#define MEM32_READ(reg) big_endian(REG32(reg))
#define MEM16_READ(reg) big_endian16(REG16(reg))
#define MEM8_READ(reg) (REG8(reg))

/*reset*/
#define RESET_GLOBAL_CONTROL_REGISTER_0_ADDR  (0xbb020000)

/*
 ***** NIC Register Mapping (0xBB68_0000 ~ 0xBB68_FFFF) *****
 */
#define SWCORE_NIC_BASE         0xBB680000

#define CPURPD_BASE                 (SWCORE_NIC_BASE + 0x0000)  /* Rx Pkthdr Descriptor m Control Register Base                     */
#define CPURPDCR0                   (0x000 + CPURPD_BASE)       /* 0xBB00_3100, Rx Pkthdr Descriptor 0 Control Register (Lowest)    */
#define CPURPDCR1                   (0x004 + CPURPD_BASE)       /* 0xBB00_3104, Rx Pkthdr Descriptor 1 Control Register             */
#define CPURPDCR2                   (0x008 + CPURPD_BASE)       /* 0xBB00_3108, Rx Pkthdr Descriptor 2 Control Register             */
#define CPURPDCR3                   (0x00C + CPURPD_BASE)       /* 0xBB00_310C, Rx Pkthdr Descriptor 3 Control Register             */
#define CPURPDCR4                   (0x010 + CPURPD_BASE)       /* 0xBB00_3110, Rx Pkthdr Descriptor 4 Control Register             */
#define CPURPDCR5                   (0x014 + CPURPD_BASE)       /* 0xBB00_3114, Rx Pkthdr Descriptor 5 Control Register             */
#define CPURPDCR6                   (0x018 + CPURPD_BASE)       /* 0xBB00_3118, Rx Pkthdr Descriptor 6 Control Register             */
#define CPURPDCR7                   (0x01C + CPURPD_BASE)       /* 0xBB00_311C, Rx Pkthdr Descriptor 7 Control Register (Highest)   */
#define CPURPDCRn(n)                (((0x004)*(n))+ CPURPD_BASE)/* Rx Pkthdr Descriptor n Control Register                          */

#define CPURMD_BASE                 (SWCORE_NIC_BASE + 0x0020)  /* Rx Pkthdr Descriptor m Control Register Base                                  */
#define CPURMDCR                    (0x000 + CPURMD_BASE)       /* 0xBB00_3120, Rx mbuf Descriptor Control Register                              */

#define CPUTPD_BASE                 (SWCORE_NIC_BASE + 0x0024)  /* Rx Pkthdr Descriptor m Control Register Base                                  */
#define CPUTPDCR0                   (0x000 + CPUTPD_BASE)       /* 0xBB00_3124, Tx Pkthdr Descriptor 0 Control Register (Low)                    */
#define CPUTPDCR1                   (0x004 + CPUTPD_BASE)       /* 0xBB00_3128, Tx Pkthdr Descriptor 1 Control Register (High)                   */

#define CPUI_BASE                   (SWCORE_NIC_BASE + 0x002C)  /* Rx Pkthdr Descriptor m Control Register Base                                  */
#define CPUIIMR                     (0x000 + CPUI_BASE)         /* 0xBB00_312C, CPU Interface Interrupt Mask Register      R/W      00000000H    */
#define CPUIISR                     (0x004 + CPUI_BASE)         /* 0xBB00_3130, CPU Interface Interrupt Status Register      R/W      00000000H  */
#define CPUICR                      (0x008 + CPUI_BASE)         /* 0xBB00_3134, CPU Interface Control Register      R/W      00000000H           */

#define CPURXDSCNT_BASE             (SWCORE_NIC_BASE + 0x0038)  /*The current number that ASIC can used to rx packet*/
#define CPURXDSC0CNT                (0x000 + CPURXDSCNT_BASE)
#define CPURXDSC1CNT                (0x004 + CPURXDSCNT_BASE)
#define CPURXDSC2CNT                (0x008 + CPURXDSCNT_BASE)
#define CPURXDSC3CNT                (0x00C + CPURXDSCNT_BASE)
#define CPURXDSC4CNT                (0x010 + CPURXDSCNT_BASE)
#define CPURXDSC5CNT                (0x014 + CPURXDSCNT_BASE)
#define CPURXDSC6CNT                (0x018 + CPURXDSCNT_BASE)
#define CPURXDSC7CNT                (0x01C + CPURXDSCNT_BASE)
#define CPURXPKT                    (0x020 + CPURXDSCNT_BASE)
#define CPURXDSCTHR                 (0x024 + CPURXDSCNT_BASE)

/* CPURPDCRm - Rx Pkthdr Descriptor m Control Register
 * CPURPDCR0, CPURPDCR1, CPURPDCR2, CPURPDCR3, CPURPDCR4, CPURPDCR5, CPURPDCR6, CPURPDCR7.
 */
#define RXFDPBASE_OFFSET            (2)                            /* [Write mode] The Rx Pkt Header First Descriptor Pointer Address(word alignment, irrelevant, LSB 2 bits).  */
#define RXFDPBASE_MASK              (0x3FFFFFFFU<<RXFDPBASE_OFFSET) /* [Write mode] The Rx Pkt Header First Descriptor Pointer Address(word alignment, irrelevant, LSB 2 bits).  */
#define RXCDPIDX_OFFSET             (2)                            /* [Read mode] The Rx Pkthdr Current Descriptor Address.                                                     */
#define RXCDPIDX_MASK               (0x3FFFFFFFU<<RXCDPIDX_OFFSET)  /* [Read mode] The Rx Pkthdr Current Descriptor Address.                                                     */

/* CPURMDCR  - Rx mbuf Descriptor Control Register  */
#define MB_FDPBASE_OFFSET           (2)                             /* [Write mode] The Rx mbuf First Descriptor Pointer Address (Word alignment, irrelevant, LSB 2 bits).  */
#define MB_FDPBASE_MASK             (0x3FFFFFFFU<<MB_FDPBASE_OFFSET) /* [Write mode] The Rx mbuf First Descriptor Pointer Address (Word alignment, irrelevant, LSB 2 bits).  */
#define MB_CDPIDX_OFFSET            (2)                             /* [Read mode] The Rx mbuf Current Descriptor Address.                                                  */
#define MB_CDPIDX_MASK              (0x3FFFFFFFU<<MB_CDPIDX_OFFSET)  /* [Read mode] The Rx mbuf Current Descriptor Address.                                                  */

/* CPUTPDCR0 - Tx Pkthdr Descriptor 0 Control Register (Low) */
/* CPUTPDCR1 - Tx Pkthdr Descriptor 1 Control Register (High) */
#define TXFDPBASE_OFFSET            (2)                             /* [Write mode] The Tx Pkt Header First Descriptor Pointer Address(word alignment, irrelevant, LSB 2 bits).  */
#define TXFDPBASE_MASK              (0x3FFFFFFFU<<TXFDPBASE_OFFSET)  /* [Write mode] The Tx Pkt Header First Descriptor Pointer Address(word alignment, irrelevant, LSB 2 bits).  */
#define TXCDPIndex_OFFSET           (2)                             /* [Read mode] The Tx Pkthdr Current Descriptor Address.                                                     */
#define TXCDPIndex_MASK             (0x3FFFFFFFU<<TXCDPIndex_OFFSET) /* [Read mode] The Tx Pkthdr Current Descriptor Address.                                                     */

/* CPUIIMR - CPU Interface Interrupt Mask Register */
#define EN_PHCNTD7_EN_PHCNTD0_OFFSET            (21)                                        /*0: Disable Interrupt for RXDSCnAVAIL reach the RxDscThr.1: Enable */
#define EN_PHCNTD7_EN_PHCNTD0_MASK              (0xFFU<<EN_PHCNTD7_EN_PHCNTD0_OFFSET)       /*0: Disable Interrupt for RXDSCnAVAIL reach the RxDscThr.1: Enable */
#define EN_PHD7_EN_PHD0_OFFSET                (13)                                         /* 0: Disable Interrupt for running out of pkthdr Descriptors1: Enable     */
#define EN_PHD7_EN_PHD0_MASK                  (0xFFU<<EN_PHD7_EN_PHD0_OFFSET)               /* 0: Disable Interrupt for running out of pkthdr Descriptors1: Enable     */
#define EN_MBD_OFFSET                         (12)                                         /* 0: Disable Interrupt for running out of mbuf Descriptors1: Enable       */
#define EN_MBD_MASK                           (0x1U<<EN_MBD_OFFSET)                         /* 0: Disable Interrupt for running out of mbuf Descriptors1: Enable       */
#define EN_TX_DONE1_EN_TX_DONE0_OFFSET        (10)                                         /* 0: Disable Interrupt for successful Transmission of each packet1: EnableSuccessful transmission: Successful DMA of all packets to the Switch Core CPU RxFIFO.  */
#define EN_TX_DONE1_EN_TX_DONE0_MASK          (0x3U<<EN_TX_DONE1_EN_TX_DONE0_OFFSET)        /* 0: Disable Interrupt for successful Transmission of each packet1: EnableSuccessful transmission: Successful DMA of all packets to the Switch Core CPU RxFIFO.  */
#define EN_RX_DONE7_EN_RX_DONE0_OFFSET        (2)                                          /* 0: Disable Interrupt for successful Receive of each packet1: EnableSuccessful Receive: Successful DMA of all packets to the SDRAM.                             */
#define EN_RX_DONE7_EN_RX_DONE0_MASK          (0xFFU<<EN_RX_DONE7_EN_RX_DONE0_OFFSET)       /* 0: Disable Interrupt for successful Receive of each packet1: EnableSuccessful Receive: Successful DMA of all packets to the SDRAM.                             */
#define EN_TX_ALLDONE1_EN_TX_ALLDONE0_OFFSET  (0)                                          /* 0: Disable Interrupt for successful Transmission of All packets in the Tx descriptor ring (all descriptors are owned by the CPU).1: Enable                     */
#define EN_TX_ALLDONE1_EN_TX_ALLDONE0_MASK    (0x3U<<EN_TX_ALLDONE1_EN_TX_ALLDONE0_OFFSET)  /* 0: Disable Interrupt for successful Transmission of All packets in the Tx descriptor ring (all descriptors are owned by the CPU).1: Enable                     */

/* CPUIISR - CPU Interface Interrupt Status Register */
#define INT_PHCNTS7_INT_PHCNTS0_OFFSET          (21)
#define INT_PHCNTS7_INT_PHCNTS0_MASK            (0xFFU<<INT_PHCNTS7_INT_PHCNTS0_OFFSET) /*Interrupt of Interrupt of EN_PHCNTD status, write '1' to clear*/
#define INT_PHCNTS7_OFFSET                      (28)
#define INT_PHCNTS7_MASK                        (0x1U<<INT_PHCNTS7_OFFSET)
#define INT_PHCNTS6_OFFSET                      (27)
#define INT_PHCNTS6_MASK                        (0x1U<<INT_PHCNTS7_OFFSET)
#define INT_PHCNTS5_OFFSET                      (26)
#define INT_PHCNTS5_MASK                        (0x1U<<INT_PHCNTS7_OFFSET)
#define INT_PHCNTS4_OFFSET                      (25)
#define INT_PHCNTS4_MASK                        (0x1U<<INT_PHCNTS7_OFFSET)
#define INT_PHCNTS3_OFFSET                      (24)
#define INT_PHCNTS3_MASK                        (0x1U<<INT_PHCNTS7_OFFSET)
#define INT_PHCNTS2_OFFSET                      (23)
#define INT_PHCNTS2_MASK                        (0x1U<<INT_PHCNTS7_OFFSET)
#define INT_PHCNTS1_OFFSET                      (22)
#define INT_PHCNTS1_MASK                        (0x1U<<INT_PHCNTS7_OFFSET)
#define INT_PHCNTS0_OFFSET                      (21)
#define INT_PHCNTS0_MASK                        (0x1U<<INT_PHCNTS7_OFFSET)
#define INT_PHDS7_INT_PHDS0_OFFSET             (13)                                    /* Interrupt of EN_PHD running out status, write '1' to clear.     */
#define INT_PHDS7_INT_PHDS0_MASK               (0xFFU<<INT_PHDS7_INT_PHDS0_OFFSET)      /* Interrupt of EN_PHD running out status, write '1' to clear.     */
#define INT_PHDS7_OFFSET                       (20)
#define INT_PHDS7_MASK                         (0x1U<<INT_PHDS7_OFFSET)
#define INT_PHDS6_OFFSET                       (19)
#define INT_PHDS6_MASK                         (0x1U<<INT_PHDS6_OFFSET)
#define INT_PHDS5_OFFSET                       (18)
#define INT_PHDS5_MASK                         (0x1U<<INT_PHDS5_OFFSET)
#define INT_PHDS4_OFFSET                       (17)
#define INT_PHDS4_MASK                         (0x1U<<INT_PHDS4_OFFSET)
#define INT_PHDS3_OFFSET                       (16)
#define INT_PHDS3_MASK                         (0x1U<<INT_PHDS3_OFFSET)
#define INT_PHDS2_OFFSET                       (15)
#define INT_PHDS2_MASK                         (0x1U<<INT_PHDS2_OFFSET)
#define INT_PHDS1_OFFSET                       (14)
#define INT_PHDS1_MASK                         (0x1U<<INT_PHDS1_OFFSET)
#define INT_PHDS0_OFFSET                       (13)
#define INT_PHDS0_MASK                         (0x1U<<INT_PHDS0_OFFSET)
#define INT_MBDS_OFFSET                        (12)                                    /* Interrupt of EN_MBD running out status, write '1' to clear.     */
#define INT_MBDS_MASK                          (0x1U<<INT_MBDS_OFFSET)                  /* Interrupt of EN_MBD running out status, write '1' to clear.     */
#define INT_TX_DONE1_INT_TX_DONE0_OFFSET       (10)                                    /* Interrupt of EN_TX_DONE pending status, write '1' to clear.     */
#define INT_TX_DONE1_INT_TX_DONE0_MASK         (0x3U<<INT_TX_DONE1_INT_TX_DONE0_OFFSET) /* Interrupt of EN_TX_DONE pending status, write '1' to clear.     */
#define INT_TX_DONE1_OFFSET                    (11)
#define INT_TX_DONE1_MASK                      (0x1U<<INT_TX_DONE1_OFFSET)
#define INT_TX_DONE0_OFFSET                    (10)
#define INT_TX_DONE0_MASK                      (0x1U<<INT_TX_DONE0_OFFSET)
#define INT_RX_DONE7_INT_RX_DONE0_OFFSET       (2)                                     /* Interrupt of EN_RX_DONE pending status, write '1' to clear.     */
#define INT_RX_DONE7_INT_RX_DONE0_MASK         (0xFFU<<INT_RX_DONE7_INT_RX_DONE0_OFFSET)                    /* Interrupt of EN_RX_DONE pending status, write '1' to clear.     */
#define INT_RX_DONE7_OFFSET                    (9)
#define INT_RX_DONE7_MASK                      (0x1U<<INT_RX_DONE7_OFFSET)
#define INT_RX_DONE6_OFFSET                    (8)
#define INT_RX_DONE6_MASK                      (0x1U<<INT_RX_DONE6_OFFSET)
#define INT_RX_DONE5_OFFSET                    (7)
#define INT_RX_DONE5_MASK                      (0x1U<<INT_RX_DONE5_OFFSET)
#define INT_RX_DONE4_OFFSET                    (6)
#define INT_RX_DONE4_MASK                      (0x1U<<INT_RX_DONE4_OFFSET)
#define INT_RX_DONE3_OFFSET                    (5)
#define INT_RX_DONE3_MASK                      (0x1U<<INT_RX_DONE3_OFFSET)
#define INT_RX_DONE2_OFFSET                    (4)
#define INT_RX_DONE2_MASK                      (0x1U<<INT_RX_DONE2_OFFSET)
#define INT_RX_DONE1_OFFSET                    (3)
#define INT_RX_DONE1_MASK                      (0x1U<<INT_RX_DONE1_OFFSET)
#define INT_RX_DONE0_OFFSET                    (2)
#define INT_RX_DONE0_MASK                      (0x1U<<INT_RX_DONE0_OFFSET)
#define INT_TX_ALLDONE1_INT_TX_ALLDONE0_OFFSET (0)                                     /* Interrupt of EN_TX_ALLDONE pending status, write '1' to clear.  */
#define INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK   (0x3U<<INT_TX_ALLDONE1_INT_TX_ALLDONE0_OFFSET)           /* Interrupt of EN_TX_ALLDONE pending status, write '1' to clear.  */

/* CPUICR - CPU Interface Control Register */
#define PH_AUTOTAG_OFFSET                       (8)                                     /*0: don't touch cpu tag.1: Auto move CPU tag from packet to pkthdr when NIC Rx, and auto insert CPU tag from pkthdr to raw packet when NIC Tx.*/
#define PH_AUTOTAG                              (0x1U << PH_AUTOTAG_OFFSET)             /*0: don't touch cpu tag.1: Auto move CPU tag from packet to pkthdr when NIC Rx, and auto insert CPU tag from pkthdr to raw packet when NIC Tx.*/
#define TX_CMD_OFFSET                          (7)                                     /* 0: Tx Disabled, all descriptors are reset1: Tx Enabled                             */
#define TX_CMD_MASK                            (0x1U<<TX_CMD_OFFSET)                    /* 0: Tx Disabled, all descriptors are reset1: Tx Enabled                             */
#define RX_CMD_OFFSET                          (6)                                     /* 0: Rx Disabled, all descriptors are reset1: Rx Enabled                             */
#define RX_CMD_MASK                            (0x1U<<RX_CMD_OFFSET)                    /* 0: Rx Disabled, all descriptors are reset1: Rx Enabled                             */
#define BURST_SIZE_OFFSET                      (4)                                     /* Lexra Bus Burst Size.Prevents DMA occupying the Lexra bus for too long.00: 32 Bytes,  01: 64 (alignment issue must be fixed) 10: 128  */
#define BURST_SIZE_MASK                        (0x3U<<BURST_SIZE_OFFSET)                /* Lexra Bus Burst Size.Prevents DMA occupying the Lexra bus for too long.00: 32 Bytes,  01: 64 (alignment issue must be fixed) 10: 128  */
#define TXFN_OFFSET                            (3)                                     /* Tx Descriptor Fetch Notify.Set this bit to trigger the packet send                 */
#define TXFN_MASK                              (0x1U<<TXFN_OFFSET)                      /* Tx Descriptor Fetch Notify.Set this bit to trigger the packet send                 */
#define LB_MODE_OFFSET                         (2)                                     /* 0: Normal 1: Loopback mode set (CPU Tx loopback to CPU Rx)                         */
#define LB_MODE_MASK                           (0x1U<<LB_MODE_OFFSET)                   /* 0: Normal 1: Loopback mode set (CPU Tx loopback to CPU Rx)                         */
#define Mitigation_Timer1_OFFSET               (1)                                     /* Interrupt mitigation with Timer1If packet is received, Rx interrupt is asserted when Timer1 count to zero.If no packet is received, no Rx interrupt is asserted and Timer auto-reload.If this bit is asserted, the timer1 function will be disabled and only used for Rx mitigation.  */
#define Mitigation_Timer1_MASK                 (0x1U<<Mitigation_Timer1_OFFSET)         /* Interrupt mitigation with Timer1If packet is received, Rx interrupt is asserted when Timer1 count to zero.If no packet is received, no Rx interrupt is asserted and Timer auto-reload.If this bit is asserted, the timer1 function will be disabled and only used for Rx mitigation.  */
#define LENCRC_OFFSET                          (0)                                     /* Packet/mbuf length optionally with/without CRC 4 byte control setting0: Length with CRC 4 bytes1: Length without CRC 4 bytes.         */
#define LENCRC_MASK                            (0x1U<<LENCRC_OFFSET)                    /* Packet/mbuf length optionally with/without CRC 4 byte control setting0: Length with CRC 4 bytes1: Length without CRC 4 bytes.         */


/* CTIDCR - CPU Tag ID Control Register */
#define CPU_TAG_ID_OFFSET           (0)                         /* CPU Tag ID */
#define CPU_TAG_ID_MASK             (0xFFFFU<<CPU_TAG_ID_OFFSET) /* CPU Tag ID */

#endif  /*__RTL8328_ASIC_REGS_H__*/

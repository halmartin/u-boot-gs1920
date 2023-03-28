/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Implementation of the RTL8390 NIC driver for U-Boot.
 *
 * Feature : NIC driver
 *
 */


/*
 * Include Files
 */
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <soc.h>
#if defined(CONFIG_CMD_NET) && defined(CONFIG_NET_MULTI) && defined(CONFIG_RTL8390)
#include <rtk_type.h>
#include <rtk_reg.h>
#include <rtk/mac/rtl8390/rtl8390_swcore_reg.h>

/*
 * Symbol Definition
 */
#undef SWNIC_DEBUG /* Enable/Disable debug messages */

#define SWNIC_RXRING_NUM        (8)
#define SWNIC_TXRING_NUM        (2)
#define SWNIC_RXRING_SIZE       (1)
#define SWNIC_TXRING_SIZE       (1)
#define SWNIC_MBRING_SIZE       ((SWNIC_RXRING_NUM * SWNIC_RXRING_SIZE) + \
                                (SWNIC_TXRING_NUM * SWNIC_TXRING_SIZE))
#define SWNIC_PKTHDR_NUM        ((SWNIC_RXRING_NUM * SWNIC_RXRING_SIZE) + \
                                (SWNIC_TXRING_NUM * SWNIC_TXRING_SIZE))
#define SWNIC_MBUF_NUM          (SWNIC_MBRING_SIZE + (SWNIC_TXRING_NUM * SWNIC_TXRING_SIZE))
#define SWNIC_RX_CLUSTER_NUM    (2)/* multuple buffers for alternation */
#define SWNIC_CLUSTER_SIZE      (1600)

#define CPU_PORT                (52)
#define CPU_PORTMASK            (0x1 << CPU_PORT)
#define WRAP                    (0x1 << 1)
#define SWITCH_OWN              (0x1 << 0)
#define TX_CMD                  (0x1 << RTL8390_DMA_IF_CTRL_TX_EN_OFFSET)
#define RX_CMD                  (0x1 << RTL8390_DMA_IF_CTRL_RX_EN_OFFSET)
#define TX_FN                   (0x1 << RTL8390_DMA_IF_CTRL_TX_FETCH_OFFSET)

typedef struct cpuTag_s
{
    union {
        struct {
            uint8   CPUTAGIF;
            uint8       :2;
            uint8   SPN:6;
            uint16  MIR_HIT:4;
            uint16  ACL_IDX:12;
            uint16  ACL_HIT:1;
            uint16  OTAGIF:1;
            uint16  ITAGIF:1;
            uint16          :1;
            uint16  RVID:12;
            uint8   QID:3;
            uint8   ATK_TYPE:5;
            uint8   MAC_CST:1;
            uint8           :1;
            uint8   SFLOW:6;
            uint8           :2;
            uint8   DM_RXIDX:6;
            uint8   NEW_SA:1;
            uint8   STC_L2_PMV:1;
            uint8           :1;
            uint8   REASON:5;
        } __attribute__ ((aligned(1), packed)) rx;
        struct {
            uint8   CPUTAGIF;
            uint8   DPM_TYPE:1;
            uint8   ACL_ACT:1;
            uint8       :2;
            uint8   DM_PKT:1;
            uint8   DG_PKT:1;
            uint8   BP_FLTR1:1;
            uint8   BP_FLTR2:1;
            uint32      :4;
            uint32  AS_PRI:1;
            uint32  PRI:3;
            uint32  L2LEARNING:1;
            uint32  AS_TAGSTS:1;
            uint32  RVID_SEL:1;
            uint32  AS_DPM:1;
            uint32  DPM51_32:20;
            uint32  DPM31_0;
        } __attribute__ ((aligned(1), packed)) tx;
    } un;
} cpuTag_t;

typedef struct pkthdr_s
{
    uint8   *buf_addr;
    uint16  reserve;
    uint16  buf_size;
    uint16  more:1;
    uint16  rsvd:1;
    uint16  pkt_offset:14;
    uint16  buf_len;
    uint16  reserve2;
    cpuTag_t    cpuTag;
} __attribute__ ((aligned(1), packed)) pkthdr_t;

typedef struct mbuf_s
{
    struct pkthdr_s *m_pkthdr;
    struct mbuf_s *m_next;
    unsigned short m_len;
    unsigned short m_extsize;
    unsigned char *m_data;
    unsigned char *m_extbuf;
} mbuf_t;


/*
 * Data Declaration
 */
static unsigned int     RxPhRing[SWNIC_RXRING_NUM][SWNIC_RXRING_SIZE];
static unsigned int     TxPhRing[SWNIC_TXRING_NUM][SWNIC_TXRING_SIZE];
static pkthdr_t         RxPkthdr;
static pkthdr_t         TxPkthdr;
static unsigned char    RxCluster[SWNIC_CLUSTER_SIZE * SWNIC_RX_CLUSTER_NUM];
static unsigned char    TxCluster[SWNIC_CLUSTER_SIZE];
static unsigned int     *pRxRing[SWNIC_RXRING_NUM];
static unsigned int     *pTxRing[SWNIC_TXRING_NUM];
static pkthdr_t         *pRxPkthdr;
static pkthdr_t         *pTxPkthdr;
static unsigned char    *pRxCluster;
static unsigned char    *pTxCluster;


/*
 * Marco Definition
 */
#ifndef UNCACHE
  #define UNCACHE(addr)       ((unsigned int)(addr) | 0x20000000)
#endif  /* UNCACHE */
#ifndef MEMORY_BARRIER
  #define MEMORY_BARRIER()    ({ __asm__ __volatile__ ("": : :"memory"); })
#endif  /* MEMORY_BARRIER */


/*
 * Function Declaration
 */
#ifdef  SWNIC_DEBUG
/* Function Name:
 *      dump_pkt
 * Description:
 *      Dump a frame.
 * Input:
 *      pkt - pointer to the base address of the packet
 *      len - packet length
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void dump_pkt(unsigned char *pkt, int len)
{
    int i;

    for (i=0; i<len; i++)
    {
        if (i%16 == 0)
        {
            printf("[%04X] ", i);
        }

        printf("%02X ", *(pkt + i));

        if (i%16 == 15)
        {
            printf("\n");
        }
    }

    printf("\n");
} /* end of dump_pkt */
#endif  /* SWNIC_DEBUG */

/* Function Name:
 *      swnic_init
 * Description:
 *      Look for an adapter, this routine's visible to the outside.
 * Input:
 *      dev - ethernet device
 *      bis - board info
 * Output:
 *      None
 * Return:
 *      1 - one device found (Always)
 * Note:
 *      None
 */
static int swnic_init(struct eth_device *dev, bd_t *bis)
{
    int i;
    unsigned int val;

    MEM32_WRITE((SWCORE_BASE_ADDR | RTL8390_MAC_PORT_CTRL_ADDR(CPU_PORT)),0x0);
    udelay(50 * 1000); /* delay 50mS */

    /* Reset NIC only */
    val = RTL8390_RST_GLB_CTRL_SW_NIC_RST_MASK;
    MEM32_WRITE((SWCORE_BASE_ADDR | RTL8390_RST_GLB_CTRL_ADDR),val);

    do
    {
        udelay(12);
        val = REG32(SWCORE_BASE_ADDR | RTL8390_RST_GLB_CTRL_ADDR);
        val &= RTL8390_RST_GLB_CTRL_SW_NIC_RST_MASK;
    } while (val != 0);

    MEM32_WRITE((SWCORE_BASE_ADDR | RTL8390_MAC_PORT_CTRL_ADDR(CPU_PORT)),
        RTL8390_MAC_PORT_CTRL_RX_EN_MASK|RTL8390_MAC_PORT_CTRL_TX_EN_MASK);


    /* Initialize pointers */
    for (i=0; i<SWNIC_RXRING_NUM; i++)
    {
        pRxRing[i] = (unsigned int *)UNCACHE(&RxPhRing[i][0]);
        *pRxRing[i] = WRAP;
    }
    for (i=0; i<SWNIC_TXRING_NUM; i++)
    {
        pTxRing[i] = (unsigned int *)UNCACHE(&TxPhRing[i][0]);
        *pTxRing[i] = WRAP;
    }
    pRxPkthdr = (pkthdr_t *)UNCACHE(&RxPkthdr);
    pTxPkthdr = (pkthdr_t *)UNCACHE(&TxPkthdr);
    pRxCluster = (unsigned char *)UNCACHE(&RxCluster);
    pTxCluster = (unsigned char *)UNCACHE(&TxCluster);

#ifdef SWNIC_DEBUG
    /* Print messages for debug */
    printf("pRxRing[0] = %ph \n", pRxRing[0]);
    printf("pRxRing[1] = %ph \n", pRxRing[1]);
    printf("pRxRing[2] = %ph \n", pRxRing[2]);
    printf("pRxRing[3] = %ph \n", pRxRing[3]);
    printf("pRxRing[4] = %ph \n", pRxRing[4]);
    printf("pRxRing[5] = %ph \n", pRxRing[5]);
    printf("pRxRing[6] = %ph \n", pRxRing[6]);
    printf("pRxRing[7] = %ph \n", pRxRing[7]);
    printf("pTxRing[0] = %ph \n", pTxRing[0]);
    printf("pTxRing[1] = %ph \n", pTxRing[1]);
    printf("pRxPkthdr  = %ph \n", pRxPkthdr);
    printf("pTxPkthdr  = %ph \n", pTxPkthdr);
    printf("pRxCluster = %ph \n", pRxCluster);
    printf("pTxCluster = %ph \n", pTxCluster);
#endif

    /* Initialize pkthdr and mbuf descriptor for rx */
    pRxPkthdr->buf_addr = (uint8 *)((uint32)pRxCluster);
    pRxPkthdr->reserve = 0;
    pRxPkthdr->buf_size = SWNIC_CLUSTER_SIZE;
    pRxPkthdr->pkt_offset = 0;
    pRxPkthdr->more = 0;
    pRxPkthdr->buf_len = 0;
    memset(&pRxPkthdr->cpuTag, 0, sizeof(cpuTag_t));

    /* Initialize pkthdr and mbuf descriptor for tx */
    pTxPkthdr->buf_addr = (uint8 *)((uint32)pTxCluster);
    pTxPkthdr->reserve = 0;
    pTxPkthdr->buf_size = SWNIC_CLUSTER_SIZE;
    pTxPkthdr->pkt_offset = 0;
    pTxPkthdr->more = 0;
    pTxPkthdr->buf_len = SWNIC_CLUSTER_SIZE;
    memset(&pTxPkthdr->cpuTag, 0, sizeof(cpuTag_t));

    /* Initialize pkthdr rings */
    for (i=0; i<SWNIC_RXRING_NUM; i++)
        *pRxRing[i] = ((unsigned int)pRxPkthdr) | WRAP | SWITCH_OWN;
    *pTxRing[0] = ((unsigned int)pTxPkthdr) | WRAP;

    /* Add CPU port to the Lookup Miss Flooding Portmask */
    REG32(SWCORE_BASE_ADDR | RTL8390_TBL_ACCESS_L2_CTRL_ADDR) = 0x28000;
    val = REG32(SWCORE_BASE_ADDR | RTL8390_TBL_ACCESS_L2_DATA_ADDR(0));
    val |= 0x80000000;
    REG32(SWCORE_BASE_ADDR | RTL8390_TBL_ACCESS_L2_DATA_ADDR(0)) = val;
    REG32(SWCORE_BASE_ADDR | RTL8390_TBL_ACCESS_L2_CTRL_ADDR) = 0x38000;

    /* CPU port: Force link-up */
    val = REG32(SWCORE_BASE_ADDR | RTL8390_MAC_FORCE_MODE_CTRL_ADDR(CPU_PORT));
    val |= 1 << RTL8390_MAC_FORCE_MODE_CTRL_FORCE_LINK_EN_OFFSET;
    val |= 1 << RTL8390_MAC_FORCE_MODE_CTRL_MAC_FORCE_EN_OFFSET;
    REG32(SWCORE_BASE_ADDR | RTL8390_MAC_FORCE_MODE_CTRL_ADDR(CPU_PORT)) = val;

    /* Set queue number to 1 (value: 0) */
    val = REG32(SWCORE_BASE_ADDR | RTL8390_QM_PORT_QNUM_ADDR(CPU_PORT));
    val = (val & ~RTL8390_QM_PORT_QNUM_QNUM_MASK(CPU_PORT)) | (0 << RTL8390_QM_PORT_QNUM_QNUM_OFFSET(CPU_PORT));
    REG32(SWCORE_BASE_ADDR | RTL8390_QM_PORT_QNUM_ADDR(CPU_PORT)) = val;


    /* Set rx-pkthdr-ring registers */
    for (i = 0; i < 8; i++)
    {
        REG32(SWCORE_BASE_ADDR | RTL8390_DMA_IF_RX_BASE_DESC_ADDR_CTRL_ADDR(i)) = (unsigned int)pRxRing[i];
    }

    /* Set tx-pkthdr-ring registers */
    for (i = 0; i < 2; i++)
    {
        REG32(SWCORE_BASE_ADDR | RTL8390_DMA_IF_TX_BASE_DESC_ADDR_CTRL_ADDR(i)) = (unsigned int)pTxRing[i];
    }

    REG32(SWCORE_BASE_ADDR | RTL8390_DMA_IF_INTR_MSK_ADDR) = 0x7fffff;
    /* Enable the traffic and length without FCS */
    REG32(SWCORE_BASE_ADDR | RTL8390_DMA_IF_CTRL_ADDR) |= (RX_CMD | TX_CMD);


    return 1;   /* One device */
} /* end of swnic_init */


/* Function Name:
 *      swnic_halt
 * Description:
 *      Turn off ethernet interface.
 * Input:
 *      dev - ethernet device
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void swnic_halt(struct eth_device *dev)
{
    uint32 val;

    REG32(SWCORE_BASE_ADDR | RTL8390_DMA_IF_INTR_MSK_ADDR) = 0x00000000;
    REG32(SWCORE_BASE_ADDR | RTL8390_DMA_IF_INTR_STS_ADDR) = 0x007fffff;
    REG32(SWCORE_BASE_ADDR | RTL8390_DMA_IF_CTRL_ADDR) = 0x00000000;

    /* Remove the CPU port from Lookup Miss Flooding Portmask */
    REG32(SWCORE_BASE_ADDR | RTL8390_TBL_ACCESS_L2_CTRL_ADDR) = 0x28000;
    val = REG32(SWCORE_BASE_ADDR | RTL8390_TBL_ACCESS_L2_DATA_ADDR(0));
    val &= ~(0x80000000);
    REG32(SWCORE_BASE_ADDR | RTL8390_TBL_ACCESS_L2_DATA_ADDR(0)) = val;
    REG32(SWCORE_BASE_ADDR | RTL8390_TBL_ACCESS_L2_CTRL_ADDR) = 0x38000;

    /* Disable MAC TxRx */
    val = REG32(SWCORE_BASE_ADDR | RTL8390_MAC_FORCE_MODE_CTRL_ADDR(CPU_PORT));
    val &= ~(1 << RTL8390_MAC_FORCE_MODE_CTRL_FORCE_LINK_EN_OFFSET);
    REG32(SWCORE_BASE_ADDR | RTL8390_MAC_FORCE_MODE_CTRL_ADDR(CPU_PORT)) = val;
} /* end of swnic_halt */


/* Function Name:
 *      swnic_send
 * Description:
 *      Transmit a frame.
 * Input:
 *      dev    - ethernet device
 *      packet - pointer to the base address of the packet
 *      length - packet length
 * Output:
 *      None
 * Return:
 *      length - it will be zero if failed.
 * Note:
 *      None
 */
static int swnic_send(struct eth_device *dev, volatile void *packet, int length)
{
    int     len;
#ifdef  SWNIC_DEBUG
    printf("== [Tx] ===============================================\n");
    dump_pkt((unsigned char *)packet, length);
#endif

    /* ASIC expects that packet includes CRC, so we extend 4 bytes */
    if (length < 60){
        len = 64;
    }else{
        len = length + 4;
    }

    /* We can send this packet if CPU owns the descriptor */
    if (((*pTxRing[0]) & SWITCH_OWN) == 0x0)
    {
        /* Set descriptor for tx */
        pTxPkthdr->buf_addr = (uint8 *)((uint32)pTxCluster);
        pTxPkthdr->buf_size = len;
        pTxPkthdr->buf_len = len;

        /* Copy packet data to tx buffer */
        memset((uint8 *)((uint32)pTxPkthdr->buf_addr), 0, len);
        memcpy((uint8 *)((uint32)pTxPkthdr->buf_addr), (char *)packet, len);
        MEMORY_BARRIER();                         /* Make sure the data is write done */
        *pTxRing[0] = (*pTxRing[0]) | SWITCH_OWN; /* Set the descriptor to Switch-own */

        REG32(SWCORE_BASE_ADDR | RTL8390_DMA_IF_CTRL_ADDR) |= TX_FN;                   /* Tx Fetch Notify */

        /* Fix the length */
        if (length < 64)
        {
            length = 64;
        }

        return length;
    }

    return 0;                                     /* length = 0 (Failed) */
} /* end of swnic_send */


/* Function Name:
 *      swnic_recv
 * Description:
 *      Receive a frame.
 * Input:
 *      dev - ethernet device in U-Boot
 * Output:
 *      None
 * Return:
 *      0 - Done
 * Note:
 *      None
 */
static int swnic_recv(struct eth_device *dev)
{
    int i;

    /* We had received a packet if we own the descriptor */
    if (((*pRxRing[0]) & SWITCH_OWN) == 0x0) {
        unsigned char *pkt_data;
        int pkt_len;

        pkt_data = (uint8*)((uint32)pRxPkthdr->buf_addr);
        pkt_len = pRxPkthdr->buf_len;

        /* Set the another Rx buffer to recieve next packet */
        pRxPkthdr->buf_size = SWNIC_CLUSTER_SIZE;
        pRxPkthdr->buf_len = 0;
        pRxPkthdr->buf_addr += SWNIC_CLUSTER_SIZE;
        if (((uint8*)pRxPkthdr->buf_addr + SWNIC_CLUSTER_SIZE) >= (pRxCluster + (SWNIC_CLUSTER_SIZE * SWNIC_RX_CLUSTER_NUM)))
        {
            pRxPkthdr->buf_addr = (uint8 *)pRxCluster;
        }

        MEMORY_BARRIER();               /* Make sure the data is write done */

        NetReceive(pkt_data, pkt_len);

        for (i=0; i<SWNIC_RXRING_NUM; i++)
            *pRxRing[i] = ((unsigned int)pRxPkthdr) | WRAP | SWITCH_OWN;
        REG32(SWCORE_BASE_ADDR | RTL8390_DMA_IF_INTR_STS_ADDR) = 0x0000ffff;    /* Write 1 to clear ISR */

#ifdef  SWNIC_DEBUG
        printf("== [Rx] ===============================================\n");
        dump_pkt(pkt_data, pkt_len);
#endif
    }

    return 0;   /* Done and exit */
} /* end of swnic_recv */


/* Function Name:
 *      RTL8390_initialize
 * Description:
 *      Initialize RTL8390 NIC device.
 * Input:
 *      bis - board info
 * Output:
 *      None
 * Return:
 *      1 - one device found (Always)
 * Note:
 *      None
 */
int rtl8390_initialize(bd_t *bis)
{
    struct eth_device *dev;

    /* Alloc memory to ethernet device info */
    dev = (struct eth_device *)malloc(sizeof *dev);
    if (NULL == dev)
    {
        return 0;   /* No device (Out of memory) */
    }

    /* Set the ethernet device info */
    sprintf(dev->name, "rtl8390#0");
    dev->priv = NULL;
    dev->iobase = 0;
    dev->init = swnic_init;
    dev->halt = swnic_halt;
    dev->send = swnic_send;
    dev->recv = swnic_recv;
    dev->write_hwaddr = NULL;

    /* Register the ethernet device */
    eth_register(dev);

    return 1;    /* Only one device */
} /* end of rtl8390_initialize */

int swnic_rx_pkt(struct eth_device *dev,unsigned char **pkt, int max_len,int *port)
{
    int i;
    cpuTag_t    *pCputag;
#if  0
    printf("swnic_recv\n");
#endif
    if (pRxRing[0]==0)
    	return 0;

    /* We had received a packet if we own the descriptor */
    if (((*pRxRing[0]) & SWITCH_OWN) == 0x0) {
        unsigned char *pkt_data;
        int pkt_len;

        pkt_data = (uint8*)((uint32)pRxPkthdr->buf_addr | 0xa0000000);
        pkt_len = pRxPkthdr->buf_len;

        /* Set the another Rx buffer to recieve next packet */
        pRxPkthdr->buf_size = SWNIC_CLUSTER_SIZE;
        pRxPkthdr->buf_len = 0;
        pRxPkthdr->buf_addr += SWNIC_CLUSTER_SIZE;
        if (((uint8*)((uint32)pRxPkthdr->buf_addr | 0xa0000000) + SWNIC_CLUSTER_SIZE) >= (pRxCluster + (SWNIC_CLUSTER_SIZE * SWNIC_RX_CLUSTER_NUM)))
        {
            pRxPkthdr->buf_addr = (uint8 *)((uint32)pRxCluster & 0x0fffffff);
        }
        pCputag = &pRxPkthdr->cpuTag;

        MEMORY_BARRIER();               /* Make sure the data is write done */
 	*port=0xFF;
        if (pCputag->un.rx.CPUTAGIF)
        {
        	*port = pCputag->un.rx.SPN;
        }
        for (i=0; i<SWNIC_RXRING_NUM; i++)
            *pRxRing[i] = ((unsigned int)pRxPkthdr & 0x0fffffff) | WRAP | SWITCH_OWN;
        REG32(SWCORE_BASE_ADDR | RTL8390_DMA_IF_INTR_STS_ADDR) = 0x0000ffff;    /* Write 1 to clear ISR */

#ifdef  SWNIC_DEBUG
        printf("== [Rx] ===============================================\n");
        dump_pkt(pkt_data, pkt_len);
#endif

        if(pkt_len > max_len)
        {
            printf("Pkt buffer is not enough to rx pkt! \n");
            return 0;
        }
        else
        {
            //*pkt = pkt_data;
            memcpy(*pkt,pkt_data,pkt_len);
            return pkt_len;
        }
    }

    return 0;
} /* end of swnic_recv */

#endif  /* defined(CONFIG_CMD_NET) && defined(CONFIG_NET_MULTI) && defined(CONFIG_rtl8390) */


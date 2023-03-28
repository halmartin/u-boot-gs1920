#ifndef __SWCTRL_LED_H__
#define __SWCTRL_LED_H__

#define NULL                        0

typedef enum rt_error_common_e
{
    RT_ERR_FAILED = -1,                             /* General Error                                                                    */

    /* 0x0000xxxx for common error code */
    RT_ERR_OK = 0,                                  /* 0x00000000, OK                                                                   */
    RT_ERR_INPUT,                                   /* 0x00000001, invalid input parameter                                              */
    RT_ERR_UNIT_ID,                                 /* 0x00000002, invalid unit id                                                      */
    RT_ERR_PORT_ID,                                 /* 0x00000003, invalid port id                                                      */
    RT_ERR_PORT_MASK,                               /* 0x00000004, invalid port mask                                                    */
    RT_ERR_PORT_LINKDOWN,                           /* 0x00000005, link down port status                                                */
    RT_ERR_ENTRY_INDEX,                             /* 0x00000006, invalid entry index                                                  */
    RT_ERR_NULL_POINTER,                            /* 0x00000007, input parameter is null pointer                                      */
    RT_ERR_QUEUE_ID,                                /* 0x00000008, invalid queue id                                                     */
    RT_ERR_QUEUE_NUM,                               /* 0x00000009, invalid queue number                                                 */
    RT_ERR_BUSYWAIT_TIMEOUT,                        /* 0x0000000a, busy watting time out                                                */
    RT_ERR_MAC,                                     /* 0x0000000b, invalid mac address                                                  */
    RT_ERR_OUT_OF_RANGE,                            /* 0x0000000c, input parameter out of range                                         */
    RT_ERR_CHIP_NOT_SUPPORTED,                      /* 0x0000000d, functions not supported by this chip model                           */
    RT_ERR_SMI,                                     /* 0x0000000e, SMI error                                                            */
    RT_ERR_NOT_INIT,                                /* 0x0000000f, The module is not initial                                            */
    RT_ERR_CHIP_NOT_FOUND,                          /* 0x00000010, The chip can not found                                               */
    RT_ERR_NOT_ALLOWED,                             /* 0x00000011, actions not allowed by the function                                  */
    RT_ERR_DRIVER_NOT_FOUND,                        /* 0x00000012, The driver can not found                                             */
    RT_ERR_SEM_LOCK_FAILED,                         /* 0x00000013, Failed to lock semaphore                                             */
    RT_ERR_SEM_UNLOCK_FAILED,                       /* 0x00000014, Failed to unlock semaphore                                           */
    RT_ERR_THREAD_EXIST,                            /* 0x00000015, Thread exist                                                         */
    RT_ERR_THREAD_CREATE_FAILED,                    /* 0x00000016, Thread create fail                                                   */
    RT_ERR_FWD_ACTION,                              /* 0x00000017, Invalid forwarding Action                                            */
    RT_ERR_IPV4_ADDRESS,                            /* 0x00000018, Invalid IPv4 address                                                 */
    RT_ERR_IPV6_ADDRESS,                            /* 0x00000019, Invalid IPv6 address                                                 */
    RT_ERR_PRIORITY,                                /* 0x0000001a, Invalid Priority value                                               */
    RT_ERR_FID,                                     /* 0x0000001b, invalid fid                                                          */
    RT_ERR_ENTRY_NOTFOUND,                          /* 0x0000001c, specified entry not found                                            */
    RT_ERR_DROP_PRECEDENCE,                         /* 0x0000001d, invalid drop precedence                                              */
    RT_ERR_NOT_FINISH,                              /* 0x0000001e, Action not finish, still need to wait                                */
    RT_ERR_TIMEOUT,                                 /* 0x0000001f, Time out                                                             */
    RT_ERR_REG_ARRAY_INDEX_1,                       /* 0x00000020, invalid index 1 of register array                                    */
    RT_ERR_REG_ARRAY_INDEX_2,                       /* 0x00000021, invalid index 2 of register array                                    */
    RT_ERR_ETHER_TYPE,                              /* 0x00000022, invalid ether type                                                   */
    RT_ERR_MBUF_PKT_NOT_AVAILABLE,                  /* 0x00000023, mbuf->packet is not available                                        */
    RT_ERR_QOS_INVLD_RSN,                           /* 0x00000024, invalid pkt to CPU reason                                                */

    RT_ERR_COMMON_END = 0xFFFF                      /* The symbol is the latest symbol of common error                                  */
} rt_error_common_t;


/*for LED*/
#define MAX_PHY_PORT 	           (56)    /* number of physical ports */
#define MAX_PORT_ID                (MAX_PHY_PORT)
#define MIN_PORT_ID                (0)

#define REG_WIDTH 			        32

#define SWLED_REFLASH_FREQ 			CONFIG_SDK_SOFTWARE_CONTROL_LED_RATE      /* Reflash time is (10mS * SWLED_REFLASH_FREQ)*/

#define PORT_COLLISION_GET(port) (_port_collision[port])
#define PORT_COLLISION_YES(port) (_port_collision[port] = 1)
#define PORT_COLLISION_NO(port) (_port_collision[port] = 0)

#define PORT_BLOCK_GET(port) (_led_rlpp_block[port])
#define PORT_BLOCK_YES(port) (_led_rlpp_block[port] = 1)
#define PORT_BLOCK_NO(port) (_led_rlpp_block[port] = 0)
#define PORT_ACT_GET(port) (_led_act[port])
#define PORT_ACT_YES(port) (_led_act[port] = 1)
#define PORT_ACT_NO(port) (_led_act[port] = 0)

#define PORT_LED_GET(port, ledId) (_led_state[port][ledId])
#define PORT_LED_ONE(port, ledId) (_led_state[port][ledId] = 1)
#define PORT_LED_ZERO(port, ledId) (_led_state[port][ledId] = 0)
#define PORT_LED_LIGHT_ON(port, ledId) PORT_LED_ZERO(port, ledId)
#define PORT_LED_LIGHT_OFF(port, ledId) PORT_LED_ONE(port, ledId)

typedef enum {
    LED_MODE_LINK_ACT,
	LED_MODE_LINK,
	LED_MODE_ACT,
	LED_MODE_NOT_USED,
	LED_MODE_100M_10M_LINK,
	LED_MODE_COL_FULL_DUPLEX,
	LED_MODE_FULL_DUPLEX,
	LED_MODE_1000M_LINK,
	LED_MODE_100M_LINK,
	LED_MODE_10M_LINK,
	LED_MODE_1000M_LINK_ACT,
	LED_MODE_100M_LINK_ACT,
	LED_MODE_10M_LINK_ACT,
	LED_MODE_1000M_100M_LINK_ACT,
	LED_MODE_1000M_10M_LINK_ACT,
	LED_MODE_100M_10M_LINK_ACT,
	LED_MODE_NOT_USED2
} sal_led_mode;


enum RTL8389_MIB_PORT_COUNTER{
    ifOutOctets  = 0,
    ifInOctets,
    dot3StatsFCSErrors,
    etherStatsCollisions, /* 15 */
    etherStatsUndersizePkts,
    MAX_MIB_PORT_COUNTER,
};

typedef struct rtk_MIBCounterInfo_s
{
    unsigned int regAddr;
    unsigned int regMsk;
    unsigned int regOffset;
    unsigned char bitNum;   /*The bit number this counter used*/
} rtk_MIBCounterInfo_t;


typedef enum swCtrl_led_controller_e
{
    SWCTRL_LED_R8390,
    SWCTRL_LED_CHIP_END            
} swCtrl_led_controller_t;

int led_state_refresh(void);
int led_refresh(void);
int swCtrl_led_init(void);
int swCtrl_led_refresh_handler(void);
int swCtrl_led_error_handler(unsigned char index);
int swCtrl_led_getAsicReg(unsigned int reg, unsigned int *val);
int swCtrl_led_setAsicRegBits(unsigned int reg, unsigned int bits, unsigned int value);
int swCtrl_led_getPortSpeedDuplex(unsigned char chipID, int port, unsigned int *pSpeed, unsigned int *pDuplex);
int swCtrl_led_getPortLink(unsigned char chipID, int port, unsigned int *linkSts);
int swCtrl_led_getMIBPortCounter(unsigned char chipID, int port, unsigned int mibIdx, unsigned long long* counter);
int swCtrl_led_intr_handler(void *isr_param);

int rtl8390_getAsicMIBPortCounter(int port, unsigned int mibIdx, unsigned long long * counter);
int rtl8390_getAsicPortSpeedDuplex(int port, unsigned int *pSpeed, unsigned int *pDuplex);
int rtl8390_getAsicPortLinkStat(int port, unsigned int *pLinkStat);

typedef struct swCtrl_led_mapper_operation_s
{
    int   (*getPortSpeedDuplex)(int, unsigned int *, unsigned int *);	
    int   (*getPortLink)(int, unsigned int *);
    int   (*getMIBPortCounter)(int, unsigned int, unsigned long long *);

} swCtrl_led_mapper_operation_t;

#define PORT_DUPLEX_FULL           (1)
#define PORT_DUPLEX_HALF           (0)    
#define PORT_SPEED_10			   (0)	
#define PORT_SPEED_100             (1)
#define PORT_SPEED_1000            (2)

#define PORT_LINK_DOWN             (0)

#endif


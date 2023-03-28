
/*
 * Copyright (C) 2009 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * $Revision: $
 * $Date: $
 *
 * Purpose : Software Control LED
 *
 * Feature : For RTL8231 Serial Mode
 *
 * Variable:
 *   gLed0Mode : Decide the LED0's meaning
 *   gLed1ode : Decide the LED1's meaning
 *   gLed2ode : Decide the LED2's meaning
 *   led01dual : 1 = LED0&1 is dual color LED
 *   led12dual : 1 = LED1&2 is dual color LED
 *   gLedSetNum : How many LEDs are controlled by RTL8231 serial mode,
 *                this value should be right, then the LED behavior will XXX
 */

#include <config.h>
#include <rtk_reg.h>
#include <rtk_osal.h>
#include <rtk/drv/swled/swctrl_led_main.h>

/*
 * Symbol Definition
 */
/* LED definition for customization */
//#define LED_NUM_PER_PORT            (2)
//#define LED_PORT_NUM_IN_SYSTEM      (52)
#define LED_NUM_PER_PORT            (3)
#define LED_PORT_NUM_IN_SYSTEM      (56)
#define LED_GPIO_DIRECTION_REG      (0xB8003508)
#define LED_GPIO_DATA_REG           (0xB800350C)
//#define LED_GPIO_CLK_PIN_OFFSET     (11) /* Use the GPIO C3 as CLOCK pin */
//#define LED_GPIO_DATA_PIN_OFFSET    (10) /* Use the GPIO C2 as DATA pin */
#define LED_GPIO_CLK_PIN_OFFSET     (25) /* Use the GPIO A1 as CLOCK pin */
#define LED_GPIO_DATA_PIN_OFFSET    (26) /* Use the GPIO A2 as DATA pin */
#define IS_LED_0_1_BICOLOR          (0)  /* 1: bi-color */
#define IS_LED_1_2_BICOLOR          (0)  /* 1: bi-color */
#define LED_0_MODE                  LED_MODE_100M_10M_LINK_ACT
#define LED_1_MODE                  LED_MODE_1000M_LINK_ACT
#define LED_2_MODE                  LED_MODE_NOT_USED
//#define LED_0_MODE                  LED_MODE_LINK_ACT
//#define LED_1_MODE                  LED_MODE_1000M_LINK
//#define LED_2_MODE                  LED_MODE_100M_10M_LINK

/* LED definition for code usage */
#define LED_SET_NUM             gLedSetNum
#define LED_MODE(ledId)         (*gpLedMode[ledId])
#define SWCTRL_LED_CTRL(chip)   swCtrl_led_ops[chip]

typedef struct rtk_switch_model_s {
     struct {
        unsigned char  offset;
        unsigned char  count;
        unsigned char  num;
        unsigned short sel_p0_p23_led_mod;
        unsigned short sel_p24_p27_led_mod;		
 		unsigned short sel_led_group_3_0;
    } led; 
}rtk_swCtrl_led_model_t;


/*
 * Data Declaration
 */
extern swCtrl_led_mapper_operation_t swCtrl_led_ops[];

unsigned int gLedSetNum;
unsigned int gLed0Mode;
unsigned int gLed1Mode;
unsigned int gLed2Mode;
unsigned int gPortLedNum;
unsigned int *gpLedMode[] = {&gLed0Mode, &gLed1Mode, &gLed2Mode};

unsigned char  _port_collision[MAX_PHY_PORT];
unsigned long long _port_collision_num[MAX_PHY_PORT];

unsigned char  _led_act_blink = 0;
unsigned short _led_block_blink_timer;
unsigned char  _port_link[MAX_PHY_PORT];
unsigned char  _led_rlpp_block[MAX_PHY_PORT];
unsigned char  _led_act[MAX_PHY_PORT];
unsigned char  _led_state[MAX_PHY_PORT][LED_NUM_PER_PORT];
unsigned long long _port_ifin[MAX_PHY_PORT];
unsigned long long _port_inUnderSize[MAX_PHY_PORT];
unsigned long long _port_inBadCRC[MAX_PHY_PORT];
unsigned long long _port_ifout[MAX_PHY_PORT];

unsigned char _blocked_port_num_change;
unsigned char _blocked_port_num;

unsigned char led01dual, led12dual; /* decide which LED is dual color*/
unsigned int ledSignal[LED_NUM_PER_PORT]; /* decide the output value for each LED, this is low active.*/
unsigned char swCtrl_chipID;

const rtk_swCtrl_led_model_t *pBorad_Model;
const rtk_swCtrl_led_model_t *gSwCtrlLedModel = NULL;

static const rtk_swCtrl_led_model_t rtk_device_led_info = { 
	.led.offset = 0,
	.led.count = LED_PORT_NUM_IN_SYSTEM,
	.led.num = LED_NUM_PER_PORT,
	.led.sel_p0_p23_led_mod = 0x5470,
	.led.sel_p24_p27_led_mod = 0x5470,
	.led.sel_led_group_3_0 = 0x0021,
};

/* Function Name:
 *      board_led_info
 * Description:
 *      Get LED configuration code for device
 * Input:
 *      None
 * Output:
 *      Return LED Const Structure
 * Return:
 *      None
 * Note:
 *      None
 */
const rtk_swCtrl_led_model_t * board_led_info(void)
{ 
	gSwCtrlLedModel = &rtk_device_led_info;

    return (const rtk_swCtrl_led_model_t *)gSwCtrlLedModel;
} /* end of board_led_info */

/* Function Name: 
 *      swCtrl_led_allOff
 * Description: 
 *      Turn Off all LEDs
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK - initialize success
 * Note: 
 *      
 */
int swCtrl_led_allOff(void)
{	
    unsigned char port, ledId;
    unsigned int regData;
	
	for (port = 0; port < LED_SET_NUM; port ++)
	{			
		for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
		{	PORT_LED_ONE(port, ledId);
			ledSignal[ledId] = PORT_LED_GET(port, ledId);
		}
			
		if(1 == led01dual && 1 == ledSignal[0] && 1 == ledSignal[1])
		{
			ledSignal[0] = ledSignal[1] = 0;
		}
		if(1 == led12dual && 1 == ledSignal[1] && 1 == ledSignal[2])
		{
			ledSignal[1] = ledSignal[2] = 0;
		}
		for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
		{
			regData = MEM32_READ(LED_GPIO_DATA_REG);
			regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
			regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
        	MEM32_WRITE(LED_GPIO_DATA_REG, regData);
			regData |= (1<<LED_GPIO_CLK_PIN_OFFSET);
			regData |= (1<<LED_GPIO_DATA_PIN_OFFSET);
        	MEM32_WRITE(LED_GPIO_DATA_REG, regData);
			regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
			regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
        	MEM32_WRITE(LED_GPIO_DATA_REG, regData);
		}
	}
		
	return RT_ERR_OK;
    
} /* end of swCtrl_led_allOff()*/

/* Function Name: 
 *      swCtrl_led_init
 * Description: 
 *      [1] Check the Chip ID is supported or NOT?
 *      [2] Configure the LED mode.
 *      [3] Get LED Configuration from U-Boot
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_CHIP_NOT_SUPPORTED - chip desn't support this feature
 * Note: 
 *      
 */ 
int swCtrl_led_init(void)
{
	unsigned int regData; 
	int ret;	

	swCtrl_chipID = SWCTRL_LED_R8390;

	/* Port LED setting */
	gLedSetNum = LED_PORT_NUM_IN_SYSTEM;
	gLed0Mode = LED_0_MODE;	
	gLed1Mode = LED_1_MODE;
	gLed2Mode = LED_2_MODE;
	/* If led01dual = 1, LED 0&1 are a Dual Color LED
	 * If led01dual = 0, LED 0&1 are NOT a Dual Color LED
	 */
	led01dual = IS_LED_0_1_BICOLOR; 
	/* If led12dual = 1, LED 1&2 are a Dual Color LED
	 * If led12dual = 0, LED 1&2 are NOT a Dual Color LED
	 */
	led12dual = IS_LED_1_2_BICOLOR;
	/* How many LED for a port*/
	gPortLedNum = LED_NUM_PER_PORT;

	OSAL_MDELAY(2500); /* delay 0.25 sec */

    /* Configure the 8390 use GPIO A1/A2 to output pin */
	regData = MEM32_READ(LED_GPIO_DIRECTION_REG); 
	regData |= (1<<LED_GPIO_CLK_PIN_OFFSET); /* GPIO C3 (CLK) set to output direction */
	regData |= (1<<LED_GPIO_DATA_PIN_OFFSET); /* GPIO C4 (DATA) set to output direction */
	MEM32_WRITE(LED_GPIO_DIRECTION_REG, regData); 

	ret = swCtrl_led_allOff();
	
    return RT_ERR_OK;
} /* end of swCtrl_led_init */ 

/* Function Name: 
 *      swCtrl_led_refresh_handler
 * Description: 
 *      Software Control LED function's entry point.
 *	  This API is called by Timer 1 interrupt call back function.
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK     - function is successed
 *      RT_ERR_FAILED - function is failed
 * Note: 
 *      
 */
int swCtrl_led_refresh_handler(void)
{
	int ret;

	ret = led_refresh();

    return ret;
} /* end of swCtrl_led_refresh_handler */ 

/* Function Name: 
 *      swCtrl_led_error_handler
 * Description: 
 *	  This API is called by Timer 1 interrupt call back function.
 *      This API is used to check interrupt bit status.
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK         - function is successed
 * Note: 
 *      
 */
int swCtrl_led_error_handler(unsigned char index)
{
	printf("\nswCtrl_led_error_handler() %u\n", index);
    return RT_ERR_OK;
} /* end of swCtrl_led_error_handler */ 

/* Function Name: 
 *      led_state_refresh
 * Description: 
 *	  This API will be called during Timer 1 interrupt.
 *      This API will updated all ports' status.
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK         - function is successed
 * Note: 
 *      
 */
int led_state_refresh(void)
{
    unsigned long long port_ifout_current = 0, port_ifin_current = 0, port_collision_current = 0;
    unsigned long long port_inUnderSize_current = 0, port_inBadCRC_current = 0;
    unsigned int link, speed, duplex; 
    unsigned char port, ledId;
    
    for (port = 0; port < LED_SET_NUM; port ++)
    {
		swCtrl_led_getPortLink(swCtrl_chipID, port, &link);
			
        if(_port_link[port] != link)
        {
            _port_link[port] = link;
            if(!link) /*link down*/
            {
				swCtrl_led_getMIBPortCounter(swCtrl_chipID, (int)port, (unsigned int)ifOutOctets, &port_ifout_current);
                swCtrl_led_getMIBPortCounter(swCtrl_chipID, (int)port, (unsigned int)ifInOctets, &port_ifin_current);
                swCtrl_led_getMIBPortCounter(swCtrl_chipID, (int)port, (unsigned int)dot3StatsFCSErrors, &port_inBadCRC_current);
                swCtrl_led_getMIBPortCounter(swCtrl_chipID, (int)port, (unsigned int)etherStatsUndersizePkts, &port_inUnderSize_current);
                swCtrl_led_getMIBPortCounter(swCtrl_chipID, (int)port, (unsigned int)etherStatsCollisions, &port_collision_current);				
                _port_ifout[port] = port_ifout_current;
                _port_ifin[port] = port_ifin_current;
                _port_inUnderSize[port] = port_inUnderSize_current;
                _port_inBadCRC[port] = port_inBadCRC_current;
                _port_collision_num[port] = port_collision_current;

                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    if(LED_MODE(ledId) != LED_MODE_NOT_USED)
                        PORT_LED_LIGHT_OFF(port, ledId);
                    else
                        PORT_LED_LIGHT_ON(port, ledId);
                }
                
                PORT_ACT_NO(port);
                PORT_COLLISION_NO(port);
            }
        }
        if(link)
        {     	
			swCtrl_led_getPortSpeedDuplex(swCtrl_chipID, port, &speed, &duplex);

            if(PORT_DUPLEX_FULL== duplex)
            {
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_COL_FULL_DUPLEX:
                        case LED_MODE_FULL_DUPLEX:
                            PORT_LED_LIGHT_ON(port, ledId);
                            break;
                        default:
                            break;
                    }
                }
            }
            else
            {
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_COL_FULL_DUPLEX:
                        case LED_MODE_FULL_DUPLEX:
                            PORT_LED_LIGHT_OFF(port, ledId);
                            break;
                        default:
                            break;
                    }
                }
            }
            
            if(PORT_SPEED_10 == speed)
            {
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_LINK:
                        case LED_MODE_LINK_ACT:
                        case LED_MODE_100M_10M_LINK_ACT:
                        case LED_MODE_NOT_USED:
						case LED_MODE_100M_10M_LINK:							
                            PORT_LED_LIGHT_ON(port, ledId);
                            break;
                        case LED_MODE_100M_LINK:
                        case LED_MODE_1000M_LINK:
                        case LED_MODE_100M_LINK_ACT:
                        case LED_MODE_1000M_LINK_ACT:
                            PORT_LED_LIGHT_OFF(port, ledId);
                            break;
                        default:
                            break;
                    }
                }
            }
            else if(PORT_SPEED_100 == speed)
            {
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_LINK:
                        case LED_MODE_LINK_ACT:
                        case LED_MODE_100M_10M_LINK_ACT:
                        case LED_MODE_100M_LINK:
                        case LED_MODE_100M_LINK_ACT:
                        case LED_MODE_NOT_USED:
						case LED_MODE_100M_10M_LINK:	
                            PORT_LED_LIGHT_ON(port, ledId);
                            break;
                        case LED_MODE_1000M_LINK:
                        case LED_MODE_1000M_LINK_ACT:
                            PORT_LED_LIGHT_OFF(port, ledId);
                            break;
                        default:
                            break;
                    }
                }
            }
            else 
            {
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_LINK:
                        case LED_MODE_LINK_ACT:
                        case LED_MODE_1000M_LINK:
                        case LED_MODE_1000M_LINK_ACT:
                        case LED_MODE_NOT_USED:                            
                            PORT_LED_LIGHT_ON(port, ledId);
                            break;
                        case LED_MODE_100M_10M_LINK_ACT:
                        case LED_MODE_100M_LINK:
                        case LED_MODE_100M_LINK_ACT:
						case LED_MODE_100M_10M_LINK:							
                            PORT_LED_LIGHT_OFF(port, ledId);
                            break;
                        default:
                            break;
                    }
                }
            }

            swCtrl_led_getMIBPortCounter(swCtrl_chipID, port, (unsigned int)ifOutOctets, &port_ifout_current);
            swCtrl_led_getMIBPortCounter(swCtrl_chipID, port, (unsigned int)ifInOctets, &port_ifin_current);
            swCtrl_led_getMIBPortCounter(swCtrl_chipID, port, (unsigned int)dot3StatsFCSErrors, &port_inBadCRC_current);
            swCtrl_led_getMIBPortCounter(swCtrl_chipID, port, (unsigned int)etherStatsUndersizePkts, &port_inUnderSize_current);      
            if (_port_ifout[port] != port_ifout_current || _port_ifin[port] != port_ifin_current 
                || _port_inUnderSize[port] != port_inUnderSize_current || _port_inBadCRC[port] != port_inBadCRC_current)
            {
                PORT_ACT_YES(port);
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_ACT:
                            PORT_LED_LIGHT_ON(port, ledId);
                            break;
                        default:
                            break;
                    }
                }                
            }
            else
            {
                PORT_ACT_NO(port);

                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_ACT:
                            PORT_LED_LIGHT_OFF(port, ledId);
                            break;
                        default:
                            break;
                    }
                }
            }

            if(duplex == PORT_DUPLEX_HALF)
            {
                swCtrl_led_getMIBPortCounter(swCtrl_chipID, port, (unsigned int)etherStatsCollisions, &port_collision_current);
	
                if (_port_collision_num[port] != port_collision_current)
                {
                    PORT_COLLISION_YES(port);
                }
                else
                    PORT_COLLISION_NO(port);

                _port_collision_num[port] = port_collision_current;
            }
            else
                PORT_COLLISION_NO(port);

            _port_ifout[port] = port_ifout_current;
            _port_ifin[port] = port_ifin_current;
            _port_inUnderSize[port] = port_inUnderSize_current;
            _port_inBadCRC[port] = port_inBadCRC_current;
        }
    }
    return RT_ERR_OK;
} /* end of led_state_refresh */

/* Function Name: 
 *      led_refresh
 * Description: 
 *	  This API will be called during Timer 1 interrupt.
 *      This API is used to output the LED control signals.
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK         - function is successed
 * Note: 
 *      
 */
int led_refresh(void)
{
    unsigned char port, ledId;
    unsigned int regData;
   
    _led_act_blink = ~_led_act_blink;

    if (_led_act_blink)
        led_state_refresh();
	
    for (port = 0; port < LED_SET_NUM; port ++)
    {
		if(PORT_ACT_GET(port) && _led_act_blink) /*Light off active LED*/
        {
            regData = MEM32_READ(LED_GPIO_DATA_REG);

            if(PORT_COLLISION_GET(port)) /*blink if collision*/
            {
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_COL_FULL_DUPLEX:
                            ledSignal[ledId] = 1;
                            break;
                        default:
                            break;
                    }
                }
            }
            
            for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
            {
                switch(LED_MODE(ledId))
                {
                    case LED_MODE_ACT:
                    case LED_MODE_LINK_ACT:
                    case LED_MODE_100M_LINK_ACT:
                    case LED_MODE_1000M_LINK_ACT:
                    case LED_MODE_100M_10M_LINK_ACT:
                        ledSignal[ledId] = 1;
                        break;
                    default:
                        ledSignal[ledId] = PORT_LED_GET(port, ledId);
                        break;
                }
            }

            if(1 == led01dual && 1 == ledSignal[0] && 1 == ledSignal[1])
            {
                ledSignal[0] = ledSignal[1] = 0;
            }
            if(1 == led12dual && 1 == ledSignal[1] && 1 == ledSignal[2])
            {
                ledSignal[1] = ledSignal[2] = 0;
            }

            for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
            {
    			regData = MEM32_READ(LED_GPIO_DATA_REG);
    			regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
    			regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
            	MEM32_WRITE(LED_GPIO_DATA_REG, regData);
    			regData |= (1<<LED_GPIO_CLK_PIN_OFFSET);
    			regData |= (ledSignal[ledId]<<LED_GPIO_DATA_PIN_OFFSET);
            	MEM32_WRITE(LED_GPIO_DATA_REG, regData);
    			regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
    			regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
            	MEM32_WRITE(LED_GPIO_DATA_REG, regData);
            }
        }
        else /*Light on*/
        {
            for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                ledSignal[ledId] = PORT_LED_GET(port, ledId);
          
            if(1 == led01dual && 1 == ledSignal[0] && 1 == ledSignal[1])
            {
                ledSignal[0] = ledSignal[1] = 0;
            }
            if(1 == led12dual && 1 == ledSignal[1] && 1 == ledSignal[2])
            {
                ledSignal[1] = ledSignal[2] = 0;
            }

            for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
            {
    			regData = MEM32_READ(LED_GPIO_DATA_REG);
    			regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
    			regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
            	MEM32_WRITE(LED_GPIO_DATA_REG, regData);
    			regData |= (1<<LED_GPIO_CLK_PIN_OFFSET);
    			regData |= (ledSignal[ledId]<<LED_GPIO_DATA_PIN_OFFSET);
            	MEM32_WRITE(LED_GPIO_DATA_REG, regData);
    			regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
    			regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
            	MEM32_WRITE(LED_GPIO_DATA_REG, regData);
            }            
        }
    }

	regData = MEM32_READ(LED_GPIO_DATA_REG);
	regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
	regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
	MEM32_WRITE(LED_GPIO_DATA_REG, regData);

    return RT_ERR_OK;
} /* end of led_refresh */

/* Function Name: 
 *		swCtrl_led_getPortSpeedDuplex
 * Description: 
 *		Get the specified port's speed and duplex status.
 * Input:  
 *		chipID	- the convert Chip ID.
 *		port		- the specified port
 * Output: 
 *		
 *		  pSpeed	   - the link status of specified poty. 
 *                              10M = 0; 100M = 1; 1000M =2 
 *		  pDuplex	   - the link status of specified poty.
 *					FULL = 1; HALF = 0
 * Return: 
 *		RT_ERR_OK		  - function is successed
 *		RT_ERR_FAILED	  - function is failed
 * Note: 
 *		  
 */
int swCtrl_led_getPortSpeedDuplex(unsigned char chipID, int port, unsigned int *pSpeed, unsigned int *pDuplex)
{
	
	if(RT_ERR_OK != swCtrl_led_ops[chipID].getPortSpeedDuplex(port, pSpeed, pDuplex))
		return RT_ERR_FAILED;

    return RT_ERR_OK;
} /* end of swCtrl_led_getPortSpeedDuplex */

/* Function Name: 
 *		swCtrl_led_getPortLink
 * Description: 
 *		Get the specified port's link status.
 * Input:  
 *		chipID	- the convert Chip ID.
 *		port		- the specified port
 * Output: 
 *		
 *	      linkSts 	   - the link status of specified poty.
 * Return: 
 *		RT_ERR_OK		  - function is successed
 *		RT_ERR_FAILED	  - function is failed
 * Note: 
 *		  
 */
int swCtrl_led_getPortLink(unsigned char chipID, int port, unsigned int *linkSts)
{	

	if(RT_ERR_OK != swCtrl_led_ops[chipID].getPortLink(port, linkSts))
		return RT_ERR_FAILED;

    return RT_ERR_OK;
} /* end of  swCtrl_led_getPortLink */ 

/* Function Name: 
 *      swCtrl_led_getAsicReg
 * Description: 
 *      Set bits value of a specified register.
 * Input:  
 *      reg 		- Register's address.
 * Output: 
 *      
 *	  value	       - Register's value.
 * Return: 
 *      RT_ERR_OK         - function is successed
 * Note: 
 *        
 */
int swCtrl_led_getAsicReg(unsigned int reg, unsigned int *val)
{
    *val = MEM32_READ(reg);
    return RT_ERR_OK;
} /* end of swCtrl_led_getAsicReg */

/* Function Name: 
 *      swCtrl_ledsetAsicRegBits
 * Description: 
 *      Set bits value of a specified register.
 * Input:  
 *      reg 		- Register's address.
 *	  bits		- Bits mask for setting.
 *  	  value		- Bits value for setting. Value of bits will be set with mapping mask bit is 1.
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK         - function is successed
 * Note: 
 *      Set bits of a specified register to value. Both bits and value are be treated as bit-mask.
 *        
 */
int swCtrl_led_setAsicRegBits(unsigned int reg, unsigned int bits, unsigned int value)
{
    unsigned int regData;

    regData = MEM32_READ(reg);

    regData = regData & (~bits);
    regData = regData | (value & bits);

    MEM32_WRITE(reg, regData);
    return RT_ERR_OK;
} /* end of swCtrl_led_setAsicRegBits */

/* Function Name: 
 *      swCtrl_led_getMIBPortCounter
 * Description: 
 *      Set bits value of a specified register.
 * Input:  
 *      chipID  - the convert Chip ID.
 *	    port    - the specified port.
 *	    mibIdx  - the specified MIB counter index.
 * Output: 
 *      counter - MIB counter value. 
 * Return: 
 *      RT_ERR_OK - function is successed
 * Note: 
 *        
 */
int swCtrl_led_getMIBPortCounter(unsigned char chipID, int port, unsigned int mibIdx, unsigned long long* counter)
{
    if ((RT_ERR_OK !=  swCtrl_led_ops[chipID].getMIBPortCounter(port, mibIdx, counter)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
} /* end of swCtrl_led_getMIBPortCounter */

#if defined(CONFIG_SOFTWARE_CONTROL_LED)
int swCtrl_led_intr_handler(void *isr_param)
{
	int ret;
	
	ret = swCtrl_led_refresh_handler();

    /* Clear TC0 ISR */
    *((volatile int*)(0xb8003114)) |= (1 << 29);

	/* Checking and Double checking the Interrupt Flags are cleared*/
    if (REG32(0xB8003114) & (1 << 29))
        ret = swCtrl_led_error_handler(3);
    if (REG32(0xB8003004) & (1 << 29))
        ret = swCtrl_led_error_handler(4);

    return RT_ERR_OK;
} /* end of swCtrl_led_intr_handler */ 
#endif

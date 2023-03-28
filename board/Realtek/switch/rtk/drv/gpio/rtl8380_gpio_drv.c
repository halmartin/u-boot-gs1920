
/*
 * Include Files
 */
#include <rtk_debug.h>
#include <rtk/drv/gpio/rtl8390_gpio_drv.h>
#include <rtk_reg.h>
#include <rtk/mac/rtl8380/rtl8380_mdc_mdio.h>

static uint32 PABCDDAT_shadow = 0; /* Shadow of GPIO ABCD data */
static uint32 PABCDDAT_mask = 0;   /* Shadow of GPIO ABCD mask */

typedef struct
{
  unsigned int bitoffset;
  unsigned int direction;
} GpioConf_t;

static GpioConf_t gpio_info[32]= { \
 {GPIO_A0,GPIO_DIR_END}, {GPIO_A1,GPIO_DIR_END}, {GPIO_A2,GPIO_DIR_END}, \
 {GPIO_A3,GPIO_DIR_END}, {GPIO_A4,GPIO_DIR_END}, {GPIO_A5,GPIO_DIR_END}, \
 {GPIO_A6,GPIO_DIR_END}, {GPIO_A7,GPIO_DIR_END}, {GPIO_B0,GPIO_DIR_END}, \
 {GPIO_B1,GPIO_DIR_END}, {GPIO_B2,GPIO_DIR_END}, {GPIO_B3,GPIO_DIR_END},  \
 {GPIO_B4,GPIO_DIR_END}, {GPIO_B5,GPIO_DIR_END}, {GPIO_B6,GPIO_DIR_END}, \
 {GPIO_B7,GPIO_DIR_END}, {GPIO_C0,GPIO_DIR_END}, {GPIO_C1,GPIO_DIR_END}, \
 {GPIO_C2,GPIO_DIR_END}, {GPIO_C3,GPIO_DIR_END}, {GPIO_C4,GPIO_DIR_END}, \
 {GPIO_C5,GPIO_DIR_END}, {GPIO_C6,GPIO_DIR_END}, {GPIO_C7,GPIO_DIR_END}, \
 };




/*
 * Macro Definition
 */

/*
 * Function Declaration
 */
 
void rtl8380_intGpio_DirSet(uint32 pin, drv_gpio_direction_t dir)
{
    uint32 val;
    uint32 offset;
    
    if (gpio_info[pin].direction == dir)
    		return;
            
    offset = gpio_info[pin].bitoffset;
            
		/* configure as gpio pin*/
    val = REG32(GPIO_PABC_CNR);
    REG32(GPIO_PABC_CNR) = val & (~((1<<offset) | (1<<offset)));
            
    /*configure pin direction*/
    val = REG32(GPIO_PABC_DIR);
    val = val & (~(1<<offset));

    if(dir == GPIO_DIR_IN)
        REG32(GPIO_PABC_DIR) = val & (~(1<<offset));
            else
        REG32(GPIO_PABC_DIR) = val | (1<<offset);
            
    gpio_info[pin].direction = dir;
    PABCDDAT_mask |= (uint32)1 << offset;
    return;
}

void rtl8380_intGpio_pindata_set(uint32 pin, uint32 bitData)
{
    uint32 val;
    uint32 offset;
            
    offset = gpio_info[pin].bitoffset;
    rtl8380_intGpio_DirSet(pin, GPIO_DIR_OUT);
    val = REG32(GPIO_PABC_DATA);
		if ( PABCDDAT_mask & ((uint32)1 << offset))
            {
		    PABCDDAT_shadow &= ~((uint32)1 << offset);
		    PABCDDAT_shadow |= ((uint32)bitData << offset);
            }
    val &= ~(PABCDDAT_mask);
    val |= (PABCDDAT_shadow & PABCDDAT_mask);

            /* Set GPIO port[pin] bit */
    if (bitData)
            {
        val |= (uint32)1 << offset;
            }
            else
            {
        val &= ~((uint32)1 << offset);
            }
    REG32(GPIO_PABC_DATA) = val;    
            
		return;
}

void rtl8380_intGpio_pindata_get(uint32 pin, uint32 *pBitData)
{
    uint32 val;
    uint32 offset;
    
    offset = gpio_info[pin].bitoffset;
    rtl8380_intGpio_DirSet(pin, GPIO_DIR_IN);
    val = REG32(GPIO_PABC_DATA);
    
		PABCDDAT_shadow &= ~((uint32)1 << offset);

    if(val & (1<<offset))
{
        *pBitData = 1;
         PABCDDAT_shadow |= ((uint32)1 << offset);
}
    else
        *pBitData = 0;

    return;
}

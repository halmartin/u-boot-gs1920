
include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

# CPPFLAGS += -I$(TOPDIR)/board/Realtek/switch -I$(TOPDIR)/board/Realtek/include
AFLAGS += -I$(TOPDIR)/board/Realtek/switch -I$(TOPDIR)/board/Realtek/include

COBJS	:= $(BOARD).o board_cmd.o

ifdef CONFIG_CMD_DRAM_TEST
COBJS	+= memctl_dram.o
endif

ifdef CONFIG_CMD_FLASH_TEST
COBJS	+= memctl_flash.o
endif

SWBOARD := rtl8380

SWITCH_DIR 	= ../switch
ifeq ($(CONFIG_CUSTOMER_BOARD),y)
COBJS += $(SWITCH_DIR)/customer/customer_probe.o $(SWITCH_DIR)/customer/mac_init.o $(SWITCH_DIR)/customer/customer_cmd.o $(SWITCH_DIR)/customer/customer_diag.o
endif
COBJS += interrupt.o
COBJS += $(SWITCH_DIR)/common/cmd_rtk.o
COBJS += $(SWITCH_DIR)/init.o $(SWITCH_DIR)/probe.o
COBJS += $(SWITCH_DIR)/common/util.o
ifeq ($(CONFIG_RTK_BOARD),y)
COBJS += $(SWITCH_DIR)/rtk/rtk_probe.o
endif
COBJS += $(SWITCH_DIR)/rtk/mac/mac_init.o $(SWITCH_DIR)/rtk/mac/$(SWBOARD)/$(SWBOARD)_init.o
COBJS += $(SWITCH_DIR)/rtk/mac/$(SWBOARD)/$(SWBOARD)_drv.o $(SWITCH_DIR)/rtk/mac/$(SWBOARD)/$(SWBOARD)_rtk.o
COBJS += $(SWITCH_DIR)/rtk/mac/$(SWBOARD)/$(SWBOARD)_nic.o


ifeq ($(CONFIG_RTL8208),y)
COBJS += $(SWITCH_DIR)/rtk/phy/rtl8208.o
endif

ifeq ($(CONFIG_RTL8212F),y)
COBJS += $(SWITCH_DIR)/rtk/phy/rtl8212f.o
endif

ifeq ($(CONFIG_RTL8214),y)
COBJS += $(SWITCH_DIR)/rtk/phy/rtl8214f.o
else ifeq ($(CONFIG_RTL8214F),y)
COBJS += $(SWITCH_DIR)/rtk/phy/rtl8214f.o
endif

ifeq ($(CONFIG_RTL8218),y)
COBJS += $(SWITCH_DIR)/rtk/phy/rtl8218.o
endif

ifeq ($(CONFIG_RTL8218B),y)
COBJS += $(SWITCH_DIR)/rtk/phy/rtl8218b.o
else ifeq ($(CONFIG_RTL8218FB),y)
COBJS += $(SWITCH_DIR)/rtk/phy/rtl8218b.o
endif

ifeq ($(CONFIG_RTL8214FC),y)
COBJS += $(SWITCH_DIR)/rtk/phy/rtl8214fc.o
endif

COBJS += $(SWITCH_DIR)/rtk/mac/$(SWBOARD)/$(SWBOARD)_mdc_mdio.o
COBJS += $(SWITCH_DIR)/rtk/drv/rtl8231/rtl8231_drv.o
COBJS += $(SWITCH_DIR)/rtk/drv/rtl8231/rtl8390_8231_drv.o
COBJS += $(SWITCH_DIR)/rtk/drv/rtl8231/rtl8380_8231_drv.o
COBJS += $(SWITCH_DIR)/rtk/drv/gpio/ext_gpio.o
COBJS += $(SWITCH_DIR)/rtk/drv/gpio/gpio.o
COBJS += $(SWITCH_DIR)/rtk/drv/smi/smi.o
COBJS += $(SWITCH_DIR)/rtk/drv/gpio/rtl8380_gpio_drv.o


SOBJS :=

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

all:	$(subdir) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################

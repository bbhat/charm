INCLUDES	+=	includes/cores/common
INCLUDES	+=	includes/util
INCLUDES	+=	includes/soc/$(SOC)
ifeq ($(SOC), s5pv210)
	INCLUDES	+=	includes/soc/$(SOC)/vic
endif
INCLUDES	+=	includes/soc/common/drivers/uart
INCLUDES	+=	includes/soc/common/drivers/rtc
INCLUDES	+=	includes/soc/common/drivers/timer
INCLUDES	+=	includes/target/$(TARGET)
INCLUDES	+=	includes/filesystem
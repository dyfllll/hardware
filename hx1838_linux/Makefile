
KERNEL_DIR:=/media/dy/disk/linux/LubanCat/RK3576/kernel

ARCH=arm64
CROSS_COMPILE=aarch64-linux-gnu-
export  ARCH  CROSS_COMPILE

ccflags-y += -Wno-error=unused-function
ccflags-y += -Wno-error=unused-label
ccflags-y += -Wno-error=unused-variable
ccflags-y += -Wno-error=unused-result

obj-m += red_nec_dev.o


.PHONY: all clean modules_build modules_clean app_build app_clean
all:modules_build app_build
clean:modules_clean app_clean

modules_build:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules	

modules_clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean	

app_build:
	$(CROSS_COMPILE)gcc red_nec_app.c -o red_nec_app
	@echo "test_app built."

app_clean:
	@rm -f red_nec_app
	@echo "test_app removed."


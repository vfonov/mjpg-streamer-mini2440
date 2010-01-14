# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.

# or specify your own in command line
KERNELDIR ?=../../linux-2.6.29


ifneq ($(KERNELRELEASE),)
	obj-m := s3c2440camera.o
	s3c2440camera-objs      :=      s3c2440_ov9650.o sccb.o s3c2440camif.o

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
	PWD  := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm modules

test_capture: test_capture.c
	$(CC) -I$(KERNELDIR)/include -O3 test_capture.c -o test_capture

clean:
	rm -f s3c2440camera.mod.o  s3c2440camera.o  s3c2440camif.o  s3c2440_ov9650.o  sccb.o	s3c2440camera.ko s3c2440camera.mod.c 

endif



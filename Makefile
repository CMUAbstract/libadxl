LIB = libadxl

#include ../Makefile.config
#include ../Makefile.options

OBJECTS = \
	adxl362.o \
	spi.o \

DEPS += \
	libmsp \

override SRC_ROOT = ../../src

include $(MAKER_ROOT)/Makefile.$(TOOLCHAIN)

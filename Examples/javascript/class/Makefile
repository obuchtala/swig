TOP = ../..
SWIG = $(TOP)/../preinst-swig
SRCS = example.cpp
JSCXXSRCS = runme.cpp
TARGET = example
INTERFACE = example.i
SWIGOPT = -I$(TOP)/../Lib/javascript -I$(TOP)/../Lib/javascript/jsc

all::
	$(MAKE) -f $(TOP)/Makefile SRCS='$(SRCS)' SWIG='$(SWIG)' \
SWIGOPT='$(SWIGOPT)' TARGET='$(TARGET)' INTERFACE='$(INTERFACE)' javascript_cpp

clean::
	$(MAKE) -f $(TOP)/Makefile javascript_clean

check:: all
	
	$(MAKE) -f $(TOP)/Makefile JSCXXSRCS='$(JSCXXSRCS)' TARGET='$(TARGET)' javascript_run



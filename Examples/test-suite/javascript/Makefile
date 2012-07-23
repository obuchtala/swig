#######################################################################
# Makefile for javascript test-suite
#######################################################################

LANGUAGE     = javascript
JAVASCRIPT   = ../../../Tools/javascript/javascript
SCRIPTSUFFIX = _runme.js
srcdir       = .
top_srcdir   = ../../..
top_builddir = ../../..

#CPP_TEST_CASES =
#C_TEST_CASES +=

include $(srcdir)/../common.mk

# Overridden variables here

# Custom tests - tests with additional commandline options

# Rules for the different types of tests
%.cpptest: 
	$(setup)
	+$(swig_and_compile_cpp)
	$(run_testcase)

%.ctest:
	$(setup)
	+$(swig_and_compile_c)
	$(run_testcase)

%.multicpptest: 
	$(setup)
	+$(swig_and_compile_multi_cpp)
	$(run_testcase)

# Runs the testcase. A testcase is only run if
# a file is found which has _runme.js appended after the testcase name.
run_testcase = \
	if [ -f $(srcdir)/$(SCRIPTPREFIX)$*$(SCRIPTSUFFIX) ]; then \
	  env LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH $(RUNTOOL) $(JAVASCRIPT) -l $* $(srcdir)/$(SCRIPTPREFIX)$*$(SCRIPTSUFFIX); \
	fi

# Clean
%.clean:
	

clean:
	$(MAKE) -f $(top_builddir)/$(EXAMPLES)/Makefile javascript_clean

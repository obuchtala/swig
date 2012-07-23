#######################################################################
# Makefile for javascript test-suite
#######################################################################

LANGUAGE     = javascript
JAVASCRIPT_EXE   = ../../../Tools/javascript/javascript
JAVASCRIPT_EXE_SRC   = ../../../Tools/javascript/javascript.cxx
SCRIPTSUFFIX = _runme.js
srcdir       = .
top_srcdir   = ../../..
top_builddir = ../../..
JS_INCLUDE   = -I"/usr/include/webkitgtk-1.0/"
JS_DLNK      = -L/usr/lib/ -ljavascriptcoregtk-1.0

#CPP_TEST_CASES =
#C_TEST_CASES +=

include $(srcdir)/../common.mk

# Overridden variables here

# Custom tests - tests with additional commandline options

javascript_exe:
	$(CXX) $(CXXFLAGS) $(JS_INCLUDE) $(JAVASCRIPT_EXE_SRC) $(LIBS) $(JS_DLNK) -g -o $(JAVASCRIPT_EXE)

# Rules for the different types of tests
%.cpptest: javascript_exe
	$(setup)
	+$(swig_and_compile_cpp)
	$(run_testcase)

%.ctest: javascript_exe
	$(setup)
	+$(swig_and_compile_c)
	$(run_testcase)

%.multicpptest: javascript_exe
	$(setup)
	+$(swig_and_compile_multi_cpp)
	$(run_testcase)

# Runs the testcase. A testcase is only run if
# a file is found which has _runme.js appended after the testcase name.
run_testcase = \
	if [ -f $(srcdir)/$(SCRIPTPREFIX)$*$(SCRIPTSUFFIX) ]; then \
	  env LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH $(RUNTOOL) $(JAVASCRIPT_EXE) -l $* $(srcdir)/$(SCRIPTPREFIX)$*$(SCRIPTSUFFIX); \
	fi

# Clean
%.clean:
	

clean:
	$(MAKE) -f $(top_builddir)/$(EXAMPLES)/Makefile javascript_clean

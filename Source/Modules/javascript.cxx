#include "swigmod.h"

#include <cparse.h>
#include <parser.h>
#include <ctype.h>

#include "javascript_emitter.h"

extern swig::JSEmitter *swig_javascript_create_JSC_emitter();

/* ********************************************************************
 * JAVASCRIPT
 * ********************************************************************/

class JAVASCRIPT:public Language {

public:
  JAVASCRIPT() {
  } ~JAVASCRIPT() {
  }

  virtual int functionHandler(Node *n);
  virtual int globalfunctionHandler(Node *n);
  virtual int variableHandler(Node *n);
  virtual int globalvariableHandler(Node *n);
  virtual int classHandler(Node *n);
  virtual int functionWrapper(Node *n);
  virtual int constantWrapper(Node *n);
    /**
     *  Registers all %fragments assigned to section "templates" with the Emitter.
     **/
  virtual int fragmentDirective(Node *n);

  virtual void main(int argc, char *argv[]);
  virtual int top(Node *n);

private:

  swig::JSEmitter * emitter;
};

/* ---------------------------------------------------------------------
 * functionWrapper()
 *
 * Low level code generator for functions 
 * --------------------------------------------------------------------- */

int JAVASCRIPT::functionWrapper(Node *n) {

  // note: the default implementation only prints a message
  // Language::functionWrapper(n);

  emitter->emitWrapperFunction(n);

  return SWIG_OK;
}

/* ---------------------------------------------------------------------
 * functionHandler()
 *
 * Function handler for generating wrappers for functions 
 * --------------------------------------------------------------------- */

int JAVASCRIPT::functionHandler(Node *n) {

  emitter->enterFunction(n);

  Language::functionHandler(n);

  emitter->exitFunction(n);

  return SWIG_OK;
}

/* ---------------------------------------------------------------------
 * globalfunctionHandler()
 *
 * Function handler for generating wrappers for functions 
 * --------------------------------------------------------------------- */

int JAVASCRIPT::globalfunctionHandler(Node *n) {
  emitter->switchNamespace(n);

  Language::globalfunctionHandler(n);
  return SWIG_OK;
}

/* ---------------------------------------------------------------------
 * variableHandler()
 *
 * Function handler for generating wrappers for variables 
 * --------------------------------------------------------------------- */

int JAVASCRIPT::variableHandler(Node *n) {

  emitter->enterVariable(n);

  Language::variableHandler(n);

  emitter->exitVariable(n);

  return SWIG_OK;
}

/* ---------------------------------------------------------------------
 * globalvariableHandler()
 *
 * Function handler for generating wrappers for global variables 
 * --------------------------------------------------------------------- */

int JAVASCRIPT::globalvariableHandler(Node *n) {

  emitter->switchNamespace(n);

  Language::globalvariableHandler(n);
  return SWIG_OK;
}

/* ---------------------------------------------------------------------
 * constantHandler()
 *
 * Function handler for generating wrappers for constants 
 * --------------------------------------------------------------------- */


int JAVASCRIPT::constantWrapper(Node *n) {

  Language::constantWrapper(n);
  emitter->emitConstant(n);

  return SWIG_OK;
}

/* ---------------------------------------------------------------------
 * classHandler()
 *
 * Function handler for generating wrappers for class 
 * --------------------------------------------------------------------- */

int JAVASCRIPT::classHandler(Node *n) {
  emitter->switchNamespace(n);

  emitter->enterClass(n);
  Language::classHandler(n);
  emitter->exitClass(n);

  return SWIG_OK;
}

int JAVASCRIPT::fragmentDirective(Node *n) {

  // catch all fragment directives that have "templates" as location
  // and register them at the emitter.
  String *section = Getattr(n, "section");

  if (Cmp(section, "templates") == 0) {
    emitter->registerTemplate(Getattr(n, "value"), Getattr(n, "code"));
  } else {
    Swig_fragment_register(n);
  }

  return SWIG_OK;
}

/* ---------------------------------------------------------------------
 * top()
 *
 * Function handler for processing top node of the parse tree 
 * Wrapper code generation essentially starts from here 
 * --------------------------------------------------------------------- */

int JAVASCRIPT::top(Node *n) {

  emitter->initialize(n);

  Language::top(n);

  emitter->dump(n);
  emitter->close();

  delete emitter;

  return SWIG_OK;
}

/* ---------------------------------------------------------------------
 * main()
 *
 * Entry point for the JAVASCRIPT module
 * --------------------------------------------------------------------- */

void JAVASCRIPT::main(int argc, char *argv[]) {

  // Set javascript subdirectory in SWIG library
  SWIG_library_directory("javascript");

  int mode = -1;

  for (int i = 1; i < argc; i++) {
    if (argv[i]) {
      if (strcmp(argv[i], "-v8") == 0) {
        Swig_mark_arg(i);
        mode = swig::JSEmitter::V8;
        SWIG_library_directory("javascript/v8");
      } else if (strcmp(argv[i], "-jsc") == 0) {
        Swig_mark_arg(i);
        mode = swig::JSEmitter::JavascriptCore;
        SWIG_library_directory("javascript/jsc");
      } else if (strcmp(argv[i], "-qt") == 0) {
        Swig_mark_arg(i);
        mode = swig::JSEmitter::QtScript;
        SWIG_library_directory("javascript/qt");
      }
    }
  }

  switch (mode) {
  case swig::JSEmitter::V8:
    {
      // TODO: emitter = create_v8_emitter();
      break;
    }
  case swig::JSEmitter::JavascriptCore:
    {
      emitter = swig_javascript_create_JSC_emitter();
      break;
    }
  case swig::JSEmitter::QtScript:
    {
      // TODO: emitter = create_qtscript_emitter();
      break;
    }
  default:
    {
      Printf(stderr, "Unknown emitter type.");
      SWIG_exit(-1);
    }
  }


  // Add a symbol to the parser for conditional compilation
  Preprocessor_define("SWIGJAVASCRIPT 1", 0);

  // Add typemap definitions    
  SWIG_typemap_lang("javascript");

  // Set configuration file 
  SWIG_config_file("javascript.swg");

  allow_overloading();
}

/* -----------------------------------------------------------------------------
 * swig_JAVASCRIPT()    - Instantiate module
 * ----------------------------------------------------------------------------- */

static Language *new_swig_javascript() {
  return new JAVASCRIPT();
}

extern "C" Language *swig_javascript(void) {
  return new_swig_javascript();
}

#ifndef JAVASCRIPT_JSC_H
#define JAVASCRIPT_JSC_H

#include "javascript_emitter.h"

namespace swig {

class JSCEmitter:public JSEmitter {

private:

  enum MarshallingMode {
    Setter,
    Getter,
    Ctor,
    Function
  };

public:

  JSCEmitter();

  virtual ~ JSCEmitter();

  virtual int initialize(Node *n);

  virtual int dump(Node *n);

  virtual int close();


protected:

  virtual int emitCtor(Node *n);

  virtual int emitDtor(Node *n);

  virtual int enterVariable(Node *n);

  virtual int exitVariable(Node *n);

  virtual int enterFunction(Node *n);

  virtual int exitFunction(Node *n);

  virtual int enterClass(Node *n);

  virtual int exitClass(Node *n);

  virtual int emitFunction(Node *n, bool is_member);

  virtual int emitGetter(Node *n, bool is_member);

  virtual int emitSetter(Node *n, bool is_member);

  void marshalInputArgs(ParmList *parms, Wrapper *wrapper, MarshallingMode mode, bool is_member);

  void marshalOutput(Node *n, String *actioncode, Wrapper *wrapper);

  Parm *skipIgnoredArgs(Parm *p);

private:

  File *f_wrap_cpp;
  File *f_runtime;
  File *f_header;
  File *f_wrappers;
  File *f_init;

  String *NULL_STR;

  // dynamically filled code parts
  String *js_global_functions_code;
  String *js_global_variables_code;

  String *js_class_functions_code;
  String *js_class_variables_code;
  String *js_class_static_functions_code;
  String *js_class_static_variables_code;

  String *js_ctor_wrappers;
  String *js_ctor_dispatcher_code;

  String *js_initializer_code;

  // state variables
  String *current_propertyname;
  String *current_getter;
  String *current_setter;
  String *current_classname;
  String *current_functionwrapper;
  String *current_functionname;

};

} // namespace swig

#endif // JAVASCRIPT_JSC_H

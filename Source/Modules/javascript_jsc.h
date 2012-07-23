#ifndef JAVASCRIPT_JSC_H
#define JAVASCRIPT_JSC_H

#include "javascript_emitter.h"

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

  virtual int emitFunctionDispatcher(Node *n, bool is_member);

  virtual int emitGetter(Node *n, bool is_member);

    virtual int emitSetter(Node *n, bool is_member);

    void marshalInputArgs(ParmList *parms, Wrapper *wrapper, MarshallingMode mode, bool is_member, bool is_static = false);

    void marshalOutput(Node *n, String *actioncode, Wrapper *wrapper);

    Parm *skipIgnoredArgs(Parm *p);

    virtual int switchNamespace(Node *n);

    virtual int createNamespace(String *scope);

    virtual Hash *createNamespaceEntry(const char *name, const char *parent);

  virtual int emitNamespaces();
  
  virtual int emitConstant(Node *n);

private:

    File *f_wrap_cpp;
    File *f_runtime;
    File *f_header;
    File *f_wrappers;
    File *f_init;

    String *NULL_STR;
    const char *GLOBAL_STR;

    // contains context specific structs
    // to allow generation different class definition tables
    // which are switched on namespace change
    Hash *namespaces;
    Hash *current_namespace;

    // dynamically filled code parts

    String *create_namespaces_code;
    String *register_namespaces_code;

    String *current_class_functions;
    String *class_variables_code;
    String *class_static_functions_code;
    String *class_static_variables_code;

    String *ctor_wrappers;
    String *ctor_dispatcher_code;

    String *initializer_code;
    String *function_dispatcher_code;

    // state variables
    String* current_propertyname;
    String* current_getter;
    String* current_setter;

    String* current_classname;
    String* current_classname_mangled;
    String* current_classtype;

    String *current_functionwrapper;
    String *current_functionname;
  };

#endif                          // JAVASCRIPT_JSC_H

#ifndef JAVASCRIPT_JSC_H
#define JAVASCRIPT_JSC_H

#include "javascript_emitter.h"

class JSCEmitter: public JSEmitter {

private:

    enum MarshallingMode {
        Setter,
        Getter,
        Ctor,
        Function
    };

public:

    JSCEmitter();

    virtual ~JSCEmitter();
    
    virtual int Initialize(Node *n);

    virtual int Dump(Node *n);
    
    virtual int Close();

    
protected:

    virtual int EmitCtor(Node *n);
    
    virtual int EmitDtor(Node *n);

    virtual int EnterVariable(Node *n);
   
    virtual int ExitVariable(Node *n);

    virtual int EnterFunction(Node *n);

    virtual int ExitFunction(Node *n);

    virtual int EnterClass(Node *n);
    
    virtual int ExitClass(Node *n);

    virtual int EmitFunction(Node *n, bool is_member);

    virtual int EmitGetter(Node *n, bool is_member);

    virtual int EmitSetter(Node *n, bool is_member);

    void marshalInputArgs(ParmList *parms, Wrapper *wrapper, 
                          MarshallingMode mode, bool is_member);
    
    void marshalOutput(Node *n, String *actioncode, Wrapper *wrapper);

    Parm *skipIgnoredArgs(Parm *p);

private:

    File *f_wrap_cpp;
    File *f_runtime;
    File *f_header;
    File *f_wrappers;
    File *f_init;

    // state variables

    String* current_propertyname;
    String* current_getter;
    String* current_setter;
    String* current_classname;
    String* NULL_STR;
    String *js_global_functions_code;
    String *js_global_variables_code;
    String *js_class_functions_code;
    String *js_class_variables_code;
    String *js_initializer_code;
    String *js_ctor_wrappers;
    String *js_ctor_dispatcher_code;
    String *current_functionwrapper;
    String *current_functionname;
    
};

#endif // JAVASCRIPT_JSC_H



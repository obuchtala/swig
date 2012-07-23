#ifndef JAVASCRIPT_EMITTER_H
#define JAVASCRIPT_EMITTER_H

#include "swigmod.h"

/**
 *  A class that wraps a code snippet used as template for code generation.
 */
  class Template {

  public:
    Template(const String *code);

     Template(const String *code, const String *templateName, bool debug);

    ~Template();

    String *str();

     Template & replace(const String *pattern, const String *repl);

  private:
    String *code;
    String *templateName;
    bool debug;
  };

  class JSEmitter {

  public:

    enum JSEmitterType {
      JavascriptCore,
      V8,
      QtScript
    };

     JSEmitter();

     virtual ~ JSEmitter();

    /**
     * Opens output files and temporary output DOHs.
     */
    virtual int initialize(Node *n) = 0;

    /**
     * Writes all collected code into the output file(s).
     */
    virtual int dump(Node *n) = 0;

    /**
     * Cleans up all open output DOHs.
     */
    virtual int close() = 0;

    /**
     * Switches the context for code generation.
     * 
     * Classes, global variables and global functions may need to
     * be registered in certain static tables.
     * This method should be used to switch output DOHs correspondingly.
     */
    virtual int switchNamespace(Node *) {
      return SWIG_OK;
    };

    /**
     * Invoked at the beginning of the classHandler.
     */
    virtual int enterClass(Node *) {
      return SWIG_OK;
    };

    /**
     * Invoked at the end of the classHandler.
     */
    virtual int exitClass(Node *) {
      return SWIG_OK;
    };

    /**
     * Invoked at the beginning of the variableHandler.
     */
    virtual int enterVariable(Node *) {
      return SWIG_OK;
    };

    /**
     * Invoked at the end of the variableHandler.
     */
    virtual int exitVariable(Node *) {
      return SWIG_OK;
    };

    /**
     * Invoked at the beginning of the functionHandler.
     */
    virtual int enterFunction(Node *) {
      return SWIG_OK;
    };

    /**
     * Invoked at the end of the functionHandler.
     */
    virtual int exitFunction(Node *) {
      return SWIG_OK;
    };

    /**
     * Invoked by functionWrapper callback after call to Language::functionWrapper.
     */
    virtual int emitWrapperFunction(Node *n);

    /**
     * Invoked from constantWrapper after call to Language::constantWrapper.
     **/
  virtual int emitConstant(Node *n) = 0;

    /**
     * Registers a given code snippet for a given key name.
     * 
     * This method is called by the fragmentDirective handler
     * of the JAVASCRIPT language module.
     **/
    int registerTemplate(const String *name, const String *code);

    /**
     * Retrieve the code template registered for a given name.
     */
    Template getTemplate(const String *name);

    void enableDebug();

  protected:

    virtual int emitCtor(Node *n) = 0;

    virtual int emitDtor(Node *n) = 0;

    virtual int emitFunction(Node *n, bool is_member) = 0;

    virtual int emitGetter(Node *n, bool is_member) = 0;

  virtual int emitSetter(Node *n, bool is_member) = 0;
  
  bool isSetterMethod(Node *n);

    Node *getBaseClass(Node *n);

    const String *typemapLookup(Node *n, const_String_or_char_ptr tmap_method, SwigType *type, int warning, Node *typemap_attributes = 0);

    void enableDebugTemplates();

  protected:

    // empty string used at different places in the code
    String *empty_string;

    Hash *templates;

    Wrapper *current_wrapper;

    bool debug;
  };


#endif                          // JAVASCRIPT_EMITTER_H

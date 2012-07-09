#include "javascript_jsc.h"
#include "swigmod.h"

swig::JSCEmitter::JSCEmitter()
:  JSEmitter(), NULL_STR(NewString("NULL")) {
}

swig::JSCEmitter::~JSCEmitter() {
  Delete(NULL_STR);
}

/* ---------------------------------------------------------------------
 * skipIgnoredArgs()
 * --------------------------------------------------------------------- */

Parm *swig::JSCEmitter::skipIgnoredArgs(Parm *p) {
  while (checkAttribute(p, "tmap:in:numinputs", "0")) {
    p = Getattr(p, "tmap:in:next");
  }
  return p;
}

/* ---------------------------------------------------------------------
 * marshalInputArgs()
 *
 * Process all of the arguments passed into the argv array
 * and convert them into C/C++ function arguments using the
 * supplied typemaps.
 * --------------------------------------------------------------------- */

void swig::JSCEmitter::marshalInputArgs(ParmList *parms, Wrapper *wrapper, MarshallingMode mode, bool is_member) {
  String *tm;
  Parm *p;

  int startIdx = 0;
  if (is_member)
    startIdx = 1;

  int i = 0;
  for (p = parms; p; p = nextSibling(p), i++) {
    SwigType *pt = Getattr(p, "type");
    String *arg = NewString("");

    switch (mode) {
    case Getter:
    case Function:
      if (is_member && i == 0) {
        Printv(arg, "thisObject", 0);
      } else {
        Printf(arg, "argv[%d]", i - startIdx);
      }
      break;
    case Setter:
      if (is_member && i == 0) {
        Printv(arg, "thisObject", 0);
      } else {
        Printv(arg, "value", 0);
      }
      break;
    case Ctor:
      Printf(arg, "argv[%d]", i);
      break;
    default:
      throw "Illegal state.";
    }

    if ((tm = Getattr(p, "tmap:in")))   // Get typemap for this argument
    {
      Replaceall(tm, "$input", arg);
      Setattr(p, "emit:input", arg);
      Printf(wrapper->code, "%s\n", tm);
    } else {
      Swig_warning(WARN_TYPEMAP_IN_UNDEF, input_file, line_number, "Unable to use type %s as a function argument.\n", SwigType_str(pt, 0));
      p = nextSibling(p);
    }
    Delete(arg);
  }
}

/* ---------------------------------------------------------------------
* marshalOutput()
*
* Process the return value of the C/C++ function call
* and convert them into the Javascript types using the
* supplied typemaps.
* --------------------------------------------------------------------- */

void swig::JSCEmitter::marshalOutput(Node *n, String *actioncode, Wrapper *wrapper) {
  SwigType *type = Getattr(n, "type");
  Setattr(n, "type", type);
  String *tm;
  if ((tm = Swig_typemap_lookup_out("out", n, "result", wrapper, actioncode))) {
    Replaceall(tm, "$result", "jsresult");
    // TODO: May not be the correct way
    Replaceall(tm, "$objecttype", Swig_scopename_last(SwigType_str(SwigType_strip_qualifiers(type), 0)));
    Printf(wrapper->code, "%s", tm);
    if (Len(tm))
      Printf(wrapper->code, "\n");
  } else {
    Swig_warning(WARN_TYPEMAP_OUT_UNDEF, input_file, line_number, "Unable to use return type %s in function %s.\n", SwigType_str(type, 0), Getattr(n, "name"));
  }
  emit_return_variable(n, type, wrapper);
}

int swig::JSCEmitter::initialize(Node *n) {

  /* Get the output file name */
  String *outfile = Getattr(n, "outfile");

  /* Initialize I/O */
  f_wrap_cpp = NewFile(outfile, "w", SWIG_output_files());
  if (!f_wrap_cpp) {
    FileErrorDisplay(outfile);
    SWIG_exit(EXIT_FAILURE);
  }

  /* Initialization of members */
  f_runtime = NewString("");
  f_init = NewString("");
  f_header = NewString("");
  f_wrappers = NewString("");

  /* Initialization of members */
  f_runtime = NewString("");
  f_init = NewString("");
  f_header = NewString("");
  f_wrappers = NewString("");

  create_namespaces_code = NewString("");
  register_namespaces_code = NewString("");
  initializer_code = NewString("");


  namespaces = NewHash();
  Hash *global_namespace = createNamespaceEntry(Char(Getattr(n, "name")), "global");
  Setattr(namespaces, "::", global_namespace);
  current_namespace = global_namespace;

  /* Register file targets with the SWIG file handler */
  Swig_register_filebyname("begin", f_wrap_cpp);
  Swig_register_filebyname("header", f_header);
  Swig_register_filebyname("wrapper", f_wrappers);
  Swig_register_filebyname("runtime", f_runtime);
  Swig_register_filebyname("init", f_init);

  return SWIG_OK;
}

int swig::JSCEmitter::dump(Node *n) {
  /* Get the module name */
  String *module = Getattr(n, "name");

  // write the swig banner
  Swig_banner(f_wrap_cpp);

  Printv(f_wrap_cpp, f_runtime, "\n", 0);
  Printv(f_wrap_cpp, f_header, "\n", 0);
  Printv(f_wrap_cpp, f_wrappers, "\n", 0);

  emitNamespaces();

  // compose the initializer function using a template
  Template initializer(getTemplate("initializer"));
  initializer.replace("${modulename}", module)
      .replace("${initializercode}", initializer_code)
      .replace("${create_namespaces}", create_namespaces_code)
      .replace("${register_namespaces}", register_namespaces_code);
  Wrapper_pretty_print(initializer.str(), f_wrap_cpp);

  return SWIG_OK;
}


int swig::JSCEmitter::close() {
  /* strings */
  Delete(f_runtime);
  Delete(f_header);
  Delete(f_wrappers);
  Delete(f_init);

  Delete(create_namespaces_code);
  Delete(register_namespaces_code);
  Delete(initializer_code);

  Delete(namespaces);

  /* files */
  ::Close(f_wrap_cpp);
  Delete(f_wrap_cpp);

  return SWIG_OK;
}

int swig::JSCEmitter::enterFunction(Node *n) {

  bool is_overloaded = GetFlag(n, "sym:overloaded");
  
  current_functionname = Getattr(n, "sym:name");

  if(is_overloaded && function_dispatcher_code == 0) {
    function_dispatcher_code = NewString("");
  }

  return SWIG_OK;
}

int swig::JSCEmitter::exitFunction(Node *n) {
  Template t_function = getTemplate("functiondecl");

  String *functionname = current_functionname;
  String *functionwrapper = current_functionwrapper;
  
  bool is_member = GetFlag(n, "ismember");
  
  // handle overloaded functions
  // Note: wrappers for overloaded functions are currently
  //       not made available (e.g., foo_double, foo_int)
  //       maybe this could be enabled by an extra feature flag 
  bool is_overloaded = GetFlag(n, "sym:overloaded");

  if(is_overloaded) {
    if(!Getattr(n, "sym:nextSibling")) {

      functionwrapper = Swig_name_wrapper(Getattr(n, "name"));
      // note: set this attribute to transfer ownership
      Setattr(n, "wrap:dispatcher", functionwrapper);

      // create dispatcher
      emitFunctionDispatcher(n, is_member);
      
      Delete(function_dispatcher_code);
      function_dispatcher_code = 0;

    } else {
      
      //don't register wrappers of overloaded functions in function tables
      return SWIG_OK;
    }
  }

  t_function.replace("${functionname}", functionname)
      .replace("${functionwrapper}", functionwrapper);

  if (is_member) {
    
    if (Equal(Getattr(n, "storage"), "static")) {
      Printv(class_static_functions_code, t_function.str(), 0);

    } else {
      Printv(current_class_functions, t_function.str(), 0);
    }

  } else {
    Printv(Getattr(current_namespace, "functions"), t_function.str(), 0);
  }

  return SWIG_OK;
}

int swig::JSCEmitter::enterVariable(Node *n) {
  current_getter = NULL_STR;
  current_setter = NULL_STR;
  current_propertyname = Swig_scopename_last(Getattr(n, "name"));
  return SWIG_OK;
}


int swig::JSCEmitter::exitVariable(Node *n) {
  Template t_variable(getTemplate("variabledecl"));
  t_variable.replace("${setname}", current_setter)
      .replace("${getname}", current_getter)
      .replace("${propertyname}", current_propertyname);


  if (GetFlag(n, "ismember")) {
    if (Equal(Getattr(n, "storage"), "static") || (Equal(Getattr(n, "nodeType"), "enumitem"))) {

      Printv(class_static_variables_code, t_variable.str(), 0);

    } else {
      Printv(class_variables_code, t_variable.str(), 0);
    }
  } else {
    Printv(Getattr(current_namespace, "values"), t_variable.str(), 0);
  }

  return SWIG_OK;
}

int swig::JSCEmitter::enterClass(Node *n) {

  current_classname = Swig_scopename_last(Getattr(n, "classtype"));
  class_variables_code = NewString("");
  current_class_functions = NewString("");
  class_static_variables_code = NewString("");
  class_static_functions_code = NewString("");
  ctor_wrappers = NewString("");
  ctor_dispatcher_code = NewString("");
  return SWIG_OK;
}

int swig::JSCEmitter::exitClass(Node *n) {

  Template t_class(getTemplate("classdefn"));
  String *mangled_name = Swig_name_mangle(Getattr(n, "name"));

  t_class.replace("${classname_mangled}", mangled_name)
      .replace("${jsclassvariables}", class_variables_code)
      .replace("${jsclassfunctions}", current_class_functions)
      .replace("${jsstaticclassfunctions}", class_static_functions_code)
      .replace("${jsstaticclassvariables}", class_static_variables_code);
  Wrapper_pretty_print(t_class.str(), f_wrappers);

  /* 
   * Add the ctor wrappers at this position 
   * Note: this is necessary to avoid extra forward declarations.
   */
  Printv(f_wrappers, ctor_wrappers, 0);

  /* adds the main constructor wrapper function */
  Template t_mainctor(getTemplate("mainctordefn"));
  t_mainctor.replace("${classname_mangled}", mangled_name)
      .replace("${DISPATCH_CASES}", ctor_dispatcher_code);
  Wrapper_pretty_print(t_mainctor.str(), f_wrappers);

  /* adds the dtor wrapper */
  Template t_dtor(getTemplate("destructordefn"));
  t_dtor.replace("${classname_mangled}", mangled_name)
      .replace("${type}", Getattr(n, "classtype"));
  Wrapper_pretty_print(t_dtor.str(), f_wrappers);

  /* adds a class template statement to initializer function */
  Node *base_class = getBaseClass(n);
  String *parentClassDefn = NewString("");
  if (base_class != NULL) {
    Template t_inherit(getTemplate("inheritance"));
    t_inherit.replace("${classname_mangled}", mangled_name)
        .replace("${base_classname}", Swig_name_mangle(Getattr(base_class, "name")));
    Printv(parentClassDefn, t_inherit.str(), 0);
  }

  Template t_classtemplate(getTemplate("create_class_template"));
  t_classtemplate.replace("${classname_mangled}", mangled_name)
      .replace("${parent_class_defintion}", parentClassDefn);
  Wrapper_pretty_print(t_classtemplate.str(), initializer_code);

  /* adds a class registration statement to initializer function */
  Template t_registerclass(getTemplate("register_class"));
  t_registerclass.replace("${classname}", current_classname)
      .replace("${classname_mangled}", mangled_name)
      .replace("${namespace}", Getattr(current_namespace, "name"));
  Wrapper_pretty_print(t_registerclass.str(), initializer_code);

  /* clean up all DOHs */
  Delete(class_variables_code);
  Delete(current_class_functions);
  Delete(class_static_variables_code);
  Delete(class_static_functions_code);
  Delete(ctor_wrappers);
  Delete(mangled_name);
  Delete(parentClassDefn);
  Delete(ctor_dispatcher_code);
  class_variables_code = 0;
  current_class_functions = 0;
  class_static_variables_code = 0;
  class_static_functions_code = 0;
  ctor_wrappers = 0;
  ctor_dispatcher_code = 0;

  return SWIG_OK;
}

int swig::JSCEmitter::emitCtor(Node *n) {

  Template t_ctor(getTemplate("ctordefn"));

  String *mangled_name = Swig_name_mangle(Getattr(n, "name"));

  String *name = (Getattr(n, "wrap:name"));
  String *overname = Getattr(n, "sym:overname");
  String *wrap_name = Swig_name_wrapper(name);
  Setattr(n, "wrap:name", wrap_name);

  ParmList *params = Getattr(n, "parms");
  emit_parameter_variables(params, current_wrapper);
  emit_attach_parmmaps(params, current_wrapper);

  Printf(current_wrapper->locals, "%sresult;", SwigType_str(Getattr(n, "type"), 0));

  int num_args = emit_num_arguments(params);
  String *action = emit_action(n);
  marshalInputArgs(params, current_wrapper, Ctor, true);

  Printv(current_wrapper->code, action, "\n", 0);
  t_ctor.replace("${classname_mangled}", mangled_name)
      .replace("${overloadext}", overname)
      .replace("${LOCALS}", current_wrapper->locals)
      .replace("${CODE}", current_wrapper->code);

  Wrapper_pretty_print(t_ctor.str(), ctor_wrappers);

  String *argcount = NewString("");
  Printf(argcount, "%d", num_args);

  Template t_ctor_case(getTemplate("ctor_dispatch_case"));
  t_ctor_case.replace("${classname_mangled}", mangled_name)
      .replace("${overloadext}", overname)
      .replace("${argcount}", argcount);
  Printv(ctor_dispatcher_code, t_ctor_case.str(), 0);
  Delete(argcount);

  return SWIG_OK;

}

int swig::JSCEmitter::emitDtor(Node *) {
  // TODO:find out how to register a dtor in JSC
  return SWIG_OK;
}

int swig::JSCEmitter::emitGetter(Node *n, bool is_member) {
  Template t_setter(getTemplate("getproperty"));

  String *name = Getattr(n, "wrap:name");
  String *wrap_name = Swig_name_wrapper(name);
  current_getter = wrap_name;
  Setattr(n, "wrap:name", wrap_name);

  ParmList *params = Getattr(n, "parms");
  emit_parameter_variables(params, current_wrapper);
  emit_attach_parmmaps(params, current_wrapper);
  Wrapper_add_local(current_wrapper, "jsresult", "JSValueRef jsresult");

  String *action = emit_action(n);
  marshalInputArgs(params, current_wrapper, Getter, is_member);
  marshalOutput(n, action, current_wrapper);

  t_setter.replace("${getname}", wrap_name)
      .replace("${LOCALS}", current_wrapper->locals)
      .replace("${CODE}", current_wrapper->code);

  Wrapper_pretty_print(t_setter.str(), f_wrappers);

  return SWIG_OK;
}


int swig::JSCEmitter::emitSetter(Node *n, bool is_member) {
  Template t_setter(getTemplate("setproperty"));

  String *name = Getattr(n, "wrap:name");
  String *wrap_name = Swig_name_wrapper(name);
  current_setter = wrap_name;
  Setattr(n, "wrap:name", wrap_name);

  ParmList *params = Getattr(n, "parms");
  emit_parameter_variables(params, current_wrapper);
  emit_attach_parmmaps(params, current_wrapper);
  Wrapper_add_local(current_wrapper, "jsresult", "JSValueRef jsresult");

  String *action = emit_action(n);
  marshalInputArgs(params, current_wrapper, Setter, is_member);
  marshalOutput(n, action, current_wrapper);

  t_setter.replace("${setname}", wrap_name)
      .replace("${LOCALS}", current_wrapper->locals)
      .replace("${CODE}", current_wrapper->code);

  Wrapper_pretty_print(t_setter.str(), f_wrappers);

  return SWIG_OK;
}


int swig::JSCEmitter::emitFunction(Node *n, bool is_member) {
  Template t_function(getTemplate("functionwrapper"));

  bool is_overloaded = GetFlag(n, "sym:overloaded");
  
  String *name = Getattr(n, "sym:name");
  String *wrap_name = Swig_name_wrapper(name);
  
  if(is_overloaded) {
    Append(wrap_name, Getattr(n, "sym:overname"));
  }
  
  current_functionwrapper = wrap_name;
  Setattr(n, "wrap:name", wrap_name);

  ParmList *params = Getattr(n, "parms");
  emit_parameter_variables(params, current_wrapper);
  emit_attach_parmmaps(params, current_wrapper);
  Wrapper_add_local(current_wrapper, "jsresult", "JSValueRef jsresult");

  String *action = emit_action(n);
  marshalInputArgs(params, current_wrapper, Function, is_member);
  marshalOutput(n, action, current_wrapper);

  t_function.replace("${functionname}", wrap_name)
      .replace("${LOCALS}", current_wrapper->locals)
      .replace("${CODE}", current_wrapper->code);
  Wrapper_pretty_print(t_function.str(), f_wrappers);

  if(is_overloaded) {
    Template t_dispatch_case = getTemplate("function_dispatch_case");
    
    int argc = emit_num_arguments(params);
    String *argcount = NewString("");
    Printf(argcount, "%d", argc);
    
    t_dispatch_case.replace("${functionwrapper}", wrap_name)
      .replace("${argcount}", argcount);
    
    Printv(function_dispatcher_code, t_dispatch_case.str(), 0);
    
    Delete(argcount);
  }

  return SWIG_OK;
}

int swig::JSCEmitter::emitFunctionDispatcher(Node *n, bool /*is_member*/) {

  Template t_function(getTemplate("functionwrapper"));

  Wrapper *wrapper = NewWrapper();
  String *wrap_name = Swig_name_wrapper(Getattr(n, "name"));
  Setattr(n, "wrap:name", wrap_name);

  Wrapper_add_local(wrapper, "jsresult", "JSValueRef jsresult");

  Append(wrapper->code, function_dispatcher_code);
  Append(wrapper->code, getTemplate("function_dispatch_case_default").str());
  
  t_function.replace("${functionname}", wrap_name)
      .replace("${LOCALS}", wrapper->locals)
      .replace("${CODE}", wrapper->code);
  
  Wrapper_pretty_print(t_function.str(), f_wrappers);

  DelWrapper(wrapper);
  
  return SWIG_OK;
}

int swig::JSCEmitter::switchNamespace(Node *n) {
  if (!GetFlag(n, "feature:nspace")) {
    current_namespace = Getattr(namespaces, "::");

  } else {

    String *scope = Swig_scopename_prefix(Getattr(n, "name"));

    if (scope) {
      // if the scope is not yet registered
      // create all scopes/namespaces recursively
      if (!Getattr(namespaces, scope)) {
        createNamespace(scope);
      }

      current_namespace = Getattr(namespaces, scope);

    } else {
      current_namespace = Getattr(namespaces, "::");
    }
  }

  return SWIG_OK;
}

int swig::JSCEmitter::createNamespace(String *scope) {

  String *parent_scope = Swig_scopename_prefix(scope);

  Hash *parent_namespace;

  if (parent_scope == 0) {
    parent_namespace = Getattr(namespaces, "::");
  } else if (!Getattr(namespaces, parent_scope)) {
    createNamespace(parent_scope);
    parent_namespace = Getattr(namespaces, parent_scope);
  } else {
    parent_namespace = Getattr(namespaces, parent_scope);
  }

  assert(parent_namespace != 0);

  Hash *new_namespace = createNamespaceEntry(Char(scope), Char(Getattr(parent_namespace, "name")));
  Setattr(namespaces, scope, new_namespace);

  Delete(parent_scope);

  return SWIG_OK;
}

Hash *swig::JSCEmitter::createNamespaceEntry(const char *name, const char *parent) {
  Hash *entry = NewHash();
  Setattr(entry, "name", NewString(name));
  Setattr(entry, "parent", NewString(parent));
  Setattr(entry, "functions", NewString(""));
  Setattr(entry, "values", NewString(""));

  return entry;
}

int swig::JSCEmitter::emitNamespaces() {
  Iterator it;
  for (it = First(namespaces); it.item; it = Next(it)) {
    Hash *entry = it.item;

    String *name = Getattr(entry, "name");
    String *parent = Getattr(entry, "parent");
    String *functions = Getattr(entry, "functions");
    String *variables = Getattr(entry, "values");

    Template namespace_definition(getTemplate("globaldefn"));
    namespace_definition.replace("${jsglobalvariables}", variables)
        .replace("${jsglobalfunctions}", functions)
        .replace("${namespace}", name);
    Wrapper_pretty_print(namespace_definition.str(), f_wrap_cpp);

    Template t_createNamespace(getTemplate("create_namespace"));
    t_createNamespace.replace("${namespace}", name);
    Printv(create_namespaces_code, t_createNamespace.str(), 0);

    Template t_registerNamespace(getTemplate("register_namespace"));
    t_registerNamespace.replace("${namespace}", name)
        .replace("${parent_namespace}", parent);
    Printv(register_namespaces_code, t_registerNamespace.str(), 0);
  }

  return SWIG_OK;
}


swig::JSEmitter * swig_javascript_create_JSC_emitter() {
  return new swig::JSCEmitter();
}

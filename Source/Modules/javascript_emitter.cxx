#include "javascript_emitter.h"

#include "swigmod.h"

/* -----------------------------------------------------------------------------
 * JSEmitter()
 * ----------------------------------------------------------------------------- */

swig::JSEmitter::JSEmitter()
:  empty_string(NewString("")), debug(false) {
  templates = NewHash();
}

/* -----------------------------------------------------------------------------
 * ~JSEmitter()
 * ----------------------------------------------------------------------------- */

swig::JSEmitter::~JSEmitter() {
  Delete(empty_string);
  Delete(templates);
}

/* -----------------------------------------------------------------------------
 * swig::JSEmitter::RegisterTemplate() :  Registers a code template
 * ----------------------------------------------------------------------------- */

int swig::JSEmitter::registerTemplate(const String *name, const String *code) {
  return Setattr(templates, name, code);
}

/* -----------------------------------------------------------------------------
 * swig::JSEmitter::GetTemplate() :  Retrieves a registered a code template
 * ----------------------------------------------------------------------------- */

swig::Template swig::JSEmitter::getTemplate(const String *name) {
  String *templ = Getattr(templates, name);

  if (!templ) {
    Printf(stderr, "Could not find template %s\n.", name);
    SWIG_exit(EXIT_FAILURE);
  }

  Template t(templ, name, debug);

  return t;
}

void swig::JSEmitter::enableDebug() {
  debug = true;
}


/* -----------------------------------------------------------------------------
 * swig::JSEmitter::typemapLookup()
 * 
 *  n - for input only and must contain info for Getfile(n) and Getline(n) to work
 *  tmap_method - typemap method name
 *  type - typemap type to lookup
 *  warning - warning number to issue if no typemaps found
 *  typemap_attributes - the typemap attributes are attached to this node and will 
 *                       also be used for temporary storage if non null
 * return is never NULL, unlike Swig_typemap_lookup()
 * ----------------------------------------------------------------------------- */

const String *swig::JSEmitter::typemapLookup(Node *n, const_String_or_char_ptr tmap_method, SwigType *type, int warning, Node *typemap_attributes) {
  Node *node = !typemap_attributes ? NewHash() : typemap_attributes;
  Setattr(node, "type", type);
  Setfile(node, Getfile(n));
  Setline(node, Getline(n));
  const String *tm = Swig_typemap_lookup(tmap_method, node, "", 0);
  if (!tm) {
    tm = empty_string;
    if (warning != WARN_NONE) {
      Swig_warning(warning, Getfile(n), Getline(n), "No %s typemap defined for %s\n", tmap_method, SwigType_str(type, 0));
    }
  }
  if (!typemap_attributes) {
    Delete(node);
  }
  return tm;
}

/* -----------------------------------------------------------------------------
 * swig::JSEmitter::getBaseClass() :  the node of the base class or NULL
 * ----------------------------------------------------------------------------- */

Node *swig::JSEmitter::getBaseClass(Node *n) {
  // retrieve the first base class that is not %ignored
  List *baselist = Getattr(n, "bases");
  if (baselist) {
    Iterator base = First(baselist);
    while (base.item && GetFlag(base.item, "feature:ignore")) {
      base = Next(base);
    }

    return base.item;
  }

  return NULL;
}

 /* -----------------------------------------------------------------------------
  * swig::JSEmitter::emitWrapperFunction() :  dispatches emitter functions
  * ----------------------------------------------------------------------------- */

int swig::JSEmitter::emitWrapperFunction(Node *n) {
  int ret = SWIG_OK;

  current_wrapper = NewWrapper();
  Setattr(n, "wrap:name", Getattr(n, "sym:name"));

  String *kind = Getattr(n, "kind");

  if (kind) {
    bool is_member = GetFlag(n, "ismember");
    if (Cmp(kind, "function") == 0) {
      ret = emitFunction(n, is_member);

    } else if (Cmp(kind, "variable") == 0) {
      if (isSetterMethod(n)) {
        ret = emitSetter(n, is_member);

      } else {
        ret = emitGetter(n, is_member);

      }
    } else {
      Printf(stderr, "Warning: unsupported wrapper function type\n");
      Swig_print_node(n);

      ret = SWIG_ERROR;
    }
  } else {
    String *view = Getattr(n, "view");

    if (Cmp(view, "constructorHandler") == 0) {
      ret = emitCtor(n);

    } else if (Cmp(view, "destructorHandler") == 0) {
      ret = emitDtor(n);

    } else {
      Printf(stderr, "Warning: unsupported wrapper function type");
      Swig_print_node(n);
      ret = SWIG_ERROR;
    }
  }

  DelWrapper(current_wrapper);
  current_wrapper = 0;

  return ret;
}

/* -----------------------------------------------------------------------------
 * swig::JSEmitter::emitConstant() :  triggers code generation for constants
 * ----------------------------------------------------------------------------- */

int swig::JSEmitter::emitConstant(Node *n) {
  current_wrapper = NewWrapper();
  String *action = NewString("");

  Setattr(n, "wrap:name", Getattr(n, "sym:name"));
  //TODO
  Printf(action, "result = %s;", Getattr(n, "name"));
  Setattr(n, "wrap:action", action);

  enterVariable(n);
  emitGetter(n, false);
  exitVariable(n);

  DelWrapper(current_wrapper);

  current_wrapper = 0;

  return SWIG_OK;
}

/* -----------------------------------------------------------------------------
 * __swigjs_str_ends_with() :  c string helper to check suffix match
 * ----------------------------------------------------------------------------- */

int __swigjs_str_ends_with(const char *str, const char *suffix) {

  if (str == NULL || suffix == NULL)
    return 0;

  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);

  if (suffix_len > str_len)
    return 0;

  return 0 == strncmp(str + str_len - suffix_len, suffix, suffix_len);
}


/* -----------------------------------------------------------------------------
 * swig::JSEmitter::isSetterMethod() :  helper to check if a method is a setter function
 * ----------------------------------------------------------------------------- */

bool swig::JSEmitter::isSetterMethod(Node *n) {
  String *symname = Getattr(n, "sym:name");
  return (__swigjs_str_ends_with((char *) Data(symname), "_set") != 0);
}

/* -----------------------------------------------------------------------------
 * swig::Template::Template() :  creates a Template class for given template code
 * ----------------------------------------------------------------------------- */

swig::Template::Template(const String *code_) {

  if (!code_) {
    Printf(stdout, "Template code was null. Illegal input for template.");
    SWIG_exit(EXIT_FAILURE);
  }

  code = NewString(code_);
  templateName = NewString("");
}

swig::Template::Template(const String *code_, const String *templateName_, bool debug_) {

  if (!code_) {
    Printf(stdout, "Template code was null. Illegal input for template.");
    SWIG_exit(EXIT_FAILURE);
  }

  code = NewString(code_);
  templateName = NewString(templateName_);
  debug = debug_;
}


/* -----------------------------------------------------------------------------
 * swig::Template::~Template() :  cleans up of Template.
 * ----------------------------------------------------------------------------- */

swig::Template::~Template() {
  Delete(code);
  Delete(templateName);
}

/* -----------------------------------------------------------------------------
 * String* swig::Template::str() :  retrieves the current content of the template.
 * ----------------------------------------------------------------------------- */

String *swig::Template::str() {
  if (debug) {
    String *pre_code = NewString("");
    String *post_code = NewString("");
    String *debug_code = NewString("");
    Printf(pre_code, "//begin fragment(\"%s\")\n", templateName);
    Printf(post_code, "//end fragment(\"%s\")\n", templateName);
    Printf(debug_code, "%s\n%s\n%s", pre_code, code, post_code);

    Delete(code);
    Delete(pre_code);
    Delete(post_code);

    code = debug_code;
  }
  return code;
}

/* -----------------------------------------------------------------------------
 * Template&  swig::Template::replace(const String* pattern, const String* repl) :
 * 
 *  replaces all occurances of a given pattern with a given replacement.
 * 
 *  - pattern:  the pattern to be replaced
 *  - repl:     the replacement string
 *  - returns a reference to the Template to allow chaining of methods.
 * ----------------------------------------------------------------------------- */

swig::Template & swig::Template::replace(const String *pattern, const String *repl) {
  ::Replaceall(code, pattern, repl);
  return *this;
}

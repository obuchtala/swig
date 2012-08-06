

%fragment(SWIG_From_frag(bool),"header") {
SWIGINTERNINLINE 
JSValueRef SWIG_From_dec(bool)(bool value)
{
  return JSValueMakeBoolean(context, value);
}
}

%fragment(SWIG_AsVal_frag(bool),"header",
          fragment=SWIG_AsVal_frag(long)) {
SWIGINTERN
int SWIG_AsVal_dec(bool)(JSValueRef obj, bool *val)
{
  if(!JSValueIsBoolean(context, obj)) {
    return SWIG_ERROR;
  }
  if (val) *val = JSValueToBoolean(context, obj);
  return SWIG_OK;
}
}

%fragment(SWIG_From_frag(int),"header") {
SWIGINTERNINLINE JSValueRef
  SWIG_From_dec(int)(int value)
{
  return JSValueMakeNumber(context, value);
}
}

%fragment(SWIG_From_frag(long),"header") {
SWIGINTERNINLINE JSValueRef
  SWIG_From_dec(long)(long value)
{
  return JSValueMakeNumber(context, value);
}
}

%fragment(SWIG_AsVal_frag(long),"header",
          fragment="SWIG_CanCastAsInteger") {
SWIGINTERN int
SWIG_AsVal_dec(long)(JSValueRef obj, long* val)
{
  if (!JSValueIsNumber(context, obj)) {
    return SWIG_TypeError;
  }
  if(val) *val = (long) JSValueToNumber(context, obj, NULL);
  
  return SWIG_OK;
}
}

/* unsigned long */

%fragment(SWIG_From_frag(unsigned long),"header",
          fragment=SWIG_From_frag(long)) {
SWIGINTERNINLINE JSValueRef 
SWIG_From_dec(unsigned long)(unsigned long value)
{
  return (value > LONG_MAX) ?
    JSValueMakeNumber(context, value) : JSValueMakeNumber(context, %numeric_cast(value,long)); 
}
}

%fragment(SWIG_AsVal_frag(unsigned long),"header",
          fragment="SWIG_CanCastAsInteger") {
SWIGINTERN int
SWIG_AsVal_dec(unsigned long)(JSValueRef obj, unsigned long *val) 
{
  if(!JSValueIsNumber(context, obj)) {
    return SWIG_TypeError;
  }
  
  long longVal = (long) JSValueToNumber(context, obj, NULL);
  
  if(longVal < 0) {
      return SWIG_OverflowError;
  }
  
  if(val) *val = longVal; 
  
  return SWIG_OK;
}
}

%fragment(SWIG_From_frag(double),"header") {
SWIGINTERN JSValueRef
SWIG_From_dec(double) (double val)
{
  return JSValueMakeNumber(context, val);
}
}

%fragment(SWIG_AsVal_frag(double),"header") {
SWIGINTERN int
SWIG_AsVal_dec(double)(JSValueRef obj, double *val)
{
  if(!JSValueIsNumber(context, obj)) {
    return SWIG_TypeError;
  }
  if(val) *val = JSValueToNumber(context, obj, NULL);
  
  return SWIG_OK;
}
}


/* size_t */

%fragment(SWIG_From_frag(size_t),"header",fragment=SWIG_From_frag(unsigned long)) {
SWIGINTERNINLINE JSValueRef
SWIG_From_dec(size_t)(size_t value)
{    
  return SWIG_From(unsigned long)(%numeric_cast(value, unsigned long));
}
}

%fragment(SWIG_AsVal_frag(size_t),"header",fragment=SWIG_AsVal_frag(unsigned long)) {
SWIGINTERNINLINE int
SWIG_AsVal_dec(size_t)(JSValueRef obj, size_t *val)
{
  unsigned long v;
  int res = SWIG_AsVal(unsigned long)(obj, val ? &v : 0);
  if (SWIG_IsOK(res) && val) *val = %numeric_cast(v, size_t);
  return res;
}
}


// Primitive types
%typemap(in) char,
            signed char,
            unsigned char,
            short,
            unsigned short,
            int,
            unsigned int,
            long,
            unsigned long,
            long long,
            unsigned long long,
            float,
            double
%{ $1 = ($1_ltype)JSValueToNumber(context, $input, NULL); %}

%typemap(in) const bool &,              bool &,
            const char &,              char &,
            const signed char &,       signed char &,
            const unsigned char &,     unsigned char &,
            const short &,             short &,
            const unsigned short &,    unsigned short &,
            const int &,               int &,
            const unsigned int &,      unsigned int &,
            const long &,              long &,
            const unsigned long &,     unsigned long &,
            const long long &,         long long &,
            const unsigned long long &,unsigned long long &,
            const float &,             float &,
            const double &,            double &
%{ $1 = ($1_ltype)&$input; %}

%typemap(out) char,
            signed char,
            unsigned char,
            short,
            unsigned short,
            int,
            unsigned int,
            long,
            unsigned long,
            long long,
            unsigned long long,
            float,
            double
%{ 
  $result = JSValueMakeNumber(context, $1);
%}

%typemap(out) const bool &,             bool &,
             const char &,              char &,
             const signed char &,         signed char &,
             const unsigned char &,     unsigned char &,
             const short &,                short &,
             const unsigned short &,    unsigned short &,
             const int &,                int &,
             const unsigned int &,        unsigned int &,
             const long &,                long &,
             const unsigned long &,     unsigned long &,
             const long long &,         long long &,
             const unsigned long long &,unsigned long long &,
             const float &,             float &,
             const double &,              double &
%{ 
  $result = JSValueMakeNumber(context,*$1); 
%}


%typemap(in) short *,
            unsigned short *,
            int *,
            unsigned int *,
            long *,
            unsigned long *,
            long long *,
            unsigned long long *,
            float *,
            double *
%{
 JSObjectRef o$1 = JSValueToObject(context,$input, NULL);
 SWIG_PRV_DATA *$1_privatedata = (SWIG_PRV_DATA *)JSObjectGetPrivate(o$1);
 $1 = ($1_ltype)$1_privatedata->swigCObject;
%}


%typemap(out) short *,
            unsigned short *,
            int *,
            unsigned int *,
            long *,
            unsigned long *,
            long long *,
            unsigned long long *,
            float *,
            double *
%{
 //SWIG_PRV_DATA *privatedata = new SWIG_PRV_DATA();
 //privatedata->swigCMemOwn = false;
 //privatedata->swigCObject = result;
 // DEBUG: JSObjectMake - case 1
 // TODO: this needs to be fixed
 // $result = JSObjectMake(context, $1_mangle_classRef, privatedata);
  //  $result = JSValueMakeUndefined(context);
    $result = SWIG_JSC_NewPointerObj(context, $1, SWIGTYPE$1_mangle , SWIG_POINTER_OWN);
%}

%typemap(in) bool
%{
 $1 = ($1_ltype)JSValueToBoolean(context, $input);
%}

%typemap(out) bool
%{
 $result = JSValueMakeBoolean(context, $1);
%}


%typemap(out) void
%{ 
  $result = JSValueMakeUndefined(context); 
%}


%typemap(in) char *
%{
 JSStringRef $1_str = JSValueToStringCopy(context, $input, NULL);
 size_t $1_strsize = JSStringGetMaximumUTF8CStringSize($1_str);
 $1 = ($1_ltype) malloc($1_strsize * sizeof(char));
 JSStringGetUTF8CString($1_str, (char*) $1, $1_strsize);
%}

%typemap(out) char *
%{
 JSStringRef jsstring = JSStringCreateWithUTF8CString((char*) $1);
 $result = JSValueMakeString(context, jsstring);
 JSStringRelease(jsstring);
%}

%typemap(arginit) char * 
%{
// TODO: arginit for char* ?
%}

%typemap(out) unsigned char *
%{
  // TODO: typemap out for unsigned char*
%}

%typemap(in) char *& ($*1_ltype temp = 0) %{
   temp = ($*1_ltype)$input;
   JSStringRef $1_str = JSValueToStringCopy(context, $input, NULL);
   size_t $1_strsize = JSStringGetMaximumUTF8CStringSize($1_str);
   $1 = (char *)malloc($1_strsize * sizeof(char));
   JSStringGetUTF8CString($1_str, $1, $1_strsize);
%}

%typemap(out) char *&
%{
 JSStringRef jsstring = JSStringCreateWithUTF8CString((const char *)*$1);
 $result = JSValueMakeString(context, jsstring);
 JSStringRelease(jsstring);
%}

/* char arrays - treat as String */
%typemap(in) char[ANY], char[] %{
   JSStringRef $1_str = JSValueToStringCopy(context, $input, NULL);
   size_t $1_strsize = JSStringGetMaximumUTF8CStringSize($1_str);
   JSStringGetUTF8CString($1_str, $1, $1_strsize);
%}

%typemap(out) char[ANY], char[], 
              unsigned char[ANY], unsigned char[]
%{
 JSStringRef jsstring = JSStringCreateWithUTF8CString((char*) $1);
 $result = JSValueMakeString(context, jsstring);
 JSStringRelease(jsstring);
%}

%typemap(freearg) char *, char *&, char[ANY], char[] //TODO: Not working: A memory leak
%{ 
  free($1); 
%}


/* Typemaps for composite types */
%typemap(in) SWIGTYPE ($&1_type argp)    // Objects passed by value, convert to a pointer
%{
 // DEBUG: typemap(in) SWIGTYPE ($&1_type argp) 
 JSObjectRef o$1 = JSValueToObject(context,$input, NULL);
 SWIG_PRV_DATA *$1_privatedata = (SWIG_PRV_DATA *)JSObjectGetPrivate(o$1);
 argp = ($&1_ltype)$1_privatedata->swigCObject;
 $1 = *argp;
%}

%typemap(out) SWIGTYPE ($&1_type temp)
#ifdef __cplusplus
%{
 temp = new $1_ltype((const $1_ltype &)$1);
 SWIG_PRV_DATA *privatedata = new SWIG_PRV_DATA();
 privatedata->swigCMemOwn = false;
 privatedata->swigCObject = temp;
 // DEBUG: JSObjectMake - case 2 
 // TODO: this needs to be fixed
 // $result = JSObjectMake(context, $1_mangle_classRef, privatedata);
 $result = JSValueMakeUndefined(context);
%}
#else
{
 $&1_ltype $1ptr = ($&1_ltype) malloc(sizeof($1_ltype));
 memmove($1ptr, &$1, sizeof($1_type));
 temp = $1ptr;
 SWIG_PRV_DATA *privatedata = (SWIG_PRV_DATA *)malloc(sizeof(SWIG_PRV_DATA());
 privatedata->swigCMemOwn = false;
 privatedata->swigCObject = temp;
 // DEBUG: JSObjectMake - case 3 
 $result = JSObjectMake(context, $*1_mangle_classRef, privatedata);
}
#endif

%typemap(in) SWIGTYPE *, SWIGTYPE &
%{
 JSObjectRef o$1 = JSValueToObject(context,$input, NULL);
 SWIG_PRV_DATA *$1_privatedata = (SWIG_PRV_DATA *)JSObjectGetPrivate(o$1);
 $1 = ($1_ltype)$1_privatedata->swigCObject;
%}

%typemap(out) SWIGTYPE *, SWIGTYPE &
%{
 SWIG_PRV_DATA *privatedata = new SWIG_PRV_DATA();
 privatedata->swigCMemOwn = false;
 privatedata->swigCObject = (void*) result;
 // DEBUG: JSObjectMake - case 4
 $result = JSObjectMake(context, _SwigObject_classRef, privatedata);
%}
%typemap(arginit) SWIGTYPE *
%{
 // TODO: arginit SWIGTYPE*
%}

%typemap(in) SWIGTYPE (CLASS::*)
%{
 // TODO: SWIGTYPE Class* in typemap
%}

%typemap(out) SWIGTYPE (CLASS::*)
%{
 // TODO: SWIGTYPE Class* out typemap
 // DEBUG: JSObjectMake - case 5
 $result = JSObjectMake(context, $*1, result);
%}



/* Typecheck typemaps - The purpose of these is merely to issue a warning for overloaded C++ functions
 * that cannot be overloaded in Javascript as more than one C++ type maps to a single Javascript type */
// TODO

// Default array handling
%typemap(in) SWIGTYPE [] %{ $1 = ($1_ltype)$input; %}
%typemap(out) SWIGTYPE [] %{
 SWIG_PRV_DATA *result_privatedata = (SWIG_PRV_DATA *)malloc(sizeof(SWIG_PRV_DATA));
 result_privatedata->swigCMemOwn = false;
 result_privatedata->swigCObject = (void*) $1;
 $result = JSObjectMake(context, _SwigObject_classRef, result_privatedata);
%}

// Some ANSI C typemaps */
%apply unsigned long { size_t };
%apply const unsigned long & { const size_t & };

// Array reference typemaps
%apply SWIGTYPE & { SWIGTYPE ((&)[ANY]) }

// const pointers
%apply SWIGTYPE * { SWIGTYPE *const }

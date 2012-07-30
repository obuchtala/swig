
#define SWIG_FROM_DECL_ARGS SWIG_JSC_FROM_DECL_ARGS
#define SWIG_FROM_CALL_ARGS SWIG_JSC_FROM_CALL_ARGS
#define SWIG_AS_DECL_ARGS SWIG_JSC_AS_DECL_ARGS
#define SWIG_AS_CALL_ARGS SWIG_JSC_AS_CALL_ARGS

%include <javascriptfragments.swg>

/* Include fundamental fragemt definitions */
%include <typemaps/fragments.swg>

/* Python fragments for fundamental types */
%include <javascriptprimitives.swg>

#define SWIG_Object                     JSValueRef
#define VOID_Object                     JSValueMakeUndefined(context)

/* append output */
#define SWIG_AppendOutput(result, obj)  "/* TODO: SWIG_Javascript_AppendOutput(result, obj) */"

#define SWIG_SetConstant(name, obj) "/* TODO: SWIG_Javascript_SetConstant(d, name,obj) */"

#define SWIG_Raise(obj, type, desc)  SWIG_Javascript_Raise(context, exception, type)
 
/* override some of the macros in global valtypes.swg */
%include <javascriptvaltypes.swg>

%include <javascriptstrings.swg>

/* Include the unified typemap library */
%include <typemaps/swigtypemaps.swg>



%include <javascriptfragments.swg>

/* Include fundamental fragemt definitions */
%include <typemaps/fragments.swg>

/* Python fragments for fundamental types */
%include <javascriptprimitives.swg>

#define SWIG_Object                     JSValueRef
#define VOID_Object                     JSValueMakeUndefined(context)

/* append output */
#define SWIG_AppendOutput(result, obj)  "/* TODO: SWIG_Javascript_AppendOutput(result, obj) */"

#define SWIG_SetConstant(name, obj) "/* TODO: SWIG_Javaqscript_SetConstant(d, name,obj) */"

#define SWIG_Raise(obj, type, desc) "/* TODO: SWIG_Javascript_Raise(obj, type, desc) */"

/* override some of the macros in global valtypes.swg */
%include <javascriptvaltypes.swg>

/* Include the unified typemap library */
%include <typemaps/swigtypemaps.swg>


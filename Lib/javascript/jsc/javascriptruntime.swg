/* -----------------------------------------------------------------------------
 * javascriptruntime.swg
 *
 * Javascript support code
 * ----------------------------------------------------------------------------- */

%insert(runtime) %{
#include <JavaScriptCore/JavaScript.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
%}

%insert(runtime) %{
typedef struct {
  bool swigCMemOwn;
  void *swigCObject;
}SWIG_PRV_DATA; 
%}

%insert(runtime) %{
/* Contract support */
#define SWIG_contract_assert(expr, msg) if (!(expr)) { throw "contract violated: msg"; } else
%}

%insert(runtime) %{

JSValueRef _wrap_SwigObject_disown(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argc, const JSValueRef argv[], JSValueRef* exception)
{
  JSValueRef jsresult;
  
  JSObjectRef obj = JSValueToObject(context, thisObject, NULL);
  SWIG_PRV_DATA *cdata = (SWIG_PRV_DATA *) JSObjectGetPrivate(obj);
  
  cdata->swigCMemOwn = false;

  jsresult = JSValueMakeUndefined(context); 
  return jsresult;
}

JSValueRef _wrap_SwigObject_getCPtr(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argc, const JSValueRef argv[], JSValueRef* exception)
{
  JSValueRef jsresult;
  long result;
    
  JSObjectRef obj = JSValueToObject(context, thisObject, NULL);
  SWIG_PRV_DATA *cdata = (SWIG_PRV_DATA*) JSObjectGetPrivate(obj);
  
  result = (long) cdata->swigCObject;
  jsresult = JSValueMakeNumber(context, result); 
  
  return jsresult;
}

JSStaticValue _SwigObject_values[] = {
  {
    0, 0, 0, 0 
  }
};

JSStaticFunction _SwigObject_functions[] = {
  {
    "disown",_wrap_SwigObject_disown, kJSPropertyAttributeNone
  },{
    "getCPtr",_wrap_SwigObject_getCPtr, kJSPropertyAttributeNone
  },
  {
    0, 0, 0 
  }
};

JSClassDefinition _SwigObject_objectDefinition;

JSClassRef _SwigObject_classRef;

%}

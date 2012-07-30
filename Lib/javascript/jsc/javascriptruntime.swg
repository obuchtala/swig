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
#include <errno.h>
#include <limits.h>
%}

%insert(runtime) "swigrun.swg";         /* SWIG API */
%insert(runtime) "swigerrors.swg";      /* SWIG errors */   

#define SWIG_Error(code, msg)     SWIG_JSC_exception(context, exception, code, msg)
#define SWIG_exception(code, msg) SWIG_JSC_exception(context, exception, code, msg)
#define SWIG_fail                 /* TODO: goto fail ?*/
//#define SWIG_Raise(obj, type, desc) SWIG_Javascript_Raise(context, exception, type)


%insert(runtime) %{
typedef struct {
  bool swigCMemOwn;
  void *swigCObject;
  swig_type_info *info;
}SWIG_PRV_DATA; 
%}

%insert(runtime) %{
void SWIG_Javascript_Raise(JSContextRef context, JSValueRef *exception, const char* type) {
 JSStringRef message = JSStringCreateWithUTF8CString(type);
 *exception = JSValueMakeString(context, message);
 JSStringRelease(message);
}
%}

%insert(runtime) %{

void SWIG_JSC_exception(JSContextRef context, JSValueRef *exception, int code, const char* msg) {

  throw msg;
}

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


%insert(runtime) %{
int SWIG_JSC_ConvertPtr(JSContextRef context, JSObjectRef objRef, void** ptr, swig_type_info *info, int flags) {
  SWIG_PRV_DATA *cdata = (SWIG_PRV_DATA *) JSObjectGetPrivate(objRef);
  if(cdata == NULL) {
    return SWIG_ERROR;
  }
  if(cdata->info != info) {
    bool type_valid = false;
    swig_cast_info *t = info->cast; 
    while(t != NULL) {
      if(t->type == cdata->info) {
        type_valid = true;
        break;
      }
      t = t->next;
    }
    if(!type_valid) {
      return SWIG_TypeError;
    }
  }
  
  *ptr = cdata->swigCObject;
  
  if(flags & SWIG_POINTER_DISOWN) {
    cdata->swigCMemOwn = false;
  }
  
  return SWIG_OK;
}

int SWIG_JSC_ConvertPtr(JSContextRef context, JSValueRef valRef, void** ptr, swig_type_info *info, int flags) {
  if(!JSValueIsObject(context, valRef)) {
    return SWIG_TypeError;
  }
  
  JSObjectRef objRef = JSValueToObject(context, valRef, NULL);
  if(objRef == NULL) {
    return SWIG_ERROR;
  }
    
  return SWIG_JSC_ConvertPtr(context, objRef, ptr, info, flags);
}

JSObjectRef SWIG_JSC_NewPointerObj(JSContextRef context, void *ptr, swig_type_info *info, int flags) {

  JSClassRef classRef;
  if(info->clientdata == NULL) {
    classRef = _SwigObject_classRef;
  } else {
    classRef = (JSClassRef) info->clientdata;
  }
  
  JSObjectRef result = JSObjectMake(context, classRef, NULL);
  
  SWIG_PRV_DATA* cdata = new SWIG_PRV_DATA();
  cdata->swigCObject = ptr;  
  cdata->swigCMemOwn = (flags & SWIG_POINTER_OWN) ? 1 : 0;
  cdata->info = info;
  
  JSObjectSetPrivate(result, cdata);

  return result;
}

#define SWIG_ConvertPtr(obj, ptr, info, flags)    SWIG_JSC_ConvertPtr(context, obj, ptr, info, flags)
#define SWIG_NewPointerObj(ptr, info, flags)      SWIG_JSC_NewPointerObj(context, ptr, info, flags)
%}

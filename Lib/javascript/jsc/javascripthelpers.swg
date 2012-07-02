%insert(runtime) %{

 
bool jsc_registerClass(JSGlobalContextRef& context, JSObjectRef& parentObject,
                             const char* className,
                             JSClassDefinition* definition,
                             JSObjectCallAsConstructorCallback ctor = NULL) {

    JSStringRef js_className = JSStringCreateWithUTF8CString(className);
    
    JSObjectRef classObject;
    if(ctor == NULL) {
        classObject = JSObjectMake(context, JSClassCreate(definition), NULL);
    } else {
        classObject = JSObjectMakeConstructor(context, JSClassCreate(definition), ctor);
    }

    JSObjectSetProperty(context, parentObject,
                        js_className, classObject,
                        kJSPropertyAttributeNone, NULL);
    JSStringRelease(js_className);
    return true;
}

bool jsc_registerFunction(JSGlobalContextRef context, JSObjectRef object, 
                        const char* functionName, JSObjectCallAsFunctionCallback callback)
{
    JSStringRef js_functionName = JSStringCreateWithUTF8CString(functionName);
    JSObjectSetProperty(context, object, js_functionName,
                        JSObjectMakeFunctionWithCallback(context, js_functionName, callback), 
                        kJSPropertyAttributeNone, NULL);
    JSStringRelease(js_functionName);
    return true;
}
%}

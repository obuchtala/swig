/* ----------------------------------------------------------------------------------------------
 * javascriptcode.swg
 *
 * Additional Javascript typemaps for generating code for classes, constants and variables
 * ----------------------------------------------------------------------------- ----------------*/

%fragment ("jsc_initializer", "templates") %{
bool ${modulename}_initialize(JSGlobalContextRef context) {
   JSObjectRef globalObject = JSContextGetGlobalObject(context);
   JSClassRef cvar_classTemplate = jsc_createClassTemplate(global_values,global_functions);
 jsc_registerClass(context, globalObject, cvar_classTemplate, "cvar");
   ${initializercode}
   return true;
}
%}

%fragment ("jsc_register_global_function", "templates")
%{
jscinitfunction(${context}, ${context_object}, "${functionname}", ${functionwrapper});
%}

/* Additional typemaps for the  class code needed by the access layer. This code is copy-pasted into the wrapper header and source file */
%fragment ("getproperty", "templates") 
%{
JSValueRef ${getname}(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{ 
    ${LOCALS}
    ${CODE}
    return jsresult;
}
%}

%fragment ("setproperty", "templates") 
%{
bool ${setname}(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
    ${LOCALS}
    ${CODE}
}
%}

%fragment ("functionwrapper", "templates") 
%{
JSValueRef ${functionname}(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argc, const JSValueRef argv[], JSValueRef* exception)
{
  ${LOCALS}
  ${CODE}
  return jsresult;
}
%}

%fragment ("staticfunctiondecl", "templates") 
%{
{"${propertyname}",${getname}, ${setname},kJSPropertyAttributeNone},
%}

%fragment ("accessconstructordecl", "templates")  
%{
void $jsclassname_initialize(JSContextRef context, JSObjectRef thisObject)%}

%fragment ("accessconstructorbody", "templates") 
{
}
%fragment ("accessdestructordecl", "templates")  
%{
void $jsclassname_finalize(JSObjectRef thisObject)%}

%fragment ("accessdestructorbody", "templates") 
{
	SWIG_PRV_DATA* t = (SWIG_PRV_DATA*)JSObjectGetPrivate(thisObject);
	if(t && t->swigCMemOwn) delete ($objecttype*)(t->swigCObject);
	if(t) delete t;
}

%fragment ("accessvariablesdefn", "templates") 
%{
JSStaticValue $jsclassname_staticValues[] = {
$jsstaticvaluescode
    { 0, 0, 0, 0 }
};

JSStaticFunction $jsclassname_staticFunctions[] = {
$jsstaticfuncscode
    { 0, 0, 0 }
};
%}

%fragment("accessfunctionsdecl", "templates")  
%{
bool $jsclassname_initClass(JSGlobalContextRef& context, JSObjectRef& parentObject, const char* className);
JSClassRef $jsclassname_createJSClass(JSContextRef context);
JSObjectRef $jsclassname_createcppObject(JSContextRef context, JSObjectRef thisObject, size_t argc, const JSValueRef argv[],
JSValueRef* exception);
%}

%fragment("accessfunctionsdefn", "templates") 
%{ 
JSClassRef $jsclassname_createJSClass(JSContextRef context)
{
	static JSClassRef jsClass = NULL;
	if (!jsClass)
	{
		JSClassDefinition definition	= kJSClassDefinitionEmpty;
		definition.staticValues			= $jsclassname_staticValues;
		definition.staticFunctions		= $jsclassname_staticFunctions;
		definition.initialize			= $jsclassname_initialize;
		definition.finalize				= $jsclassname_finalize;
		jsClass 						= JSClassCreate(&definition);
	}
	return jsClass;
}

JSObjectRef $jsclassname_createcppObject(JSContextRef context, JSObjectRef thisObject, size_t argc, const JSValueRef argv[],
JSValueRef* exception)
{
  JSObjectRef jsresult;	
  $objecttype result = 0;
  $jscreatecppobjectcode
  SWIG_PRV_DATA *swigprivatedata = new SWIG_PRV_DATA();
  swigprivatedata->swigCMemOwn = true;
  swigprivatedata->swigCObject = result;
  	
  jsresult = JSObjectMake(context, $jsclassname_createJSClass(context), swigprivatedata);
  return jsresult;
}

bool $jsclassname_initClass(JSGlobalContextRef& context, JSObjectRef& parentObject, const char* className)
{
    JSStringRef jsstring = JSStringCreateWithUTF8CString(className);
	JSObjectSetProperty(context, parentObject, jsstring, 
						JSObjectMakeConstructor(context,
						$jsclassname_createJSClass(context),
						$jsclassname_createcppObject), 
						kJSPropertyAttributeNone, 
						NULL);
    JSStringRelease(jsstring);
    return true;
} 
%}


/* Additional typemaps needed for generating the code for handling constants as properties of the cconst object */
%fragment ("constantdecl", "templates") 
%{JSValueRef $constantgetname(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)%}

%fragment ("constantbody", "templates") 
%{
    SWIGUNUSEDPARM(thisObject);
    SWIGUNUSEDPARM(exception);
    JSValueRef jsresult;
%}

%fragment ("cconstdecl", "templates")  
%{
bool cconst_initClass(JSGlobalContextRef& context, JSObjectRef& parentObject, const char* constclassname);
%}

%fragment ("cconstdefn", "templates") 
%{

JSStaticValue cconst_staticValues[] = {
$jsstaticconstscode
    { 0, 0, 0, 0 }
};

JSClassRef cconst_createJSClass(JSContextRef context)
{
	static JSClassRef jsClass = NULL;
	if (!jsClass)
	{
		JSClassDefinition definition	= kJSClassDefinitionEmpty;
		definition.staticValues			= cconst_staticValues;
		jsClass 						= JSClassCreate(&definition);
	}
	return jsClass;
}

bool cconst_initClass(JSGlobalContextRef& context, JSObjectRef& parentObject, const char* constclassname)
{
    JSStringRef jsstring = JSStringCreateWithUTF8CString(constclassname);
    static JSObjectRef cconstclassObject = JSObjectMake(context, cconst_createJSClass(context), NULL);
    JSObjectSetProperty(
		context,
	    parentObject,
	    jsstring,
	    cconstclassObject,
	    kJSPropertyAttributeNone,
	    NULL);
    JSStringRelease(jsstring);
    return true;
} 
%}



/* Additional typemaps needed for generating the code for handling C global variables as properties if the cvar object */

%fragment ("variablegetdecl", "templates") 
%{
JSValueRef $variablegetname(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)%}

%fragment ("variablesetdecl", "templates") 
%{
bool $variablesetname(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, 
JSValueRef* exception)%}


%fragment ("variablebody", "templates") 
%{
    SWIGUNUSEDPARM(thisObject);
    SWIGUNUSEDPARM(exception);
    JSValueRef jsresult;
%}

%fragment ("cvardecl", "templates")  
%{
bool cvar_initClass(JSGlobalContextRef& context, JSObjectRef& parentObject, const char* cvarclassname);
}%}

%fragment ("cvardefn", "templates") 
%{

JSStaticValue cvar_staticValues[] = {
${jsstaticvarcode}
    { 0, 0, 0, 0 }
};

JSClassRef cvar_createJSClass(JSContextRef context)
{
	static JSClassRef jsClass = NULL;
	if (!jsClass)
	{
		JSClassDefinition definition	= kJSClassDefinitionEmpty;
		definition.staticValues			= cvar_staticValues;
		jsClass							= JSClassCreate(&definition);
	}
	return jsClass;
}

bool cvar_initClass(JSGlobalContextRef& context, JSObjectRef& parentObject, const char* cvarclassname)
{
    JSStringRef jsstring = JSStringCreateWithUTF8CString(cvarclassname);
    static JSObjectRef cvarclassObject = JSObjectMake(context, cvar_createJSClass(context), NULL);
    JSObjectSetProperty(
		context,
	    parentObject,
	    jsstring,
	    cvarclassObject,
	    kJSPropertyAttributeNone,
	    NULL);
   JSStringRelease(jsstring);
   return true;
} 
%}



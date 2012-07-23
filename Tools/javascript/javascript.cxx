
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <string>
#include <vector>

#include <dlfcn.h>

#include <JavaScriptCore/JavaScript.h>

using namespace std;

static JSValueRef jsc_printstring(JSContextRef context,JSObjectRef object, JSObjectRef globalobj, size_t argc, const JSValueRef	args[], JSValueRef* ex);
static char* jsccreateStringWithContentsOfFile(const char* fileName);
bool jsc_registerFunction(JSGlobalContextRef context, JSObjectRef object, const char* FunctionName,JSObjectCallAsFunctionCallback cbFunction);

typedef void* HANDLE;
typedef int (*JSCIntializer)(JSGlobalContextRef context);

void print_usage() {
    std::cout << "javascript [-l module] <js-file>" << std::endl;
}

int main(int argc, char* argv[]) {
    cout<<"main called\n";

    std::string scriptPath;
    
    std::vector<std::string> module_names;
    
    std::vector<HANDLE> loaded_modules;
    std::vector<JSCIntializer> module_initializers;

    for (int idx = 1; idx < argc; ++idx) {
      if(strcmp(argv[idx], "-l") == 0) {
        idx++;
        if(idx > argc) {
          print_usage();
          exit(-1);
        }
        std::string module_name(argv[idx]);
        module_names.push_back(module_name);
      } else {
        scriptPath = argv[idx];
      }
    }
    
    for(std::vector<std::string>::iterator it = module_names.begin();
      it != module_names.end(); ++it) {
      std::string module_name = *it;
      std::string lib_name = std::string("lib").append(module_name).append(".so");

      HANDLE handle = dlopen(lib_name.c_str(), RTLD_LAZY);
      if(handle == 0) {
        std::cout << "Could not load library " << lib_name << std::endl;
        continue;
      }
      
      std::string symname;
      symname.append(module_name).append("_initialize");
      
      JSCIntializer init_function = (JSCIntializer) dlsym(handle, symname.c_str());
      if(init_function == 0) {
        std::cout << "Could not find initializer function in module " << module_name << std::endl;
        dlclose(handle);
        continue;
      }
      
      loaded_modules.push_back(handle);
      module_initializers.push_back(init_function);
    }
    
    static int failed;

    JSGlobalContextRef context = JSGlobalContextCreate(NULL);
    JSObjectRef globalObject = JSContextGetGlobalObject(context);

    jsc_registerFunction(context, globalObject, "print", jsc_printstring); // Utility print function
    
    // Call module initializers
    for(std::vector<JSCIntializer>::iterator it = module_initializers.begin();
      it != module_initializers.end(); ++it) {
        JSCIntializer init_function = *it;
        init_function(context);
        // TODO: check return value
    }
    
    // Evaluate the javascript
    char*	szString = jsccreateStringWithContentsOfFile(scriptPath.c_str());
    JSStringRef Script;
    
    if(!szString) {
        printf("FAIL: runme script could not be loaded.\n");
        failed = 1;
    }
    else {
        Script = JSStringCreateWithUTF8CString(szString);
        
        JSValueRef ex;
        JSValueRef Result = JSEvaluateScript(context,Script,NULL,NULL,0,&ex);
        
        if (Result) 
            printf("runme.js executed successfully\n");
        
        else {
            printf("exception encountered in the script\n");
            JSStringRef exIString;
            exIString = JSValueToStringCopy(context, ex, NULL);
            char stringUTF8[256];
            JSStringGetUTF8CString(exIString, stringUTF8, 256);
            printf(":%s\n",stringUTF8);
            JSStringRelease(exIString);
        }
    }
    if (szString != NULL)
        free(szString);
    
    JSStringRelease(Script);
    globalObject = 0;
    JSGlobalContextRelease(context);

    for(std::vector<HANDLE>::iterator it = loaded_modules.begin();
      it != loaded_modules.end(); ++it) {
        HANDLE handle = *it;
        dlclose(handle);
    }
    
    if (failed) {
        printf("FAIL: Some tests failed.\n");
        return 1;
    }
    
    printf("PASS: Program exited normally.\n");
    return 0;
}

static JSValueRef jsc_printstring(JSContextRef context,JSObjectRef object, JSObjectRef globalobj, size_t argc, const JSValueRef	args[], JSValueRef* ex)
{
	if (argc > 0)
	{
		JSStringRef string = JSValueToStringCopy(context, args[0], NULL);
		size_t numChars = JSStringGetMaximumUTF8CStringSize(string);
		char stringUTF8[numChars];
		JSStringGetUTF8CString(string, stringUTF8, numChars);
		printf("%s\n", stringUTF8);
	}
    
	return JSValueMakeUndefined(context);
}

static char* jsccreateStringWithContentsOfFile(const char* fileName)
{
	char* buffer;
	
	size_t buffer_size = 0;
	size_t buffer_capacity = 1024;
	buffer = (char*)malloc(buffer_capacity);
	
	FILE* f = fopen(fileName, "r");
	if (!f)
	{
		fprintf(stderr, "Could not open file: %s\n", fileName);
		return 0;
	}
	
	while (!feof(f) && !ferror(f))
	{
		buffer_size += fread(buffer + buffer_size, 1, buffer_capacity - buffer_size, f);
		if (buffer_size == buffer_capacity)
		{ 
			// guarantees space for trailing '\0'
			buffer_capacity *= 2;
			buffer = (char*)realloc(buffer, buffer_capacity);
		}
	}
	fclose(f);
	buffer[buffer_size] = '\0';
	
	return buffer;
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

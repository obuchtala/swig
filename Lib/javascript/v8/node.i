%insert("runtime") %{
#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif

#include <node.h>
%}

%define %node(moduleName)
%insert("post-init") %{
extern "C" {

    void init(v8::Handle<v8::Object> target) {
      moduleName ## _initialize(target);
    }

    NODE_MODULE(moduleName, moduleName ## _initialize)
}
%}
%enddef

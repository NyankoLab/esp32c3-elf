#include "esp32c3.h"

_LIBCPP_BEGIN_NAMESPACE_STD
  #define _LIBCPP_EXTERN_TEMPLATE_DEFINE(...) template __VA_ARGS__;
  _LIBCPP_STRING_V1_EXTERN_TEMPLATE_LIST(_LIBCPP_EXTERN_TEMPLATE_DEFINE, char)
  template          string::basic_string(char const*);
  template string&  string::append(string const&);
  template void     string::clear();
  template bool     string::empty() const;
  template size_t   string::find(char const*, size_t) const;
  template size_t   string::find(string const&, size_t) const;
  template string&  string::operator=(string&&);
  template void     string::resize(size_t);
#if _LIBCPP_STD_VER <= 20
  template string   string::substr(size_t, size_t) const;
#else
  template string   string::substr(size_t, size_t) const&;
  template string   string::substr(size_t, size_t) &&;
#endif
_LIBCPP_END_NAMESPACE_STD

void* operator new(size_t size)
{
    return malloc(size);
}

void* operator new[](size_t size)
{
    return malloc(size);
}

void operator delete(void* pointer)
{
    free(pointer);
}

void operator delete[](void* pointer)
{
    free(pointer);
}

extern "C" void __cxa_pure_virtual()
{
    esp_restart();
}

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* dlopen(const char* filename, int flags);
void* dlsym(void* handle, const char* symbol);
int dlclose(void* handle);

#ifdef __cplusplus
}
#endif

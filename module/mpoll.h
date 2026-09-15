#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*mpoll_callback)(int, int);

void mpoll_ctl(int fd, mpoll_callback callback);
void mpoll_wait(int timeout);

#ifdef __cplusplus
}
#endif

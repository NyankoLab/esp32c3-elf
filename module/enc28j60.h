#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void enc28j60(int mosi, int miso, int sclk, int cs, int interrupt);

#ifdef __cplusplus
}
#endif

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void wifi_ap(char const* name, char const* pass);
void wifi_sta();

#ifdef __cplusplus
}
#endif

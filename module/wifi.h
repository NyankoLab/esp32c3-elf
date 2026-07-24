#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void wifi_ap(char const* name, char const* pass);
void wifi_sta(char const* name);
void wifi_config(char const* ssid, char const* password, bool connect);

#ifdef __cplusplus
}
#endif

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "SettingsManager.h"

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

void init_settings_page();
void init_settings();

extern ESP8266WebServer httpServer;
extern ESP8266HTTPUpdateServer httpUpdater;
extern SettingsManager smgr;

extern Setting_Int32 setting_interval;
extern Setting_String setting_wifi_ssid;
extern Setting_String setting_wifi_psk;
extern Setting_IPAddress setting_server_ip;
extern Setting_Int32 setting_server_port;
extern Setting_String setting_tags;

#endif

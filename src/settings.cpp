#include "Arduino.h"
#include <Wire.h>
#include "settings.h"

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
SettingsManager smgr(&httpServer);


Setting_Int32 setting_interval;

Setting_String setting_wifi_ssid;
Setting_String setting_wifi_psk;

Setting_IPAddress setting_server_ip;
Setting_Int32 setting_server_port;
Setting_String setting_tags;

#define EEPROM_ADDRESS 0x50

#if BUFFER_LENGTH < 34
#error I2C BUFFER TOO SMALL, change BUFFER_LENGTH to at least 34 in /libraries/Wire/Wire.h
#endif

bool eeprom_write(uint16_t address, uint8_t length, uint8_t *buffer)
{
	address *= 32;

	uint8_t written = 0;

	while (written < length) {
		if ((address % 32) == 0) {
			if (written) {
				if (Wire.endTransmission())
					return false;
				delay(5);
			}
			Wire.beginTransmission(EEPROM_ADDRESS);
			Wire.write((address >> 8) & 0x7F);
			Wire.write(address & 0xFF);
		}
		Wire.write(*(buffer++));
		written++;
		address++;
	}
	if (Wire.endTransmission())
		return false;

	return true;
}

bool eeprom_read(uint16_t address, uint8_t length, uint8_t *buffer)
{
	address *= 32;
	uint8_t read = 0;

	while (read < length) {
		if ((address % 32) == 0) {
			Wire.beginTransmission(EEPROM_ADDRESS);
			Wire.write((address >> 8) & 0x7F);
			Wire.write(address & 0xFF);
			if (Wire.endTransmission(false))
				return false;
			uint8_t toread = min((uint8_t)(length - read), (uint8_t)32);
			if (Wire.requestFrom((uint8_t)EEPROM_ADDRESS, toread) != toread)
				return false;
		}
		*(buffer++) = Wire.read();
		read++;
		address++;
	}

	return true;
}

void init_settings()
{
	setting_interval.address = 0x00;
	setting_server_ip.address = 0x01;
	setting_server_port.address = 0x02;

	setting_wifi_ssid.address = 0x10;
	setting_wifi_psk.address = 0x11;

	setting_tags.address = 0x20;

	smgr.readfunc = eeprom_read;
	smgr.writefunc = eeprom_write;

	smgr.settings.push_back(&setting_interval);
	smgr.settings.push_back(&setting_wifi_ssid);
	smgr.settings.push_back(&setting_wifi_psk);
	smgr.settings.push_back(&setting_server_ip);
	smgr.settings.push_back(&setting_server_port);
	smgr.settings.push_back(&setting_tags);

	setting_interval.abbrev = PSTR("intv");
	setting_interval.name = PSTR("measurement interval (min)");
	setting_interval.val_default = 5;
	setting_interval.min = 1;
	setting_interval.max = 60;

	setting_wifi_ssid.abbrev = PSTR("ssid");
	setting_wifi_ssid.name = PSTR("WIFI SSID");
	setting_wifi_ssid.val_default = PSTR("SSID-XXXX");
	setting_wifi_ssid.value = (char *)malloc(32);
	setting_wifi_ssid.max_length = 31;

	setting_wifi_psk.abbrev = PSTR("psk");
	setting_wifi_psk.name = PSTR("WIFI PSK");
	setting_wifi_psk.val_default = PSTR("12345678");
	setting_wifi_psk.value = (char *)malloc(32);
	setting_wifi_psk.max_length = 31;
	setting_wifi_psk.options.hide_value = 1;

	setting_server_ip.abbrev = PSTR("ip");
	setting_server_ip.name = PSTR("Push IP Address");
	setting_server_ip.val_default = IPAddress(192, 168, 1, 1);

	setting_server_port.abbrev = PSTR("port");
	setting_server_port.name = PSTR("Push Port");
	setting_server_port.val_default = 8001;
	setting_server_port.min = 0;
	setting_server_port.max = (1 << 16) - 1;

	setting_tags.abbrev = PSTR("meas");
	setting_tags.name = PSTR("Measurement Tags");
	setting_tags.val_default = PSTR("name:sensor loc:test");
	setting_tags.value = (char *)malloc(64);
	setting_tags.max_length = 63;

	smgr.load();
}

void handleSettingsGet()
{
	smgr.handleSettingsGet(0xFF);
	Serial.println("settings get request");
}

void handleSettingsPost()
{
	smgr.handleSettingsPost();
	Serial.println("settings post request");
}

void init_settings_page()
{
	httpUpdater.setup(&httpServer, "/update", "admin", "admin");
	httpServer.on("/", HTTP_GET, handleSettingsGet);
	httpServer.on("/", HTTP_POST, handleSettingsPost);
}

#include "Arduino.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "settings.h"
#include "bme280.h"

const IPAddress apsubnet(255, 255, 255, 0);
const IPAddress apip(192, 168, 4, 1);
const IPAddress apgateway(192, 168, 4, 1);

void init_config()
{
	pinMode(2, OUTPUT);

	Serial.println("starting AP");
	WiFi.mode(WIFI_AP);
	WiFi.softAP("SensorNodeConfig");
	WiFi.softAPConfig(apip, apgateway, apsubnet);
	init_settings_page();

	Serial.println("starting http server");
	httpServer.begin();
}

void sleep()
{
	Serial.println("sleep");
	ESP.deepSleep(60UL * 1000000UL * setting_interval.value - micros());
}

void read_sensor()
{
	struct bme280_dev dev;

	if (!setup_bme280(dev)) {
		Serial.println("bme280 setup failed");
		sleep();
	}

	// ##############################

	Serial.println("wifi");
	WiFi.mode(WIFI_STA);
	WiFi.begin(setting_wifi_ssid.value, setting_wifi_psk.value);

	uint8_t retrycounter = 2;
	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		WiFi.begin(setting_wifi_ssid.value, setting_wifi_psk.value);
		Serial.println("WiFi failed, retrying.");

		if (!(retrycounter--)) {
			Serial.println("too many wifi reconnect attempts");
			sleep();
		}
	}

	// ##############################

	struct bme280_data data;
	if (!read_bme280(dev, data))
		sleep();

	// ##############################

	int rssi = WiFi.RSSI();
	double voltage = analogRead(A0);
	voltage *= 5.3 / 5.6 * (1.0 / 1024) * (180 + 1000) / 180.0;

	// ##############################

	Serial.print("temp: ");
	Serial.println(data.temperature);
	Serial.print("hum: ");
	Serial.println(data.humidity);
	Serial.print("bat: ");
	Serial.println(voltage);
	Serial.print("rssi: ");
	Serial.println(rssi);

	// ##############################

	Serial.println("post");

	WiFiClient pushClient;
	pushClient.setTimeout(500);

	uint8_t attempts = 0;
	while (!pushClient.connect(setting_server_ip.value, setting_server_port.value))
	{
		if (++attempts > 3)
		{
			Serial.print("too many failed attempts to connect");
			sleep();
		}
	}

	Serial.println("connected");

	String content = String(setting_tags.value) + "|";
	content += "name:temperature " + String(data.temperature, 2) + "|";
	content += "name:humidity " + String(data.humidity, 2) + "|";
	content += "name:voltage pos:battery " + String(voltage, 2) + "|";
	content += "name:rssi " + String(rssi) + "|";
	content += "\n";

	pushClient.println(content);
	pushClient.flush();
	pushClient.stop();

	sleep();
}

void setup(void)
{
	Serial.begin(115200);
	Serial.println("\n\nstart");

	// ##############################

	WiFi.persistent(false);
	Wire.begin();
	init_settings();

	// ##############################
	pinMode(14, INPUT_PULLUP);

	if (digitalRead(14))
		read_sensor();
	else
		init_config();
}

void loop(void)
{
	httpServer.handleClient();
}

#include "device/DeviceInfo.h"
#include <ESP8266WiFi.h>
#include "device/Firmware.h"

DeviceInfo::DeviceInfo(const WiFiService& wifi) : _wifi(wifi) {}

String DeviceInfo::getDeviceId() const
{
    char deviceId[16];
    snprintf(deviceId, sizeof(deviceId), "ESP-%06X", ESP.getChipId());

    return String(deviceId);
}

String DeviceInfo::getFeederId() const
{
    return getDeviceId();
}

String DeviceInfo::getFirmwareVersion() const
{
    return Firmware::VERSION;
}

String DeviceInfo::getModel() const
{
    return Firmware::MODEL;
}

String DeviceInfo::getManufacturer() const
{
    return Firmware::MANUFACTURER;
}

String DeviceInfo::getChipModel() const
{
    return "ESP8266";
}

String DeviceInfo::getIpAddress() const
{
    return _wifi.getIpAddress();
}

String DeviceInfo::getMacAddress() const
{
    return _wifi.getMacAddress();
}

int DeviceInfo::getRssi() const
{
    return _wifi.getRssi();
}

uint32_t DeviceInfo::getFreeHeap() const
{
    return ESP.getFreeHeap();
}

void DeviceInfo::printBootInfo() const
{
    Serial.println();
    Serial.println("==================================================");
    Serial.println("                CatFeeder Boot");
    Serial.println("==================================================");
    Serial.println();

    Serial.print("Model          : ");
    Serial.println(getModel());

    Serial.print("Manufacturer   : ");
    Serial.println(getManufacturer());

    Serial.print("Firmware       : ");
    Serial.println(getFirmwareVersion());

    Serial.println();

    Serial.print("Chip           : ");
    Serial.println(getChipModel());

    Serial.print("Device ID      : ");
    Serial.println(getDeviceId());

    Serial.print("Feeder ID      : ");
    Serial.println(getFeederId());

    Serial.print("MAC Address    : ");
    Serial.println(getMacAddress());

    Serial.print("IP Address     : ");
    Serial.println(getIpAddress());

    Serial.print("WiFi RSSI      : ");
    Serial.print(getRssi());
    Serial.println(" dBm");

    Serial.print("Free Heap      : ");
    Serial.print(getFreeHeap());
    Serial.println(" bytes");

    Serial.println();
    Serial.println("==================================================");
    Serial.println();
}
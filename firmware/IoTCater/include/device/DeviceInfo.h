#pragma once

#include "ESP8266WiFi.h"
#include "Arduino.h"
#include "Firmware.h"
#include "services/WifiService.h"
class DeviceInfo
{
   public:
    explicit DeviceInfo(const WiFiService& wifi);

    String getDeviceId() const;
    String getFirmwareVersion() const;
    String getModel() const;
    String getChipModel() const;
    String getManufacturer() const;
    String getMacAddress() const;
    String getIpAddress() const;

    void printBootInfo() const;

    int getRssi() const;

    uint32_t getFreeHeap() const;

   private:
    const WiFiService& _wifi;
};
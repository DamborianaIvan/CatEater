#pragma once

#include <Arduino.h>
#include <EEPROM.h>

class WifiCredentialsStorage
{
   public:
    void begin();
    bool load(String& ssid, String& password) const;
    bool save(const String& ssid, const String& password);
    bool clear();

    static bool areValid(const String& ssid, const String& password);

   private:
    static constexpr int EEPROM_SIZE = 512;
    static constexpr int CREDENTIALS_ADDRESS = 256;
    static constexpr uint8_t SIGNATURE = 0xD3;
    static constexpr uint8_t VERSION = 1;
    static constexpr size_t MAX_SSID_LENGTH = 32;
    static constexpr size_t MAX_PASSWORD_LENGTH = 63;

    struct StoredCredentials
    {
        uint8_t signature;
        uint8_t version;
        uint16_t length;
        char ssid[MAX_SSID_LENGTH + 1];
        char password[MAX_PASSWORD_LENGTH + 1];
        uint32_t crc;
    };

    static uint32_t calculateCrc(const uint8_t* data, size_t length);
    static bool hasTerminator(const char* value, size_t maxLength);
};

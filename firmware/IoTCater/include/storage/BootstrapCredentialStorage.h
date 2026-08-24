#pragma once

#include <Arduino.h>
#include <EEPROM.h>

class BootstrapCredentialStorage
{
   public:
    void begin();

    bool load(String& credential) const;
    bool save(const String& credential);
    bool clear();

    static bool isValid(const String& credential);

   private:
    static constexpr int EEPROM_SIZE = 512;
    static constexpr int CREDENTIAL_ADDRESS = 320;
    static constexpr uint8_t SIGNATURE = 0xB2;
    static constexpr uint8_t VERSION = 1;
    static constexpr size_t CREDENTIAL_LENGTH = 64;

    struct StoredCredential
    {
        uint8_t signature;
        uint8_t version;
        uint16_t length;
        char credential[CREDENTIAL_LENGTH + 1];
        uint32_t crc;
    };

    static uint32_t calculateCrc(const uint8_t* data, size_t length);
    static bool hasTerminator(const char* value, size_t maxLength);
};

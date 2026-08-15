#include "storage/WifiCredentialsStorage.h"

#include <cstddef>
#include <cstring>

void WifiCredentialsStorage::begin()
{
    EEPROM.begin(EEPROM_SIZE);
}

bool WifiCredentialsStorage::load(String& ssid, String& password) const
{
    StoredCredentials stored{};
    EEPROM.get(CREDENTIALS_ADDRESS, stored);

    const uint32_t expectedCrc =
        calculateCrc(reinterpret_cast<const uint8_t*>(&stored), offsetof(StoredCredentials, crc));
    if (stored.signature != SIGNATURE || stored.version != VERSION ||
        stored.length != sizeof(StoredCredentials) || stored.crc != expectedCrc ||
        !hasTerminator(stored.ssid, sizeof(stored.ssid)) ||
        !hasTerminator(stored.password, sizeof(stored.password)))
    {
        return false;
    }

    ssid = stored.ssid;
    password = stored.password;
    return areValid(ssid, password);
}

bool WifiCredentialsStorage::save(const String& ssid, const String& password)
{
    if (!areValid(ssid, password))
    {
        return false;
    }

    StoredCredentials stored{};
    stored.signature = SIGNATURE;
    stored.version = VERSION;
    stored.length = sizeof(StoredCredentials);
    ssid.toCharArray(stored.ssid, sizeof(stored.ssid));
    password.toCharArray(stored.password, sizeof(stored.password));
    stored.crc =
        calculateCrc(reinterpret_cast<const uint8_t*>(&stored), offsetof(StoredCredentials, crc));

    EEPROM.put(CREDENTIALS_ADDRESS, stored);
    return EEPROM.commit();
}

bool WifiCredentialsStorage::clear()
{
    StoredCredentials empty{};
    EEPROM.put(CREDENTIALS_ADDRESS, empty);
    return EEPROM.commit();
}

bool WifiCredentialsStorage::areValid(const String& ssid, const String& password)
{
    if (ssid.isEmpty() || ssid.length() > MAX_SSID_LENGTH || password.length() > MAX_PASSWORD_LENGTH)
    {
        return false;
    }

    return password.isEmpty() || password.length() >= 8;
}

uint32_t WifiCredentialsStorage::calculateCrc(const uint8_t* data, size_t length)
{
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i)
    {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit)
        {
            const bool lsbSet = (crc & 1U) != 0;
            crc >>= 1U;
            if (lsbSet)
            {
                crc ^= 0xEDB88320U;
            }
        }
    }
    return ~crc;
}

bool WifiCredentialsStorage::hasTerminator(const char* value, size_t maxLength)
{
    return memchr(value, '\0', maxLength) != nullptr;
}

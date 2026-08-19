#include "storage/DeviceCredentialStorage.h"

void DeviceCredentialStorage::begin()
{
    EEPROM.begin(EEPROM_SIZE);
}

bool DeviceCredentialStorage::save(const String& credential)
{
    if (!isValid(credential))
    {
        return false;
    }

    StoredCredential stored{};
    stored.signature = SIGNATURE;
    stored.version = VERSION;
    stored.length = static_cast<uint16_t>(credential.length());
    credential.toCharArray(stored.credential, sizeof(stored.credential));
    stored.crc = calculateCrc(reinterpret_cast<const uint8_t*>(&stored),
                              offsetof(StoredCredential, crc));

    EEPROM.put(CREDENTIAL_ADDRESS, stored);

    return EEPROM.commit();
}

bool DeviceCredentialStorage::load(String& credential) const
{
    StoredCredential stored{};
    EEPROM.get(CREDENTIAL_ADDRESS, stored);

    if (stored.signature != SIGNATURE || stored.version != VERSION ||
        stored.length != CREDENTIAL_LENGTH ||
        !hasTerminator(stored.credential, sizeof(stored.credential)))
    {
        return false;
    }

    const uint32_t expectedCrc = calculateCrc(reinterpret_cast<const uint8_t*>(&stored),
                                              offsetof(StoredCredential, crc));

    if (stored.crc != expectedCrc)
    {
        return false;
    }

    credential = String(stored.credential);
    return isValid(credential);
}

bool DeviceCredentialStorage::clear()
{
    StoredCredential empty{};
    EEPROM.put(CREDENTIAL_ADDRESS, empty);
    return EEPROM.commit();
}

bool DeviceCredentialStorage::isValid(const String& credential)
{
    return credential.length() == CREDENTIAL_LENGTH;
}

uint32_t DeviceCredentialStorage::calculateCrc(const uint8_t* data, size_t length)
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

bool DeviceCredentialStorage::hasTerminator(const char* value, size_t maxLength)
{
    for (size_t i = 0; i < maxLength; ++i)
    {
        if (value[i] == '\0')
        {
            return true;
        }
    }

    return false;
}

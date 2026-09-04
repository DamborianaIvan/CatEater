#include <Arduino.h>
#include <EEPROM.h>
#include <LittleFS.h>

namespace
{
constexpr uint32_t SERIAL_BAUD_RATE = 115200;
constexpr size_t EEPROM_SIZE = 512;

bool clearEeprom()
{
    if (!EEPROM.begin(EEPROM_SIZE))
    {
        return false;
    }

    for (size_t address = 0; address < EEPROM_SIZE; ++address)
    {
        EEPROM.write(address, 0xFF);
    }

    const bool committed = EEPROM.commit();
    EEPROM.end();

    return committed;
}

bool formatLittleFs()
{
    if (!LittleFS.begin())
    {
        return false;
    }

    const bool formatted = LittleFS.format();
    LittleFS.end();

    return formatted;
}

void printDeviceIdentity()
{
    char deviceId[16];
    snprintf(deviceId, sizeof(deviceId), "ESP-%06X", ESP.getChipId());

    Serial.println();
    Serial.println("Device");
    Serial.print("  Feeder ID       : ");
    Serial.println(deviceId);
}

void runFactoryReset()
{
    Serial.println();
    Serial.println("EEPROM");
    const bool eepromOk = clearEeprom();
    Serial.print("  Reset           : ");
    Serial.println(eepromOk ? "OK" : "ERROR");

    Serial.println();
    Serial.println("LittleFS");
    const bool littleFsOk = formatLittleFs();
    Serial.print("  Format          : ");
    Serial.println(littleFsOk ? "OK" : "ERROR");

    printDeviceIdentity();

    Serial.println();
    Serial.println("==================================================");
    if (eepromOk && littleFsOk)
    {
        Serial.println("  DEVICE READY FOR PROVISIONING");
    }
    else
    {
        Serial.println("  FACTORY INITIALIZATION FAILED");
    }
    Serial.println("==================================================");
}
}  // namespace

void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    delay(100);

    Serial.println();
    Serial.println("==================================================");
    Serial.println("               CatFeeder Factory");
    Serial.println("==================================================");

    runFactoryReset();
}

void loop()
{
    delay(1000);
}

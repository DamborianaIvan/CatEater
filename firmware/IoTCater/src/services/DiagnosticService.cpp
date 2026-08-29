#include "services/DiagnosticService.h"

bool DiagnosticService::begin()
{
    if (!LittleFS.begin())
    {
        return false;
    }

    return true;
}

void DiagnosticService::rotateIfNeeded(size_t nextEntrySize)
{
    if (!LittleFS.exists(DIAGNOSTIC_FILE))
    {
        return;
    }

    File file = LittleFS.open(DIAGNOSTIC_FILE, "r");
    if (!file)
    {
        return;
    }

    const size_t currentSize = file.size();
    file.close();

    if (currentSize + nextEntrySize > MAX_DIAGNOSTIC_FILE_SIZE)
    {
        LittleFS.remove(DIAGNOSTIC_FILE);
    }
}

void DiagnosticService::record(const char* code)
{
    record(code, "");
}

void DiagnosticService::record(const char* code, const char* detail)
{
    if (!code || !detail)
    {
        return;
    }

    const unsigned long timestamp = millis();
    const size_t entrySize = strlen(code) + strlen(detail) + 32;
    rotateIfNeeded(entrySize);

    File file = LittleFS.open(DIAGNOSTIC_FILE, "a");
    if (!file)
    {
        return;
    }

    file.print(timestamp);
    file.print('|');
    file.print(code);

    if (detail[0] != '\0')
    {
        file.print('|');
        file.print(detail);
    }

    file.println();
    file.close();
}

#pragma once

#include <Arduino.h>
#include <LittleFS.h>

class DiagnosticService
{
   public:
    bool begin();
    void record(const char* code);
    void record(const char* code, const char* detail);
    String read();
    void clear();

   private:
    static constexpr const char* DIAGNOSTIC_FILE = "/diagnostics.log";
    static constexpr size_t MAX_DIAGNOSTIC_FILE_SIZE = 4096;

    void rotateIfNeeded(size_t nextEntrySize);
};

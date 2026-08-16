#pragma once

#include <Arduino.h>

class ConfigurationRevisionStorage
{
   public:
    bool load(uint32_t& revision);
    bool save(uint32_t revision);
    bool clear();

   private:
    static constexpr char REVISION_FILE[] = "/config_revision.json";
};
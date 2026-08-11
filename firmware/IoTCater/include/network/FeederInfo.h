#pragma once

#include <Arduino.h>

struct FeederInfo
{
    int feederQuantity = 0;
    String feederName;
    String feederLogo;
    String lastConnection;
};
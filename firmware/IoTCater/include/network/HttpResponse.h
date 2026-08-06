#pragma once

#include <Arduino.h>

struct HttpResponse
{
    bool success = false;
    int statusCode = -1;
    String body;
};
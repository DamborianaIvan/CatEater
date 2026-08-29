#pragma once

#include <Arduino.h>

struct HttpResponse
{
    // True when the HTTP request reached the server and returned a valid HTTP
    // status code. This does not mean the HTTP operation itself succeeded.
    bool transportSuccess = false;

    int statusCode = -1;
    String body;

    bool isHttpSuccess() const
    {
        return transportSuccess && statusCode >= 200 && statusCode < 300;
    }
};

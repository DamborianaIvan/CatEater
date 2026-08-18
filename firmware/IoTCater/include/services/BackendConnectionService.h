#pragma once

#include <Arduino.h>

class BackendConnectionService
{
   public:
    void begin();

    bool canAttempt() const;

    void recordSuccess();
    void recordFailure();

    bool isAvailable() const;

   private:
    static constexpr unsigned long INITIAL_BACKOFF_MS = 10000;
    static constexpr unsigned long MAX_BACKOFF_MS = 120000;

    bool _available = false;
    unsigned long _nextAttempt = 0;
    unsigned long _backoffMs = INITIAL_BACKOFF_MS;
    bool _hasAttempted = false;
};

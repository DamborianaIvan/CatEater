#include "services/BackendConnectionService.h"

void BackendConnectionService::begin()
{
    _available = false;
    _nextAttempt = 0;
    _backoffMs = INITIAL_BACKOFF_MS;
    _hasAttempted = false;
}

bool BackendConnectionService::canAttempt() const
{
    if (!_hasAttempted)
    {
        return true;
    }

    return static_cast<long>(millis() - _nextAttempt) >= 0;
}

void BackendConnectionService::recordSuccess()
{
    _available = true;
    _hasAttempted = true;
    _backoffMs = INITIAL_BACKOFF_MS;
    _nextAttempt = millis();
}

void BackendConnectionService::recordFailure()
{
    _available = false;
    _hasAttempted = true;
    _nextAttempt = millis() + _backoffMs;

    if (_backoffMs < MAX_BACKOFF_MS)
    {
        _backoffMs *= 2;

        if (_backoffMs > MAX_BACKOFF_MS)
        {
            _backoffMs = MAX_BACKOFF_MS;
        }
    }
}

bool BackendConnectionService::isAvailable() const
{
    return _available;
}

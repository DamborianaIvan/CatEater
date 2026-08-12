#include "FeedingService.h"

FeedingService::FeedingService(Motor& motor) : _motor(motor) {}

bool FeedingService::feed(int portions)
{
    if (portions <= 0)
    {
        return false;
    }

    return _motor.feed(portions);
}
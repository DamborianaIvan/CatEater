#pragma once

#include "services/FeedingService.h"

class ButtonService
{
   public:
    ButtonService(FeedingService& feedingService, uint8_t pin);

    void begin();
    void update();

   private:
    FeedingService& _feedingService;
    uint8_t _pin;

    bool _lastState = HIGH;
    bool _lastDebouncedState = HIGH;
    unsigned long _lastStateChangeAt = 0;

    static constexpr unsigned long DEBOUNCE_MS = 50;
    static constexpr int DEFAULT_PORTIONS = 1;
};

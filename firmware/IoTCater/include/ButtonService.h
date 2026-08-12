#pragma once

#include "FeedingService.h"

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

    static constexpr int DEFAULT_PORTIONS = 1;
};
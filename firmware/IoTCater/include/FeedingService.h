#pragma once

#include "Motor.h"

class FeedingService
{
   public:
    explicit FeedingService(Motor& motor);

    bool feed(int portions);

   private:
    Motor& _motor;
};
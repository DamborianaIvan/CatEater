#pragma once

#include "Motor.h"
#include "FeedingEvent.h"
#include "FeedingService.h"
#include "FeedingHistoryService.h"

class FeedingService
{
   public:
    explicit FeedingService(Motor& motor, FeedingHistoryService& historyService);

    bool feed(int portions, FeedingSource source);
    bool isFeeding() const;

   private:
    Motor& _motor;
    FeedingHistoryService& _historyService;
};
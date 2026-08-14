#pragma once

#include "hardware/Motor.h"
#include "domain/FeedingEvent.h"
#include "services/FeedingService.h"
#include "services/FeedingHistoryService.h"
#include "services/TimeService.h"

class FeedingService
{
   public:
    explicit FeedingService(Motor& motor, FeedingHistoryService& historyService,
                            TimeService& timeService);

    bool feed(int portions, FeedingSource source);
    bool isFeeding() const;

   private:
    Motor& _motor;
    FeedingHistoryService& _historyService;
    TimeService& _timeService;
};
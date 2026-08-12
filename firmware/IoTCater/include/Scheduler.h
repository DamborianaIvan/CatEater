#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "TimeService.h"
#include "SchedulerConstants.h"
#include "FeedSchedule.h"
#include "Configuration.h"
#include "FeedingService.h"

class Scheduler
{
   public:
    Scheduler(TimeService& timeService, FeedingService& feedingService,
              Configuration& configuation);

    bool begin();

    void update();

    bool setSchedule(int hour, int minute);

   private:
    TimeService& _timeService;
    FeedingService& _feedingService;
    Configuration& _configuration;

    bool isScheduledTime(const FeedSchedule& schedule) const;
    bool wasExecutedThisMinute() const;
    bool _executionRegistered;
    bool hasEnabledSchedules() const;
    bool isValidSchedule(int hour, int minute) const;
    void markExecution();

    int _lastExecutionHour;
    int _lastExecutionMinute;
};
#endif
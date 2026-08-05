#ifndef SCHEDULER_H
#define SCHEDULER_H


#include "TimeService.h"
#include "Motor.h"
#include "SchedulerConstants.h"
#include "FeedSchedule.h"
#include "Configuration.h"


class Scheduler
{
public:
    Scheduler(TimeService& timeService, Motor& motor, Configuration& configuation);

    bool begin();

    void update();

    bool setSchedule(int hour,int minute);

private:
    TimeService& _timeService;
    Motor& _motor;
    Configuration& _configuration;

    bool isScheduledTime(const FeedSchedule& schedule) const;
    bool wasExecutedThisMinute() const;
    bool _executionRegistered;
    bool hasEnabledSchedules() const;
    bool isValidSchedule( int hour, int minute) const;
    void markExecution();
    
    int _lastExecutionHour;
    int _lastExecutionMinute;
};
#endif
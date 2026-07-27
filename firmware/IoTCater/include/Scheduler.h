#ifndef SCHEDULER_H
#define SCHEDULER_H


#include "TimeService.h"
#include "Motor.h"
#include "SchedulerConstants.h"
#include "FeedSchedule.h"


class Scheduler
{
public:
    Scheduler(TimeService& timeService, Motor& motor);

    bool begin();

    void update();

    bool setSchedule(int hour,int minute);
    bool configure(int hour, int minute);

private:
    TimeService& _timeService;
    Motor& _motor;
    FeedSchedule _schedules[MAX_SCHEDULES];

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
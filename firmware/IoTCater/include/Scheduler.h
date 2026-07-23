#include "TimeService.h"
#include "Motor.h"

class Scheduler
{
public:
    Scheduler(TimeService& timeService, Motor& motor);

    bool begin();

    void update();

    bool setSchedule(int hour,int minute);

private:
    TimeService& _timeService;
    Motor& _motor;

    bool isScheduledTime() const;
    bool wasExecutedThisMinute() const;
    bool _executionRegistered;
    bool _scheduleConfigured;
    bool isValidSchedule( int hour, int minute) const;
    void markExecution();

    static constexpr int DEFAULT_FEED_PORTIONS = 1;
    
    int _scheduledHour;
    int _scheduledMinute;
    int _lastExecutionHour;
    int _lastExecutionMinute;
};

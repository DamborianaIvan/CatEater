#include "Scheduler.h"
#include "Motor.h"
Scheduler::Scheduler(
    TimeService& timeService,
    Motor& motor
)
    : _timeService(timeService),
      _motor(motor),
      _executionRegistered(false),
      _scheduleConfigured(false),
      _scheduledHour(0),
      _scheduledMinute(0),
      _lastExecutionHour(0),
      _lastExecutionMinute(0)
{
}

bool Scheduler::begin()
{
    Serial.println("Scheduler iniciado.");
    return true;
}

bool Scheduler::setSchedule(
    int hour,
    int minute
)
{
    if (hour < 0 || hour > 23)
    {
        return false;
    }

    if (minute < 0 || minute > 59)
    {
        return false;
    }

    _scheduledHour = hour;
    _scheduledMinute = minute;

    _scheduleConfigured = true;
    _executionRegistered = false;
    _lastExecutionHour = 0;
    _lastExecutionMinute = 0;

    Serial.printf(
    "Horario configurado: %02d:%02d\n",
    hour,
    minute
);
    return true;
}

bool Scheduler::isScheduledTime() const
{
    return _timeService.getHour() == _scheduledHour &&
           _timeService.getMinute() == _scheduledMinute;
}

bool Scheduler::wasExecutedThisMinute() const
{
    return _timeService.getHour() == _lastExecutionHour &&
           _timeService.getMinute() == _lastExecutionMinute;
}

void Scheduler::markExecution()
{
    _lastExecutionHour = _timeService.getHour();
    _lastExecutionMinute = _timeService.getMinute();
}
bool Scheduler::isValidSchedule(int hour, int minute) const
{
    return hour >= 0 && hour <= 23 &&
           minute >= 0 && minute <= 59;
}

void Scheduler::update()
{
    if (!_scheduleConfigured)
    {
        return;
    }

    if (!_timeService.isTimeAvailable())
    {
        return;
    }

    if (!isScheduledTime())
    {
        return;
    }

    if (wasExecutedThisMinute())
    {
        return;
    }

    if (_motor.feed(DEFAULT_FEED_PORTIONS))
    {   
        markExecution();
        _executionRegistered = true;
        Serial.println("Alimentación programada ejecutada.");
    }
    else
    {
        Serial.println("No fue posible ejecutar la alimentación programada.");
    }
}
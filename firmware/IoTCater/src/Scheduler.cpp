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
      _lastExecutionHour(0),
      _lastExecutionMinute(0)
{
}

bool Scheduler::begin()
{
    Serial.println("Scheduler iniciado.");
    return true;
}

bool Scheduler::setSchedule(int hour,int minute)
{
    if (hour < 0 || hour > 23)
    {
        return false;
    }

    if (minute < 0 || minute > 59)
    {
        return false;
    }

    //Aca estamos guardamos el valor en el struct que viene referenciado (revisar en .h)
    _schedules[0].hour = hour;
    _schedules[0].minute = minute;
    _schedules[0].portions = 1;
    _schedules[0].enabled = true;

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

bool Scheduler::configure(int hour, int minute)
{
    if (!isValidSchedule(hour, minute))
    {
        return false;
    }
        return setSchedule(hour, minute);
}

bool Scheduler::isScheduledTime(const FeedSchedule& schedule) const
{
    return schedule.enabled &&
           _timeService.getHour() == schedule.hour &&
           _timeService.getMinute() == schedule.minute;
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

bool Scheduler::hasEnabledSchedules() const
{
    for (const FeedSchedule& schedule : _schedules)
    {
        if (schedule.enabled)
        {
            return true;
        }
    }

    return false;
}

void Scheduler::update()
{
    if (!hasEnabledSchedules())
    {
        return;
    }

    if (!_timeService.isTimeAvailable())
    {
        return;
    }

    if (wasExecutedThisMinute())
    {
        return;
    }

    for (uint8_t i = 0; i < MAX_SCHEDULES; ++i)
    {
        const FeedSchedule& schedule = _schedules[i];

        if (!isScheduledTime(schedule))
        {
            continue;
        }

        if (_motor.feed(schedule.portions))
        {
            markExecution();

            Serial.printf(
                "Alimentación programada ejecutada (Horario %u).\n",
                i
            );
        }
        else
        {
            Serial.printf(
                "No fue posible ejecutar el horario %u.\n",
                i
            );
        }

        break;
    }
}
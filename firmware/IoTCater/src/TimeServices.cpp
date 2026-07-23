#include "TimeService.h"
#include "WifiServices.h"
#include <Arduino.h>
#include <time.h>

TimeService::TimeService(WiFiService& wifi)
    : _wifi(wifi),
      _timeAvailable(false),
      _ntpStarted(false)
{
}

void TimeService::begin()
{
    Serial.println("TimeService inicializado.");
}



void TimeService::update()
{
     if (_timeAvailable)
    {
        return;
    }
    if (!_wifi.isConnected())
    {
        return;
    }
    if (!_ntpStarted)
    {
        
        initializeNtp();
        _ntpStarted = true;
        Serial.println("NTP iniciado.");
    }
    time_t currentTime = time(nullptr);

    if (currentTime < MIN_VALID_UNIX_TIME)
    {
        return;
    }

    _timeAvailable = true;

    Serial.println("Hora sincronizada.");
    Serial.print("TZ = ");
    Serial.println(getenv("TZ"));
}

void TimeService::initializeNtp(){
        configTime(
            ARGENTINA_UTC_OFFSET,
            0,
            "pool.ntp.org",
            "time.nist.gov"
        );
}
bool TimeService::isTimeAvailable() const
{
    return _timeAvailable;
}

bool TimeService::getLocalTime(tm& timeInfo) const
{
    if (!_timeAvailable)
    {
        return false;
    }

    time_t currentTime = time(nullptr);
    
    localtime_r(&currentTime, &timeInfo);

    return true;
}

int TimeService::getHour() const
{
    tm timeInfo;

    if (!getLocalTime(timeInfo))
    {
        return -1;
    }

    return timeInfo.tm_hour;
}

int TimeService::getMinute() const
{
    tm timeInfo;

    if (!getLocalTime(timeInfo))
    {
        return -1;
    }

    return timeInfo.tm_min;
}

int TimeService::getSecond() const
{
    tm timeInfo;

    if (!getLocalTime(timeInfo))
    {
        return -1;
    }

    return timeInfo.tm_sec;
}
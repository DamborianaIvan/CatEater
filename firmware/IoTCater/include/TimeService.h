#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H
#include "WiFiServices.h"

class TimeService
{
public:
    explicit TimeService(WiFiService& wifi);

    void begin();
    [[nodiscard]]
    bool isTimeAvailable() const;

    void update();


    int getHour() const;
    int getMinute() const;
    int getSecond() const;
private:
    WiFiService& _wifi; 
    static constexpr time_t MIN_VALID_UNIX_TIME = 100000;
    static constexpr long ARGENTINA_UTC_OFFSET = -3 * 3600;
    bool getLocalTime(tm& timeInfo) const;
    bool _timeAvailable;
    bool _ntpStarted;

    void initializeNtp();
   

};

#endif
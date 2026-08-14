#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "domain/FeedSchedule.h"
#include "config/SchedulerConstants.h"

struct Configuration
{
    static constexpr int DEFAULT_STEPS_PER_FEED = 2048;
    static constexpr uint8_t DEFAULT_FEED_PORTIONS = 1;
    static constexpr uint8_t DEFAULT_SCHEDULE_HOUR = 9;
    static constexpr uint8_t DEFAULT_SCHEDULE_MINUTE = 0;

    Configuration();

    int stepsPerFeed;

    FeedSchedule schedules[MAX_SCHEDULES];

   private:
    void initializeDefaultSchedules();
};

#endif
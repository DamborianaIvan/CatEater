#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "domain/FeedSchedule.h"
#include "config/SchedulerConstants.h"

struct Configuration
{
    static constexpr int DEFAULT_STEPS_PER_FEED = 2048;
    static constexpr int MIN_STEPS_PER_FEED = 1;
    // Bound calibration to the project's existing maximum configured portions.
    static constexpr int MAX_STEPS_PER_FEED = DEFAULT_STEPS_PER_FEED * MAX_SCHEDULES;
    static constexpr uint8_t DEFAULT_FEED_PORTIONS = 1;
    // This matches the maximum number of independently configured schedules.
    static constexpr uint8_t MIN_FEED_PORTIONS = 1;
    static constexpr uint8_t MAX_FEED_PORTIONS = MAX_SCHEDULES;
    static constexpr uint8_t DEFAULT_SCHEDULE_HOUR = 9;
    static constexpr uint8_t DEFAULT_SCHEDULE_MINUTE = 0;

    Configuration();

    int stepsPerFeed;

    FeedSchedule schedules[MAX_SCHEDULES];

    static bool isValidStepsPerFeed(int stepsPerFeed);
    static bool isValidPortions(int portions);

   private:
    void initializeDefaultSchedules();
};

#endif

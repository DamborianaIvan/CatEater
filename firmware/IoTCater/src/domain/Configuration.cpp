#include "domain/Configuration.h"

Configuration::Configuration() : stepsPerFeed(DEFAULT_STEPS_PER_FEED)
{
    initializeDefaultSchedules();
}

void Configuration::initializeDefaultSchedules()
{
    schedules[0].hour = DEFAULT_SCHEDULE_HOUR;
    schedules[0].minute = DEFAULT_SCHEDULE_MINUTE;
    schedules[0].portions = DEFAULT_FEED_PORTIONS;
    schedules[0].enabled = true;
}
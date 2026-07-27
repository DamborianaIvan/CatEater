#ifndef FEED_SCHEDULE_H
#define FEED_SCHEDULE_H

struct FeedSchedule
{
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t portions = 1;
    bool enabled = false;
};

#endif
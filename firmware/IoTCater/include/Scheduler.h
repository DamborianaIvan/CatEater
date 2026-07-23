class Scheduler
{
public:

    Scheduler(
        Motor& motor,
        TimeService& timeService
    );

    void update();
};
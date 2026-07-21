class Scheduler
{
public:
    Scheduler(Motor& motor);

    void begin();

    void update();

    void addSchedule(int hour, int minute);

private:
    Motor& motor;
};
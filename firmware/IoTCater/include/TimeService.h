class TimeService
{
public:

    bool begin();

    void update();

    bool isTimeAvailable() const;

    int getHour() const;

    int getMinute() const;
};
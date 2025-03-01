#ifndef TIMER_HELPER_H_
#define TIMER_HELPER_H_

#include <chrono>

class TimerHelper
{
public:
    TimerHelper();
    ~TimerHelper();

    void start();
    void stop();
    double elapsed_time();
    void fps();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_end;
};

#endif // TIMER_HELPER_H_
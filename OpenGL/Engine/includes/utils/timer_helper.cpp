#include "timer_helper.h"
// #include "Logger.h"


// constructor
TimerHelper::TimerHelper()
{
    m_start = std::chrono::high_resolution_clock::now();
}

// destructor
TimerHelper::~TimerHelper()
{
    m_end = std::chrono::high_resolution_clock::now();
}

// start the timer
void TimerHelper::start()
{
    m_start = std::chrono::high_resolution_clock::now();
}

// stop the timer
void TimerHelper::stop()
{
    m_end = std::chrono::high_resolution_clock::now();
}

// calculate the elapsed time
double TimerHelper::elapsed_time()
{
    std::chrono::duration<double> elapsed = m_end - m_start;
    return elapsed.count();
}

void TimerHelper::fps()
{

}
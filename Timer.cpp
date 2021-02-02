#include <windows.h>
#include "Timer.h"
#include "linterpolation.h"


void Timer::Start()
{
    m_StartTime = std::chrono::high_resolution_clock::now();
}

float Timer::GetDuration()
{
    std::chrono::duration<float> duration = std::chrono::high_resolution_clock::now() - m_StartTime;
    return duration.count();
}

void Timer::SetInterval( linterpolation inputFunc, std::vector<dot>& dots, double k)
{
    for (int i = 1; i <= dots.size()-1; i++)
    {
        inputFunc.SetPos(dots[i-1], dots[i], k);
    }
}
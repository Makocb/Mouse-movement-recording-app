#pragma once

#include <chrono>
#include <vector>
#include "dot.h"
class linterpolation;
class dot;

class Timer
{
private:
    std::chrono::time_point<std::chrono::steady_clock> m_StartTime;

public:
    void Start();

    float GetDuration();

    void SetInterval(int interval, linterpolation inputFunc, std::vector<dot>& dots, double k);
};


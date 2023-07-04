#ifndef LIMITER_H
#define LIMITER_H

#include <queue>
#include <chrono>
#include <mutex>

class Limiter
{
public:
    bool allow_request();

private:
    std::queue<std::chrono::steady_clock::time_point> requests_time_points;
    std::mutex mutex;
};

#endif
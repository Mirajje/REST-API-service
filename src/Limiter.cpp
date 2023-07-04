#include "Limiter.h"

bool Limiter::allow_request()
{
    auto now = std::chrono::steady_clock::now();

    std::unique_lock<std::mutex> lock(mutex);

    if (requests_time_points.size() < 10)
    {
        requests_time_points.push(now);
        return true;
    }

    if (std::chrono::duration_cast<std::chrono::seconds>(now - requests_time_points.front()) <= std::chrono::seconds(1))
        return false;
    else
    {
        while (!requests_time_points.empty() && std::chrono::duration_cast<std::chrono::seconds>(now - requests_time_points.front()) > std::chrono::seconds(1))
            requests_time_points.pop();
        requests_time_points.push(now);
        return true;
    }
}
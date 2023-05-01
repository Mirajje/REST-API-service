#include <iostream>
#include <Crow.h>
#include <future>
#include <chrono>
#include <vector>
#include <pqxx/pqxx>
#include "Database.h"
#include <mutex>
#include <queue>

class Limiter 
{
public:
    bool allow_request() 
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

private:
    std::queue<std::chrono::steady_clock::time_point> requests_time_points;
    std::mutex mutex;
};

int main(int argc, char* argv[])
{
    Database db;
     
    crow::SimpleApp app;
    app.loglevel(crow::LogLevel::Warning);

    crow::json::wvalue json;

    std::unordered_map<std::string, Limiter> rateLimiter;

    CROW_ROUTE(app, "/couriers") 
        .methods(crow::HTTPMethod::POST) 
        ([&db, &rateLimiter](const crow::request& req) 
            {
                if (!rateLimiter[req.url].allow_request()) return crow::response(429);

                crow::json::wvalue output;
                try
                {
                    crow::json::rvalue body = crow::json::load(req.body);
                    output = std::move(db.post_couriers(body));

                    return crow::response(200, output);
                }
                catch (const std::exception& error)
                {
                    output = crow::json::load("{}");
                    return crow::response(400, output);
                }
            }
        );

    CROW_ROUTE(app, "/couriers")
        .methods(crow::HTTPMethod::GET)
        ([&db, &rateLimiter](const crow::request& req)
            {
                if (!rateLimiter[req.url].allow_request()) return crow::response(429);

                crow::json::wvalue output;
                try
                {
                    int limit = req.url_params.get("limit") == nullptr ? 1 : std::stoi(req.url_params.get("limit"));
                    int offset = req.url_params.get("offset") == nullptr ? 0 : std::stoi(req.url_params.get("offset"));

                    output = std::move(db.get_couriers(limit, offset));

                    if (!output.count("couriers"))
                    {
                        output = crow::json::load("{\"couriers\":[]}");
                        return crow::response(200, output);
                    }

                    output["limit"] = limit;
                    output["offset"] = offset;

                    return crow::response(200, output);
                }
                catch (const std::exception& error)
                {
                    output = crow::json::load("{}");
                    return crow::response(400, output);
                }
            }
        );

    CROW_ROUTE(app, "/couriers/<int>")
        .methods(crow::HTTPMethod::GET)
        ([&db, &rateLimiter](const crow::request& req, int courier_id)
            {
                if (!rateLimiter[req.url].allow_request()) return crow::response(429);

                crow::json::wvalue output;
                try
                {
                    output = std::move(db.get_couriers(1, courier_id - 1));

                    if (!output.count("couriers") || courier_id < 1)
                    {
                        output = crow::json::load("{}");
                        return crow::response(404, output);
                    }
                        
                    return crow::response(200, output);
                }
                catch (const std::exception& error)
                {
                    output = crow::json::load("{}");
                    return crow::response(400, output);
                }
            }
        );

    CROW_ROUTE(app, "/orders")
        .methods(crow::HTTPMethod::POST)
        ([&db, &rateLimiter](const crow::request& req)
            {
                if (!rateLimiter[req.url].allow_request()) return crow::response(429);

                crow::json::wvalue output;
                try 
                {
                    crow::json::rvalue body = crow::json::load(req.body);
                    output = std::move(db.post_orders(body));

                    return crow::response(200, output);
                }
                catch (const std::exception& error)
                {
                    output = crow::json::load("{}");
                    return crow::response(400, output);
                }
            }
        );

    CROW_ROUTE(app, "/orders")
        .methods(crow::HTTPMethod::GET)
        ([&db, &rateLimiter](const crow::request& req)
            {
                if (!rateLimiter[req.url].allow_request()) return crow::response(429);

                crow::json::wvalue output;
                try
                {
                    int limit = req.url_params.get("limit") == nullptr ? 1 : std::stoi(req.url_params.get("limit"));
                    int offset = req.url_params.get("offset") == nullptr ? 0 : std::stoi(req.url_params.get("offset"));

                    output = std::move(db.get_orders(limit, offset));

                    if (output.dump() == "null")
                    {
                        output = crow::json::load("{\"orders\":[]}");
                        return crow::response(200, output);
                    }

                    return crow::response(200, output);
                }
                catch (const std::exception& error)
                {
                    output = crow::json::load("{}");
                    return crow::response(400);
                }
                    
            }
    );

    CROW_ROUTE(app, "/orders/<int>")
        .methods(crow::HTTPMethod::GET)
        ([&db, &rateLimiter](const crow::request& req, int order_id)
            {
                if (!rateLimiter[req.url].allow_request()) return crow::response(429);

                crow::json::wvalue output;
                try
                {
                    output = std::move(db.get_orders(1, order_id - 1));

                    if (!output.size() || order_id < 1)
                    {
                        output = crow::json::load("{}");
                        return crow::response(404, output);
                    }

                    return crow::response(200, output);
                }
                catch (const std::exception& error)
                {
                    output = crow::json::load("{}");
                    return crow::response(400, output);
                }

            }
        );
    
    app.validate();
    app.port(18080).multithreaded().run_async();
}
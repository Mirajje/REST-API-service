#include "Routes.h"

Routes::Routes(crow::SimpleApp& app)
	: m_App(app) {};

void Routes::couriers_routes()
{
    CROW_ROUTE(m_App, "/couriers")
        .methods(crow::HTTPMethod::POST)
        ([this](const crow::request& req)
            {
                if (!rateLimiter[req.url].allow_request()) return crow::response(429);

                crow::json::wvalue output;
                try
                {
                    output = std::move(db.post_couriers(crow::json::load(req.body)));

                    return crow::response(200, output);
                }
                catch (const std::exception& error)
                {
                    output = crow::json::load("{}");
                    return crow::response(400, output);
                }
            }
    );

    CROW_ROUTE(m_App, "/couriers")
        .methods(crow::HTTPMethod::GET)
        ([this](const crow::request& req)
            {
                if (!rateLimiter[req.url].allow_request()) return crow::response(429);

                crow::json::wvalue output;
                try
                {
                    int limit = req.url_params.get("limit") == nullptr ? 1 : std::stoi(req.url_params.get("limit"));
                    int offset = req.url_params.get("offset") == nullptr ? 0 : std::stoi(req.url_params.get("offset"));

                    output = std::move(db.get_couriers(limit, offset));

                    if (output.dump() == "null")
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

    CROW_ROUTE(m_App, "/couriers/<int>")
        .methods(crow::HTTPMethod::GET)
        ([this](const crow::request& req, int courier_id)
            {
                if (!rateLimiter[req.url].allow_request()) return crow::response(429);

                crow::json::wvalue output;
                try
                {
                    output = std::move(db.get_couriers(1, courier_id - 1));

                    if (output.dump() == "null" || courier_id < 1)
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

    CROW_ROUTE(m_App, "/couriers/meta-info/<int>")
        .methods(crow::HTTPMethod::GET)
        ([this](const crow::request& req, int courier_id)
            {
                if (!rateLimiter[req.url].allow_request()) return crow::response(429);

                crow::json::wvalue output;
                try
                {
                    std::string start_date = req.url_params.get("startDate");
                    std::string end_date = req.url_params.get("endDate");

                    output = db.courier_meta_info(courier_id, start_date, end_date);

                    return crow::response(200, output);
                }
                catch (const std::exception& error)
                {
                    output = crow::json::load("{}");
                    return crow::response(400, output);
                }

            }
    );
}

void Routes::orders_routes()
{
    CROW_ROUTE(m_App, "/orders")
        .methods(crow::HTTPMethod::POST)
        ([this](const crow::request& req)
            {
                if (!rateLimiter[req.url].allow_request()) return crow::response(429);

                crow::json::wvalue output;
                try
                {
                    output = std::move(db.post_orders(crow::json::load(req.body)));

                    return crow::response(200, output);
                }
                catch (const std::exception& error)
                {
                    output = crow::json::load("{}");
                    return crow::response(400, output);
                }
            }
    );

    CROW_ROUTE(m_App, "/orders")
        .methods(crow::HTTPMethod::GET)
        ([this](const crow::request& req)
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

    CROW_ROUTE(m_App, "/orders/<int>")
        .methods(crow::HTTPMethod::GET)
        ([this](const crow::request& req, int order_id)
            {
                if (!rateLimiter[req.url].allow_request()) return crow::response(429);

                crow::json::wvalue output;
                try
                {
                    output = std::move(db.get_orders(1, order_id - 1));

                    if (output.dump() == "null" || order_id < 1)
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

    CROW_ROUTE(m_App, "/orders/complete")
        .methods(crow::HTTPMethod::POST)
        ([this](const crow::request& req)
            {
                if (!rateLimiter[req.url].allow_request()) return crow::response(429);

                crow::json::wvalue output;
                try
                {
                    output = std::move(db.orders_complete(crow::json::load(req.body)));

                    if (output.dump() == "null")
                    {
                        output = crow::json::load("{}");
                        return crow::response(400, output);
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
}
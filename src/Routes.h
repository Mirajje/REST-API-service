#ifndef ROUTES_H
#define ROUTES_H

#include "Database.h"
#include "Limiter.h"
#include <crow.h>

class Routes
{
public:
	Routes(crow::SimpleApp&);
	void couriers_routes();
	void orders_routes();

private:
	Database db;
	std::unordered_map<std::string, Limiter> rateLimiter;
	crow::SimpleApp& m_App;

};

#endif
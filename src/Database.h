#ifndef DATABASE
#define DATABASE

#include <pqxx/pqxx>
#include <crow.h>

class Database
{
public:
	Database();
	~Database();

	crow::json::wvalue post_couriers(const crow::json::rvalue&);
	crow::json::wvalue get_couriers(int limit, int offset);

	crow::json::wvalue post_orders(const crow::json::rvalue&);
	crow::json::wvalue get_orders(int limit, int offset);
private:
	pqxx::connection C;
};

#endif 

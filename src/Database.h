#ifndef DATABASE
#define DATABASE

#include <pqxx/pqxx>
#include <crow.h>

class Database
{
public:
	Database();
	~Database();

	crow::json::wvalue post_couriers(const crow::json::rvalue& new_data);
	crow::json::wvalue get_couriers(int limit, int offset);
	crow::json::wvalue courier_meta_info(int courier_id, const std::string& start_date, const std::string& end_date);

	crow::json::wvalue post_orders(const crow::json::rvalue& new_data);
	crow::json::wvalue get_orders(int limit, int offset);
	crow::json::wvalue orders_complete(const crow::json::rvalue& data);

private:
	pqxx::connection C;
};

#endif 

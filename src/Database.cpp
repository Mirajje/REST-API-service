#include "Database.h"

Database::Database()
	: C("dbname = postgres user = postgres password = 123 hostaddr = 127.0.0.1 port = 5432") 
{
	std::string query;

	try
	{	
		pqxx::work W(C);
		query = "CREATE DATABASE DB";
		W.exec(query);
		W.commit();
	}
	catch (std::exception& e) {}; // in case DB exists an error will be thrown

	C.close();
	C = pqxx::connection("dbname = DB user = postgres password = 123 hostaddr = 127.0.0.1 port = 5432");

	query = "CREATE TABLE IF NOT EXISTS couriers(courier_id SERIAL PRIMARY KEY, courier_type VARCHAR(7), regions JSON, working_hours JSON); \
				CREATE TABLE IF NOT EXISTS orders(order_id SERIAL PRIMARY KEY, weight FLOAT, regions INT, delivery_hours JSON, cost INT, completed_time varchar(100));";																\

	pqxx::work W(C);
	W.exec(query);
	W.commit();
};

Database::~Database()
{
	C.close();
}

crow::json::wvalue Database::get_couriers(int limit, int offset)
{
	pqxx::nontransaction N(C);
	std::string query;
	crow::json::wvalue output;

	if (limit < -1 || offset < 0)
		return output;

	if (limit == -1)
		query = "SELECT courier_id, courier_type, regions, working_hours FROM couriers WHERE courier_id > " + std::to_string(offset);
	else
		query = "SELECT courier_id, courier_type, regions, working_hours FROM couriers WHERE courier_id > " + std::to_string(offset) + " LIMIT " + std::to_string(limit);

	pqxx::result res(N.exec(query));

	int i = 0;
	for (pqxx::result::const_iterator it = res.begin(); it != res.end(); ++it)
	{
		output["couriers"][i]["courier_id"] = it[0].as<int>();
		output["couriers"][i]["courier_type"] = it[1].as<std::string>();
		output["couriers"][i]["regions"] = crow::json::load(it[2].c_str());
		output["couriers"][i]["working_hours"] = crow::json::load(it[3].c_str());
		++i;
	}

	return output;
}

crow::json::wvalue Database::post_couriers(const crow::json::rvalue& new_data)
{
	for (int i = 0; i < new_data["couriers"].size(); ++i)
	{
		std::string type = crow::json::wvalue(new_data["couriers"][i]["courier_type"]).dump();
		std::string reg = crow::json::wvalue(new_data["couriers"][i]["regions"]).dump();
		std::string hours = crow::json::wvalue(new_data["couriers"][i]["working_hours"]).dump();

		type = type.substr(1, type.size() - 2);

		std::string query = "INSERT INTO couriers(courier_type, regions, working_hours) VALUES('" + type + "', '" + reg + "', '" + hours + "');";

		pqxx::work W(C);
		W.exec(query);
		W.commit();
	}

	return get_couriers(-1, 0);
}

crow::json::wvalue Database::get_orders(int limit, int offset)
{
	pqxx::nontransaction N(C);
	std::string query;
	crow::json::wvalue output;

	if (limit < -1 || offset < 0)
		return output;

	if (limit == -1)
		query = "SELECT order_id, weight, regions, delivery_hours, cost, completed_time FROM orders WHERE order_id > " + std::to_string(offset);
	else
		query = "SELECT order_id, weight, regions, delivery_hours, cost, completed_time FROM orders WHERE order_id > " + std::to_string(offset) + " LIMIT " + std::to_string(limit);

	pqxx::result res(N.exec(query));

	int i = 0;

	for (pqxx::result::const_iterator it = res.begin(); it != res.end(); ++it)
	{
		output[i]["order_id"] = it[0].as<int>();
		output[i]["weight"] = it[1].as<float>();
		output[i]["regions"] = it[2].as<int>();
		output[i]["delivery_hours"] = crow::json::load(it[3].c_str());
		output[i]["cost"] = it[4].as<int>();
		if (!it[5].is_null())
			output[i]["completed_time"] = it[5].as<std::string>();

		++i;
	}

	return output;
}

crow::json::wvalue Database::post_orders(const crow::json::rvalue& new_data)
{
	for (int i = 0; i < new_data["orders"].size(); ++i)
	{
		std::string weight = crow::json::wvalue(new_data["orders"][i]["weight"]).dump();
		std::string regions = crow::json::wvalue(new_data["orders"][i]["regions"]).dump();
		std::string hours = crow::json::wvalue(new_data["orders"][i]["delivery_hours"]).dump();
		std::string cost = crow::json::wvalue(new_data["orders"][i]["cost"]).dump();

		std::string query = "INSERT INTO orders(weight, regions, delivery_hours, cost) VALUES(" + weight + ", " + regions + ", '" + hours + "', "
			+ cost + ");";

		pqxx::work W(C);
		W.exec(query);
		W.commit();
	}

	crow::json::wvalue output;
	output["orders"] = get_orders(-1, 0);
	return output;
}


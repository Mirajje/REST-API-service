#include "Database.h"

Database::Database()
	: C("dbname = postgres user = postgres password = 123 hostaddr = 127.0.0.1 port = 5432") 
{
	std::string query;

	try
	{	
		pqxx::work W(C);
		query = "CREATE DATABASE db";
		W.exec(query);
		W.commit();
	}
	catch (std::exception& e) {}; // in case db exists an error will be thrown

	C.close();
	C = pqxx::connection("dbname = db user = postgres password = 123 hostaddr = 127.0.0.1 port = 5432");

	query = "CREATE TABLE IF NOT EXISTS couriers(courier_id SERIAL PRIMARY KEY, courier_type VARCHAR(7), regions JSON, working_hours JSON); \
			 CREATE TABLE IF NOT EXISTS orders(order_id SERIAL PRIMARY KEY, weight FLOAT, regions INT, delivery_hours JSON, cost INT, completed_time varchar(100)); \
		     CREATE TABLE IF NOT EXISTS order_courier(order_id INT REFERENCES orders(order_id) PRIMARY KEY, courier_id INT REFERENCES couriers(courier_id), date date, completed bool); ";

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
		output[i]["delivery_hours"] = crow::json::load(it[3].as<std::string>());
		output[i]["cost"] = it[4].as<int>();
		if (!it[5].is_null())
			output[i]["completed_time"] = crow::json::load(it[5].as<std::string>());

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

crow::json::wvalue Database::orders_complete(const crow::json::rvalue& data)
{
	crow::json::wvalue output;

	for (int i = 0; i < data["complete_info"].size(); ++i)
	{
		std::string courier_id = crow::json::wvalue(data["complete_info"][i]["courier_id"]).dump();
		std::string order_id = crow::json::wvalue(data["complete_info"][i]["order_id"]).dump();

		std::string query = "SELECT * FROM order_courier WHERE order_id = " + order_id + " AND courier_id = " + courier_id + ";";

		pqxx::nontransaction N(C);
		pqxx::result res(N.exec(query));

		if (res.empty())
		{
			return output;
		}
	}

	for (int i = 0; i < data["complete_info"].size(); ++i)
	{
		std::string courier_id = crow::json::wvalue(data["complete_info"][i]["courier_id"]).dump();
		std::string order_id = crow::json::wvalue(data["complete_info"][i]["order_id"]).dump();
		std::string complete_time = crow::json::wvalue(data["complete_info"][i]["complete_time"]).dump();

		std::string query = "SELECT weight, regions, delivery_hours, cost FROM order_courier JOIN orders ON order_courier.order_id = orders.order_id \
							 WHERE order_courier.order_id = " + order_id + " AND order_courier.courier_id = " + courier_id + ";";

		pqxx::nontransaction N(C);
		pqxx::result res(N.exec(query));
		N.abort();

		auto elem = res.begin();

		output[i]["order_id"] = std::stoi(order_id);
		output[i]["weight"] = elem[0].as<double>();
		output[i]["regions"] = elem[1].as<int>();
		output[i]["delivery_hours"] = crow::json::load(elem[2].as<std::string>());
		output[i]["cost"] = elem[3].as<int>();
		output[i]["completed_time"] = crow::json::load(complete_time);
		
		pqxx::work W(C);

		query = "UPDATE order_courier SET completed = true WHERE order_id = " + order_id + " AND courier_id = " + courier_id + ";";
		W.exec(query);

		query = "UPDATE orders SET completed_time = '" + complete_time + "' WHERE order_id = " + order_id + ";";
		W.exec(query);

		W.commit();
	}

	return output;
}

crow::json::wvalue Database::courier_meta_info(int courier_id, const std::string& start_date, const std::string& end_date)
{
	crow::json::wvalue output = get_couriers(1, courier_id - 1)["couriers"][0];

	pqxx::nontransaction N(C);
	std::string query = "SELECT SUM(cost) as raw_earnings, COUNT(*) / (EXTRACT (EPOCH FROM AGE('" + start_date + "', '" + end_date + "')) / 3600) as rating FROM order_courier JOIN orders ON orders.order_id = order_courier.order_id WHERE order_courier.courier_id = " + std::to_string(courier_id) + " AND completed = true AND date > '" + start_date + "' AND date < '" + end_date + "';";

	pqxx::result res(N.exec(query));
	auto elem = res.begin();

	std::unordered_map<std::string, int> earning_coef = { {"\"FOOT\"", 2}, {"\"AUTO\"", 4}, {"\"BIKE\"", 3} };
	std::unordered_map<std::string, int> rating_coef  = { {"\"FOOT\"", 3}, {"\"AUTO\"", 1}, {"\"BIKE\"", 2} };

	if (elem == res.end())
	{
		std::string courier_type = crow::json::wvalue(output["courier_type"]).dump();
		output["earnings"] = elem[0].as<int>() * earning_coef[courier_type];
		output["rating"] = (int) (std::abs(elem[1].as<double>()) * earning_coef[courier_type]);
	}

	return output;
}

crow::json::wvalue Database::couriers_assignments(const std::string& date, int courier_id)
{
	crow::json::wvalue output;

	/*pqxx::nontransaction N(C);
	std::string query;

	if (courier_id == -1)
		query = "SELECT courier_id, "*/

	return output;
}
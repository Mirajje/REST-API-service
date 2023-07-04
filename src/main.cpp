#include <crow.h>
#include <pqxx/pqxx>
#include "Routes.h"

void create_database()
{
    pqxx::connection C("postgresql://postgres:password@db/postgres");
    std::string query;

    try
    {
        pqxx::nontransaction W(C);
        query = "CREATE DATABASE yandexlavka";
        W.exec(query);
        W.commit();
    } 
    catch (std::exception& e) {}; // in case yandexlavka exists an error will be thrown
}

int main(int argc, char* argv[])
{
    create_database();
     
    crow::SimpleApp app;

    Routes routes(app);
    routes.couriers_routes();
    routes.orders_routes();

    app.validate();
    app.port(8080).multithreaded().run_async();
}
cmake_minimum_required(VERSION 3.10)

project(Yandex)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(/vcpkg/scripts/buildsystems/vcpkg.cmake)

# find required pqxx and required libraries
find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)

# find required crow
find_package(Crow CONFIG REQUIRED)

# Find the threads package of your system
find_package(Threads REQUIRED)

add_executable(app main.cpp Database.cpp Limiter.cpp Routes.cpp)

# link libraries to your project
target_link_libraries(app PRIVATE ${PQXX_LIB} ${PQ_LIB} Threads::Threads Crow::Crow)
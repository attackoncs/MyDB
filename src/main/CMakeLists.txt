set(MY_DB_SRC main.cpp)

add_executable(my_db ${MY_DB_SRC})

target_link_libraries(my_db ${CMAKE_SOURCE_DIR}/sql-parser/lib/libsqlparser.so)
#include "SQLParser.h"
#include "SQLParserResult.h"
#include "util/sqlhelper.h"

#include <cstdlib.h>
#include <iostream>
#include <string>

int main()
{
    std::cout << "# Welcome to MyDB DB!!!" << std::endl;
    std::cout << "# Input your query in one line." << std::endl;
    std::cout << "# Enter 'exit' to quit this program." << std::endl;
    std::string cmd;
    while (true)
    {
        std::getline(std::cin, cmd);
        if (cmd == "exit")
        {
            break;
        }
    }

    std::cout << "# Farewell~~~ " << std::endl;
    return 0;
}

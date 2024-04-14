#include "UCI.h"
#include "../Application.h"

void UCIInterface::UCILog(std::string message)
{
    // append message to a log file
    std::ofstream logFile;
    logFile.open("UCI.log", std::ios_base::app);
    logFile << message << std::endl;
    logFile.close();
    // also print to the console in UCI info mode
    std::cout << "info string " << message << std::endl;
}


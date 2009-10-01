#ifndef LOGMANAGER_EXCEPTIONS_H
#define LOGMANAGER_EXCEPTIONS_H

#include <exception>
using namespace std;

namespace LogManager
{
   class LoggerAlreadyExistsException : public exception {};
}
#endif
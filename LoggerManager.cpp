/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LoggerManager.cpp
 */
#include "LoggerManager.hpp"
#include "AppMates.hpp"
#include "CustomMessage.hpp"
#include "FileManager.hpp"
#include <iostream>
#include <typeinfo>

using namespace std;

//==============================================================================
LoggerManager::LoggerManager()
{
    _loggers.clear();
    _className = typeid(this).name();
    AppMates::getFileManager().addDependee(this);
}

//==============================================================================
Logger* LoggerManager::logger(const string& name)
{
#ifndef ENABLE_LOGGER
    return nullptr;
#endif

    auto itr = _loggers.find(name);
    if (itr != _loggers.end())
    {
        // 既にロガーが存在する場合はそのポインタを返す
        // If the logger already exists, return the pointer.
        return (*itr).second;
    }
    else
    {
        // 見つからないロガーは新たに生成する
        // Generate a new logger that is not found
        Logger* logger = new Logger(name);
        _loggers.insert(make_pair(name, logger));

        ostringstream oss;
        oss << "Logger[" << name << "] created.";
        amu::msg::status(cout, oss.str());

        return logger;
    }
}

//==============================================================================
void LoggerManager::writeAllLogs()
{
#ifndef ENABLE_LOGGER
    return;
#endif

    for (auto itr : _loggers)
    {
        itr.second->writeLog();
        itr.second->clearLog();
    }
}

//==============================================================================
void LoggerManager::deleteAllLoggers()
{
#ifndef ENABLE_LOGGER
    return;
#endif

    for (auto itr : _loggers)
    {
        delete itr.second;
    }
    _loggers.clear();
}

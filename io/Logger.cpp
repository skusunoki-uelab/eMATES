/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file Logger.cpp
 */
#include "Logger.hpp"
#include "../AppMates.hpp"
#include "../FileManager.hpp"
#include "../GVManager.hpp"
#include "../TimeManager.hpp"
#include <fstream>
#include <map>

using namespace std;

//======================================================================
Logger::Logger(const string& name)
{
#ifndef ENABLE_LOGGER
    return;
#endif //ENABLE_LOGGER

    _fileName
        = AppMates::getGVManager().getString("LOG_OUTPUT_DIRECTORY")
          + name
          + AppMates::getGVManager().getString("LOG_FILE_EXTENSION");
    _logs.erase(_logs.begin(), _logs.end());

#ifdef _OPENMP
    omp_init_lock(&_lock);
#endif //_OPENMP
}

//======================================================================
Logger::~Logger()
{
#ifdef _OPENMP
    omp_destroy_lock(&_lock);
#endif //_OPENMP
}

//======================================================================
void Logger::saveLog(const string& key, const string& str)
{
#ifndef ENABLE_LOGGER
    return;
#endif //ENABLE_LOGGER

#ifdef _OPENMP
    omp_set_lock(&_lock);
#endif //_OPENMP
    auto itr = _logs.find(key);
    if (itr == _logs.end())
    {
        // 新規keyとして登録
        // Register as new key
        _logs.insert(make_pair(key, str));
    }
    else
    {
        // 既存keyの場合は保存されている文字列に追加
        // Add to stored string if key exists
        (*itr).second += ", " + str;
    }
#ifdef _OPENMP
    omp_unset_lock(&_lock);
#endif //_OPENMP
}

//======================================================================
void Logger::writeLog()
{
#ifndef ENABLE_LOGGER
    return;
#endif //ENABLE_LOGGER

    if (_logs.size() == 0)
    {
        return;
    }

    // 出力用にソートする
    // sort for output
    map<string, string> ordered(_logs.begin(), _logs.end());

    ofstream* fout = AppMates::getFileManager().getOFStream(_fileName);
    for (auto itr = ordered.begin(); itr != ordered.end(); itr++)
    {
        *fout << "At time: " << AppMates::getTimeManager().time()
              << "| " << (*itr).first << ", " << (*itr).second << endl;
    }
    fout->close();
}

//======================================================================
void Logger::clearLog()
{
#ifndef ENABLE_LOGGER
    return;
#endif //ENABLE_LOGGER

    _logs.clear();
}

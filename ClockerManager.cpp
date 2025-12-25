/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ClockerManager.cpp
 */
#include "ClockerManager.hpp"
#include "Clocker.hpp"
#include "CustomMessage.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <typeinfo>
#include <utility>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

using namespace std;

//==============================================================================
ClockerManager::ClockerManager()
{
    _className = typeid(this).name();
}

//==============================================================================
Clocker* ClockerManager::findClock(const string& clockerName)

{
    Clocker* result     = nullptr;
    string   uniqueName = _getUniqueName(clockerName);

    auto itr = _clockers.find(uniqueName);
    if (itr == _clockers.end())
    {
#pragma omp critical(newClock)
        {
            // 見つからなければ新たに作成する
            // Create new timer if not found
            result = new Clocker(uniqueName);
            _clockers.insert(make_pair(uniqueName, result));
            ostringstream oss;
            oss << "Clock[" << uniqueName << "] created.";
            amu::msg::status(cout, oss.str());
        }
    }
    else
    {
        result = (*itr).second;
    }

    return result;
}

//==============================================================================
bool ClockerManager::startClock(const string& clockerName)
{
    bool result = false;

    // 該当するタイマーの検索
    // Search for matching timer
    Clocker* clocker = findClock(clockerName);
    result           = clocker->startClock();
    if (!result)
    {
        ostringstream ssw;
        ssw << "Clock[" << _getUniqueName(clockerName)
            << "] has already started" << endl;
        amu::msg::warn(ssw.str());
    }
    return result;
}

//==============================================================================
bool ClockerManager::stopClock(const string& clockerName)
{
    bool result = false;

    /*
     * 該当するタイマーの検索
     *   見つからなくても新たに生成しない
     *
     * Search for matching timer
     *   Not create a new timer here even if it is not found.
     */
    string uniqueName = _getUniqueName(clockerName);
    auto   itr        = _clockers.find(uniqueName);
    if (itr == _clockers.end())
    {
        ostringstream ssw;
        ssw << "Clock[" << uniqueName << "] is not found";
        amu::msg::warn(ssw.str());
    }
    else
    {
        result = (*itr).second->stopClock();
        if (!result)
        {
            ostringstream ssw;
            ssw << "Clock[" << uniqueName << "] has not start";
            amu::msg::warn(ssw.str());
        }
    }
    return result;
}

//==============================================================================
std::string ClockerManager::_getUniqueName(const std::string& clockerName) const
{
    stringstream uniqueNameStream;
#ifdef _OPENMP
    int threadNum = omp_get_thread_num();
    uniqueNameStream << clockerName << "@" << threadNum;
#else
    uniqueNameStream << clockerName;
#endif

    return uniqueNameStream.str();
}

//==============================================================================
void ClockerManager::printAllClockers() const
{
    amu::msg::title(cout, "Timing Result");

    ostringstream oss;
    oss << "name/ "
        << "num. called, total cpu time [s], total wallclock time [s]" << endl;

    // 表示のためにソートする
    // Sort for display
    map<string, Clocker*> sorted(_clockers.begin(), _clockers.end());

    for (auto itr : sorted)
    {
        oss << itr.first << "/ " << itr.second->numCalled() << ", "
            << itr.second->totalCpuTime() << ", "
            << itr.second->totalWallclockTime() << endl;
    }
    amu::msg::message(cout, oss.str());
}

//==============================================================================
void ClockerManager::deleteAllClockers()
{
    for (auto itr : _clockers)
    {
        delete itr.second;
    }
    _clockers.clear();
}

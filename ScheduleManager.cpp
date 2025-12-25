/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ScheduleManager.cpp
 */
#include "ScheduleManager.hpp"
#include "AppMates.hpp"
#include "CustomMessage.hpp"
#include "Lane.hpp"
#include "Section.hpp"
#include "TimeManager.hpp"
#include <iostream>

using namespace std;

//#define SCHEDULE_DEBUG

//==============================================================================
ScheduleManager::ScheduleManager()
{
    _environmentList.clear();
    _className = typeid(this).name();
#ifdef _OPENMP
    omp_init_lock(&_lock);
#endif //_OPENMP
}

//==============================================================================
ScheduleManager::~ScheduleManager()
{
#ifdef _OPENMP
    omp_destroy_lock(&_lock);
#endif //_OPENMP
}

//==============================================================================
void ScheduleManager::addEnvironmentItem(ulint time, ScheduleItemBase* item)
{
    _environmentList.insert(std::make_pair(time, item));
}

//==============================================================================
void ScheduleManager::activateEnvironmentItem()
{
    ulint time = AppMates::getTimeManager().time();
    while (true)
    {
        if (_environmentList.empty())
        {
            break;
        }
        if ((*_environmentList.begin()).first > time)
        {
            break;
        }

        // 現時点で有効なアイテム
        // Currently active item
        ScheduleItemBase* item = (*_environmentList.begin()).second;

#ifdef SCHEDULE_DEBUG
        cout << "Time: " << time << ", ";
        item->print(cout);
#endif

        item->activate();

        // 使用されたアイテムをリストから除外する
        // Exclude used cells from list
        _environmentList.erase(_environmentList.begin());
    }
}

//==============================================================================
void ScheduleManager::print()
{
    amu::msg::title(cout, "Environment List");
    for (auto itr : _environmentList)
    {
        itr.second->print(cout);
    }
}

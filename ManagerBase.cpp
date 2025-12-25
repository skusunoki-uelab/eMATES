/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ManagerBase.cpp
 */
#include "ManagerBase.hpp"
#include <algorithm>
#include <iostream>
#include <typeinfo>

using namespace std;

//======================================================================
ManagerBase::ManagerBase()
{
    _dependees.clear();
    _isFinalized = false;
}

//======================================================================
ManagerBase::~ManagerBase()
{
    _dependees.clear();
#ifdef MANAGER_DEBUG
    cout << _className << " destructed." << endl;
#endif //MANAGER_DEBUG
}

//======================================================================
void ManagerBase::addDependee(ManagerBase* mng)
{
    // 重複登録を防ぐ
    // Prevent duplicate registration
    if (find(_dependees.begin(), _dependees.end(), mng)
        != _dependees.end())
    {
        cerr << "WARNING: duplicate registration" << endl;
        cerr << "manager[" << mng->className()
             << "] already depends on manager[" << _className << "]"
             << endl;
        return;
    }

    // 循環登録を防ぐ
    // Prevent circular registration
    for (auto itr : mng->dependees())
    {
        if (itr == this)
        {
            cerr << "WARNING: circular registration" << endl;
            cerr << "manager[" << _className
                 << "] opposingly depends on manager["
                 << mng->className() << "]" << endl;
            return;
        }
    }

    _dependees.push_back(mng);
}

//======================================================================
void ManagerBase::finalize()
{
#ifdef MANAGER_DEBUG
    cout << _className << ":finalize()" << endl;
#endif //MANAGER_DEBUG

    for (auto itr : _dependees)
    {
#ifdef MANAGER_DEBUG
        cout << "  calls " << itr->className() << ":finalize()" << endl;
#endif //MANAGER_DEBUG
        itr->finalize();
    }
    if (!_isFinalized)
    {
        _finalizeInside();
        _isFinalized = true;
    }
#ifdef MANAGER_DEBUG
    else
    {
        cout << "  " << _className << " already finalized." << endl;
    }
#endif //MANAGER_DEBUG
}

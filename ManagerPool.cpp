/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ManagerPool.cpp
 */
#include "ManagerPool.hpp"

using namespace std;

// 静的変数の初期化
// Initialization of static variables
ManagerPool* ManagerPool::_instance = nullptr;

//======================================================================
ManagerPool::~ManagerPool()
{
#ifdef MANAGER_DEBUG
    cout << "ManagerPool::~ManagerPool() called" << endl;
#endif //MANAGER_DEBUG
    for (auto itr : _managers)
    {
        // デストラクタを呼び出す前に安全に終了処理をおこなう
        // Clean up safely before calling the destructors
        itr->finalize();
    }
    for (auto itr = _managers.rbegin(); itr != _managers.rend(); itr++)
    {
        delete *itr;
    }
    reset();
    _instance = nullptr;
}

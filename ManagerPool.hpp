/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ManagerPool.hpp
 */
#ifndef __MANAGER_POOL_HPP__
#define __MANAGER_POOL_HPP__
#include "ClockerManager.hpp"
#include "FileManager.hpp"
#include "GVManager.hpp"
#include "LoggerManager.hpp"
#include "ManagerBase.hpp"
#include "ObjectManager.hpp"
#include "RandomSeedManager.hpp"
#include "RouteCacheManager.hpp"
#include "RouterManager.hpp"
#include "ScheduleManager.hpp"
#include "TimeManager.hpp"
#include "VehicleTypeManager.hpp"
#ifdef INCLUDE_TRAMS
#include "tram/TramRouteManager.hpp"
#endif //INCLUDE_TRAMS
#include <iostream>
#include <string>
#include <vector>

//######################################################################
/**
 * @~japanese
 * マネージャの生成・破棄を管理する
 *
 * AppMatesからのみ生成可能．いわゆるObject Poolで，各マネージャが1つで
 * あることを保証する．このクラス自体はシングルトンとして実装する．
 *
 * @~english
 * Manage managers generation/destruction
 *
 * Can only be generated from AppMates. A kind of Object Pool, ensuring
 * that each manager is unique. This class itself is implemented as a
 * singleton.
 *
 * @~ @ingroup Manager
 */
class ManagerPool
{
    friend class AppCalc;
    friend class AppMates;
    friend class AppSim;

private:
    ManagerPool()
    {
        reset();
    }
    ~ManagerPool();

    /**
     * @~japanese 唯一のインスタンスへのポインタを戻す
     * @~english  Return point to the unique instance
     */
    static ManagerPool* instance()
    {
        if (!_instance)
        {
            _instance = new ManagerPool();
        }
        return _instance;
    }

    /**
     * @~japanese 状態をリセットする
     * @~english  Reset state
     */
    void reset()
    {
        _clocker    = nullptr;
        _file       = nullptr;
        _gv         = nullptr;
        _logger     = nullptr;
        _obj        = nullptr;
        _rand       = nullptr;
        _routeCache = nullptr;
        _router     = nullptr;
        _schedule   = nullptr;
        _time       = nullptr;
        _vt         = nullptr;
#ifdef INCLUDE_TRAMS
        _tramRoute = nullptr;
#endif //INCLUDE_TRAMS
        _managers.clear();
    }

    /**
     * @~japanese 唯一のインスタンス
     * @~english  Unique instance
     */
    static ManagerPool* _instance;

    /**
     * @~japanese 保持されるマネージャの集合
     * @~english  Set of retained managers
     */
    std::vector<ManagerBase*> _managers;

    //==================================================================
    /**
     * @~japanese @name 各マネージャのポインタとアクセッサ
     * @~english  @name Pointers and accessors for each manager
     */
    ///@{

    ClockerManager* _clocker;
    ClockerManager& clocker()
    {
        if (!_clocker)
        {
            _clocker = new ClockerManager();
            _managers.emplace_back(_clocker);
        }
        return *_clocker;
    }

    FileManager* _file;
    FileManager& file()
    {
        if (!_file)
        {
            _file = new FileManager();
            _managers.emplace_back(_file);
        }
        return *_file;
    }

    GVManager* _gv;
    GVManager& gv()
    {
        if (!_gv)
        {
            _gv = new GVManager();
            _managers.emplace_back(_gv);
        }
        return *_gv;
    }

    LoggerManager* _logger;
    LoggerManager& logger()
    {
        if (!_logger)
        {
            _logger = new LoggerManager();
            _managers.emplace_back(_logger);
        }
        return *_logger;
    }

    ObjectManager* _obj;
    ObjectManager& obj()
    {
        if (!_obj)
        {
            _obj = new ObjectManager();
            _managers.emplace_back(_obj);
        }
        return *_obj;
    }

    RandomSeedManager* _rand;
    RandomSeedManager& rand()
    {
        if (!_rand)
        {
            _rand = new RandomSeedManager();
            _managers.emplace_back(_rand);
        }
        return *_rand;
    }

    RouteCacheManager* _routeCache;
    RouteCacheManager& routeCache()
    {
        if (!_routeCache)
        {
            _routeCache = new RouteCacheManager();
            _managers.emplace_back(_routeCache);
        }
        return *_routeCache;
    }

    RouterManager* _router;
    RouterManager& router()
    {
        if (!_router)
        {
            _router = new RouterManager();
            _managers.emplace_back(_router);
        }
        return *_router;
    }

    ScheduleManager* _schedule;
    ScheduleManager& schedule()
    {
        if (!_schedule)
        {
            _schedule = new ScheduleManager();
            _managers.emplace_back(_schedule);
        }
        return *_schedule;
    }

    TimeManager* _time;
    TimeManager& time()
    {
        if (!_time)
        {
            _time = new TimeManager();
            _managers.emplace_back(_time);
        }
        return *_time;
    }

    VehicleTypeManager* _vt;
    VehicleTypeManager& vt()
    {
        if (!_vt)
        {
            _vt = new VehicleTypeManager();
            _managers.emplace_back(_vt);
        }
        return *_vt;
    }

#ifdef INCLUDE_TRAMS
    TramRouteManager* _tramRoute;
    TramRouteManager& tramRoute()
    {
        if (!_tramRoute)
        {
            _tramRoute = new TramRouteManager();
            _managers.emplace_back(_tramRoute);
        }
        return *_tramRoute;
    }
#endif //INCLUDE_TRAMS

    ///@}
};

#endif //__MANAGER_POOL_HPP__

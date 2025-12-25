/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ScheduleManager.hpp
 */
#ifndef __SCHEDULE_MANAGER_HPP__
#define __SCHEDULE_MANAGER_HPP__
#include "Config.hpp"
#include "ManagerBase.hpp"
#include "ScheduleItemBase.hpp"
#include "SpeedLimitItem.hpp"
#include <map>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

//##############################################################################
/**
 * @~japanese  時間ドリブンのイベントを管理する
 * @~english   Manage time-driven events
 * @~
 * @ingroup Manager
 * @see ScheduleItemAbst
 */
class ScheduleManager : public ManagerBase
{
    friend class ManagerPool;

private:
    ScheduleManager();
    ~ScheduleManager();

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void _finalizeInside() override {};

    //==========================================================================
public:
    /**
     * @~japanese
     * @p time をキーとしてアイテム @p item を環境変更リストに格納する
     *
     * @~english
     * Add @p item in environment change list using @p time as key
     */
    void addEnvironmentItem(ulint time, ScheduleItemBase* item);

    /**
     * @~japanese
     * 環境変更リストのアイテムを有効化し環境を更新する
     * 
     * @~english
     * Enable items in the environment change list and update the environment
     */
    void activateEnvironmentItem();

    /**
     * @~japanese 登録された環境変更リストのアイテムを出力する
     * @~english  Output registered environment change list items
     */
    void print();

private:
    /**
     * @~japanese 環境変更情報を格納するリスト
     * 
     * サブコンテナであるので，デストラクタでの delete は不要．
     * 
     * @~english  List storing environment change information
     * 
     * Since it is a subcontainer, there is no need to delete in the destructor.
     */
    std::multimap<ulint, ScheduleItemBase*> _environmentList;

#ifdef _OPENMP
    /**
     * @~japanese ロック変数
     * @~english  Lock variable
     */
    omp_lock_t _lock;
#endif //_OPENMP
};

#endif //__SCHEDULE_MANAGER_HPP__

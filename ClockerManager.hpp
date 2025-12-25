/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ClockerManager.hpp
 */
#ifndef __CLOCKER_MANAGER_HPP__
#define __CLOCKER_MANAGER_HPP__
#include "Clocker.hpp"
#include "Config.hpp"
#include "ManagerBase.hpp"
#include <unordered_map>
#include <string>

//##############################################################################
/**
 * @~japanese タイマーを管理する
 * @~english  Manage timers
 * @~ @ingroup Manager
 */
class ClockerManager : public ManagerBase
{
    friend class ManagerPool;

protected:
    ClockerManager();
    ~ClockerManager()
    {
        deleteAllClockers();
    }

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void _finalizeInside() override
    {
        printAllClockers();
    }

    //==========================================================================
public:
    /**
     * @~japanese @p clockNameを持つタイマーを戻す
     * @note 該当するタイマーが存在しない場合は新規に生成する
     *
     * @~english  Return the timer specified by @p clockName
     * @note Create a new timer if the corresponding one does not exist.
     */
    Clocker* findClock(const std::string& clockerName);

    /**
     * @~japanese @p clockName で指定したタイマーによる計時を開始する
     * @~english  Start timing with the timer specified by @p clockName
     */
    bool startClock(const std::string& clockerName);

    /**
     * @~japanese @p clockName で指定したタイマーによる計時を停止する
     * @~english  Stop timing with the timer specified by @p clockName
     */
    bool stopClock(const std::string& clockerName);

private:
    /**
     * @~japanese タイマーのユニークな名称を決定して戻す
     * @note マルチスレッドで実行中は，clockerNameにスレッド番号を付与する
     *
     * @~english  Determine and return a unique name for a timer
     * @note Add thread number to clockerName when running on multi threads
     */
    std::string _getUniqueName(const std::string& clockerName) const;

public:
    /**
     * @~japanese すべてのタイマーの計時結果を表示する
     * @~english  Display timing results of all timers
     */
    void printAllClockers() const;

    /**
     * @~japanese すべてのタイマーを消去する
     * @~english  Delete all times
     */
    void deleteAllClockers();

private:
    /**
     * @~japanese タイマーのメインコンテナ
     * @~english  Main container of timers
     */
    std::unordered_map<std::string, Clocker*> _clockers;
};

#endif //__TIME_MANAGER_H__

/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file TrafficCounter.hpp
 */
#ifndef __TRAFFIC_COUNTER_HPP__
#define __TRAFFIC_COUNTER_HPP__
#include "Config.hpp"
#include "ObserverBase.hpp"
#include "Section.hpp"
#include "TimeManager.hpp"
#include "TrafficCounterComponent.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

//######################################################################
/**
 * @~japanese 車両感知器
 *
 * 各レーンを担当するTrafficCounterComponentの集合．
 *
 * @~english  Traffic counter
 *
 * Set of TrafficCounterComponents responsible for each lane
 *
 * @~
 * @ingroup Monitoring
 * @see TrafficCounterComponent
 */
class TrafficCounter : public ObserverBase
{
public:
    //==================================================================
    /**
     * @~japanese 集計データを格納する構造体
     *
     * 指定された間隔でファイルに書き出されクリアされる
     *
     * @~english  Struct string aggregated record
     *
     * Written to file and cleared at specified interval.
     *
     * @~ @ingroup Monitoring
     */
    struct AggregatedRecord
    {
    private:
        /**
         * @~japanese 集計開始時刻
         * @~english  Start time of aggregation
         */
        ulint _beginTime;

        /**
         * @~japanese
         * シミュレーション開始からの通過普通車数
         *
         * @~english
         * Number of passing regular vehicles from simulation start
         */
        int _totalPassengers;

        /**
         * @~japanese
         * シミュレーション開始からの通過大型車数
         *
         * @~english
         * Number of passing large vehicles from simulation start
         */
        int _totalTrucks;

        /**
         * @~japanese
         * 集計対象時間内の通過乗用車数
         *
         * @~english
         * Number of passing regular vehicles within aggregation duration
         */
        int _sumPassengers;

        /**
         * @~japanese
         * 集計対象時間内の通過大型車数
         *
         * @~english
         * Number of passing large vehicles within aggregation duration
         */
        int _sumTrucks;

        /**
         * @~japanese
         * 集計対象時間内の車線別通過普通車数
         *
         * @~english
         * Number of passing regular vehicles passing by lane during
         * aggregation duration
         */
        std::vector<int> _numPassengers;

        /**
         * @~japanese
         * 集計対象時間内の車線別通過大型車数
         *
         * @~english
         * Number of passing large vehicles passing by lane during
         * aggregation duration
         */
        std::vector<int> _numTrucks;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        AggregatedRecord()
        {
            _beginTime       = 0;
            _totalPassengers = 0;
            _totalTrucks     = 0;
            _sumPassengers   = 0;
            _sumTrucks       = 0;
            _numPassengers.clear();
            _numTrucks.clear();
        }

        /**
         * @~japanese 集計結果をクリアする
         * @~english  Clear aggregated record
         */
        void clear();

        /**
         * @~japanese 集計結果のコンテナのサイズを変更する
         * @~english  Resize aggregated result container
         */
        void resize(int size)
        {
            _numPassengers.resize(size);
            _numTrucks.resize(size);
        }

        /**
         * @~japanese 普通車通過台数をインクリメントする
         * @~english  Increment number of passing regular vehicles
         */
        void incrementNumPassengers(unsigned int laneIndex)
        {
            _numPassengers[laneIndex]++;
            _sumPassengers++;
            _totalPassengers++;
        }

        /**
         * @~japanese 大型車通過台数をインクリメントする
         * @~english  Increment number of passing large vehicles
         */
        void incrementNumTrucks(unsigned int laneIndex)
        {
            _numTrucks[laneIndex]++;
            _sumTrucks++;
            _totalTrucks++;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /**
         * @~japanese @name アクセッサ
         * @~english  @name Accessor
         */
        ///@{
    public:
        ulint beginTime() const
        {
            return _beginTime;
        }

        int totalPassengers() const
        {
            return _totalPassengers;
        }

        int totalTrucks() const
        {
            return _totalTrucks;
        }

        int sumPassengers() const
        {
            return _sumPassengers;
        }

        int sumTrucks() const
        {
            return _sumTrucks;
        }

        int numPassengers(int i) const
        {
            return _numPassengers[i];
        }

        int numTrucks(int i) const
        {
            return _numTrucks[i];
        }

        ///@}
    };

    //==================================================================
public:
    explicit TrafficCounter(const std::string& id) : _id(id)
    {
        _section  = nullptr;
        _isUp     = false;
        _distance = 0.0;
        _lanes.clear();

        _interval = 100;
        _aggregatedRecord.clear();

        _components.clear();
    }

    ~TrafficCounter()
    {
        for (auto itr : _components)
        {
            delete itr;
        }
        _components.clear();
    }

    /**
     * @~japanese
     * 感知器を配置し，個々のcomponentを生成する
     *
     * @todo 関数名の検討，io/TrafficCounterBuilderに移行を検討
     *
     * @~english
     * Deploy traffic counter and generate individual components
     */
    void setPosition(
        Section* section, bool isUp, double distance, ulint interval);

    /**
     * @~japanese 個々の車線の記録を集計する
     * @~english  Aggregate records for individual lanes
     */
    void aggregateRecords();

    /**
     * @~japanese 集計結果をクリアする
     * @~english  Clear aggregated record
     */
    void clearAggregatedRecord()
    {
        _aggregatedRecord.clear();
    }

    /**
     * @~japanese 属性を @p out に出力する
     * @~english  Output attributes to @p out
     */
    void print(std::ostream& out) const;

private:
    /**
     * @~japanese 識別番号
     * @~english  ID number
     */
    std::string _id;

    /**
     * @~japanese 設置されている単路部
     * @~english  Section that this is installed
     */
    Section* _section;

    /**
     * @~japanese 観測対象の車線の集合
     * @~english  
     */
    std::vector<Lane*> _lanes;

    /**
     * @~japanese
     * sectionの上り方向に設置されているかどうか
     *
     * @~english
     * Whether being installed in the ascending direction of the section
     */
    bool _isUp;

    /**
     * @~japanese 始点交差点からの距離
     * @~english  Distance from starting intersection
     */
    double _distance;

    /**
     * @~japanese 集計データを出力する時間間隔 [ms]
     * @~english  Time interval to output aggregated record [ms]
     */
    ulint _interval;

    /**
     * @~japanese
     * 各レーンを担当するTrafficCounterComponentの集合
     *
     * @~english
     *  Set of TrafficCounterComponents responsible for each lane
     */
    std::vector<TrafficCounterComponent*> _components;

    /**
     * @~japanese 集計データ
     * @~english  Aggregated record
     */
    AggregatedRecord _aggregatedRecord;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    virtual const std::string& id() const override
    {
        return _id;
    }

    Section* section() const
    {
        return _section;
    }

    const std::vector<Lane*>& lanes() const
    {
        return _lanes;
    }

    bool isUp() const
    {
        return _isUp;
    }

    double distance() const
    {
        return _distance;
    }

    ulint interval() const
    {
        return _interval;
    }

    const std::vector<TrafficCounterComponent*>& components() const
    {
        return _components;
    }

    AggregatedRecord& aggregatedRecord()
    {
        return _aggregatedRecord;
    }

    ///@}
};

#endif //__TRAFFIC_COUNTER_HPP__

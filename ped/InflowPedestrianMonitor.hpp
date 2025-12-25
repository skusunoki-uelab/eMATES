/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file InflowPedestrianMonitor.hpp
 */
#ifndef __INFLOW_PEDESTRIAN_MONITOR_HPP__
#define __INFLOW_PEDESTRIAN_MONITOR_HPP__
#include "Zebra.hpp"
#include "../Config.hpp"
#include "../ObserverBase.hpp"
#include <string>
#include <vector>

class LaneBundle;
class Pedestrian;
class Zebra;
class ZebraODEdge;

//##############################################################################
/**
 * @~japanese 歩行者流入検出器
 *
 * ZebraODEdgeに設置され，流入する歩行者を記録する．
 *
 * @~english  Inflow Pedestrian Monitor
 *
 * Installed on ZebraODEdges and records inflow pedestrians.
 *
 * @~ @ingroup Monitoring
 */
class InflowPedestrianMonitor : public ObserverBase
{
    //==========================================================================
    /**
     * @~japanese 記録を格納する構造体
     * @~english  Struct to store record
     * @~ @ingroup Monitoring
     */
    struct Record
    {
    private:
        /**
         * @~japanese 発生歩行者の識別番号
         * @~english  ID number of generated pedestrian
         */
        const std::string _pedestrianId;

        /**
         * @~japanese 時間間隔 [ms]
         * @~english  Time-headway [ms]
         */
        ulint _headway;

        /**
         * @~japanese 発生時刻 [ms]
         * @~english  Time of generation [ms]
         */
        ulint _genTime;

        /**
         * @~japanese 歩行者生成時間間隔 [ms]
         * @~english  Pedestrian generation time interval [ms]
         */
        ulint _genInterval;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        Record(
            const std::string& pedestrianId, ulint headway, ulint genTime,
            ulint genInterval)
            : _pedestrianId(pedestrianId),
              _headway(headway),
              _genTime(genTime),
              _genInterval(genInterval) {};

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /**
         * @~japanese @name アクセッサ
         * @~english  @name Accessor
         */
        ///@{
    public:
        const std::string& pedestrianId() const
        {
            return _pedestrianId;
        }

        ulint headway() const
        {
            return _headway;
        }

        ulint genTime() const
        {
            return _genTime;
        }

        ulint genInterval()
        {
            return _genInterval;
        }

        ///@}
    };

    //==========================================================================
public:
    explicit InflowPedestrianMonitor(const std::string& id) : _id(id)
    {
        _edge = nullptr;
        _records.clear();
    }

    ~InflowPedestrianMonitor()
    {
        clearRecords();
    }

    /**
     * @~japanese 流入歩行者を記録する
     * @~english  Record inflow pedestrian
     */
    void recordInflowPedestrian(
        const Pedestrian* pedestrian, ulint headway, ulint genTime,
        ulint genInterval);

    /**
     * @~japanese 記録を消去する
     * @~english  Clear records
     */
    void clearRecords()
    {
        for (auto itr : _records)
        {
            delete itr;
        }
        _records.clear();
    }

    /**
     * @~japanese 検知器が設置されているレーン束を戻す
     * @~english  Return LaneBundle where the monitor is installed
     */
    const LaneBundle* laneBundle() const;

    /**
     * @~japanese 検知器が設置されている横断歩道を戻す
     * @~english  Return zebra where the monitor is installed
     */
    const Zebra* zebra() const;

    /**
     * @~japanese 属性を @p out に出力する
     * @~english  Output attributes to @p out
     */
    void print(std::ostream& out) const;

    //==========================================================================
private:
    /**
     * @~japanese 識別番号
     * @~english  ID number
     */
    std::string _id;

    /**
     * @~japanese 観測対象のZebraODEdge
     * @~english  ZebraODEdge to be monitored
     */
    ZebraODEdge* _edge;

    /**
     * @~japanese 現在のタイムステップで流入した歩行者の記録
     * @~english  Record of inflow pedestrian at the current timestep
     */
    std::vector<InflowPedestrianMonitor::Record*> _records;

    //==========================================================================
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

    const ZebraODEdge* zebraODEdge() const
    {
        return _edge;
    }

    void setZebraODEdge(ZebraODEdge* edge)
    {
        _edge = edge;
    }

    const std::vector<InflowPedestrianMonitor::Record*>& records() const
    {
        return _records;
    }

    ///@}
};

#endif //__INFLOW_PEDESTRIAN_MONITOR_HPP__

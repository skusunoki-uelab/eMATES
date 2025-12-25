/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ConvoyMonitor.hpp
 */
#ifndef __CONVOY_MONITOR_HPP__
#define __CONVOY_MONITOR_HPP__
#include "ObserverBase.hpp"
#include <map>
#include <string>
#include <vector>

class Lane;
class Section;

//##############################################################################
/**
 * @~japanese 車列観測器
 *
 * 単路部に設置され，定められた周期で最後尾停止車両の下流交差点からの距離を
 * 計測し出力する．
 *
 * @~english  Vehicle convoy monitor
 *
 * Installed on a section, and measures and outputs the distance of the
 * rearmost stopped vehicle from the downstream intersection at specified time
 * intervals.
 *
 * @~ @ingroup Monitoring
 */
class ConvoyMonitor : public ObserverBase
{
public:
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
         * @~japanese 車線の識別番号
         * @~english  ID number of lane
         */
        const std::string _laneId;

        /**
         * @~japanese 下流交差点からの距離
         * @~english  Distance from the downstream intersection
         */
        double _distance;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        explicit Record(const std::string& laneId)
            : _laneId(laneId), _distance(0.0)
        {
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /**
         * @~japanese @name アクセッサ
         * @~english  @name Accessor
         */
        ///@{
    public:
        const std::string& laneId() const
        {
            return _laneId;
        }

        double distance() const
        {
            return _distance;
        }

        void setDistance(double distance)
        {
            _distance = distance;
        }

        ///@}
    };

    //==========================================================================
public:
    explicit ConvoyMonitor(const std::string& id)
        : _id(id), _section(nullptr), _isUp(true)
    {
        _records.clear();
    }

    ~ConvoyMonitor()
    {
        for (auto itr : _records)
        {
            delete itr.second;
        }
        _records.clear();
    }

    /**
     * @~japanese 車線 @p lane を観測対象の車線として追加する
     * @~english* Add @p lane as a lane to be monitored
     */
    void addMonitoredLane(Lane* lane);

    /**
     * @~japanese 各車線を計測する
     * @~english  Measured each lane
     */
    void monitorLanes();

    /**
     * @~japanese の属性を @p out に出力する
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
     * @~japanese 観測対象の単路部
     * @~english  Section to be measured
     */
    Section* _section;

    /**
     * @~japanese 観測対象の車線
     * @~english  Lanes to be measured
     */
    std::vector<Lane*> _lanes;

    /**
     * @~japanese
     * 観測対象が単路部の上り方向であるかどうか
     *
     * @~english
     * Whether the measurement target is the ascending direction
     * ob the section
     */
    bool _isUp;

    /**
     * @~japanese 観測結果
     * @~english  Measured results
     */
    std::map<const std::string, ConvoyMonitor::Record*> _records;

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::string& id() const override
    {
        return _id;
    }

    Section* section() const
    {
        return _section;
    }

    void setSection(Section* section)
    {
        _section = section;
    }

    bool isUp() const
    {
        return _isUp;
    }

    void setIsUp(bool isUp)
    {
        _isUp = isUp;
    }

    const std::map<const std::string, ConvoyMonitor::Record*>& records() const
    {
        return _records;
    }

    ///@}
};

#endif //__CONVOY_MONITOR_HPP__

/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LinkFlowRecord.hpp
 */
#ifndef __LINK_FLOW_RECORD_HPP__
#define __LINK_FLOW_RECORD_HPP__
#include "Config.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class Intersection;
class Section;
class Vehicle;

//##############################################################################
/**
 * @~japanese リンク交通流の記録
 *
 * 単路部と交差点を通過した車両の走行時間から単路部の交通流諸量を求め，経路探索
 * などで参照する．
 * 
 * @note
 * LinkTrafficRecordの1つのインスタンスが単路部の1つの進行方向に対応．ポインタを
 * Intersectionが管理し，Sectionが直接参照可能とする．
 *  
 * @~english  Link traffic flow record
 * 
 * @note
 * One LinkTrafficRecord object corresponds to one traffic direction in a 
 * section. The intersection class object manages the pointer, and the connected
 * section class object can refer to it directly.
 */
class LinkFlowRecord
{
public:
    LinkFlowRecord()
    {
        _inter   = nullptr;
        _dir     = -1;
        _section = nullptr;

        _sumAdditionalTimeRecords.clear();
        _sumDistanceRecordsInSection = 0.0;
        _sumTimeRecordsInSection     = 0;

        _estimatedTravelTimeInSection = 0.0;
        _estimatedTravelTimes.clear();

        _startTime = 0;
        _startPositionRecords.clear();

        _volumeInSection       = 0.0;
        _densityInSection      = 0.0;
        _meanVelocityInSection = 0.0;
    }

    ~LinkFlowRecord() {};

    //==========================================================================
    /**
     * @~japanese 記録をリセットする 
     * @~english  Reset records
     */
    void reset()
    {
        _sumDistanceRecordsInSection = 0.0;
        _sumTimeRecordsInSection     = 0;
        for (unsigned int i = 0; i < _numOutflows.size(); i++)
        {
            _numOutflows[i]              = 0;
            _sumAdditionalTimeRecords[i] = 0.0;
        }

        _startPositionRecords.clear();
    }

    /**
     * @~japanese 記録を準備する
     * @~english  prepare recording
     */
    void prepareRecording(unsigned int size)
    {
        _numOutflows.resize(size);
        _sumAdditionalTimeRecords.resize(size);
        _estimatedTravelTimes.resize(size);
    }

    //==========================================================================
    /**
     * @~japanese 単路部における車両位置を記録する
     * 
     * 計測時間帯の最初に呼ばれる．
     * 
     * @~english  Record vehicle positions in the section
     * 
     * Called at the beginning of the observation period.
     */
    void recordPositionsInSection();

    /**
     * @~japanese 単路部からの車両の流出を記録する
     *
     * 車両が単路部から流出するたびに呼ばれる．
     * 
     * @~english  Record vehicle outflow from the section
     * 
     * Called every time a vehicle flows out from the section.
     */
    void recordOutflowFromSection(const Vehicle* vehicle);

    /**
     * @~japanese 交差点からの車両の流出を記録する
     *
     * 車両が交差点から流出するたびに呼ばれる．
     * 
     * @param to 流出方向を表す境界番号
     * 
     * @~english  Record vehicle outflow from the intersection
     * 
     * Called every time a vehicle flows out from the intersection.
     * 
     * @param to Border number representing the outflow direction
     */
    void recordOutflowFromIntersection(const Vehicle* vehicle, int to);

    /**
     * @~japanese 巨視的諸量を計算する
     * @~english  Calculate macroscopic quantities
     */
    void calcMacroQuantities();

    /**
     * @~japanese LinkFlowMonitorにファイル出力を指示する
     * @~english  Instruct Link flow monitor to output file
     */
    void instructOutput() const;

    //==========================================================================
    /**
     * @~japanese 属性を @p out に出力する
     * @~english  Output attributes to @p out
     */
    void print(std::ostream& out) const;

    /**
     * @~japanese 記録を @p out に出力する
     * @~english  Output records to @p out
     */
    void printRecords(std::ostream& out) const;

    //==========================================================================
private:
    /**
     * @~japanese 計測対象交差点
     * @~english  Intersection to be observed
     */
    Intersection* _inter;

    /**
     * @~japanese 境界番号
     * @~english  Border number
     */
    int _dir;

    /**
     * @~japanese 計測対象単路部
     * @~english  Sction to be observed
     */
    Section* _section;

    //==========================================================================

    std::vector<int> _numOutflows;

    /**
     * @~japanese 方向別の追加リンク旅行時間 [ms]
     * @~english  Additional link travel time by direction [ms]
     */
    std::vector<double> _sumAdditionalTimeRecords;

    //==========================================================================
    /**
     * @~japanese
     * 単路部を走行する車両の走行距離の和
     *
     * @~english
     * Sum of travel distance of vehicles traveling on the section
     */
    double _sumDistanceRecordsInSection;

    /**
     * @~japanese
     * 単路部を走行する車両の走行時間の和
     * 
     * @~english
     * Sum of travel time of vehicles traveling on the section
     */
    ulint _sumTimeRecordsInSection;

    /**
     * @~japanese 単路部の予想リンク旅行時間 [ms]
     * 
     * 直前の計測時間帯の空間平均速度から求める
     * 
     * @~english  Estimated link travel time in  section [ms]
     *
     * Calculated from the space mean speed of the previous observation
     * period.
     */
    double _estimatedTravelTimeInSection;

    /**
     * @~japanese
     * 単路部に流入し交差点から流出するまでの方向別の予想リンク旅行時間 [ms]
     * 
     * @~english
     * Estimated link travel time by direction from flowing into the section to
     * flowing out from the intersection [ms]
     */
    std::vector<double> _estimatedTravelTimes;

    /**
     * @~japanese 計測開始時刻
     * @~english  Start time of the observation
     */
    ulint _startTime;

    /**
     * @~japanese
     * 計測時間帯開始時における車両の位置
     * 
     * キーは車両ID．単路部通過時の車両の速度を計算するために用いる．
     * 
     * @~english 
     * Vehicle positions at the start of the observation period
     * 
     * The key is the vehicle ID. Used to calculate the vehicle speed when
     * passing through a section.
     */
    std::unordered_map<std::string, double> _startPositionRecords;

    //==========================================================================
    /**
     * @~japanese @name 巨視的諸量
     * 
     * <div style="font-style:normal;">
     * 観測結果をもとに，交通量 @f(q@f) [veh/km]，車両密度 @f(k@f) [veh/km]，
     * 空間平均速度 @f(v_s@f) [km/h] を以下の式によって求める．@f(|A|@f) は
     * 時空間領域 @f(A@f) の面積 [h km]，@f(d_i(A)@f)，@f(\tau_i​​(A)@f) は
     * それぞれ車両@f(i@f)の領域@f(A@f) 内の走行距離 [km]，走行時間 [h]を表す．
     * 
     * @f[
     * q(A)=\frac{1}{|A|}\sum_i{d_i(A)}\\
     * v_s(A)=\frac{\sum_i{d_i(A)}}{\sum_i{\tau_i(A)}}\\
     * k(A)=\frac{q(A)}{v_s(A)}
     * @f]
     * </div>
     * 
     * @~english  @name Macroscopic quantities
     *
     * <div style="font-style:normal;">
     * Based on the observation results, the traffic volume @f(q@f) [veh/km],
     * vehicle density @f(k@f) [veh/km], and space mean speed @f(v_s@f) [km/h]
     * using the following formula. Where @f(|A|@f) is the area [h km] of the
     * spatiotemporal region @f(A@f), @f(d_i(A)@f), @f(\tau_i​​(A)@f) represents
     * the travel distance [km] and travel time [h] of the vehicle @f(i@f)
     * within the area @f(A@f), respectively. 
     * 
     * @f[
     * q(A)=\frac{1}{|A|}\sum_i{d_i(A)},\\
     * v_s(A)=\frac{\sum_i{d_i(A)}}{\sum_i{\tau_i(A)}},\\
     * k(A)=\frac{q(A)}{v_s(A)}
     * @f]
     * </div>
     */
    ///@{

    /**
     * @~japanese 交通量 [veh/ms]
     * @~english  Traffic volume [veh/ms]
     */
    double _volumeInSection;

    /**
     * @~japanese 車両密度 [veh/m]
     * @~english  Vehicle density [veh/m]
     */
    double _densityInSection;

    /**
     * @~japanese 空間平均速度 [m/ms]
     * @~english  Space mean speed [m/ms]
     */
    double _meanVelocityInSection;

    ///@}

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    Intersection* intersection() const
    {
        return _inter;
    }

    void setIntersection(Intersection* inter)
    {
        _inter = inter;
    }

    void setDirection(int dir)
    {
        _dir = dir;
    }

    Section* section() const
    {
        return _section;
    }

    void setSection(Section* section)
    {
        _section = section;
    }

    double estimatedTravelTime(unsigned int dir) const
    {
        return _estimatedTravelTimes[dir];
    }

    double volumeInSection() const
    {
        return _volumeInSection;
    }

    double densityInSection() const
    {
        return _densityInSection;
    }

    double meanVelocityInSection() const
    {
        return _meanVelocityInSection;
    }
};

#endif //__LINK_FLOW_RECORD_HPP__
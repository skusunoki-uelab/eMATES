/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ZebraODEdge.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __ZEBRA_OD_EDGE_HPP__
#define __ZEBRA_OD_EDGE_HPP__
#include "../Config.hpp"
#include "../RandomNumberGenerator.hpp"
#include <AmuLineSegment.hpp>
#include <AmuVector.hpp>
#include <deque>
#include <iostream>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

class InflowPedestrianMonitor;
class Pedestrian;
class Zebra;

//##############################################################################
/**
 * @~japanese
 * 横断歩道における歩行者の出発地・目的地となる辺
 *
 * @~english
 * Edge that is the origin and destination of pedestrian at a crosswalk
 *
 * @~ @ingroup PedSim RoadNetwork
 */
class ZebraODEdge
{
public:
    ZebraODEdge();
    ~ZebraODEdge();

    /**
     * @~japanese _waitingPedestrians に歩行者 @p ped を追加する
     * @~english  Append Pedestrian @p ped to _waitingPedestrians
     */
    void appendWaitingPedestrian(Pedestrian* ped);

    /**
     * @~japanese
     * _waitingPedestrians から歩行者をポップしシミュレーションに登録する
     *
     * @~english
     * Pop a pedestrian from _waitingPedestrians and register for simulation  
     */
    void pushPedestriansToRoadMap();

    /**
     * @~japanese
     * 横断歩道を渡り終えた歩行者 @p ped を登録する
     *
     * @~english
     * Add @p ped after crossing the crosswalk to _finishedPedestrians
     */
    void appendFinishedPedestrian(Pedestrian* ped);

    /**
     * @~japanese 横断歩道を渡り終えた歩行者を消去する
     * @~english  Delete pedestrians who finished crossing the crosswalk
     */
    void deleteFinishedPedestrians();

private:
    /**
     * @~japanese 歩行開始時の位置を戻す
     * @param baselineRatio スタートライン上の位置
     *
     * @note
     * baselineRatio が0.0の場合は左端，1.0の場合が右端．
     *
     * @~english Return the speed at the start of walking
     * @param baselineRatio position on the starting line
     *
     * @note
     * If baselineRatio is 0.0, it means the left end, and if baselineRatio is
     * 1.0, it means the right end.
     */
    amu::geometry::AmuPoint _initialPosition(double baselineRatio);

    //==================================================================
private:
    /**
     * @~japanese この辺が設置されている横断歩道
     * @~english  Crosswalks this edge is located on
     */
    Zebra* _zebra;

    /**
     * @~japanese 境界を表す線分
     * @~english  Line segment representing this edge
     */
    amu::geometry::AmuLineSegment _lineSegment;

    /**
     * @~japanese 横断方向
     * @~english  Crossing direction
     */
    int _crossingDirection;

    /**
     * @~japanese 発生する歩行者の希望歩行方向
     * @~english  Desired walking direction of the pedestrian
     */
    amu::math::AmuVector _direction;

    /**
     * @~japanese 歩行者の発生交通量 [ped./h]
     * @~english  Pedestrian generation volume [ped./h]
     */
    double _generationVolume;

    /**
     * @~japanese 歩行者の発生確率 [/step]
     *
     * _generationVolume に連動して与える．
     *
     * @~english  Pedestrian generation probability [/step]
     *
     * Give in conjunction with _generationVolume.
     */
    double _generationProbability;

    /**
     * @~japanese シミュレーションへの登場を待つ歩行者
     *
     * @attention
     * メインコンテナであり，ObjectManager で管理されない．このクラスの
     * デストラクタで格納されている歩行者を delete する必要がある．
     *
     * @~english  Pedestrians waiting to appear in the simulation
     *
     * @attention
     * It is main container and is NOT managed by ObjectManager. Need to delete
     * the stored pedestrians in the destructor of this class.
     */
    std::deque<Pedestrian*> _waitingPedestrians;

    /**
     * @~japanese 横断歩道を渡り終えた歩行者
     * @~english  Pedestrians who finished crossing the crosswalk
     */
    std::vector<Pedestrian*> _finishedPedestrians;

    /**
     * @~japanese 直前の歩行者を生成した時刻
     * @~english  The time generating the last pedestrian
     */
    ulint _lastGenerationTime;

    /**
     * @~japanese 直前の歩行者を横断歩道に配置した時刻
     * @~english  The time placing the last pedestrian on a zebra
     */
    ulint _lastInflowTime;

    /**
     * @~japanese 発生歩行者データを出力するかどうか
     * @~english  Whether to output generated pedestrian record 
     */
    bool _outputsGeneratedPedestrianRecord;

    /**
     * @~japanese 流入歩行者検出器
     * @~english  Inflow pedestrian monitor
     */
    InflowPedestrianMonitor* _inflowMonitor;

    /**
     * @~japanese 乱数生成器
     * @~english  Random number generator
     */
    RandomNumberGenerator _rng;

#ifdef _OPENMP
    /**
     * @~japanese ロック変数
     * @~english  Lock variable
     */
    omp_lock_t _lock;
#endif //_OPENMP

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const Zebra* zebra() const
    {
        return _zebra;
    }

    void setZebra(Zebra* zebra)
    {
        _zebra = zebra;
    }

    const amu::geometry::AmuLineSegment& lineSegment() const
    {
        return _lineSegment;
    }

    void setLineSegment(const amu::geometry::AmuLineSegment& line)
    {
        _lineSegment = line;
    }

    int crossingDirection() const
    {
        return _crossingDirection;
    }

    void setCrossingDirection(int dir)
    {
        _crossingDirection = dir;
    }

    const amu::math::AmuVector& direction() const
    {
        return _direction;
    }

    void setDirection(const amu::math::AmuVector& dir)
    {
        _direction = dir;
    }

    double generationVolume() const
    {
        return _generationVolume;
    }

    void setGenerationVolume(double volume);

    double generationProbability() const
    {
        return _generationProbability;
    }

    const InflowPedestrianMonitor* inflowMonitor()
    {
        return _inflowMonitor;
    }

    void setInflowMonitor(InflowPedestrianMonitor* monitor)
    {
        // inflowPedestrianMonitorは1回しかセットできない
        // inflowPedestrianMonitor can be set only once
        assert(!_inflowMonitor);
        _inflowMonitor = monitor;
    }

    bool outputsGeneratedPedestrianRecord() const
    {
        return _outputsGeneratedPedestrianRecord;
    }

    void setOutputsGeneratedPedestrianRecord(bool flag)
    {
        _outputsGeneratedPedestrianRecord = flag;
    }

    ///@}
};

#endif //__ZEBRA_OD_EDGE_HPP__
#endif //INCLUDE_PEDESTRIANS

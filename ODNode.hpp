/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ODNode.hpp
 */
#ifndef __OD_NODE_HPP__
#define __OD_NODE_HPP__
#include "Config.hpp"
#include "Intersection.hpp"
#include "RandomNumberGenerator.hpp"
#include <cassert>
#include <string>
#include <deque>
#include <vector>

class InflowMonitor;
class RoadMap;
class Vehicle;

#ifdef INCLUDE_TRAMS
class ODNodeTramExt;
#endif //INCLUDE_TRAMS

//##############################################################################
/**
 * @~japanese
 * 車両の起終点となる次数1の交差点
 *
 * Intersection クラスの派生クラス
 *
 * @~english
 * Intersection with degree 1 that is the origin and destination of vehicles
 *
 * Derived class of Intersection class
 *
 * @~ @ingroup RoadNetwork
 */
class ODNode : public Intersection
{
#ifdef INCLUDE_TRAMS
    friend class ODNodeTramExt;
#endif //INCLUDE_TRAMS

    //==========================================================================
public:
    ODNode(const std::string& id, const std::string& type, RoadMap* parent);
    ~ODNode();

    /**
     * @~japanese レーンの接続をチェックする
     * @param isIntersection 交差点かどうか
     * @return    レーンの接続が無矛盾かどうか
     *
     * @note
     * 単路部が接続しない境界では，上流レーンあるいは下流レーンが存在しない
     * ことを許容する．
     *
     * @~english  Check lane connectivity
     * @param isIntersection Whether it is intersection
     * @return    Whether the lane connectivity are consistent
     *
     * It is allowed that there is no upstream or downstream lane at borders
     * where sections do not connect. 
     */
    virtual bool checkLaneConnectivity(bool isIntersection) const override;

    //==========================================================================
    /**
     * @~japanese @name 車両の発生と消去に関わる関数
     * @~english  @name Functions related to generating and deleting vehicles
     */
    ///@{
public:
    /**
     * @~japanese _waitingVehicles に車両がセットされているかどうかを戻す
     * @~english  Return whether there any vehicles set in _watingVehicles
     */
    bool hasWaitingVehicles() const
    {
        return (_waitingVehicles.size() > 0);
    }

    /**
     * @~japanese _waitingVehicles に車両 @p vehicle を追加する
     * @~english  Append @p vehicle to _waitingVehicles
     */
    void appendWaitingVehicle(Vehicle* vehicle)
    {
        assert(vehicle);
        _waitingVehicles.push_back(vehicle);
    }

    /**
     * @~japanese _waitingVehicles の先頭に車両 @p vehicle を追加する
     *
     * @note
     * マニュアルで生成された車両や出発時刻が指定された車両用
     *
     * @~english  Prepend @p vehicle to _waitingVehicles
     *
     * @note
     * For manually generated vehicles or vehicles with a fixed
     * departure time
     */
    void prependWaitingVehicle(Vehicle* vehicle)
    {
        assert(vehicle);
        _waitingVehicles.push_front(vehicle);
    }

    /**
     * @~japanese このODノードに含まれるレーン上のエージェントを消去する
     * @~english  Delete agents on lanes contained in this ODNode
     */
    void deleteAgent();

    /**
     * @~japanese
     * _waitingVehicles から車両をポップしシミュレーションに登録する
     *
     * @~english
     * Pop a vehicle from _waitingVehicles and register for simulation
     */
    void pushVehicleToRoadMap(RoadMap* roadMap);

private:
    /**
     * @~japanese
     * このODノードから流出し，十分な空きのあるレーンを抽出する
     * 
     * @~english
     * Extract lanes with enough space from those flowing out from this ODNode.
     */
    void _extractOutflowLanesWithEnoughSpace(
        std::deque<const Lane*>& result_lanes);

    /**
     * @~japanese 車両 @p vehicle をレーン @p lane に配置する
     * @~english  Place @p vehicle in @p lane
     */
    void _placeVehicleInLane(
        Vehicle* vehicle, const Lane* lane, RoadMap* roadMap);

    ///@}

    //==========================================================================
protected:
    /**
     * @~japanese シミュレーションへの登場を待つ車両
     *
     * 生成された車両は，発生点下流に十分なスペースがあればレーンに配置される．
     * スペースがない場合はスペースができるまで待機する．
     *
     * @attention
     * メインコンテナであり，ObjectManager で管理されない．このクラスの
     * デストラクタで格納されている車両を delete する必要がある．
     *
     * @~english  Vehicles waiting to appear in the simulation
     *
     * A generated vehicle will be placed in a lane if there is enough space
     * downstream of the origin point. If there is not enough space, wait until
     * there is enough space.
     *
     * @attention
     * It is main container and is NOT managed by ObjectManager. Need to delete
     * the stored vehicles in the destructor of this class.
     */
    std::deque<Vehicle*> _waitingVehicles;

    /**
     * @~japanese
     * このODノードが pushVehicleToRoadMap の対象となっているかどうか
     *
     * @~english
     * Whether this ODNode is subject to pushVehicleToRoadMap 
     */
    bool _waitsToPushVehicle;

    /**
     * @~japanese 直前の車両を生成した時刻
     * @~english  The time generating the last vehicle
     */
    ulint _lastGenerationTime;

    /**
     * @~japanese 直前の車両をレーンに配置した時刻
     * @~english  The time placing the last vehicle on a lane
     */
    ulint _lastIncomeTime;

    /**
     * @~japanese 流入車両検出器
     * @~english  Inflow vehicle monitor
     */
    InflowMonitor* _inflowMonitor;

    /**
     * @~japanese 乱数生成器
     * @~english  Random number generator
     */
    RandomNumberGenerator _rng;

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    bool waitsToPushVehicle() const
    {
        return _waitsToPushVehicle;
    }

    void setWaitsToPushVehicle(bool flag)
    {
        _waitsToPushVehicle = flag;
    }

    const InflowMonitor* inflowMonitor()
    {
        return _inflowMonitor;
    }

    void setInflowMonitor(InflowMonitor* monitor)
    {
        // inflowMonitorは1回しかセットできない
        // inflowMonitor can be set only once
        assert(!_inflowMonitor);
        _inflowMonitor = monitor;
    }

    ///@}

#ifdef INCLUDE_TRAMS
    //==========================================================================
    /**
     * @~japanese @name 路面電車用拡張
     * @~english  @name Extension for tram
     */
    ///@{
protected:
    ODNodeTramExt* _odNodeTramExt;

public:
    ODNodeTramExt* odNodetramExt()
    {
        return _odNodeTramExt;
    }

    ///@}
#endif //INCLUDE_TRAMS
};

#endif //__OD_NODE_HPP__

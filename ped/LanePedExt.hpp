/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file LanePedExt.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __LANE_PED_EXT_HPP__
#define __LANE_PED_EXT_HPP__
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

class Lane;
class SubLaneBundle;
class Vehicle;

//######################################################################
/**
 * @~japanese 交差点クラスの歩行者拡張
 * @~english  Pedestrian extension of intersection class
 * @~ @ingroup PedSim RoadNetwork
 */
class LanePedExt
{
public:
    explicit LanePedExt(Lane* lane);
    ~LanePedExt();

    /**
     * @~japanese 接近する自動車の順序列を更新する
     * @~english  Update the sequence of approaching cars
     */
    void renewApproachingVehicleOrder();

    /**
     * @~japanese 接近する自動車 @p vehicle を追加する
     * @~english  Add approaching car @vehicle
     */
    void putApproachingVehicle(Vehicle* vehicle);

    /**
     * @~japanese 接近する自動車が存在するかどうかを戻す
     * @~english  Return whether there is an approaching car
     */
    bool hasApproachingVehicles() const
    {
        return (_approachingVehicles.size() > 0);
    }

private:
    /**
     * @~japanese 対応するレーン
     * @~english  Corresponding lane
     */
    Lane* _lane;

    /**
     * @~japanese レーンが含まれるサブセクション
     * @~english  Subsection containing this lane
     */
    SubLaneBundle* _subsec;

    /**
     * @~japanese 歩行者が接近しているかどうか
     * @~english  Whether there is an approaching pedestrian
     */
    bool _hasApproachingPedestrian;

    /**
     * @~japanese 接近している自動車の集合
     *
     * レーンに接近する自動車はまず _tmpApproachingVehicles にコピーされ，
     * 次のステップの認知処理の前に renewApproachingVehicleOrder() で
     * _approachingVehicles コピーされる．
     *
     * @~english  Set of approaching cars
     *
     * The car approaching the lane is first copied to
     * _tmpApproachingVehicles, and then copied to _approachingVehicles
     * in renewApproachingVehicleOrder() before recognition processing
     * in the next step.
     */
    std::vector<Vehicle*> _approachingVehicles;

    /**
     * @~japanese このレーンに接近している自動車の一時コンテナ
     * @~english  Temporary container for cars approaching this lane
     */
    std::vector<Vehicle*> _tmpApproachingVehicles;

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
    SubLaneBundle* subLaneBundle()
    {
        return _subsec;
    }

    void setSubLaneBundle(SubLaneBundle* subsec)
    {
        _subsec = subsec;
    }

    bool hasApproachingPedestrian() const
    {
        return _hasApproachingPedestrian;
    }

    void setHasApproachingPedestrian(bool flag)
    {
        _hasApproachingPedestrian = flag;
    }

    ///@}
};

#endif //__INTERSECTION_PED_EXT_HPP__
#endif //INCLUDE_PEDESTRIANS

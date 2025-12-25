/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file IntersectionPedExt.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __INTERSECTION_PED_EXT_HPP__
#define __INTERSECTION_PED_EXT_HPP__
#include "Zebra.hpp"
#include "ZebraODEdge.hpp"
#include <unordered_map>

class Intersection;

//######################################################################
/**
 * @~japanese 交差点クラスの歩行者拡張
 * @~english  Pedestrian extension of intersection class
 * @~ @ingroup PedSim RoadNetwork
 */
class IntersectionPedExt
{
public:
    IntersectionPedExt() {}
    explicit IntersectionPedExt(Intersection* inter) : _inter(inter) {}
    ~IntersectionPedExt() {}

    /**
     * @~japanese 境界方向 @p dir にある横断歩道を戻す
     * @~english  Return the crosswalk in the border direction @p dir
     */
    Zebra* zebra(int dir) const;

    /**
     * @~japanese 横断歩道 @p zebra を追加する
     * @~english  Add @p zebra
     */
    void addZebra(Zebra* zebra)
    {
        _zebras.insert(std::make_pair(zebra->id(), zebra));
    }

    /**
     * @~japanese 歩行者の順序列を更新する
     * @~english  Update the pedestrian sequence
     */
    void renewPedestrianOrder()
    {
        for (auto itr : _zebras)
        {
            itr.second->renewPedestrianOrder();
        }
    }

    /**
     * @~japanese 接近する自動車の順序列を更新する
     * @~english  Update the sequence of approaching cars
     */
    void renewOncomingVehicleOrder()
    {
        for (auto itr : _zebras)
        {
            itr.second->renewApproachingVehicleOrder();
        }
    }

    /**
     * @~japanese レーンに歩行者の接近を通知する
     * @~english  Notify lanes of approaching pedestrians
     */
    void notifyLaneOfApproachingPedestrian()
    {
        for (auto itr : _zebras)
        {
            itr.second->notifyLaneOfApproachingPedestrian();
        }
    }

    /**
     * @~japanese
     * 横断歩道を渡り終えた歩行者を消去する
     *
     * @~english
     *  Delete pedestrians who finished crossing the crosswalk
     */
    void deleteFinishedPedestrians()
    {
        for (auto itr : _zebras)
        {
            for (unsigned int dir = 0; dir < 2; dir++)
            {
                itr.second->odEdge(dir)->deleteFinishedPedestrians();
            }
        }
    }

private:
    /**
     * @~japanese 対応する交差点
     * @~english  Corresponding intersection
     */
    Intersection* _inter;

    /**
     * @~japanese 交差点に設置された横断歩道
     *
     * @note
     * Zebra は SubIntersection のサブクラスであるため， LaneBundleの
     * _subsecs に含まれるが，横断歩道特有の処理のためコンテナが必要
     *
     * @~english  Set of crosswalks installed at the intersection
     *
     * @note
     * Since Zebra is a subclass of SubIntersection, it is included in
     * _subsecs of LaneBundle, but a container is needed for crosswalk-
     * specific processing.
     */
    std::unordered_map<std::string, Zebra*> _zebras;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::unordered_map<std::string, Zebra*>& zebras() const
    {
        return _zebras;
    }

    ///@}
};

#endif //__INTERSECTION_PED_EXT_HPP__
#endif //INCLUDE_PEDESTRIANS

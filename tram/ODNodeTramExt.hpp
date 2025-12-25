/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ODNodeTramExt.hpp
 */
#ifdef INCLUDE_TRAMS
#ifndef __OD_NODE_TRAM_EXT_HPP__
#define __OD_NODE_TRAM_EXT_HPP__
#include <vector>
#include <deque>

class Lane;
class ODNode;
class RoadMap;
class Vehicle;
class VehicleTram;

//##############################################################################
/**
 * @~japanese ODノードの路面電車拡張
 * @~english  Tram extension of ODNode
 * @~ @ingroup TramSim
 */
class ODNodeTramExt
{
public:
    ODNodeTramExt() {};
    ODNodeTramExt(ODNode* odNode) : _odNode(odNode) {};
    ~ODNodeTramExt() {};

    /// 路面電車を発生させる
    /**
     * @~japanese
     * waitingVehicles から路面電車をポップしシミュレーションに登録する
     *
     * 発生できなかった路面電車を @p result_skippedTrams に格納する．
     *
     * @~english
     * Pop a tram from waitingVehicles and register for simulation
     *
     * Store the remains after popping the tram in @p result_remains .
     */
    void pushTramToRoadMap(
        RoadMap* roadMap, std::deque<Vehicle*>& waitingVehicles,
        std::vector<Vehicle*>& result_skippedTrams);
private:
    /**
     * @~japanese
     * 路面電車 @p tram を路面電車レーン @p tramLane に配置する
     *
     * @~english
     * Place @p tram in tram lane @p tramLane
     */
    void _placeTramInTramLane(
        VehicleTram* tram, const Lane* lane, RoadMap* roadMap);

private:
    /**
     * @~japanese 対応するODノード
     * @~english  Corresponding ODNode
     */
    ODNode* _odNode;
};

#endif //__OD_NODE_TRAM_EXT_HPP__
#endif //INCLUDE_TRAMS

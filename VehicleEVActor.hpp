#ifndef __VEHICLE_EV_ACTOR_HPP__
#define __VEHICLE_EV_ACTOR_HPP__

#include "VehicleActor.hpp"

class VehicleEV;

// EV特有の挙動を表現するための拡張
// [eMATES]
class VehicleEVActor : public VehicleActor
{
public:

    /**
     * @~japanese 行動する
     * @~english  Perform action
     */
    virtual void act() override;

    // 充電中の処理
    void charge();

    // Vehicleの処理を上書き（CS再探索処理を追加）
    // Vehicle版を呼び出すこともあるので注意
    void _runIntersection2Section();

    /// 単路からCSに移る
    void _runSection2CS();

    // ev->searchCS() で求めた候補CS or 目的地を経路に適用する
    // target != destination の場合は、経由地リストの現在位置にtarget(CS)を挿入する
    // target == destination の場合は、経由地リストをtargetに置き換える
    void _setTargetIntoGates(Intersection* nextIntersection, Intersection* target);
};

#endif // __VEHICLE_EV_ACTOR_HPP__

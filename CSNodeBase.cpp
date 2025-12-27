#include "CSNodeBase.hpp"
#include "Vehicle.hpp"
#include "AppMates.hpp"
#include "VehicleEV.hpp"

#include <AmuConverter.hpp>
#include <string>

using amu::converter::formatId;

////======================================================================
CSNodeBase::CSNodeBase(const std::string& id, const std::string& type, RoadMap* parent) : ODNode(id, type, parent)
{
  // NOTE 25.03.14 abe -- Intersection* ptrのptr->isCS()を廃止する。
  // dynamic_cast<CSNodeBase*>(ptr) != nullptr で代用する。
  _integratedCharge = 0.0;

  _occupancy = 0;
  _servedEV = 0;

  // 追加251225：フィーダー関連メンバ変数の初期化
  _feederID = "";
  _gridCostWeight = 1.0;
}

////======================================================================
void CSNodeBase::sumOccupancy()
{
    _occupancy += AppMates::getTimeManager().unit();
}

////======================================================================
long long CSNodeBase::occupancy() const
{
    return _occupancy;
}


////======================================================================
void CSNodeBase::sumCharge(double chargingValue)
{
    _instantaneousCharge += chargingValue;
}

////======================================================================
void CSNodeBase::initCharge()
{
    _instantaneousCharge = 0;
}

////======================================================================
double CSNodeBase::instantaneousCharge() const
{
    return _instantaneousCharge;
}

////======================================================================
void CSNodeBase::integrated(double instantaneousCharge)
{
    _integratedCharge += instantaneousCharge;
}

////======================================================================
double CSNodeBase::integratedCharge() const
{
    return _integratedCharge;
}

////======================================================================
void CSNodeBase::initIntegrated()
{
    _integratedCharge = 0.0;
}

////======================================================================
void CSNodeBase::addEV(VehicleEV* vehicle)
{
    _waitingLine.push_back(vehicle);
    vehicle->setWaiting(true);

    // 追加直後に充電すべきEVの指定
    _assignEVIntoCharger();
}

////======================================================================
int CSNodeBase::capacity() const
{
    return _capacity;
}

////======================================================================
double CSNodeBase::outPower() const
{
    return _outPower;
}

////======================================================================
int CSNodeBase::servedEV() const
{
    return _servedEV;
}

////======================================================================
// 追加251226：配電網コスト計算（楠木）
double CSNodeBase::calculateGridCost() const
{
    // フィーダー別のペナルティ係数を返す
    // OpenDSS連成時は外部から_gridCostWeightが設定される想定
    // スケーリング: route/waitingコスト(600-1000)と同オーダーにする
    return _gridCostWeight * GRID_PENALTY_SCALE;
}

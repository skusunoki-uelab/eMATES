#include "CSNodeNormal.hpp"

#include "AppMates.hpp"
#include "VehicleEV.hpp"
#include "VehicleEVBodyProperty.hpp"
#include "GVManager.hpp"
#include "Charger.hpp"
#include <string>
#include <vector>

using std::vector;
using std::string;

//======================================================================
CSNodeNormal::CSNodeNormal(const string& id,
                const string& type,
                RoadMap* parent)
    : CSNodeBase(id, type, parent)
{
}

//======================================================================
CSNodeNormal::~CSNodeNormal()
{
    // by abe 2022/2/25
    delete _charger;
}

////======================================================================
void CSNodeNormal::_assignEVIntoCharger()
{
    vector<VehicleEV*>::iterator itr = _waitingLine.begin();
    for (int i = 0; i < _capacity; i++)
    {
        if (itr == _waitingLine.end())
        {
            break;
        }

        // add by abe 2022/3/11
        _charger->setVehicle(*itr);

        (*itr)->setChargingInCS(true);
        itr++;
    }
}

////======================================================================
void CSNodeNormal::removeEV(VehicleEV* vehicle)
{
    // by abe 2022/3/11
    // remove対象EVを限定
    auto itr = find(_waitingLine.begin(), _waitingLine.end(), vehicle);
    // 車両が見つからなければ終了
    if (itr == _waitingLine.end())
    {
        return;
    }

    // 車両が見つかった場合
    // 充電器内の当該車両削除
    _charger->removeVehicle(*itr);

    (*itr)->setWaiting(false);
    _waitingLine.erase(itr);

    _servedEV++;

    // 削除直後にも充電すべきEVの指定
    // modified by abe 2022/3/11
    if (_waitingLine.size() != 0)
    {
        _assignEVIntoCharger();
    }
}

////======================================================================
void CSNodeNormal::setCapacity(int capacity)
{
    _capacity = capacity;
    // add by abe 2022/2/25
    assert(_charger == nullptr);
    int timeSlots = AppMates::getGVManager().getNumeric("EV_COMM_PREDICTION_TIME_SLOT");
    _charger = new ChargerNormal(0, capacity, timeSlots);

}

//======================================================================
void CSNodeNormal::setRatingPower(double ratingPower)
{
    _ratingPower = ratingPower;
    // 定格と同時に制御値も書き換える
    _outPower    = ratingPower * _capacity;
}

//======================================================================
void CSNodeNormal::setOutPower(double outPower)
{
    _outPower = std::min(_ratingPower * _capacity, outPower);
}

//======================================================================
double CSNodeNormal::chargingTimeForRouting(VehicleEV* ev) const
{
    double batteryCapacityWs
        = static_cast<const VehicleEVBodyProperty*>(ev->body())
        ->batteryCapacityWs();
    double requiredPowerWs = (0.8 - ev->SOC()) * batteryCapacityWs;
    // 出力制御値[kW]はCSに紐付く
    // 注意：充電中車両台数に依存する
    double numChargingVehicles = static_cast<double>(_charger->vehicles().size());
    double outPowerW = 1000.0 * std::min(
            _outPower / (numChargingVehicles + 1.0),
            _ratingPower);
    return requiredPowerWs / outPowerW;
}

//======================================================================
double CSNodeNormal::priceForRouting() const
{
    return _charger->prices()[0];
}

//======================================================================
ChargerBase* CSNodeNormal::charger(int) const
{
  return _charger;
}

//======================================================================
double CSNodeNormal::ratingPower() const
{
    return _ratingPower;
}


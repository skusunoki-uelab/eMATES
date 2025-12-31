#include "CSNodeFast.hpp"

#include "AppMates.hpp"
#include "Charger.hpp"
#include "GVManager.hpp"
#include "VehicleEV.hpp"
#include "VehicleEVBodyProperty.hpp"
#include <string>
#include <vector>

using std::string;
using std::vector;

//======================================================================
CSNodeFast::CSNodeFast(const string &id, const string &type, RoadMap *parent)
    : CSNodeBase(id, type, parent) {}

//======================================================================
CSNodeFast::~CSNodeFast() {
  // by abe 2022/2/25
  for (auto *charger : _chargers) {
    delete charger;
  }
}

////======================================================================
void CSNodeFast::_assignEVIntoCharger() {
  vector<VehicleEV *>::iterator itr = _waitingLine.begin();

  // // DEBUG2024/05/28
  // cout << "_waitingLine:" << endl;
  // for(int i = 0; i < _waitingLine.size(); i++)
  // {
  //     cout << i << " " << _waitingLine[i]->id();
  //     //待ち行列のindexとそれに対応する車両ID
  // }
  // cout << endl;
  // cout << "chargers:" << endl;
  // for(int i = 0; i < _chargers.size(); i++)
  // {
  //     cout << i << " " << _chargers[i]->id(); //該当するCSの充電器index
  // }
  // cout << endl;

  for (int i = 0; i < _capacity; i++) {
    // cout << "capacity: " << _capacity << endl;// DEBUG2024/05/28

    if (itr == _waitingLine.end()) {
      break;
    }

    // by abe 2022/2/25
    // 充電器に登録済か確認
    auto itrCgr =
        find_if(_chargers.begin(), _chargers.end(),
                [itr](ChargerFast *cgr) { return cgr->vehicle() == *itr; });

    // 未登録の場合、空きを探して登録
    if (itrCgr == _chargers.end()) {
      auto itrNull =
          find_if(_chargers.begin(), _chargers.end(),
                  [itr](ChargerFast *cgr) { return !cgr->isFull(); });
      // 必ず見つかるはず
      assert(itrNull != _chargers.end());
      (*itrNull)->setVehicle(*itr);
    }

    // debug by takusagawa 2018/11/10
    // cout << "CSid: " << id() << " Vehicle ID: " << (*itr)->id() << endl;
    (*itr)->setChargingInCS(true);
    itr++;
  }
}

////======================================================================
void CSNodeFast::removeEV(VehicleEV *vehicle) {
  // by abe 2022/3/11
  // remove対象EVを限定
  auto itr = find(_waitingLine.begin(), _waitingLine.end(), vehicle);
  // 車両が見つからなければ終了
  if (itr == _waitingLine.end()) {
    return;
  }

  // 車両が見つかった場合
  // 充電器内の当該車両削除
  auto itrCgr =
      find_if(_chargers.begin(), _chargers.end(), [vehicle](ChargerFast *cgr) {
        return cgr->vehicle() == vehicle;
      });
  assert(itrCgr != _chargers.end());
  (*itrCgr)->removeVehicle(*itr);

  // cout << "remove vehicle id: " << (*itr)->id() << endl;
  (*itr)->setWaiting(false);
  _waitingLine.erase(itr);

  // vector<Vehicle*>::iterator itr = _waitingLine.begin();
  //// (*itr)->setWaiting(false);
  //// debug by takusagawa 2018/11/10
  //// cout << "erase vehicle ID: " << (*itr)->id() << " SOC: " << (*itr)->SOC()
  ///<< endl;

  //// debug by uchida 2017/6/23
  //// cout << "~ " << TimeManager::time() / 1000 << " [s] "
  ////     << (*itr)->id() << " : restart from " << _id << endl;

  //// by takusagawa 2018/11/10
  ////
  ///_waitingLine内において,removeEV()を呼び出した車両より前にまだ充電が終わっていない
  //// 車両が存在する場合の処理を追加
  // for (int i = 0; i < _capacity; i++)
  //{
  //     if ((*itr)-> SOC() < 0.8)
  //     {
  //         itr++;
  //         continue;
  //     }
  //     else if (itr == _waitingLine.end())
  //     {
  //         break;
  //     }
  //     else
  //     {
  //         // by abe 2022/2/25
  //         // 充電器内の当該車両削除
  //         auto itrCgr = find_if(_chargers.begin(), _chargers.end(),
  //                 [itr](ChargerFast* cgr){ return cgr->vehicle() == *itr;});
  //         if (itrCgr != _chargers.end())
  //         {
  //             (*itrCgr)->removeVehicle(*itr);
  //         }
  //
  //         // cout << "remove vehicle id: " << (*itr)->id() << endl;
  //         (*itr)->setWaiting(false);
  //         _waitingLine.erase(itr);
  //         break;
  //     }
  // }

  _servedEV++;

  // 削除直後にも充電すべきEVの指定
  // modified by abe 2022/2/25
  if (_waitingLine.size() != 0) {
    // cout << "removeEV CSid: " << id() << endl; //DEBUG2024/05/28
    _assignEVIntoCharger();
  }
}

////======================================================================
const vector<VehicleEV *> CSNodeFast::chargingVehicles() const {
  vector<VehicleEV *> v{};
  for (ChargerFast *cgr : _chargers) {
    v.push_back(cgr->vehicle());
  }
  return v;
}

////======================================================================
const vector<ChargerFast *> &CSNodeFast::chargers() const { return _chargers; }

////======================================================================
ChargerBase *CSNodeFast::charger(int cgrId) const {
  return _chargers.at(cgrId);
}

////======================================================================
void CSNodeFast::renewEstimatedWaitingTime() {
  int size = _waitingLine.size();
  
  // 初期化
  _estimatedWaitingTime = 0;

  // 待機列が空なら待ち時間はゼロ
  if (size == 0) {
    return;
  }

  // 2025/12/31 修正（capacity変更に対応）
  // _waitingLine構造: [0～capacity-1: 充電中, capacity～: 待機中]
  // 新規到着車両は待機列の最後尾に並ぶ
  
  // ケース1: 空きがある場合（size < capacity）
  if (size < _capacity) {
    // 即座に充電開始可能
    _estimatedWaitingTime = 0;
    return;
  }
  
  // ケース2: 満車の場合（size >= capacity）
  // 新規車両は、充電中車両のうち最も早く完了する1台が空くまで待ち、
  // その後、待機中の全車両の充電完了を待つ必要がある
  
  // 充電中車両（capacity台）の残り時間の最小値を計算
  double minChargingTime = std::numeric_limits<double>::max();
  for (int i = 0; i < _capacity && i < size; i++) {
    VehicleEV *ev = _waitingLine[i];
    double batteryCapacityWs =
        static_cast<const VehicleEVBodyProperty *>(ev->body())
            ->batteryCapacityWs();
    double requiredPowerWs = (0.8 - ev->SOC()) * batteryCapacityWs;
    double outPowerW = _outPower * 1000.0;
    double chargingTime = requiredPowerWs / outPowerW;
    minChargingTime = std::min(minChargingTime, chargingTime);
  }
  _estimatedWaitingTime = minChargingTime;
  
  // 待機中車両（capacity台目以降）の充電時間を順次加算
  for (int i = _capacity; i < size; i++) {
    VehicleEV *ev = _waitingLine[i];
    double batteryCapacityWs =
        static_cast<const VehicleEVBodyProperty *>(ev->body())
            ->batteryCapacityWs();
    double requiredPowerWs = (0.8 - ev->SOC()) * batteryCapacityWs;
    double outPowerW = _outPower * 1000.0;
    _estimatedWaitingTime += requiredPowerWs / outPowerW;
  }
}

////======================================================================
void CSNodeFast::setCapacity(int capacity) {
  _capacity = capacity;

  // add by abe 2022/2/25
  assert(_chargers.empty());
  int timeSlots =
      AppMates::getGVManager().getNumeric("EV_COMM_PREDICTION_TIME_SLOT");
  for (int i = 0; i < capacity; i++) {
    _chargers.emplace_back(new ChargerFast(i, timeSlots));
  }
}

//======================================================================
void CSNodeFast::setRatingPower(double ratingPower) {
  _ratingPower = ratingPower;
  // 定格と同時に制御値も書き換える
  _outPower = ratingPower;
}

//======================================================================
void CSNodeFast::setOutPower(double outPower) {
  _outPower = std::min(_ratingPower, outPower);
}

//======================================================================
double CSNodeFast::chargingTimeForRouting(VehicleEV *ev) const {
  // 2025/12/31 修正
  // CS探索を行ったEV自身の充電時間のみを計算
  // 充電時間 = 必要充電量 / 現在の充電出力値（秒）
  
  double batteryCapacityWs =
      static_cast<const VehicleEVBodyProperty *>(ev->body())
          ->batteryCapacityWs();
  double requiredPowerWs = (0.8 - ev->SOC()) * batteryCapacityWs;
  // 出力制御値[kW]はCSに紐付く
  double outPowerW = 1000.0 * _outPower;

  return requiredPowerWs / outPowerW;
}

//======================================================================
double CSNodeFast::priceForRouting() const {
  double price{0};
  // CSに複数の急速充電器がある場合、平均価格とする
  for (ChargerFast *cgr : _chargers) {
    price += cgr->prices()[0];
  }
  price /= static_cast<double>(_chargers.size());
  return price;
}

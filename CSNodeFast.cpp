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
  using std::cout;
  using std::endl;
  int size = _waitingLine.size();
  // cout << "waiting line size: " << size << endl;
  //  初期化
  _estimatedWaitingTime = 0;

  // CSが満車でなければ待ち時間はゼロ
  if (size < _capacity) {
    return;
  }

  // 終了判定にsizeをそのまま用いると、余分に推定待ち時間が加算されてしまう。
  int realSize = size - _capacity + 1;

  assert(realSize > 0);
  // TODO indexあってる？
  // capacity分差し引くとしても、iを先頭から回したら、充電中車両が抽出されてしまうのでは？
  for (int i = 0; i < realSize; i++) {
    VehicleEV *ev = _waitingLine[i];
    double batteryCapacityWs =
        static_cast<const VehicleEVBodyProperty *>(ev->body())
            ->batteryCapacityWs();                                  // [Ws]
    double requiredPowerWs = (0.8 - ev->SOC()) * batteryCapacityWs; // [Ws]
    // おそらく間違っているので変更する byKusunoki 251119
    // なぜシミュレーションの1タイムステップを1000で割ってかけているのか不明
    double outPowerW = (_outPower * 1000); // [W]
    // double outPowerWs = (_outPower * 1000) *
    // (AppMates::getTimeManager().unit() / 1000.0); // [Ws] by abe 2025/6/17
    // 定数 1000.0 と 10 の意味が不明のため、外して計算するように変更した
    //_estimatedWaitingTime += (1000.0 * requiredPowerWs) / (10 * outPowerWs);
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
  // 2025/07/10 by abe 2022年度テスト条件
  // double batteryCapacityWs
  //    = static_cast<const VehicleEVBodyProperty*>(ev->body())
  //    ->batteryCapacityWs();
  // double requiredPowerWs = (0.8 - ev->SOC()) * batteryCapacityWs;
  //// 出力制御値[kW]はCSに紐付く
  // double outPowerW = 1000.0 * _outPower;

  // TODO 2025/07/10 by abe 考え方があっているか要確認
  // 2023/1/17 by uchida
  // CSの待機列を考慮した充電待ち時間算出
  // 出力制御はCS単位であり、現時点でcharger単位で与えることはできない
  // したがって、ここではoutPowerとchargerの基数（=capacity）の積を利用する
  double requiredPowerWs = 0;
  for (VehicleEV *ev2 : _waitingLine) {
    double batteryCapacityWs =
        static_cast<const VehicleEVBodyProperty *>(ev2->body())
            ->batteryCapacityWs();
    requiredPowerWs += (0.8 - ev2->SOC()) * batteryCapacityWs;
  }
  double outPowerW = _outPower * 1000.0 * capacity();

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

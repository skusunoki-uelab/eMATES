#ifndef __VEHICLE_EV_HPP__
#define __VEHICLE_EV_HPP__

#include "Vehicle.hpp"

// [eMATES]
class RouterBase;
class VehicleEV : public Vehicle {

public:
  VehicleEV();
  virtual ~VehicleEV() = default;

protected:
  // CS探索時のコスト無限大とみなす値
  constexpr static double MAX_COST = 1e10;

  //--------------------------------------------------
  // 状態関連

  /// 目的CS
  /// 目的地直行の場合もあるため、Intersection* とした
  Intersection *_targetCS{nullptr};
  // 最後に到達したCS（旧 _stopCSTemp）
  const Intersection *_visitedCS{nullptr};

  // by uchida 2016/5/10
  // 充電走行フラグ
  bool _isRunningToCharge{false};

  // CS充電中フラグ
  bool _isChargingInCS{false};

  // EVの充電待ち用フラグ
  bool _waiting{false};

  /// 充電残量[Wsec]
  double _batteryRemain{0};

  /// 初期充電率
  double _initialSOC{0};

  /// 充電電力量[Wsec]
  double _chargingValue{0};

  /// 1タイムステップの電力消費量[W]（瞬時値）
  // double _instantaneousValue;

  /// 電装品＋空調等の消費電力[Wsec]
  double _accessory{200.0};

  /// 充電走行開始時の閾値の設定
  double _threshold{0.2};

  /// CSの待ち時間を受け取れるか
  bool _canReceiveWaitingInfo{false};

  double _swapTime{0};

  //--------------------------------------------------
  // 挙動・行動関連

  //--------------------------------------------------
  // CS関連ログ

  // by uchida 2017/2/8
  ulint _startChargingTime{0};

  ulint _restartTime{0};

  // by takusagawa 2018/11/9
  ulint _waitingLineEntryTime{0};

  // CS探索時のコスト（初期値は -1 とか）
  double _chosenCSCost = 0.0;
  double _relativeGap = 0.0;

  // 経路探索時の迂回距離追加（楠木：251228）
  double _detourDistance = 0.0;
  double _closestCSRouteCost = 0.0;
  double _chosenCSRouteCost = 0.0;

public:
  //--------------------------------------------------
  // 状態関連

  double chargingValue() const { return _chargingValue; }
  void setChargingValue(double value) { _chargingValue = value; }
  void addBatteryRemain(double value) { _batteryRemain += value; }
  double batteryRemain() const { return _batteryRemain; }
  // 最後に使った CS 探索コストを取得
  double chosenCSCost() const { return _chosenCSCost; }
  double relativeGap() const { return _relativeGap; }
  // 必要なら外部からセットできるようにする
  void setChosenCSCost(double c) { _chosenCSCost = c; }
  void setRelativeGap(double g) { _relativeGap = g; }

  // 経路探索時の迂回距離setter/getter（楠木：251228）
  double detourDistance() const { return _detourDistance; }
  void setDetourDistance(double d) { _detourDistance = d; }
  double closestCSRouteCost() const { return _closestCSRouteCost; }
  void setClosestCSRouteCost(double c) { _closestCSRouteCost = c; }
  double chosenCSRouteCost() const { return _chosenCSRouteCost; }
  void setChosenCSRouteCost(double c) { _chosenCSRouteCost = c; }
  
  // 目的CSを返す
  Intersection *targetCS() { return _targetCS; }
  void setTargetCS(Intersection *cs) { _targetCS = cs; }

  // 到達済みCS
  const Intersection *visitedCS() { return _visitedCS; }
  void setVisitedCS(const Intersection *cs) { _visitedCS = cs; }

  void setRunningToCharge(bool state) { _isRunningToCharge = state; }

  /// 充電フラグが立っているか返す
  bool isRunningToCharge() const { return _isRunningToCharge; }

  // by uchida 2017/6/21
  // _onChargingを設定する
  void setChargingInCS(bool state);

  bool isChargingInCS() const { return _isChargingInCS; }

  // _waitingを設定する
  void setWaiting(bool state) { _waiting = state; }

  bool isWaiting() { return _waiting; }

  /// 充電率を返す
  double SOC();

  /// 初期充電率を返す
  double initialSOC() const { return _initialSOC; }

  // 初期SOCを設定する
  // 注意：EVの車両属性（充電容量）設定後に実行すること
  void setInitialSOC();

  bool threshold() { return SOC() < _threshold; }

  /// 電池特性を返す
  // Battery Characteristics
  double BC();

  /// 回生率を返す
  double regenerativeRate() const;

  void setSwapTime(int time) { _swapTime = time; }

  int swapTime() const { return _swapTime; }

  double accessory() const { return _accessory; }

  //--------------------------------------------------
  // 挙動・行動関連(git dev)

public:
  // 経路コスト、CS充電時間コスト、CS充電料金コスト、配電網ペナルティ
  struct CSCost {
    double route;
    double chargeTime;
    double yen;
    double waiting;
    double gridPenalty;  // 追加251226：配電網ペナルティ（楠木）
    CSCost(double route_, double chargeTime_, double yen_, double waiting_, double gridPenalty_ = 0.0)
        : route(route_), chargeTime(chargeTime_), yen(yen_), waiting(waiting_), gridPenalty(gridPenalty_) {
    }
  };
  // CS探索の戦略メソッドのポインタ型
  using CSSearchStrategyFnPtr = CSCost (VehicleEV::*)(RouterBase *,
                                                      const Intersection *,
                                                      const Intersection *,
                                                      const Intersection *);

  // CS探索：callbackを切り替えて使う
  // NOTE 目的地直行の可能性があるため、CSでないIntersectionも返しうる。
  Intersection *searchCS(CSSearchStrategyFnPtr callback);

  /// CS探索 ランダム
  CSCost evalByRandom(RouterBase *router, const Intersection *rear,
                      const Intersection *front, const Intersection *target);

  /// CS探索 ユークリッド距離
  CSCost evalByEuclid(RouterBase *router, const Intersection *rear,
                      const Intersection *front, const Intersection *target);

  /// CS探索 経路探索あり
  CSCost evalByCost(RouterBase *router, const Intersection *rear,
                    const Intersection *front, const Intersection *target);

  /// CS探索 経由ありの経路探索
  CSCost evalBySumCost(RouterBase *router, const Intersection *rear,
                       const Intersection *front, const Intersection *target);

  // by takusagawa 2018/9/25
  // CS探索 経由ありの経路探索 推定待ち時間考慮
  CSCost evalByWaitingTimeSumCost(RouterBase *router, const Intersection *rear,
                                  const Intersection *front,
                                  const Intersection *target);

  // by takusagawa 2018/11/4
  // CS探索 経由ありの経路探索 未来の推定待ち時間考慮
  CSCost evalByFutureWaitingTimeSumCost(RouterBase *router,
                                        const Intersection *rear,
                                        const Intersection *front,
                                        const Intersection *target);

protected:
  /// CS探索時の経路探索機準備
  void _prepareRouter(RouterBase *router);

public:
  //--------------------------------------------------
  // CS関連ログ

  // by uchida 2017/2/8
  /// CSの入庫時刻を指定する
  void setStartChargingTime(ulint startChargingTime) {
    _startChargingTime = startChargingTime;
  }

  /// CSの入庫時刻を取得する
  ulint startChargingTime() const { return _startChargingTime; }

  /// CSからの再出発時刻を指定する
  void setRestartTime(ulint restartTime) { _restartTime = restartTime; }

  /// CSからの再出発時刻を取得する
  ulint restartTime() const { return _restartTime; }

  // by takusagawa 2018/11/9
  // CSのwaitingLineに入庫した時刻を登録する
  void setWaitingLineEntryTime(ulint waitingLineEntryTime) {
    _waitingLineEntryTime = waitingLineEntryTime;
  }

  // CSのwaitingLineに入庫した時刻を取得する
  ulint waitingLineEntryTime() const { return _waitingLineEntryTime; }

  bool canReceiveWaitingInfo() const { return _canReceiveWaitingInfo; }
};

#endif // __VEHICLE_EV_HPP__

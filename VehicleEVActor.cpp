#include "VehicleEVActor.hpp"
#include "AppMates.hpp"
#include "Blinker.hpp"
#include "CSNodeFast.hpp"
#include "CSNodeNormal.hpp"
#include "GVManager.hpp"
#include "Intersection.hpp"
#include "Lane.hpp"
#include "LocalLaneRouter.hpp"
// #include "RandomNumberGenerator.hpp"
#include "RoadMap.hpp"
#include "Section.hpp"
#include "TimeManager.hpp"
#include "Vehicle.hpp"
#include "VehicleBehavior.hpp"
#include "VehicleBodyProperty.hpp"
#include "VehicleDecision.hpp"
#include "VehicleEV.hpp"
#include "VehicleEVBodyProperty.hpp"
#include "VehicleGlobalRoute.hpp"
#include "VehicleLocalRoute.hpp"
#include "VehicleLocation.hpp"
#include "VehicleScene.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "ped/VehiclePedExt.hpp"
#endif // INCLUDE_PEDESTRIANS
#include <cassert>

using std::cerr;
using std::cout;
using std::endl;

//======================================================================
void VehicleEVActor::act() {
  VehicleEV *ev = static_cast<VehicleEV *>(_vehicle);
  const VehicleEVBodyProperty *body =
      static_cast<const VehicleEVBodyProperty *>(_body);
  // 充電中の処理
  // 最終的には_onChargingをfalseに戻して再出発させないと
  // あと、sectionに車配置しとくのもよくない気はする
  if (ev->isChargingInCS()) {
    // swapTimeのカウントダウン
    int swapTime = ev->swapTime();
    if (swapTime >= 1) {
      ev->setSwapTime(swapTime - 1);
      // cout << "SWAPTIME " << swapTime << " Veh " << ev->id() << endl;
      _behavior->setSleepDuration(AppMates::getTimeManager().unit());
    } else
    // swapTimeがゼロになったら充電
    {
      charge();
    }

    const_cast<Lane *>(_location->lane())->registerAgentToAdd(_vehicle);
    return;
  }

  if (ev->isWaiting()) {
    _behavior->setSleepDuration(AppMates::getTimeManager().unit());
    const_cast<Lane *>(_location->lane())->registerAgentToAdd(_vehicle);
    return;
  }

  // 充電残量が直前ステップで負になった場合
  // 充電切れとしてレーンを閉塞させるため，消去する
  // 充電切れ判定のタイミングは再考の余地あり
  // (15/11/17 by uchida)
  if (ev->batteryRemain() <= 0) {
    const_cast<RoadMap *>(_location->roadMap())->addStrandedAgent(ev);
    return;
  }

  // ここから、Vehicleの処理と同じ
  _movesToNextLane = false;

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // 加速度および速度の更新
  // Update acceleration and velocity
  _behavior->setAccel(_decision->decidedAccel());
  _renewVelocity();

  // 速度履歴の保存
  // Record velocity history
  if (AppMates::getGVManager().getFlag("VEHICLE_VELOCITY_HISTORY_RECORD") &&
      (AppMates::getTimeManager().step() - _location->startingStep()) %
              (int)AppMates::getGVManager().getNumeric(
                  "VEHICLE_VELOCITY_HISTORY_INTERVAL") ==
          0) {
    _renewVelocityHistory();
  }

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // 休止状態タイマのカウントダウン
  // Countdown timer for inactive state
  if (_behavior->sleepDuration() > 0) {
    _behavior->addSleepDuration(-AppMates::getTimeManager().unit());
  }
  // 休止状態の開始
  // Start inactive state
  else if (_decision->shouldSleep()) {
    _behavior->setSleepDuration(1500);
    _decision->setShouldSleep(false);
  }

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // 距離の更新
  // Update distance
  /**
   * @todo 2次精度にする
   */
  double gain = ev->velocity() * AppMates::getTimeManager().unit();
  _location->setOldDistance(ev->distance());
  _location->addDistance(gain);
  _location->addDistanceFromInflowBorder(gain);
  _location->addTripLength(gain);

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // レーンへの登録
  // Registration to lane
  if (_location->distance() < _location->lane()->length()) {
    //--------------------------------------------------------------
    /*
     * 車両がレーンの終点に達していない場合は同一レーンに登録する
     *
     * Register in the same lane if the car has not reached the end
     * of the lane.
     */
    const_cast<Lane *>(_location->lane())->putAgent(_vehicle);

    /*
     * 加えて，車両が単路部にいる場合で，次の交差点まで50m以内で，
     * 次に右左折するときはウィンカーを点ける
     *
     * Additionally, turn on the blinker if the car is on a section,
     * is within 50m to the next intersection, and will turn left or
     * right.
     */
    if (_location->section()) {
      if (_location->section()->distanceToNext(_location->lane(),
                                               _location->distance()) <= 50 &&
          (_localRoute->turning().value() != _blinker->direction())) {
        if (_localRoute->turning() == RD::LEFT) {
          _blinker->setLeft();
        } else if (_localRoute->turning() == RD::RIGHT) {
          _blinker->setRight();
        } else {
          _blinker->setNone();
        }
      }
    }

    // 単路部内では5秒おきに車線変更を判断する
    // Judge lane-change in every 5 seconds in a section
    if (_location->section() && (!_decision->isLaneChangeExecutable() &&
                                 !_decision->isLaneChangeActive())) {
      if (((AppMates::getTimeManager().time() - _location->startingTime()) %
               5000 ==
           0) &&
          (AppMates::getTimeManager().time() != _location->startingTime())) {
        _requiresLocalReroute = true;
      }
    }
  } else {
    //--------------------------------------------------------------
    // 次の車線に移る
    // Move to next lane

    // 次の車線を基準にするので，oldDistanceはマイナス
    // oldDistance is negative because it is based on the next lane
    _location->addOldDistance(-(_location->lane()->length()));
    _location->addDistance(-(_location->lane()->length()));

    // VehicleLocationの_lane, _intersection, _sectionの更新
    // Update _lane, _intersection, _section of VehicleLocation
    if (!_location->section()) {
      assert(_location->intersection());

      // 単路部内の停止回数をクリア
      // Clear the number of pausing in section
      _behavior->setNumPausing(0);

      if (_location->intersection()->containsLane(_location->nextLane())) {
        // 交差点内の次の車線へ
        // To the next lane in the intersection
        _runIntersection2Intersection();
      } else {
        // 交差点から単路部へ
        // From intersection to sectioS
        _location->setDistanceFromInflowBorder(0);
        _runIntersection2Section();
        if (_blinker->direction() != Blinker::NONE) {
          _blinker->setNone();
        }
      }
    } else {
      // ここまで、Vehicleの処理と同じ。

      // by abe 2025/4/10 [eMATES]
      Intersection *frontIntersection = _location->section()->intersection(
          _location->section()->isUp(_location->lane()));
      if (_location->section()->containsLane(_location->nextLane())) {
        // 単路部内の次の車線へ
        // To the next lane in the section
        _runSection2Section();
      } else {
        // by abe 2025/4/10 [eMATES]
        // もともとこの条件とANDで、Destination != targetCS が入っていたが、
        // 最終目的地かどうか問わず、targetCSで充電するのが自然と考え、条件を除外した。
        // by uchida 2025/8/21 [eMATES]
        // --- 追加: Destination(=goal) が CSNodeNormal なら必ず入庫 ---
        if (typeid(*frontIntersection) == typeid(CSNodeNormal) &&
            frontIntersection == _globalRoute->goal()) {
          // targetCS が未設定でも確実に入庫させる
          ev->setTargetCS(frontIntersection);
          _runSection2CS();
        } else if (frontIntersection == ev->targetCS() &&
                   (typeid(*frontIntersection) == typeid(CSNodeFast) ||
                    typeid(*frontIntersection) == typeid(CSNodeNormal))) {
          // 単路からCSへ
          _runSection2CS();
        } else {
          // 単路部から交差点へ
          // From section to intersection
          _location->setDistanceFromInflowBorder(0);
          _runSection2Intersection();
        }
      }
    }

    // ここから、Vehicleの処理と同じ

    const_cast<Lane *>(_location->lane())->registerAgentToAdd(_vehicle);
    const_cast<Lane *>(_location->lane())
        ->setLastArrivalTime(AppMates::getTimeManager().time());
  }

#ifdef INCLUDE_PEDESTRIANS
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  /*
   * 上流で停止できず進入してしまう予定の横断歩道レーンへの登録
   *
   * Registration to the crosswalk lane that this vehicle will enter
   * without being able to stop upstream.
   */
  if (ev->pedExt()->requiresNotification()) {
    ev->pedExt()->notifyCrosswalkLane();
  }
#endif // INCLUDE_PEDESTRIANS

  // ここまでVehicleと同じ

  // 2022/11/1 by abe
  // 1タイムステップの電力消費量[W]（瞬時値）を求める
  double instantaneousValue = 0;
  // 道路勾配[rad] -- gradient() は百分率
  double theta = atan(_location->lane()->gradient() / 100.0);

  // 転がり抵抗 -- rolling friction
  double Froll =
      (body->coeffRollingFriction() * body->bodyWeight() * GRA * cos(theta)) *
      _behavior->velocity() * (AppMates::getTimeManager().unit() / 1000.0);

  // 空気抵抗 -- air friction
  double Faero = (RHO * body->coeffDrag() * body->frontalProjectedArea() *
                  pow(_behavior->velocity(), 2.0) / 2.0) *
                 _behavior->velocity() *
                 (AppMates::getTimeManager().unit() / 1000.0);

  // 勾配抵抗 -- gradient
  double Fhill = (body->bodyWeight() * GRA * sin(theta)) *
                 _behavior->velocity() *
                 (AppMates::getTimeManager().unit() / 1000.0);

  // 加速抵抗 -- inertial force
  double KROT = 0.04 + 0.0025 * pow(body->finalGearRatio(), 2.0);
  double Faccel = ((1.0 + KROT) * body->bodyWeight() * _behavior->accel()) *
                  _behavior->velocity() *
                  (AppMates::getTimeManager().unit() / 1000.0);

  instantaneousValue += Froll + Faero + Fhill + Faccel;

  //        double e = 0.546;
  double e = 1.0;
  instantaneousValue *= e;

  if (instantaneousValue >= 0) {
    // 消費電力のSOC依存性
    // 現段階ではECR=-SOC + 2とする
    // (15/11/25 by uchida)
    //_batteryRemain -= (instantaneousValue / _mechanicalLoss) * BC();

    // 2022/11/1 by abe 仕様の式に変更
    ev->addBatteryRemain(-instantaneousValue / body->mechanicalLoss());

  } else {
    // 消費電力が負の場合は回生率をかけてエネルギーを回収
    // 回生率は速度依存性を考慮すべき
    // またSOCへの依存性があるかは要サーベイ
    // Yaoの論文からはSOCとt-1要素のどちらが結果の精度向上につながったのか不明
    // (15/11/25 by uchida)
    //_batteryRemain -= (instantaneousValue * _mechanicalLoss) *
    // regenerativeRate();

    // 2022/11/1 by abe 仕様の式に変更
    ev->addBatteryRemain(-instantaneousValue * ev->regenerativeRate());
  }

  // 消費電力に関係なく電装品分の電力は消費される
  // waitingLineに存在する車両は電力を消費しない
  // 2025/04/24 by abe waitingLineの場合はここに到達しないためif文除去
  ev->addBatteryRemain(-ev->accessory());

  // 電池残量は既定値内となる
  // SOCが一定値以下ならば経路探索のフラグを立てる
  // by uchida 2016/5/10
  // ODNode.cppのrerouteの際にも
  // このコードをコピペして利用している
  // by uchida 2016/5/30
  if (ev->threshold() && !ev->isRunningToCharge()) {
    ev->setRunningToCharge(true);
  }
}

//======================================================================
void VehicleEVActor::charge() {
  VehicleEV *ev = static_cast<VehicleEV *>(_vehicle);
  // by uchida 2017/6/23
  // とりあえず元に戻しとく
  ev->setSwapTime(0);

  // by uchida 2016/5/23
  Intersection *intersection =
      const_cast<Intersection *>(_location->intersection());
  CSNodeBase *csNode = dynamic_cast<CSNodeBase *>(intersection);
  if (!csNode) {
    return;
  }

  csNode->sumCharge(ev->chargingValue());
  csNode->sumOccupancy();
  // cout << "CHARGE " << ev->id() << " SOC" << ev->SOC() << endl;

  // これよりちょっと小さいはずだけどとりあえず検証は後で
  // データによると急速CSにおける最大は50kW
  // 家庭用は2kW程度の模様
  // その場合以下の式は25　*　1000になるか
  double chargingValue = csNode->outPower() * 1000.0 *
                         (AppMates::getTimeManager().unit() / 1000.0);
  ev->setChargingValue(chargingValue);
  ev->addBatteryRemain(chargingValue);

  // SOCが0.8以上であれば出発する
  // 急速充電でこれ以上にすることないようなので
  if (ev->SOC() >= 0.8) {
    ev->setRunningToCharge(false);
    ev->setChargingInCS(false);
    // modified by abe 2022/3/11 削除対象EVを自身に限定
    csNode->removeEV(ev);

    if (typeid(*intersection) == typeid(CSNodeFast)) {
      VehicleActor::_runIntersection2Section();
      // 充電時間の分だけ滞在時間が過大評価になるのを回避
      // (追記)　by uchida ここで回避しているのはあくまでSectionの滞在時間
      // 旅行時間全体を適正化するために_restartTimeを設定
      _location->setSectionInflowTime(AppMates::getTimeManager().time());

      // by uchida 2017/2/8
      // CS出庫時刻の登録
      ev->setRestartTime(AppMates::getTimeManager().time());
    }
  } else {
    _behavior->setSleepDuration(AppMates::getTimeManager().unit() * 2);
  }
}

//======================================================================
void VehicleEVActor::_runIntersection2Section() {
  VehicleEV *ev = static_cast<VehicleEV *>(_vehicle);
  _resetPauseState();

  _location->setSection(
      _location->intersection()->nextSection(_location->lane()));

  // 交差点に通過時間を通知
  // Notify passing time to intersection
  int from =
      _location->intersection()->direction(_location->prevIntersection());
  int to = _location->intersection()->direction(_location->section());
  assert(0 <= from && from < _location->intersection()->numNexts());
  assert(0 <= to && to < _location->intersection()->numNexts());

  // リンク旅行時間の記録
  // Record link travel time
  _location->intersection()
      ->linkFlowRecord(from)
      ->recordOutflowFromIntersection(_vehicle, to);
  _location->setSectionInflowTime(AppMates::getTimeManager().time());

  // Intersection::_watchedVehiclesのリセット
  // Reset Intersection::_watchedVehicles
  if (_behavior->isNotifying()) {
    _behavior->setIsNotifying(false);
    _decision->resetLaneChangeFlags();
    const_cast<Intersection *>(_location->intersection())
        ->eraseWatchedVehicle(_vehicle);
    /*
    const_cast<Section*>(_location->section())
    ->addWatchedVehicle(_vehicle);
    */
  }

  // ここまで、 VehicleActor::_runIntersection2Section() の
  // 車線変更に失敗した場合など〜 の直前までと同じ

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // [eMATES] ここから
  // EV用の処理
  if (ev->isRunningToCharge()) {
    Intersection *nextIntersection =
        _location->section()->anotherIntersection(_location->intersection());

    Intersection *target = ev->targetCS();

    // by takusagawa 2019/1/7
    // 情報参照エージェントはすべて交差点を通過する度に経路探索をするように変更
    if (ev->canReceiveWaitingInfo()) {
      // by uchida     2016/5/24
      // 0.ランダムor一定
      // 1.ユークリッド距離で最近傍
      // 2.経路選択の最短コスト
      // 3.現在位置→CS→Dを最小化する
      // 2.3は発生時から計算している

      VehicleEV::CSSearchStrategyFnPtr strategy;
      // by uchida 2016/5/19
      // ここでCS経由するCSを設定
      // 今はランダムとしているが、今後変更予定
      // 0
      // strategy = &VehicleEV::evalByRandom;

      // 1
      // strategy = &VehicleEV::evalByEuclid;

      // 2
      // by abe 2022/12/15
      // strategy = &VehicleEV::evalByCost;
      // softminのためstrategy変更（楠木：251114）

      // 2017/5/23
      // ここをonすることでconventional・offとすることでghost
      // 3
      // by takusagawa 2018/9/25
      // 待ち時間情報の受け取りができる場合はその情報を考慮したCS選択を行う
      // フラグが立っていた場合、待ち時間予測による情報を使用する
      // if (GVManager::getFlag("FLAG_USE_FUTURE_WAITING_LINE"))
      // {
      //     strategy = &VehicleEV::evalByFutureWaitingTimeSumCost;
      // }
      // else
      // {
      strategy = &VehicleEV::evalByWaitingTimeSumCost;
      // }

      target = ev->searchCS(strategy);
      if (target) {
        ev->setTargetCS(target);
        _setTargetIntoGates(nextIntersection, target);
      }
    }
    // 充電するCSが未決定ならば探索する
    else if (!ev->targetCS()) {
      // by uchida 2024/7/25
      // _goal(目的地)がCSに隣接しているならば普通充電扱いとする
      // 900000番台で次数1の_goalのnextSectionは一意に定まり，_goalより必ず若いID（intersection(false)）が接続CSにあたる
      // target =
      // dynamic_cast<CSNodeBase*>(_globalRoute->goal()->nextSection(0)->intersection(false));

      // 2024/7/25 by uchida
      // _goal（目的地）がCSノードであれば普通充電であるとし，CS探索不要・_goalで充電するものとする．
      // 本来の仕様ではCSNodeNormalクラスを使って実現すべきだが．
      // if (dynamic_cast<CSNodeNormal*>(target))
      // {
      VehicleEV::CSSearchStrategyFnPtr strategy;
      // strategy = &VehicleEV::evalByCost;
      strategy = &VehicleEV::evalByWaitingTimeSumCost;
      //  strategy = &VehicleEV::evalByEuclid;
      //  by abe 2022/12/15
      // strategy = &VehicleEV::evalBySumCost;
      target = ev->searchCS(strategy);
      // }
      if (target) {
        ev->setTargetCS(target);
        _setTargetIntoGates(nextIntersection, target);
      }
    }

    _requiresReroute = true;
  }
  // [eMATES] ここまで

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  /*
   * 車線変更に失敗した場合などに予定経路を達成できなくなる．その場合
   * ここで経路を再探索する．
   *
   * The planned route cannot be achieved, if the lane-change fails.
   * In that case, search the route again here.
   */
  if (_location->prevIntersection()) {
    Intersection *next = _location->intersection()->next(
        _location->intersection()->direction(_location->section()));
    if (dynamic_cast<ODNode *>(next) == nullptr &&
        _globalRoute->next(_location->intersection(), next) == nullptr) {
      // [eMATES] ここから
      // by abe 2025/5/14
      // gatesの重複登録を防ぐため、_requiresRerouteで登録済判定する
      if (ev->isRunningToCharge() && !_requiresReroute) {
        Intersection *nextIntersection;
        if (_location->section()->intersection(true) ==
            _location->intersection()) {
          nextIntersection = _location->section()->intersection(false);
        } else if (_location->section()->intersection(false) ==
                   _location->intersection()) {
          nextIntersection = _location->section()->intersection(true);
        }
        assert(nextIntersection);

        Intersection *target = ev->targetCS();

        // by uchida 2016/5/25
        // CS検索
        VehicleEV::CSSearchStrategyFnPtr strategy;
        // 0
        // strategy = &VehicleEV::evalByRandom;

        // 1
        // strategy = &VehicleEV::evalByEuclid;

        // 2
        // by abe 2022/12/15
        // strategy = &VehicleEV::evalByCost;

        // 3
        // by takusagawa 2018/9/25
        // 待ち時間情報の受け取りができる場合はその情報を考慮したCS選択を行う
        // フラグが立っていた場合、待ち時間予測による情報を使用する
        if (!ev->canReceiveWaitingInfo()) {
          strategy = &VehicleEV::evalBySumCost;
        } else {
          // if
          // (AppMates::getGVManager().getFlag("FLAG_USE_FUTURE_WAITING_LINE"))
          //{
          //     strategy = &VehicleEV::evalByFutureWaitingTimeSumCost;
          // }
          // else
          //{
          strategy = &VehicleEV::evalByWaitingTimeSumCost;
          //}
        }

        target = ev->searchCS(strategy);
        if (target) {
          ev->setTargetCS(target);
          _setTargetIntoGates(nextIntersection, target);
        }
      }
      // [eMATES] ここまで

      if (AppMates::getGVManager().getFlag("FLAG_VERBOSE")) {
        cout << "vehicle: " << ev->id()
             << ", rerouting after passing intersection: "
             << _location->intersection()->id() << endl;

        cout << "prev:" << _location->prevIntersection()->id()
             << ", curr:" << _location->intersection()->id()
             << ", next:" << next->id() << endl;
      }
      // postact() 内で reroute する
      // Reroute in postact()
      _requiresReroute = true;
    }
  }

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  _location->setPrevIntersection(_location->intersection());
  _location->setIntersection(nullptr);

  _location->setPrevLane(_location->lane());
  _location->setLane(_location->nextLane());

  // postact() 内で localReroute する
  // localReroute in postact()
  _requiresLocalReroute = true;
}

//======================================================================
void VehicleEVActor::_runSection2CS() {
  VehicleEV *ev = static_cast<VehicleEV *>(_vehicle);
  const CSNodeBase *cs = static_cast<const CSNodeBase *>(
      _location->section()->nextIntersection(_location->lane()));
  _location->setIntersection(cs);

  /*
   * 車線変更を行いながら交差点に進入する場合は強制終了
   *
   * In the case of entering an intersection while lane-changing,
   * forced to terminate it.
   */
  if (_decision->isLaneChangeActive()) {
    if (AppMates::getGVManager().getFlag("FLAG_VERBOSE")) {
      cerr << "vehicle:" << _vehicle->id()
           << " enters intersection while lane changing"
           << " from section:" << _location->section()->id() << endl;
    }
    const_cast<VehicleLaneChangeActor *>(_vehicle->laneChangeActor())
        ->abortLaneChange();
  }

  // リンク旅行時間の記録
  // Record link travel time
  bool isUp = _location->section()->isUp(_location->lane());
  _location->section()->linkFlowRecord(isUp)->recordOutflowFromSection(
      _vehicle);

  if (_location->section()->linkFlowMonitor(isUp)) {
    _location->section()->linkFlowMonitor(isUp)->recordPassedVehicle(_vehicle);
  }

  _resetPauseState();

  // 最後に通過した交差点を登録
  // Register the last passed intersection
  if (!(_location->prevIntersection())) {
    _globalRoute->setLastPassedIntersectionIndex(_location->intersection());
  } else {
    _globalRoute->setLastPassedIntersectionIndex(_location->prevIntersection(),
                                                 _location->intersection());
  }

  // ここまで VehicleActor::_runSection2Intersection() と同じ
  //==================================================

  // by takusagawa 2018/10/29
  // 経路選択の際に待ち時間を適切に考慮するために追加
  int from = cs->direction(_location->prevIntersection());
  int to = cs->direction(_location->section());
  assert(0 <= from && from < cs->numNexts());
  assert(0 <= to && to < cs->numNexts());

  _location->intersection()
      ->linkFlowRecord(from)
      ->recordOutflowFromIntersection(_vehicle, to);

  // ここは VehicleActor::_runSection2Intersection() と同じ
  // Section::_watchedVehicles のリセット
  // Reset Section::_watchedVehicles
  if (_behavior->isNotifying()) {
    const_cast<Section *>(_location->section())->eraseWatchedVehicle(_vehicle);
    _behavior->setIsNotifying(false);
    _decision->resetLaneChangeFlags();
  }

  // by abe 2025/5/26 通り抜け型でないCSでは、CS内で方向転換する（Laneを変える）
  if (cs->deadend()) {
    // 同じ進行方向でCSの次の交差点がODノードの場合
    _location->setLane(cs->lanesTo(_location->section())[0]);
  } else {
    // それ以外はそのままCS内のレーンをセット
    _location->setLane(_location->nextLane());
  }
  _location->setPrevLane(_location->lane()->previousLane(0));
  _location->setSection(nullptr);
  _requiresLocalReroute = true;

  ev->setVisitedCS(cs);
  ev->setTargetCS(nullptr);

  // by takusagawa 2018/11/9
  // CSのwaitingLineに入庫した時刻の登録
  ev->setWaitingLineEntryTime(AppMates::getTimeManager().time());

  // 充電EVの登録
  const_cast<CSNodeBase *>(cs)->addEV(ev);
}

//======================================================================
void VehicleEVActor::_setTargetIntoGates(Intersection *nextIntersection,
                                         Intersection *target) {
  if (!nextIntersection || !target) {
    return;
  }

  std::vector<const Intersection *> gates{};
  if (target == _globalRoute->goal()) {
    // gatesを置き換える
    gates = {nextIntersection, target};
  } else {
    // gatesにcsを追加する
    _globalRoute->getGatesToPass(gates);
    *(gates.begin()) = nextIntersection;
    gates.insert(gates.begin() + 1, target);
  }
  _globalRoute->setGates(gates);
}

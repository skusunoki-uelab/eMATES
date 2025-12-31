#include "VehicleEV.hpp"
#include "AppMates.hpp"
#include "CSNodeBase.hpp"
#include "RouterAStarHierarchy.hpp"
#include "VehicleEVActor.hpp"
#include "VehicleEVBodyProperty.hpp"
#include <AmuConverter.hpp>

using std::cout;
using std::endl;
using namespace amu::converter;

// vehicletripwriterで使うために追加
//==================================================
VehicleEV::VehicleEV() {
  // eMATES用のインスタンスを使う必要がある関数は、明示的にここで初期化する
  _body.reset(new VehicleEVBodyProperty);
  _actor.reset(new VehicleEVActor);

  // 以降、MATES版と同じ登録処理
  _body->setVehicle(this);
  _behavior.setVehicle(this);
  _decision.setVehicle(this);
  _localRoute.setVehicle(this);
  _location.setVehicle(this);
  _scene.setVehicle(this);

  _perceiver.setVehicle(this, &_behavior, _body.get(), &_decision,
                        &_globalRoute, &_localRoute, &_location, &_scene);
  _lcPerceiver.setVehicle(this, &_behavior, _body.get(), &_decision,
                          &_localRoute, &_location, &_scene, &_localRouter);
  _determiner.setVehicle(this, _body.get(), &_behavior, &_decision, &_location,
                         &_scene);
  _lcDeterminer.setVehicle(this, _body.get(), &_behavior, &_decision,
                           &_location, &_scene);
  _actor->setVehicle(this, &_behavior, _body.get(), &_decision, &_globalRoute,
                     &_localRoute, &_location, &_scene, &_localRouter,
                     &_blinker);
  _lcActor.setVehicle(this, _body.get(), &_behavior, &_decision, &_location,
                      &_scene);
  _localRouter.setVehicle(this, &_globalRoute, &_localRoute);

  _localRoute.clearLocalRoute();
  _preferredNetworkRank = INT_MAX;
  _blinker.setNone();

#ifdef INCLUDE_PEDESTRIANS
  _pedExt = new VehiclePedExt();
  _pedExt->setVehicle(this, &_behavior, _body.get(), &_globalRoute, &_location,
                      &_scene);
  _perceiver.setPedExt(_pedExt);
#endif // INCLUDE_PEDESTRIANS

  // 一定確率で、CSの待ち状況を確認できるようにする
  double rnd = randomNumberGenerator()->uniform();
  double receiveRate =
      AppMates::getGVManager().getNumeric("RECEIVE_WAITING_INFO_RATE");
  _canReceiveWaitingInfo = (rnd < receiveRate);
}

//====================================================================
void VehicleEV::setChargingInCS(bool state) {
  if (_isChargingInCS == state) {
    return;
  }

  // by takusagawa 2018/11/9
  // chargeフラグがonになったとき,つまり充電開始時刻を登録
  // （_runCS2Sectionに書かれていたが,こちらでは？）
  if (state) {
    // debug by takusagawa 2018/11/10
    // cout << "id: " << id() << " SOC: " << SOC() << endl;
    // debug by Mr.uchida 2024/05/27
    // cout << "id: " << id() << " CSid: " << _location.intersection()->id() <<
    // endl;
    setStartChargingTime(AppMates::getTimeManager().time());

    // 直後のperceiveを抑制するためにsleepにする
    _behavior.setSleepDuration(AppMates::getTimeManager().unit());
  }
  _isChargingInCS = state;
  _swapTime = 1;
  // debug by uchida 2017/6/23
  // cout << "~ " << TimeManager::time() / 1000 << " [s] "
  //      << _id << " : start charging at " << _intersection->id() << endl;
}

//====================================================================
double VehicleEV::SOC() {
  VehicleEVBodyProperty *body =
      static_cast<VehicleEVBodyProperty *>(_body.get());
  double soc = _batteryRemain / body->batteryCapacityWs();
  if (soc > 1) {
    _batteryRemain = body->batteryCapacityWs();
    soc = 1;
  } else if (soc < 0) {
    _batteryRemain = 0;
    soc = 0;
  }
  return soc;
}

//====================================================================
void VehicleEV::setInitialSOC() {
  // OD距離に応じて初期SoCを変化させる
  double rnd = randomNumberGenerator()->uniform();
  // rnd = (rnd * 0.6) + 0.3 * (getOdDistance() / _maxOD);
  //  35000は最大値を超えないように適当に与えた

  if (rnd >= 0.531) {
    _initialSOC = 0.8;
  } else if (rnd >= 0.129) // rnd < 0.531は前の条件から保証されている
  {
    _initialSOC = 0.75;
  } else if (rnd >= 0.08) {
    _initialSOC = 0.7;
  } else if (rnd >= 0.036) {
    _initialSOC = 0.65;
  } else if (rnd >= 0.029) {
    _initialSOC = 0.6;
  } else if (rnd >= 0.009) {
    _initialSOC = 0.55;
  } else if (rnd >= 0.006) {
    _initialSOC = 0.5;
  } else {
    _initialSOC = 0.20;
  }

  //_initialSOC = 0.15 + rnd*.1;
  //_initialSOC = 0.3;

  // if (rnd < 0.15)//0.1 2023 obinata
  // {
  //   _initialSOC = 0.2;//0.3
  // }
  // else if(rnd>=0.15||rnd<0.2)
  // {
  //   _initialSOC=0.5;
  // }
  // else
  // {
  //   _initialSOC = rnd;//0.8
  // }

  // revised by uchida 2021/7/20
  // 電費計算のために初期SoCを満充電状態に固定
  // 普通にsimする場合に以下をコメントアウトすべき
  // _initialSOC = 0.2;

  // 初期SOCに基づいて、電力残量を計算
  _batteryRemain =
      static_cast<VehicleEVBodyProperty *>(_body.get())->batteryCapacityWs() *
      _initialSOC;
}

// by uchida 2016/5/22
//======================================================================
double VehicleEV::BC() { return -1.0 * SOC() + 2.0; }

//======================================================================
double VehicleEV::regenerativeRate() const {
  assert(_behavior.velocity() >= 0);
  // double regenerativeRate = 0;
  //// Yao論文を参考に速度依存の回生率を設定
  // if (_behavior.velocity() >= 5.0)
  //{
  //   regenerativeRate = 0.3 * (_behavior.velocity() - 5.0) / 20.0 + 0.5;
  // }
  // else
  //{
  //   regenerativeRate = 0.5 * _behavior.velocity() / 5.0;
  // }
  //
  //// 138km/h（38.3m/s）以上で回生率1を上回るため
  // if (regenerativeRate > 1.0) regenerativeRate = 1.0;
  //
  // return regenerativeRate;

  // by abe 2022/11/1 仕様の式に変更
  return exp(-1.0 * 0.0411 / fabs(_behavior.accel()));
}

//======================================================================
Intersection *VehicleEV::searchCS(CSSearchStrategyFnPtr callback) {
  // 探索対象CSの集合→コメントアウト時は急速充電off

  std::vector<CSNodeBase *> csNodes = _location.roadMap()->csNodesFast();
  //std::vector<CSNodeBase *> csNodes;//からのまま＝CS候補なし
  std::vector<Intersection *> candidates(csNodes.begin(), csNodes.end());

  // // 目的地直行も考慮する（ランダム戦略以外）
  // if (callback != &VehicleEV::evalByRandom){
  //     Intersection* destination =
  // const_cast<Intersection*>(_globalRoute.goal());
  //     candidates.push_back(destination);
  // }

  const Intersection *rear = _location.intersection();
  const Intersection *front = _location.section()->anotherIntersection(rear);

  // 経路選択器を取得
  // Get router
  RouterBase *router = AppMates::getRouterManager().assignRouter();

  // 現在地からのGV最小のCSを選択
  double bestCost = MAX_COST;
  Intersection *bestCS = nullptr;

  // ソフトマックス法のパラメータ(調整が必要)251112byKusunoki
  double theta_per_1000 = 7.5; // ←ここを調整（0.05～0.3を試す）
  double theta = theta_per_1000 / 1000.0;
  //   double theta = 1.5;

  // CS候補が空の場合は早期リターン(急速充電off用)
  // if (candidates.empty()) {
  //   AppMates::getRouterManager().releaseRouter(router);
  //   cout << "No CS candidates for vehicle " << id() << endl;
  //   return nullptr;
  // }

  // softmin法による選択肢の絞り込み
  std::vector<double> costs;
  std::vector<double> route_costs;//追加：routeコストのみを記録
  std::vector<double> probabilities;
  double sum_exp = 0.0;
  for (Intersection *cs : candidates) {
    CSCost c = (this->*callback)(router, rear, front, cs);
    // 追加251226：配電網ペナルティを総コストに加算（楠木）
    double total_cost =
        std::min(MAX_COST, c.route + c.chargeTime + c.yen + c.waiting + c.gridPenalty);
    cout << "search CS"
         << " veh " << id() << " CS " << cs->id() << " cost " << total_cost
         << " (Route " << c.route << " ChgTime " << c.chargeTime << " Yen "
         << c.yen << " Wait " << c.waiting << " Grid " << c.gridPenalty << ")" << endl;
    costs.push_back(total_cost);
    route_costs.push_back(c.route);//追加：routeコストのみを記録
    double exp_cost = exp(-theta * total_cost);
    sum_exp += exp_cost;
  }

  

  for (size_t i = 0; i < candidates.size(); i++) {
    double exp_cost = exp(-theta * costs[i]);
    double prob = exp_cost / sum_exp;
    probabilities.push_back(prob);
    cout << "CS " << candidates[i]->id() << " probability " << prob << endl;
  }

  double rand = randomNumberGenerator()->uniform();
  double cumulative_prob = 0.0;
  double chosen_cost = MAX_COST;
  double best_cost = *std::min_element(costs.begin(), costs.end());
  double gap_cost = MAX_COST;
  //　最小のrouteコストを取得
  double min_route_cost = *std::min_element(route_costs.begin(), route_costs.end());

  for (size_t i = 0; i < candidates.size(); i++) {
    cumulative_prob += probabilities[i];
    if (rand <= cumulative_prob) {
      bestCS = candidates[i];
      chosen_cost = costs[i];
      gap_cost = costs[i] - best_cost;

      // 追加：routeコストと迂回距離を計算
      double chosen_route_cost = route_costs[i];
      double detour = chosen_route_cost - min_route_cost;

      setChosenCSRouteCost(chosen_route_cost);
      setClosestCSRouteCost(min_route_cost);
      setDetourDistance(detour);
      break;
    }
  }
  double relGap = gap_cost / best_cost;

  setChosenCSCost(chosen_cost);
  setRelativeGap(relGap);

  //   for(Intersection* cs : candidates)
  //   {
  //       // 各CSの評価値はコールバックで計算
  //       //
  //       この中でrouter->search()が呼ばれるため、routerの内部状態はリセットされる
  //       CSCost costs = (this->*callback)(router, rear, front, cs);
  //       // 合計コスト
  //       constexpr double max_cost = MAX_COST; //
  //       なぜか一度移し替えないとリンカエラーが発生する(g++ 13.3.0) double
  //       cost = std::min(max_cost,
  //               costs.route + costs.chargeTime + costs.yen + costs.waiting);
  //       cout << "search CS"
  //           << " veh " << id() << " CS " << cs->id()
  //           << " cost " << cost
  //           << " (Route " << costs.route
  //           << " ChgTime " << costs.chargeTime
  //           << " Yen " << costs.yen
  //           << " Wait " << costs.waiting
  //           << ")" << endl;
  //       if (bestCost > cost)
  //       {
  //           bestCost = cost;
  //           bestCS = cs;
  //       }
  //   }

  // 経路選択器を返却
  // Return path router
  AppMates::getRouterManager().releaseRouter(router);

  cout << "selected CS Veh: " << id()
       << " CS: " << (bestCS ? bestCS->id() : "nullpt")
       << " Cost: " << chosen_cost << " randomnumber: " << rand
       << " caumulative_prob: " << cumulative_prob << " gap_cost: " << gap_cost
       << " relGap: " << relGap << endl;

  return bestCS;
}

//======================================================================
void VehicleEV::_prepareRouter(RouterBase *router) {
  // 属性を設定
  // Set attributes
  router->setId(_id);
  router->setVehicleType(_body->type());
  router->setWeights(_routingParams);
  RouterAStarHierarchy *routerAStarHierarchy =
      dynamic_cast<RouterAStarHierarchy *>(router);
  if (routerAStarHierarchy) {
    routerAStarHierarchy->setPreferredNetworkRank(_preferredNetworkRank);
  }
  router->setRandomNumberGenerator(&_rng);
}

//======================================================================
VehicleEV::CSCost VehicleEV::evalByRandom(RouterBase *router,
                                          const Intersection *rear,
                                          const Intersection *front,
                                          const Intersection *target) {
  return {randomNumberGenerator()->uniform(), 0, 0, 0, 0};
}

//======================================================================
VehicleEV::CSCost VehicleEV::evalByEuclid(RouterBase *router,
                                          const Intersection *rear,
                                          const Intersection *front,
                                          const Intersection *target) {
  return {_location.position().distance(target->center()), 0, 0, 0, 0};
}

//======================================================================
VehicleEV::CSCost VehicleEV::evalByCost(RouterBase *router,
                                        const Intersection *rear,
                                        const Intersection *front,
                                        const Intersection *target) {
  // 現在地〜CSのGV
  _prepareRouter(router);
  Route route = router->search(rear, front, {front, target});
  if (!route.isValid()) {
    return {MAX_COST, 0, 0, 0, 0};
  }

  CSCost cost{route.cost(), 0, 0, 0, 0};
  auto cs = dynamic_cast<const CSNodeBase *>(target);
  if (cs) {
    // CSならその値を読み込む
    cost.chargeTime = cs->chargingTimeForRouting(this) *
                      _routingParams[toUnderlying(RoutingParamIndex::CS_TIME)];
    cost.yen = cs->priceForRouting() *
               _routingParams[toUnderlying(RoutingParamIndex::CS_YEN)];
    // 追加251226：配電網ペナルティを計算（楠木）
    cost.gridPenalty = cs->calculateGridCost();
  } else {
    // CS進入ペナルティを加算
    cost.route += CS_ENTRY_PENALTY;
  }

  return cost;
}

//======================================================================
VehicleEV::CSCost VehicleEV::evalBySumCost(RouterBase *router,
                                           const Intersection *rear,
                                           const Intersection *front,
                                           const Intersection *target) {
  const Intersection *destination = _globalRoute.goal();
  if (destination == target) {
    // targetが目的地と同一なら、現在地〜CSのGVとする
    return evalByCost(router, rear, front, target);
  }

  // 現在地〜CS〜目的地のGV
  _prepareRouter(router);
  Route route = router->search(rear, front, {front, target, destination});
  if (!route.isValid()) {
    return {MAX_COST, 0, 0, 0, 0};
  }
  // CSCost cost{0, 0, 0, 0, 0};
  // routeのコストを[m]⇒[s]に変換：秒速10mで走行すると仮定
  CSCost cost{route.cost() / 10.0, 0, 0, 0, 0};
  // routeによるコストを0にするためにコメントアウト⇒evalWaitingTimeSumCostにつながっている
  auto cs = dynamic_cast<const CSNodeBase *>(target);
  if (cs) {
    // CSならその値を読み込む
    cost.chargeTime = cs->chargingTimeForRouting(this) *
                      _routingParams[toUnderlying(RoutingParamIndex::CS_TIME)];
    cost.yen = cs->priceForRouting() *
               _routingParams[toUnderlying(RoutingParamIndex::CS_YEN)];
    // 追加251226：配電網ペナルティを計算（楠木）
    cost.gridPenalty = cs->calculateGridCost();
  } else {
    // CS進入ペナルティを加算
    cost.route += CS_ENTRY_PENALTY;
  }

  return cost;
}

//======================================================================
VehicleEV::CSCost VehicleEV::evalByWaitingTimeSumCost(
    RouterBase *router, const Intersection *rear, const Intersection *front,
    const Intersection *target) {
  // 現在地〜CS〜目的地のGV
  CSCost cost = evalBySumCost(router, rear, front, target);

  // CS待ち時間
  auto cs = dynamic_cast<const CSNodeBase *>(target);
  if (cs) {
    // cost.waiting = cs->estimatedWaitingTime();
    // cost.waiting =
    //     cs->estimatedWaitingTime() * 25 /
    //     1000.0; //
    //     待ち時間の重み付けを大きくすることで、待ち時間の影響を強める&[ms]→[s]
    // 251118 chargingTimeForRoutingがwaitingtimeを表していそうなので、修正
    cost.waiting =
        cs->estimatedWaitingTime(this) *
        _routingParams[toUnderlying(RoutingParamIndex::CS_TIME)]; //[s]
  }

  // cost /= 1000.0; // [ms]→[s]

  return cost;
}

//======================================================================
// VehicleEV::CSCost VehicleEV::evalByFutureWaitingTimeSumCost(
//        RouterBase* router, const Intersection* rear, const Intersection*
//        front, const Intersection* target)
//{
//    // 現在地〜CS〜目的地のGV
//    CSCost cost = evalBySumCost(router, rear, front, target);
//    if (cost.route == MAX_COST)
//    {
//        return cost;
//    }
//
//    // CS待ち時間（将来）
//    const CSNodeBase* cs = dynamic_cast<const CSNodeBase*>(target);
//    double costToWait = cs->estimatedFutureWaitingTime(cost.route);
//
//    // by takusagawa 2018/12/14
//    // 制御ありの場合の関数をあらためて作成しようとしたが,
//    //
//    これ以上似たような関数が増えると分かりづらい気がするのでここで条件分岐する.
//    // 制御パラメータはConf.hで管理する.
//    // by takusagawa 2019/1/4
//    // I制御項を追加
//    if
//    (AppMates::getGVManager().getFlag("FLAG_USE_PREDICTION_WITH_CONTROLLER"))
//    {
//        costToWait +=
//            - FUTURE_WAITING_TIME_CONTROL_PARAMETER * (cost.costRoute / 60000)
//            * cs->getPredictiveGradient(tmpCost)
//            + FUTURE_WAITING_TIME_INTEGRAL_PARAMETER * cs->IV();
//        costToWait = std::max(costToWait, 0.0);
//    }
//    //  arrivalTime更新が必要かも
//
//    cost.route += costToWait;
//    return cost;
//}

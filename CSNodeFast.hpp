#ifndef __CS_NODE_FAST_HPP__
#define __CS_NODE_FAST_HPP__

#include "CSNodeBase.hpp"

class ChargerFast;

/// 急速充電用CSノードのクラス [eMATES]

class CSNodeFast : public CSNodeBase{
public:
    CSNodeFast(const std::string& id, const std::string& type, RoadMap* parent);
    ~CSNodeFast();

    // --------------------------------------------------
    // 基底クラスのオーバーライド

    // 待機列から削除する
    void removeEV(VehicleEV* vehicle) override;

    // 収容台数を設定する
    void setCapacity(int capacity) override;
    // kW性能（定格）を設定する
    void setRatingPower(double ratingPower) override;
    // kW性能（制御値）を設定する
    void setOutPower(double outPower) override;

    // 充電時間を返す -- 経路選択用
    double chargingTimeForRouting(VehicleEV* ev) const override;
    // 充電料金返す -- 経路選択用
    double priceForRouting() const override;

    // 推定待ち時間を返す
    double estimatedWaitingTime() const override
    {
        return _estimatedWaitingTime;
    }

    // --------------------------------------------------
    // 急速充電特有のメソッド

    // 充電器・充電中車両のリストを返す
    const std::vector<VehicleEV*> chargingVehicles() const;

    // 急速充電器リストを返す
    // vector自体はconstだが、ポインタの示す先は書き換えられる
    const std::vector<ChargerFast*>& chargers() const;

    // 充電器を返す
    ChargerBase* charger(int cgrId) const override;

    // by uchida 2023/1/18
    // 待機列（充電中を含む）を返す，考慮しない場合コメントアウト，Router.cpp入れ替え，make忘れずに
    std::vector<VehicleEV*> waitingVehicles() const {return _waitingLine;}

    // 推定待ち時間の更新
    void renewEstimatedWaitingTime() override;

protected:

    // --------------------------------------------------
    // 急速充電特有のメンバ変数

    // 急速充電器
    std::vector<ChargerFast*> _chargers{};

    // 推定待ち時間
    double _estimatedWaitingTime {0};

    // --------------------------------------------------
    // 基底クラスのオーバーライド

    // 充電器に車両を割り当てる
    // addEV(), removeEV() 内部で用いる
    void _assignEVIntoCharger() override;

};

#endif

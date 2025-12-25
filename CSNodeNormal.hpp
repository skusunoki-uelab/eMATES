#ifndef __CS_NODE_NORMAL_HPP__
#define __CS_NODE_NORMAL_HPP__

#include "CSNodeBase.hpp"

class ChargerNormal;

/// 普通充電用CSノードのクラス [eMATES]

class CSNodeNormal : public CSNodeBase{
public:
    CSNodeNormal(const std::string& id, const std::string& type, RoadMap* parent);
    ~CSNodeNormal();

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

    // --------------------------------------------------
    // 普通充電特有のメソッド

    // 充電器を返す
    // 引数はCSNodeFastと揃えるための措置。何を入れてもよい。
    ChargerBase* charger(int cgrId = 0) const override;

    // kW性能（定格）を返す
    double ratingPower() const;

protected:

    // --------------------------------------------------
    // 普通充電特有のメンバ変数

    // 普通充電器
    ChargerNormal* _charger {nullptr};

    // --------------------------------------------------
    // 基底クラスのオーバーライド

    // 充電器に車両を割り当てる
    // addEV(), removeEV() 内部で用いる
    void _assignEVIntoCharger() override;

};

#endif


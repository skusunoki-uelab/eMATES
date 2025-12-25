#ifndef __CS_NODE_BASE_HPP__
#define __CS_NODE_BASE_HPP__

#include "Charger.hpp"
#include "ODNode.hpp"
#include <string>
#include <vector>

class RoadMap;
class VehicleEV;

/// CSノードの基底クラス [eMATES]
/*
 * Intersectionの派生クラスのODNodeの派生クラス
 *
 */
class CSNodeBase : public ODNode{
public:
    CSNodeBase(const std::string& id, const std::string& type, RoadMap* parent);
    virtual ~CSNodeBase(){}

    // 行き止まりかどうかを設定する
    void setDeadend(bool deadend)
    {
        _deadend = deadend;
    }
    // 行き止まりかどうかを返す
    bool deadend() const
    {
        return _deadend;
    }

    // 占有時間を加算する
    void sumOccupancy();
    // 占有時間を返す
    long long occupancy() const;

    // 充電電力量（瞬時）[Wsec]を合計する
    void sumCharge(double chargingValue);

    // 充電電力量を初期化する
    void initCharge();

    // 充電電力量（瞬時）[Wsec]を返す
    double instantaneousCharge() const;

    // 充電電力を時間方向に積算する．
    void integrated(double instantaneousCharge);

    // 積算充電電力を返す
    double integratedCharge() const;

    // 積算充電電力を初期化する
    void initIntegrated();

    // 待機列に追加する
    void addEV(VehicleEV* vehicle);

    // 待機列から削除する
    virtual void removeEV(VehicleEV* vehicle) = 0;

    // 収容台数を設定する
    virtual void setCapacity(int capacity) = 0;
    // kW性能（定格）を設定する
    virtual void setRatingPower(double ratingPower) = 0;
    // kW性能（制御値）を設定する
    virtual void setOutPower(double outPower) = 0;

    // 収容台数を取得する
    int capacity() const;

    // kW性能（制御値）を返す
    double outPower() const;

    // 充電時間を返す -- 経路選択用
    virtual double chargingTimeForRouting(VehicleEV* ev) const = 0;
    // 充電料金返す -- 経路選択用
    virtual double priceForRouting() const = 0;

    // 捌け台数を返す
    int servedEV() const;

    // 充電器を返す
    // 基底クラスを返すため注意
    virtual ChargerBase* charger(int cgrId) const = 0;

    // 推定待ち時間を返す
    virtual double estimatedWaitingTime() const
    {
        return 0;
    }

    // 推定待ち時間の更新
    virtual void renewEstimatedWaitingTime() { }

    // kW性能（定格）を返す -- Fastのみ
    // 充電器（リスト）を返す -- Fastのみ
    // 待機台数を返す -- Fastのみ
    // 田草川くんコード -- Fastのみ

     // 待機列を返す
     //2025/10/14 Kusunoki 追加
    const std::vector<VehicleEV*>& waitingVehicles() const
    {
        return _waitingLine;
    }

    // 追加251225：フィーダー関連（楠木）
    void setFeederID(const std::string& id) { _feederID = id; }
    std::string feedrId() const { return _feederID; }
    void setGridCostWeight(double weight) { _gridCostWeight = weight; }
    double gridCostWeight() const { return _gridCostWeight; }

    // 追加251225：配電網コスト計算（楠木）
    double calculateGridCost() const;

protected:
    // 行き止まりならば true、通り抜け可能なら false
    // CSNodeNormal はすべてfalse
    // CSNodeFast のうち、2方向の道路に流入出できるものはfalse
    bool _deadend {false};

    // CSの収容台数
    int _capacity;

    // 捌け台数
    int _servedEV;

    // 占有時間
    long long _occupancy;

    // 充電電力量（瞬時）[Wsec]
    double _instantaneousCharge;

    // 充電電力量（積算）[Wsec]
    double _integratedCharge;

    // kW性能
    // 定格 [kW]（CSリストから定まる）
    // - 注意：充電器の定格を表す
    double _ratingPower;
    // 制御値 [kW]
    // - OpenDSS連成時はOpenDSSから設定される
    // - 非連成時は _ratingPower と同一
    // - 注意：CSの制御値を表す
    double _outPower;

    // 待機列
    // 充電中と充電前の両方を含む
    std::vector<VehicleEV*> _waitingLine;

    // 追加251225：フィーダー関連（楠木）
    std::string _feederID;         // フィーダーID
    double _gridCostWeight;     // 配電網コスト重み係数

    // 充電器 -- Normal/Fastで異なる

    // --------------------------------------------------

    // 充電器に車両を割り当てる
    // addEV(), removeEV() 内部で用いる
    virtual void _assignEVIntoCharger() = 0;
};


#endif

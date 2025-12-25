#ifndef __CHARGER_HPP__
#define __CHARGER_HPP__

#include <unordered_set>
#include <vector>
#include <cstddef>

class VehicleEV;

// by abe 2025/03/18
// クラス名変更
// Charger -> ChargerBase
// NormalCharger -> ChargerNormal
// FastCharger -> ChargerFast

//======================================================================
/// 充電器クラス [eMATES]
class ChargerBase{
    public:
        /// コンストラクタ
        // timeSlot: 占有状態予測で何時間帯見るか
        ChargerBase(int id, int timeSlot);

        /// IDを返す
        int id() const;

        /// 車両をセットする
        virtual bool setVehicle(VehicleEV* vehicle) = 0;

        /// 車両を削除する
        virtual bool removeVehicle(VehicleEV* vehicle) = 0;

        /// 満車か
        virtual bool isFull() const = 0;

        /// 空車か
        virtual bool isEmpty() const = 0;

        /// 占有状態の予測リストを返す
        std::vector<int>& volumes();
        const std::vector<int>& volumesConst() const;

        /// 出力の予測リストを返す
        std::vector<double>& outPowers();
        const std::vector<double>& outPowersConst() const;

        /// 金額の予測リストを返す
        std::vector<double>& prices();
        const std::vector<double>& pricesConst() const;

    protected:
        int _id;

        /// 占有状態の予測 -- 現在時刻を含む
        std::vector<int> _volumes;

        /// 出力の予測 -- 現在時刻を含む
        std::vector<double> _outPowers;

        /// 金額の予測 -- 現在時刻を含む
        std::vector<double> _prices;
};


//======================================================================
/// 急速充電器想定。1台の車両のみ扱う
class ChargerFast : public ChargerBase{
    public:
        /// コンストラクタ
        // timeSlot: 占有状態予測で何時間帯見るか
        ChargerFast(int id, int timeSlot);

        virtual ~ChargerFast() = default;

        /// 占有車両を返す
        VehicleEV* vehicle();

        /// 車両をセットする
        bool setVehicle(VehicleEV* vehicle) override;

        /// 車両を削除する
        bool removeVehicle(VehicleEV* vehicle) override;

        /// 満車か
        bool isFull() const override;

        /// 空車か
        bool isEmpty() const override;

    protected:
        /// 占有車両 -- 容量は必ず1台
        VehicleEV* _vehicle;
};


//======================================================================
class ChargerNormal : public ChargerBase{
    public:
        /// コンストラクタ
        // timeSlot: 占有状態予測で何時間帯見るか
        ChargerNormal(int id, int capacity, int timeSlot);

        virtual ~ChargerNormal() = default;

        /// 占有車両を返す
        std::unordered_set<VehicleEV*>& vehicles();

        /// 車両をセットする
        bool setVehicle(VehicleEV* vehicle) override;

        /// 車両を削除する
        bool removeVehicle(VehicleEV* vehicle) override;

        /// 満車か
        bool isFull() const override;

        /// 空車か
        bool isEmpty() const override;

    protected:
        /// 占有可能台数
        size_t _capacity;

        /// 占有車両 -- 集合として扱う
        std::unordered_set<VehicleEV*> _vehicles;
};

#endif // __CHARGER_HPP__

#ifndef __VEHICLE_EV_BODY_PROPERTY_HPP__
#define __VEHICLE_EV_BODY_PROPERTY_HPP__

#include "VehicleBodyProperty.hpp"

// by abe 2025/04/18 EV用の車種属性 [eMATES]
struct VehicleEVBodyProperty : public VehicleBodyProperty
{
public:
    VehicleEVBodyProperty() = default;

    ~VehicleEVBodyProperty() = default;

private:
    /// 重量[kg]（車体重量+乗員重量）
    double _bodyWeight {};
    /// 電池容量[Wsec]
    double _batteryCapacityWs {};
    /// 全面投影面積[m2]
    double _frontalProjectedArea {};
    /// 空気抵抗係数 Cd
    double _coeffDrag {};
    /// 転がり摩擦係数 τ
    double _coeffRollingFriction {};
    /// 機械効率 η
    double _mechanicalLoss {};
    /// 最終減速比 G
    double _finalGearRatio {};

public:
    double bodyWeight() const
    {
      return _bodyWeight;
    }

    double batteryCapacityWs() const
    {
      return _batteryCapacityWs;
    }

    double frontalProjectedArea() const
    {
      return _frontalProjectedArea;
    }

    double coeffDrag() const
    {
      return _coeffDrag;
    }

    double coeffRollingFriction() const
    {
      return _coeffRollingFriction;
    }

    double mechanicalLoss() const
    {
      return _mechanicalLoss;
    }

    double finalGearRatio() const
    {
      return _finalGearRatio;
    }

    void setEVspec(
        double bodyWeight,
        double batteryCapacityWs,
        double frontalProjectedArea,
        double coeffDrag,
        double coeffRollingFriction,
        double mechanicalLoss,
        double finalGearRatio)
    {
      _bodyWeight = bodyWeight;
      _batteryCapacityWs = batteryCapacityWs;
      _frontalProjectedArea = frontalProjectedArea;
      _coeffDrag = coeffDrag;
      _coeffRollingFriction = coeffRollingFriction;
      _mechanicalLoss = mechanicalLoss;
      _finalGearRatio = finalGearRatio;
    }

};

#endif //__VEHICLE_EV_BODY_PROPERTY_HPP__

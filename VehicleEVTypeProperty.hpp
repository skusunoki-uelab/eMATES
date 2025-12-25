#ifndef __VEHICLE_EV_TYPE_PROPERTY_HPP__
#define __VEHICLE_EV_TYPE_PROPERTY_HPP__

#include "VehicleTypeProperty.hpp"

// by abe 2025/04/18 EV用の車種属性 [eMATES]
struct VehicleEVTypeProperty : public VehicleTypeProperty
{
private:
    const double _batteryCapacityWs;
    const double _frontalProjectedArea;
    const double _coeffDrag;
    const double _coeffRollingFriction;
    const double _mechanicalLoss;
    const double _finalGearRatio;

public:
    VehicleEVTypeProperty(
        // VehicleTypeと共通
        VehicleType type, double bodyLength, double bodyWidth,
        double bodyHeight, double bodyWeight, int numCars,
        double maxAcceleration, double maxDeceleration,
        double bodyColorR, double bodyColorG, double bodyColorB,
        // VehicleEV用
        double batteryCapacityWs,
        double frontalProjectedArea,
        double coeffDrag,
        double coeffRollingFriction,
        double mechanicalLoss,
        double finalGearRatio)
        // VehicleTypeと共通
      : VehicleTypeProperty(
            type, bodyLength, bodyWidth,
            bodyHeight, bodyWeight, numCars,
            maxAcceleration, maxDeceleration,
            bodyColorR, bodyColorG, bodyColorB),
        // VehicleEV用
        _batteryCapacityWs(batteryCapacityWs),
        _frontalProjectedArea(frontalProjectedArea),
        _coeffDrag(coeffDrag),
        _coeffRollingFriction(coeffRollingFriction),
        _mechanicalLoss(mechanicalLoss),
        _finalGearRatio(finalGearRatio) {};
    ~VehicleEVTypeProperty() = default;

public:
    void getEVspec(
        double* pBodyWeight,
        double* pBatteryCapacityWs,
        double* pFrontalProjectedArea,
        double* pCoeffDrag,
        double* pCoeffRollingFriction,
        double* pMechanicalLoss,
        double* pFinalGearRatio) const
    {
        *pBodyWeight = _bodyWeight;
        *pBatteryCapacityWs = _batteryCapacityWs;
        *pFrontalProjectedArea = _frontalProjectedArea;
        *pCoeffDrag = _coeffDrag;
        *pCoeffRollingFriction = _coeffRollingFriction;
        *pMechanicalLoss = _mechanicalLoss;
        *pFinalGearRatio = _finalGearRatio;
    }
};

#endif

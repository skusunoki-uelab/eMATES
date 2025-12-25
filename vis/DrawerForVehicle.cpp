/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForVehicle.cpp
 */
#include "DrawerForVehicle.hpp"
#include "GLColor.hpp"
#include "GuiVehicleView.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../GVManager.hpp"
#include "../TimeManager.hpp"
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <autogl.h>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//#define SLEEP_DEBUG
//#define VALIDROUTE_DEBUG
#define REROUTING_DEBUG

//======================================================================
void DrawerForVehicle::draw(const Vehicle& vehicle) const
{
    // 車体の描画
    // Draw vehicle body
    switch (vehicle.body()->numCars())
    {
    case 1:
        _draw1Car(vehicle);
        break;
    case 2:
        _draw2Car(vehicle);
        break;
    case 3:
        _draw3Car(vehicle);
        break;
    default:
        _draw1Car(vehicle);
    }

    auto& gv = AppMates::getGVManager();
    if (gv.getFlag("VIS_VEHICLE_ID"))
    {
      // 識別番号の描画
      // Draw ID number
      AmuPoint pv = vehicle.location()->position();
      string   id = vehicle.id();
      if (id != "")
      {
        GLColor::setVehicleId();
        AutoGL_DrawString(pv.x(), pv.y(), pv.z() + 5, id.c_str());
      }
    }

    // [eMATES] SOCの描画
    if (gv.getFlag("VIS_VEHICLE_SOC"))
    {
        VehicleEV* ev = const_cast<VehicleEV*>(dynamic_cast<const VehicleEV*>(&vehicle));
        if (ev)
        {
          // ガソリン車は表示しない
          int SOCperc = int(ev->SOC() * 100);
          stringstream ss;
          ss << setw(3) << SOCperc;
          string sSOC = ss.str();
          AmuPoint pv = vehicle.location()->position();

          GLColor::setVehicleId();
          AutoGL_DrawString(pv.x(), pv.y()-5, pv.z() + 5, sSOC.c_str());
        }
    }
}

//======================================================================
void DrawerForVehicle::drawSimple(const Vehicle& vehicle, double size) const
{
    const VehicleBehavior* behavior = vehicle.behavior();
    const VehicleLocation* location = vehicle.location();

    // 遅い車両を強調するためz座標を増加させる
    // Inflate z-coordinate to emphasize slow vehicles
    AmuPoint pv = location->position();
    pv.setZ(pv.z() + 5 * (1 - behavior->aveVelocityRate()));

    /*
     * 車両の位置と方向を表す3角形の頂点のxy座標の決定
     *
     * Determine the x,y-coordinates of vertexes of the triangle
     * representing the vehicle position and orientation
     */
    AmuVector v0 = vehicle.directionVector();
    v0.setZ(0);
    v0.normalize();
    AmuVector v1 = v0;
    v1.revoltXY(M_PI_2);

    AmuPoint p0, p1, p2;
    p0 = pv + size * v0;
    p1 = pv - size * v0 + size * v1;
    p2 = pv - size * v0 - size * v1;

    // 車体の描画
    // Draw vehicle body
    _setColor(vehicle);
    AutoGL_DrawTriangle(
        p0.x(), p0.y(), pv.z(), p1.x(), p1.y(), pv.z(), p2.x(), p2.y(), pv.z());

    if (!(AppMates::getGVManager().getFlag("VIS_VEHICLE_ID")))
    {
        return;
    }

    // 識別番号の描画
    // Draw ID number
    GLColor::setVehicleId();
    AutoGL_DrawString(pv.x(), pv.y(), pv.z() + 5, vehicle.id().c_str());
}

//======================================================================
void DrawerForVehicle::_setColor(const Vehicle& vehicle) const
{
    const VehicleBodyProperty* body     = vehicle.body();
    const VehicleBehavior*     behavior = vehicle.behavior();

#ifdef SLEEP_DEBUG
    if (vehicle.isSleep())
    {
        GLColor::setSleepingVehicle();
        return;
    }
#endif //SLEEP_DEBUG
#ifdef REROUTING_DEBUG
    if (vehicle.globalRoute()->numRerouting() > 0)
    {
        //AutoGL_SetColor(0, 0, 0);
        AutoGL_SetColor(0.3, 0.3, 0.3);
        return;
    }
#endif
#ifdef VALIDROUTE_DEBUG
    if (!(vehicle.globalRoute()->route().isValid()))
    {
        AutoGL_SetColor(0, 0, 0);
        return;
    }
#endif //VALIDROUTE_DEBUG

    double r, g, b;
    switch ((int)AppMates::getGVManager().getNumeric("VIS_VEHICLE_COLOR_MODE"))
    {
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    case GuiVehicleView::VEH_COL_VEHICLE_FAMILY:
        r = body->bodyColorR();
        g = body->bodyColorG();
        b = body->bodyColorB();
        break;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    case GuiVehicleView::VEH_COL_MEAN_SPEED:
        _getVelocityColor(&r, &g, &b, behavior->aveVelocityRate());
        break;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    case GuiVehicleView::VEH_COL_HYBRID:
        if (vehicle.body()->type()->category() == VehicleCategory::PASSENGER)
        {
            _getVelocityColor(&r, &g, &b, behavior->aveVelocityRate());
        }
        else
        {
            r = body->bodyColorR();
            g = body->bodyColorG();
            b = body->bodyColorB();
        }
        break;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    default:
        r = body->bodyColorR();
        g = body->bodyColorG();
        b = body->bodyColorB();
        break;
    }

    // 2025/04/21 [eMATES]
    GVManager& gv = AppMates::getGVManager();
    const VehicleEV* ev = dynamic_cast<const VehicleEV*>(&vehicle);
    if (gv.getFlag("FLAG_GEN_EV") && ev)
    {
      // CSに向かう車両は青に
      if (ev->isRunningToCharge())
      {
        r = 0;
        g = 0;
        b = 1;
      }
      // 充電中車両は橙色に
      if (ev->isChargingInCS())
      {
        r = 1;
        g = 0.7;
        b = 0;
      }
    }

    AutoGL_SetColor(r, g, b);
}

//======================================================================
void DrawerForVehicle::_getVelocityColor(
    double* result_r, double* result_g, double* result_b, double rate) const
{
    switch (static_cast<int>(
        AppMates::getGVManager().getNumeric("VIS_VELOCITY_COLOR_MODE")))
    {
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    case GuiVehicleView::VEL_COL_BINARY:
        if (rate < AppMates::getGVManager().getNumeric(
                "VIS_VELOCITY_RATE_THRESHOLD"))
        {
            AutoGL_GetContourColor(result_r, result_g, result_b, 0.0);
        }
        else
        {
            AutoGL_GetContourColor(result_r, result_g, result_b, 1.0);
        }
        break;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    case GuiVehicleView::VEL_COL_GRADATION:
        AutoGL_GetContourColor(result_r, result_g, result_b, rate);
        break;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    default:
        cout << "default" << endl;
        AutoGL_GetContourColor(result_r, result_g, result_b, rate);
        break;
    }
}

//======================================================================
void DrawerForVehicle::_draw1Car(const Vehicle& vehicle) const
{
    AmuPoint  pv = vehicle.location()->position();
    AmuVector dv = vehicle.directionVector();
    _drawCarInside(vehicle, 1.0, pv, dv, true);
}

//======================================================================
void DrawerForVehicle::_draw2Car(const Vehicle& vehicle) const
{
    const VehicleLocation* location    = vehicle.location();
    double                 totalLength = vehicle.body()->bodyLength();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車体1
    // Car 1
    {
        double bodyLength = totalLength * 0.49;

        // 車体がレーン終端を超えるているかどうかを判定
        // Judge if the car has crossed the end of the lane
        AmuPoint  pv;
        AmuVector dv;
        if (vehicle.distance() + bodyLength / 2 > location->lane()->length())
        {
            const Lane* nextLane = location->nextLane();
            if (!nextLane)
            {
                nextLane = location->lane();
            }
            dv = nextLane->directionVector();
            pv = nextLane->position(
                vehicle.distance() + totalLength * 0.02 + bodyLength / 2
                - location->lane()->length());
        }
        else
        {
            dv = vehicle.directionVector();
            pv = location->lane()->position(
                vehicle.distance() + totalLength * 0.02 + bodyLength / 2);
        }

        _drawCarInside(vehicle, 0.48, pv, dv, true);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車体2
    // Car 2
    {
        double bodyLength = totalLength * 0.49;

        // 車体がレーン始端を超えるているかどうかを判定する
        // Judge if the car has crossed the start of the lane
        AmuPoint  pv;
        AmuVector dv;
        if (vehicle.distance() - bodyLength / 2 < 0)
        {
            dv = location->prevLane()->directionVector();
            pv = location->prevLane()->position(
                vehicle.distance() - totalLength * 0.02 - bodyLength / 2
                + location->prevLane()->length());
        }
        else
        {
            dv = vehicle.directionVector();
            pv = location->lane()->position(
                vehicle.distance() - totalLength * 0.02 - bodyLength / 2);
        }

        _drawCarInside(vehicle, 0.48, pv, dv, false);
    }
}

//======================================================================
void DrawerForVehicle::_draw3Car(const Vehicle& vehicle) const
{
    const VehicleLocation* location    = vehicle.location();
    double                 totalLength = vehicle.body()->bodyLength();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車体1
    // Car 1
    {
        double bodyLength = totalLength * 0.35;

        // 車体がレーン終端を超えるているかどうかを判定
        // Judge if the car has crossed the end of the lane
        AmuPoint  pv;
        AmuVector dv;
        if (vehicle.distance() + totalLength * 0.15 + bodyLength / 2
            > location->lane()->length())
        {
            const Lane* nextLane = location->nextLane();
            if (!nextLane)
            {
                nextLane = location->lane();
            }
            dv = nextLane->directionVector();
            pv = nextLane->position(
                vehicle.distance() + totalLength * 0.15 + bodyLength / 2
                - location->lane()->length());
        }
        else
        {
            dv = vehicle.directionVector();
            pv = location->lane()->position(
                vehicle.distance() + totalLength * 0.15 + bodyLength / 2);
        }

        _drawCarInside(vehicle, 0.35, pv, dv, true);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車体2
    // Car 2
    {
        AmuPoint  pv = location->position();
        AmuVector dv = vehicle.directionVector();
        _drawCarInside(vehicle, 0.24, pv, dv, false);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車体3
    // Car 3
    {
        double bodyLength = totalLength * 0.35;

        // 車体がレーン始端を超えるているかどうかを判定する
        // Judge if the car has crossed the start of the lane
        AmuPoint  pv;
        AmuVector dv;
        if (vehicle.distance() - totalLength * 0.15 - bodyLength / 2 < 0)
        {
            dv = location->prevLane()->directionVector();
            pv = location->prevLane()->position(
                vehicle.distance() - totalLength * 0.15 - bodyLength / 2
                + location->prevLane()->length());
        }
        else
        {
            dv = vehicle.directionVector();
            pv = location->lane()->position(
                vehicle.distance() - totalLength * 0.15 - bodyLength / 2);
        }

        _drawCarInside(vehicle, 0.22, pv, dv, false);
    }
}

//======================================================================
void DrawerForVehicle::_drawCarInside(
    const Vehicle& vehicle, double lengthRatio, AmuPoint& pv, AmuVector& dvec,
    bool showsBlinker) const
{
    // 車体色
    _setColor(vehicle);

    // 車体サイズ
    // Body size
    double bodyLength = vehicle.bodyLength() * lengthRatio;
    double bodyWidth  = vehicle.bodyWidth();
    double bodyHeight = vehicle.bodyHeight();

    // 単位方向ベクトル，単位法線ベクトルの決定
    // Determine unit direction vector and unit normal vector
    dvec.setZ(0);
    dvec.normalize();
    AmuVector nvec = dvec;
    nvec.revoltXY(M_PI_2);

    /*
     * 車両の位置と方向を表す4角形の頂点の決定
     *
     * Determine vertexes of the quadrangle representing the vehicle
     * position and orientation
     */
    AmuPoint p0, p1, p2, p3;
    pv.setZ(pv.z() + bodyHeight);
    p0 = pv + bodyLength / 2 * dvec + bodyWidth / 2 * nvec;
    p1 = pv - bodyLength / 2 * dvec + bodyWidth / 2 * nvec;
    p2 = pv - bodyLength / 2 * dvec - bodyWidth / 2 * nvec;
    p3 = pv + bodyLength / 2 * dvec - bodyWidth / 2 * nvec;

    // 車体の描画
    // Draw vehicle body
    AutoGL_DrawQuadrangle(
        p0.x(), p0.y(), pv.z(), p1.x(), p1.y(), pv.z(), p2.x(), p2.y(), pv.z(),
        p3.x(), p3.y(), pv.z());

    // ウインカーの表示
    // Draw blinker
    if (showsBlinker && AppMates::getTimeManager().time() % 2000 < 1000)
    {
        double zBlink = 0.2;
        AutoGL_SetColor(1, 1, 0);

        switch (vehicle.blinker()->direction())
        {
        case Blinker::RIGHT:
            AutoGL_DrawCircle3D(p3.x(), p3.y(), pv.z() + zBlink, 0, 0, 1, 1, 3);
            break;
        case Blinker::LEFT:
            AutoGL_DrawCircle3D(p0.x(), p0.y(), pv.z() + zBlink, 0, 0, 1, 1, 3);
            break;
        }
    }
}

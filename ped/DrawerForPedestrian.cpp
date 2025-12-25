/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForPedestrian.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "DrawerForPedestrian.hpp"
#include "Pedestrian.hpp"
#include "PedestrianScene.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../vis/GLColor.hpp"
#include <AmuPoint.hpp>
#include <autogl.h>
#include <string>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//======================================================================
void DrawerForPedestrian::draw(const Pedestrian& ped) const
{
    const AmuPoint& position = ped.location()->position();
    double          x        = position.x();
    double          y        = position.y();
    double          z        = position.z();
    double          radius   = ped.body()->radius();
    double          zoffset  = 2.0;

    if (radius < 0.1)
    {
        radius = 0.1;
    }

    GLColor::setPedestrian();
    AutoGL_DrawCircle3D(x, y, z + zoffset, 0, 0, 1, radius, 6);

    if (AppMates::getGVManager().getFlag("VIS_PEDESTRIAN_VELOCITY"))
    {
        // 速度を1000倍に拡大して表示する
        // Display speed magnified by 1000x
        const AmuVector& velocity = ped.behavior()->velocity();
        double           vx       = velocity.x() * 1000;
        double           vy       = velocity.y() * 1000;
        double           vzoffset = 2.5;

        if (!(ped.scene()->nearestPedestrian())
            && !(ped.scene()->nearestVehicle()))
        {
            GLColor::setPedestrianArrow();
        }
        else
        {
            GLColor::setShiftedPedestrianArrow();
        }
        AutoGL_DrawLine(
            x, y, z + vzoffset, x + vx, y + vy, z + vzoffset);
    }

    if (AppMates::getGVManager().getFlag("VIS_PEDESTRIAN_ID"))
    {
        GLColor::setPedestrianId();
        AutoGL_DrawString(x, y, z + 5, ped.id().c_str());
    }
}

#endif //INCLUDE_PEDESTRIANS

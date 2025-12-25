/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file DrawerForSignal.cpp
 */
#include "DrawerForSignal.hpp"
#include "GLColor.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../Signal.hpp"
#include "../SignalColor.hpp"
#include "../TimeManager.hpp"
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <autogl.h>
#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//==============================================================================
void DrawerForSignal::draw(const Intersection& inter) const
{
    for (int dir = 0; dir < inter.numNexts(); dir++)
    {
        _drawMainLight(inter, dir);
        _drawSubLight(inter, dir);
    }
}

//==============================================================================
void DrawerForSignal::_drawMainLight(const Intersection& inter, int dir) const
{
    const Signal* signal = inter.signal();
    const Border* border = inter.border(dir);
    if (!signal || !border)
    {
        return;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 描画色
    // Drawing color
    SignalColor::MainState state = signal->mainColor(dir);
    if (state == SignalColor::MainState::BLUE)
    {
        GLColor::setBlueSignal();
    }
    else if (state == SignalColor::MainState::RED)
    {
        GLColor::setRedSignal();
    }
    else if (state == SignalColor::MainState::YELLOW)
    {
        GLColor::setYellowSignal();
    }
    else if (state == SignalColor::MainState::REDBLINK)
    {
        if (AppMates::getTimeManager().time() % 2000 < 1200)
        {
            GLColor::setRedSignal();
        }
        else
        {
            AutoGL_SetColor(1, 1, 1);
        }
    }
    else if (state == SignalColor::MainState::YELLOWBLINK)
    {
        if (AppMates::getTimeManager().time() % 2000 < 1200)
        {
            GLColor::setYellowSignal();
        }
        else
        {
            AutoGL_SetColor(1, 1, 1);
        }
    }
    else
    {
        AutoGL_SetColor(0, 0, 0);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 上三角形の描画
    // Draw upper triangles
    for (int i = 0; i < border->numIn(); i++)
    {
        AmuPoint point = border->connector(i)->point();
        AutoGL_DrawTriangle(
            point.x() + 0.6, point.y() + 0.6, point.z() + 4.0, point.x() + 0.6,
            point.y() - 0.6, point.z() + 4.0, point.x() - 0.6, point.y() + 0.6,
            point.z() + 4.0);
    }
}

//==============================================================================
void DrawerForSignal::_drawSubLight(const Intersection& inter, int dir) const
{
    const Signal* signal = inter.signal();
    const Border* border = inter.border(dir);
    if (!signal || !border)
    {
        return;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 描画色
    // Drawing color
    SignalColor::SubState state = signal->subColor(dir);
    if (state == SignalColor::SubState::NONE)
    {
        GLColor::setNoneSignal();
    }
    else if (state == SignalColor::SubState::ALL)
    {
        GLColor::setAllSignal();
    }
    else if (state == SignalColor::SubState::STRAIGHT)
    {
        GLColor::setStraightSignal();
    }
    else if (state == SignalColor::SubState::LEFT)
    {
        GLColor::setLeftSignal();
    }
    else if (state == SignalColor::SubState::RIGHT)
    {
        GLColor::setRightSignal();
    }
    else if (state == SignalColor::SubState::STRAIGHTLEFT)
    {
        GLColor::setStraightLeftSignal();
    }
    else if (state == SignalColor::SubState::STRAIGHTRIGHT)
    {
        GLColor::setStraightRightSignal();
    }
    else if (state == SignalColor::SubState::LEFTRIGHT)
    {
        GLColor::setLeftRightSignal();
    }
    else
    {
        AutoGL_SetColor(0, 0, 0);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 下三角形の描画
    // Draw lower triangles
    for (int i = 0; i < border->numIn(); i++)
    {
        AmuPoint point = border->connector(i)->point();
        AutoGL_DrawTriangle(
            point.x() + 0.6, point.y() - 0.6, point.z() + 4.0, point.x() - 0.6,
            point.y() - 0.6, point.z() + 4.0, point.x() - 0.6, point.y() + 0.6,
            point.z() + 4.0);
    }
}


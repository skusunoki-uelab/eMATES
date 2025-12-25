/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GLColor.cpp
 */
#include "GLColor.hpp"
#include <autogl.h>
#include <cassert>
#include <iostream>

using namespace std;

//======================================================================
// 背景，地面など全般的なオブジェクトの描画色
// Drawing color of general objects as background and ground
void GLColor::setBackground()
{
    AutoGL_SetBackgroundColor(0.5, 0.6, 1.0);
}

//----------------------------------------------------------------------
void GLColor::setGround()
{
    AutoGL_SetColor(0.8, 0.7, 0.5);
}

//----------------------------------------------------------------------
void GLColor::setSimpleNetwork()
{
    AutoGL_SetColor(1.0, 1.0, 1.0);
}

//======================================================================
// サブセクション
// Subsections
void GLColor::setSubsection()
{
    AutoGL_SetColor(0.8, 0.8, 0.9);
}

//----------------------------------------------------------------------
void GLColor::setCrossWalk()
{
    AutoGL_SetColor(0.9, 0.9, 0.9);
}

//----------------------------------------------------------------------
void GLColor::setSideWalk()
{
    AutoGL_SetColor(0.9, 0.9, 0.7);
}

//======================================================================
// サブネットワーク
// Subnetworks
void GLColor::setSubnetwork()
{
    AutoGL_SetColor(1, 1, 1);
}

//----------------------------------------------------------------------
void GLColor::setSubsectionEdge()
{
    AutoGL_SetColor(0.8, 0.6, 0.6);
}

//----------------------------------------------------------------------
void GLColor::setWalkerGateway()
{
    AutoGL_SetColor(0.3, 0.3, 0.5);
}

//======================================================================
// レーン
// Lane
void GLColor::setLane()
{
    AutoGL_SetColor(1, 1, 1);
}

//----------------------------------------------------------------------
void GLColor::setStraightLane()
{
    AutoGL_SetColor(0.7, 1, 0.2);
}

//----------------------------------------------------------------------
void GLColor::setLeftLane()
{
    AutoGL_SetColor(1, 0.5, 0);
}

//----------------------------------------------------------------------
void GLColor::setRightLane()
{
    AutoGL_SetColor(0, 1, 1);
}

//----------------------------------------------------------------------
void GLColor::setUpLane()
{
    AutoGL_SetColor(1, 1, 0);
}

//----------------------------------------------------------------------
void GLColor::setDownLane()
{
    AutoGL_SetColor(1, 0.8, 0);
}

//----------------------------------------------------------------------
void GLColor::setTramLane()
{
    AutoGL_SetColor(1, 1, 0.5);
}

//----------------------------------------------------------------------
void GLColor::setCrosswalkPedestrianLane()
{
    AutoGL_SetColor(0.7, 1, 1);
}

//----------------------------------------------------------------------
void GLColor::setCrosswalkVehicleLane()
{
    AutoGL_SetColor(1, 0.8, 0.7);
}

//======================================================================
// ボーダー，コネクタほか
// Borders, connectors, etc.
void GLColor::setBorder()
{
    AutoGL_SetColor(1, 1, 1);
}

//----------------------------------------------------------------------
void GLColor::setInPoint()
{
    AutoGL_SetColor(0, 0.5, 1);
}

//----------------------------------------------------------------------
void GLColor::setOutPoint()
{
    AutoGL_SetColor(0, 1, 0.5);
}

//----------------------------------------------------------------------
void GLColor::setTrafficCounter()
{
    AutoGL_SetColor(0, 0.5, 0.7);
}

//======================================================================
// 道路構造の識別番号
// ID numbers of road structures
void GLColor::setInterId()
{
    AutoGL_SetColor(0, 0, 0);
}

//----------------------------------------------------------------------
void GLColor::setLaneId()
{
    AutoGL_SetColor(1, 0.5, 0.5);
}

//----------------------------------------------------------------------
void GLColor::setSubsectionId()
{
    AutoGL_SetColor(1, 0, 0);
}

//----------------------------------------------------------------------
void GLColor::setMonitorId()
{
    AutoGL_SetColor(0, 0, 1);
}

//----------------------------------------------------------------------
void GLColor::setCSId()
{
    AutoGL_SetColor(1, 1, 0);
}

//----------------------------------------------------------------------
void GLColor::setCSValue()
{
    AutoGL_SetColor(1, 0, 0);
}



//======================================================================
// 経路探索用ネットワーク
// Network for routing
void GLColor::setRoutingLink()
{
    AutoGL_SetColor(0, 0.5, 0);
}

//----------------------------------------------------------------------
void GLColor::setRoutingLinkString()
{
    AutoGL_SetColor(0, 1, 0);
}

//----------------------------------------------------------------------
void GLColor::setRoutingNodeString()
{
    AutoGL_SetColor(0, 1, 0);
}

//----------------------------------------------------------------------
void GLColor::setRoutingRecord()
{
    AutoGL_SetColor(0.5, 0.5, 0.5);
}

//----------------------------------------------------------------------
void GLColor::setRoutingRecord(unsigned int rank)
{
    switch (rank)
    {
    case 0:
        AutoGL_SetColor(0.0, 0.0, 0.0);
        break;
    case 1:
        AutoGL_SetColor(0.6, 0.0, 1.0);
        break;
    case 2:
        AutoGL_SetColor(0.0, 0.4, 1.0);
        break;
    case 3:
        AutoGL_SetColor(0.6, 1.0, 0.0);
        break;
    case 4:
        AutoGL_SetColor(1.0, 1.0, 0.0);
        break;
    case 5:
        AutoGL_SetColor(1.0, 0.4, 0.0);
        break;
    default:
        AutoGL_SetColor(0.5, 0.5, 0.5);
    }
}

//----------------------------------------------------------------------
void GLColor::setRoutingLastRecord()
{
    AutoGL_SetColor(1, 0, 0);
}

//======================================================================
// 信号機
// Traffic lights
void GLColor::setBlueSignal()
{
    AutoGL_SetColor(0, 0, 1);
}

//----------------------------------------------------------------------
void GLColor::setRedSignal()
{
    AutoGL_SetColor(1, 0, 0);
}

//----------------------------------------------------------------------
void GLColor::setYellowSignal()
{
    AutoGL_SetColor(1, 1, 0);
}

//----------------------------------------------------------------------
void GLColor::setNoneSignal()
{
    AutoGL_SetColor(0, 0, 0);
}

//----------------------------------------------------------------------
void GLColor::setAllSignal()
{
    AutoGL_SetColor(1, 1, 1);
}

//----------------------------------------------------------------------
void GLColor::setStraightSignal()
{
    AutoGL_SetColor(1, 0, 0);
}

//----------------------------------------------------------------------
void GLColor::setLeftSignal()
{
    AutoGL_SetColor(0, 1, 0);
}

//----------------------------------------------------------------------
void GLColor::setRightSignal()
{
    AutoGL_SetColor(0, 0, 1);
}

//----------------------------------------------------------------------
void GLColor::setStraightLeftSignal()
{
    AutoGL_SetColor(1, 1, 0);
}

//----------------------------------------------------------------------
void GLColor::setStraightRightSignal()
{
    AutoGL_SetColor(1, 0, 1);
}

//----------------------------------------------------------------------
void GLColor::setLeftRightSignal()
{
    AutoGL_SetColor(0, 1, 1);
}

//======================================================================
// 自動車
// Vehicles
void GLColor::setVehicle()
{
    AutoGL_SetColor(1, 0, 0);
}

//----------------------------------------------------------------------
void GLColor::setSleepingVehicle()
{
    AutoGL_SetColor(1, 1, 1);
}

//----------------------------------------------------------------------
void GLColor::setTruck()
{
    AutoGL_SetColor(0.3, 0.7, 1);
}

//----------------------------------------------------------------------
void GLColor::setBus()
{
    AutoGL_SetColor(0, 0.8, 0.5);
}

//----------------------------------------------------------------------
void GLColor::setTram()
{
    AutoGL_SetColor(1, 0.5, 0);
}

//======================================================================
// 歩行者
// Pedestrians
void GLColor::setPedestrian()
{
    AutoGL_SetColor(0, 0.8, 0);
}

//----------------------------------------------------------------------
void GLColor::setPedestrianArrow()
{
    AutoGL_SetColor(0, 0, 0);
}

//----------------------------------------------------------------------
void GLColor::setShiftedPedestrianArrow()
{
    AutoGL_SetColor(1, 0, 0);
}

//======================================================================
// エージェントの識別番号
// ID numbers of agents
void GLColor::setVehicleId()
{
    AutoGL_SetColor(0, 0, 0);
}
//----------------------------------------------------------------------
void GLColor::setPedestrianId()
{
    AutoGL_SetColor(0, 0, 0);
}

/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GLColor.hpp
 */
#ifndef __GL_COLOR_HPP__
#define __GL_COLOR_HPP__

//######################################################################
/**
 * @~japanese AutoGL を用いて描画する際の色を管理する
 *
 * @note このクラスは静的クラスとする
 *
 * @~english  Manage colors when drawing with AutoGL
 *
 * @note This class is static class.
 *
 * @~ @ingroup Drawing
 */
class GLColor
{
public:
    //==================================================================
    /**
     * @~japanese
     * @name 背景，地面など全般的なオブジェクトの描画色
     *
     * @~english
     * @name Drawing color of general objects as background and ground
     */
    ///@{
    static void setBackground();
    static void setGround();
    static void setSimpleNetwork();
    ///@}

    //==================================================================
    /**
     * @~japanese @name サブセクションに関する描画色
     * @~english  @name Drawing color for subsections
     */
    ///@{
    static void setSubsection();
    static void setCrossWalk();
    static void setSideWalk();
    ///@}

    /**
     * @~japanese @name サブネットワークに関する描画色
     * @~english  @name Drawing color for subnetworks
     */
    ///@{
    static void setSubnetwork();
    static void setSubsectionEdge();
    static void setWalkerGateway();
    ///@}

    /**
     * @~japanese @name レーンに関する描画色
     * @~english  @name Drawing colors for lanes
     */
    ///@{
    static void setLane();
    static void setStraightLane();
    static void setLeftLane();
    static void setRightLane();
    static void setUpLane();
    static void setDownLane();
    static void setTramLane();
    static void setCrosswalkPedestrianLane();
    static void setCrosswalkVehicleLane();
    ///@}

    /**
     * @~japanese @name ボーダー，コネクタほかに関する描画色
     * @~english  @name Drawing colors for borders, connectors, etc.
     */
    ///@{
    static void setBorder();
    static void setInPoint();
    static void setOutPoint();
    static void setTrafficCounter();
    ///@}

    /**
     * @~japanese @name 道路構造の識別番号に関する描画色t
     * @~english  @name Drawing color for ID numbers of road structures
     */
    ///@{
    static void setInterId();
    static void setLaneId();
    static void setSubsectionId();
    static void setMonitorId();
    static void setCSId(); // [eMATES]
    static void setCSValue(); // [eMATES]
    ///@}

    //==================================================================
    /**
     * @~japanese @name 経路探索用ネットワークに関する描画色
     * @~english  @name Drawing color for network for routing
     */
    ///@{
    static void setRoutingLink();
    static void setRoutingLinkString();
    static void setRoutingNodeString();
    static void setRoutingRecord();
    static void setRoutingRecord(unsigned int rank);
    static void setRoutingLastRecord();
    ///@}

    //==================================================================
    /**
     * @~japanese @name 信号機に関する描画色
     * @~english  @name Drawing color for traffic lights
     */
    ///@{
    static void setBlueSignal();
    static void setRedSignal();
    static void setYellowSignal();
    static void setNoneSignal();
    static void setAllSignal();
    static void setStraightSignal();
    static void setLeftSignal();
    static void setRightSignal();
    static void setStraightLeftSignal();
    static void setStraightRightSignal();
    static void setLeftRightSignal();
    ///@}

    //====================================================================
    /**
     * @~japanese @name 自動車に関する描画色
     * @~english  @name Drawing color for vehicles
     */
    ///@{
    static void setVehicle();
    static void setSleepingVehicle();
    static void setTruck();
    static void setBus();
    static void setTram();
    ///@}

    /**
     * @~japanese @name 歩行者に関する描画色
     * @~english  @name Drawing color for pedestrians
     */
    ///@{
    static void setPedestrian();
    static void setPedestrianArrow();
    static void setShiftedPedestrianArrow();
    ///@}

    /**
     * @~japanese @name エージェントの識別番号に関する描画色
     * @~english  @name Drawing color for ID numbers of agents
     */
    ///@{
    static void setVehicleId();
    static void setPedestrianId();
    ///@}

    ///@}

    //==================================================================
private:
    GLColor()  = delete;
    ~GLColor() = delete;
};

#endif //__GL_COLOR_H__

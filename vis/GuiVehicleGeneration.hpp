/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiVehicleGeneration.hpp
 */
#ifndef __GUI_VEHICLE_GENERATION_HPP__
#define __GUI_VEHICLE_GENERATION_HPP__

//######################################################################
/**
 * @~japanese 車両を手動で生成する
 *
 * Visualizer の部分クラス
 *
 * @~english  Generate vehicle manually
 *
 * A part class of Visualizer
 *
 * @~ @ingroup Visualization
 */
class GuiVehicleGeneration
{
public:
    GuiVehicleGeneration();
    ~GuiVehicleGeneration() {};

    /**
     * @~japanese パネルを作成する
     * @~english  Create panel
     */
    static void makePanel();

    /**
     * @~japanese 入力情報にもとづいて車両を生成する
     *
     * Generate Vehicle ボタンが押されたときの動作
     *
     * @~english  Generate vehicle based on input information
     *
     * The action when Generate Vehicle button is pressed. 
     */
    static void generateVehicleButtonCallback();

protected:
    /**
     * @~japanese 車種
     * @~english  Vehicle type
     */
    static char _vehicleType[16];

    /**
     * @~japanese 出発地の識別番号
     * @~english  Origin ID number
     */
    static char _startId[16];

    /**
     * @~japanese 目的地の識別番号
     * @~english  Destination ID number
     */
    static char _goalId[16];

    /**
     * @~japanese 経由地の識別番号
     * @~english  ID numbers of intersections of pass
     */
    static char _gateId[256];

    /**
     * @~japanese 経路探索用パラメータ 
     * @~english  Parameters for routing
     */
    static char _routingParams[256];

    /**
     * @~japanese 経路探索で選好するネットワークランク
     * @~english  Preferred network rank for routing
     */
    static char _prefRank[16];
};

#endif //__GUI_VEHICLE_GENERATION_HPP__

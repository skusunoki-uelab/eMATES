/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SimulatorPedExt.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __SIMULATOR_PED_EXT_HPP__
#define __SIMULATOR_PED_EXT_HPP__

/**
 * @defgroup PedSim
 * @~japanese 歩行者シミュレーション用モジュール
 * @~english  Module for Pedestrian simulation
 */

class Simulator;
class PedestrianGenerator;

//######################################################################
/**
 * @~japanese シミュレータ管理クラスの歩行者拡張
 * @~english  Pedestrian extension of simulation manager class
 * @~ @ingroup Initialization PedSim Running
 */
class SimulatorPedExt
{
public:
    SimulatorPedExt() {};
    explicit SimulatorPedExt(Simulator* simulator);
    ~SimulatorPedExt();

    /**
     * @~japanese 歩行者生成の準備をおこなう
     * @return 準備に成功したかどうか
     *
     * @~english  Prepare for pedestrian generation
     * @return Whether the preparation was successful
     */
    bool getReadyPedestrians();

    /**
     * @~japanese 歩行者を発生させる
     * @~english  Generate pedestrians
     */
    void generatePedestrian();

private:
    /**
     * @~japanese 対応するシミュレータ
     * @~english  Corresponding simulator
     */
    Simulator* _simulator;

    /**
     * @~japanese 歩行者生成用コントローラ
     * @~english  Pedestrian generation controller
     */
    PedestrianGenerator* _pedestrianGenerator;
};

#endif //__SIMULATOR_PED_EXT_HPP__
#endif //INCLUDE_PEDESTRIANS

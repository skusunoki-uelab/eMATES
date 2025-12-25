/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file Simulator.hpp
 */
#ifndef __SIMULATOR_HPP__
#define __SIMULATOR_HPP__
#include "Config.hpp"
#include "RoadMap.hpp"
#include "TimeManager.hpp"
#include "VehicleGenerator.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "ped/SimulatorPedExt.hpp"
#endif //INCLUDE_PEDESTRIANS
#include <string>

class AppMates;
class RoutingManager;
class ODNode;
class Vehicle;

/**
 * @defgroup Running
 * @ingroup Procedure
 * @~japanese シミュレーションの実行
 * @~english  Simulation run
 */

//######################################################################
/**
 * @~japanese シミュレーションを管理する
 * @~english  Manage simulation
 * @~ @ingroup Initialization Running
 */
class Simulator
{
public:
#ifdef INCLUDE_PEDESTRIANS
    friend class SimulatorPedExt;
#endif //INCLUDE_PEDESTRIANS

    Simulator();
    virtual ~Simulator();

    //==================================================================
    /**
     * @~japanese @name シミュレーションの準備に関する関数群
     * @~english  @name Functions for preparation of simulation
     */
    ///@{
public:
    /**
     * @~japanese ファイルを読み込んで道路環境を作成する
     *
     * 地図オブジェクトを作成し，単路部の属性を付与する．この情報を
     * もとに経路探索用ネットワークを作成する．同時に信号を生成する．
     *
     * @return 作成に成功したかどうか
     *
     * @~english  Load a file and prepare a road environment
     *
     * Create a road map object and give it the attributes of sections.
     * Based on this, create a network for routing. At the same time,
     * create signals.
     *
     * @return Whether the creation was successful
     */
    bool getReadyRoadEnvironment();

    /**
     * @~japanese 観測器の準備をおこなう
     * @return 準備に成功したかどうか
     *
     * @~english  Prepare monitors
     * @return Whether the preparation was successful
     */
    bool getReadyMonitors();

    /**
     * @~japanese 車両生成の準備をおこなう
     * @return 準備に成功したかどうか
     *
     * @~english  Prepare for vehicle generation
     * @return Whether the preparation was successful
     */
    bool getReadyVehicles();

    /**
     * @~japanese
     * 作成した地図オブジェクトの情報を出力する
     *
     * @~english
     *  Output the information of the created road map object
     */
    void writeInitializedRoadMap();

    ///@}

    //==================================================================
    /**
     * @~japanese @name シミュレーションの実行に関する関数群
     * @~english  @name Functions for simulation run
     */
    ///@{
public:
    /**
     * @~japanese 時刻 @p time までシミュレーションを進める
     * @~english  Advance the simulation to @p time
     */
    bool run(ulint time);

    /**
     * @~japanese 1ステップだけシミュレーションを進める
     * @~english  Advance the simulation by one step
     */
    bool incrementStep();

private:
    /**
     * @~japanese コンソールへ時刻を表示する
     * @~english  Display time to the console
     */
    void _printTime() const;

    /**
     * @~japanese
     * タイマー @p clockName の計時を開始する
     *
     * @~english
     * Start timing with the timer specified by @p clockerName
     */
    void _startClock(const std::string& clockerName);

    /**
     * @~japanese
     * タイマー @p clockName の計時を終了する
     *
     * @~english
     * Stop timing with the timer specified by @p clockerName
     */
    void _stopClock(const std::string& clockerName);

    /**
     * @~japanese リンク交通流を観測する
     * @~english  Observe link traffic flow
     */
    void _observeLinkFlow();

    // OpenDSS用のファイル出力を行う
    void _writeEVComm();

    ///@}

    //==================================================================
    /**
     * @~japanese @name 結果のファイル出力に関する関数群
     * @~english  @name Functions for outputting result to files
     */
    ///@{
public:
    /**
     * @~japanese
     * 総ステップ数と時間刻み幅をファイルに出力する
     *
     * @note
     * 本来は終了時に出力すればよいはすであるが，実行時エラーの発生や
     * 強制終了に対処するため，各ステップが終わるたびに書き換える．
     *
     * @~english
     * Output total number of steps and time step size [ms] to file
     *
     * @note
     * It should be output at exit originally, but in order to cope
     * with the occurrence of runtime errors and forced termination,
     * it is rewritten every time each step end.
     */
    void writeRunInfo() const;

    /**
     * @~japanese 観測器の結果をファイルに出力する
     * @~english  Output monitor result to file
     */
    void writeMonitorResult();

    /**
     * @~japanese 各ステップの結果をファイルに出力する
     * @~english  Output results of each step to file
     */
    void writeStepResult() const;

    /**
     * @~japanese シミュレーション終了時に結果をファイルに出力する
     * @~english  Output results to file when simulation exits
     */
    void writeResultAtExit() const;

    ///@}

    //==================================================================
protected:
    /**
     * @~japanese 地図オブジェクト
     * @~english  Road map object
     */
    RoadMap* _roadMap;

    /**
     * @~japanese 車両生成用コントローラ
     * @~english  Vehicle generation controller
     */
    VehicleGenerator* _vehicleGenerator;

    /**
     * @~japanese レーンチェックエラーが発生したかどうか
     * @~english  Whether a lane check error occurred
     */
    bool _failsLaneCheck;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    RoadMap* roadMap()
    {
        return _roadMap;
    }

    VehicleGenerator* vehicleGenerator()
    {
        return _vehicleGenerator;
    }

    bool failsLaneCheck() const;

    ///@}

#ifdef INCLUDE_PEDESTRIANS
    //==================================================================
    /**
     * @~japanese @name 歩行者用拡張
     * @~english  @name Extension for pedestrian
     */
    ///@{
protected:
    SimulatorPedExt* _pedExt;

public:
    SimulatorPedExt* pedExt()
    {
        return _pedExt;
    }

    ///@}
#endif //INCLUDE_PEDESTRIANS
};

#endif //__SIMULATOR_HPP__

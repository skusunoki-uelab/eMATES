/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file GuiSimulationControl.hpp
 */
#ifndef __GUI_SIMULATION_CONTROL_HPP__
#define __GUI_SIMULATION_CONTROL_HPP__

//##############################################################################
/**
 * @~japanese シミュレーションの進行とファイル出力を制御する
 *
 * Visualizer の部分クラス
 *
 * @~english  Control simulation running and file output
 *
 * A part class of Visualizer
 *
 * @~ @ingroup Visualization
 */
class GuiSimulationControl
{
public:
    GuiSimulationControl();
    ~GuiSimulationControl() {}

    /**
     * @~japanese フラグを更新する
     * @~english  Update flags
     */
    static void renewFlags();

    /**
     * @~japanese パネルを作成する
     * @~english  Create panel
     */
    static void makePanel();

public:
    /**
     * @~japanese シミュレーションを1ステップ進める
     *
     * Time Increment ボタンが押されたときの動作．
     *
     * @~english  Advance the simulation by one step
     *
     * Action when Time Increment button is pressed.
     */
    static void incrementButtonCallback();

    /**
     * @~japanese _poseTime までシミュレーションを動かす
     *
     * Continuous Run ボタンが押されたときの動作として用いられる．また，
     * Visualizer::viewRenderCallback() においても使用される．時刻が
     * _poseTime になったらシミュレーションの進行が停止する．
     *
     * @~english Run the simulation until _poseTime
     *
     * Used as an action when Continuous Run button is pressed. Also used in
     * Visualizer::viewRenderCallback(). The simulation stops progressing when
     * the time reaches _poseTTime.
     */
    static void runButtonCallback();

    /**
     * @~japanese シミュレーションの動作と停止を切り替える
     *
     * Auto Time Increment ボタンが押されたときの動作．ボタンが
     * 押されたらシミュレータが進行し，ふたたび押されたら進行が
     * 停止する．
     *
     * @~english  Toggle between running and stopping the simulation
     *
     * Action when Auto Time Increment button is pressed. When a button
     * pressed, the simulator progresses, and when it is pressed again,
     * it stops progressing.
     */
    static void autoIncrementButtonCallback();

private:
    /**
     * @~japanese run および autoIncrement 用の内部関数
     * @~english  Internal function for run and autoIncrement
     */
    static void _incrementStepInside();

    /**
     * @~japanese 画像を保存する
     * @~english  Save image
     */
    static void _saveImage();

private:
    /**
     * @~japanese アイドルイベントが有効かどうか
     * @~english  Whether idle event is enabled
     */
    static int _isIdleEventOn;

    /**
     * @~japanese run の目標時刻 [ms]
     * @~english  Target time of run [ms]
     */
    static int _poseTime;

    /**
     * @~japanese シミュレーション1ステップあたりの時間刻み幅 [ms]
     * @~english  Time step size [ms] per simulation step
     */
    static int _unit;

    /**
     * @~japanese 描画1回あたりの時間刻み幅 [ms]
     *
     * たとえば，_unitForDrawing に _unit の10倍の値を設定すると描画は
     * シミュレーション10ステップに1回の頻度になる．
     *
     * @~english  Time step size [ms] per one drawing
     *
     * For example, if _unitForDrawing is set to 10 times the value of _unit,
     * drawing is done once every 10 simulation steps. 
     */
    static int _unitForDrawing;

    /**
     * @~japanese 描画ステップを間引くかどうか
     * @~english  Whether to thin out drawing steps
     */
    static int _thinsOutDrawingStep;

    /**
     * @~japanese ビューをキャプチャするかどうか
     * @~english  Whether to capture view
     */
    static int _capturesView;

    /**
     * @~japanese キャプチャファイルに付与される番号
     *
     * あとで結合して動画を生成するため連番で出力する必要がある．
     *
     * @~english  Number assigned to the capture files
     *
     * Necessary to output in serial numbers in order to combine them
     * later to crate a movie.
     */
    static int _frameNumber;

    /**
     * @~japanese 時系列統計データを出力するかどうか
     * @~english  Whether to output aggregated time-series data
     */
    static int _outputsTimeSeriesAggregated;

    /**
     * @~japanese 車両の時系列詳細データを出力するかどうか
     * @~english  Whether to output detailed vehicle time-series data
     */
    static int _outputsVehicleTimeSeriesDetailed;

#ifdef INCLUDE_PEDESTRIANS
    /**
     * @~japanese 歩行者の時系列詳細データを出力するかどうか
     * @~english  Whether to output detailed pedestrian time-series data
     */
    static int _outputsPedestrianTimeSeriesDetailed;
#endif //INCLUDE_PEDESTRIANS

    /**
     * @~japanese
     * 信号の時系列詳細データを出力するかどうか
     *
     * @~english
     * Whether to output detailed traffic light time-series data
     */
    static int _outputsSignalTimeSeriesDetailed;

    /**
     * @~japanese 車両感知器の集計データを出力するかどうか
     * @~english  Whether to output aggregated data of traffic counters
     */
    static int _outputsTrafficCounterAggregated;

    /**
     * @~japanese 車両感知器の詳細データを出力するかどうか
     * @~english  Whether to output detailed data of traffic counters
     */
    static int _outputsTrafficCounterDetailed;

    /**
     * @~japanese リンク交通流検知器の集計データを出力するかどうか
     * @~english  Whether to output aggregated data of link traffic flow moitors
     */
    static int _outputsLinkFlowMonitorAggregated;

    /**
     * @~japanese リンク交通流検知器の詳細データを出力するかどうか
     * @~english  Whether to output detailed data of link traffic flow moitors
     */
    static int _outputsLinkFlowMonitorDetailed;

    /**
     * @~japanese 流入検知器の結果を出力するかどうか
     * @~english  Whether to output inflow monitoring data
     */
    static int _outputsInflowMonitor;

    /**
     * @~japanese 車列観測結果を出力するかどうか
     * @~english  Whether to output vehicle convoy monitoring data 
     */
    static int _outputsConvoyMonitor;

    /**
     * @~japanese 車両のトリップデータを出力するかどうか
     * @~english  Whether to output vehicle trip data
     */
    static int _outputsVehicleTrip;
};

#endif //__GUI_SIMULATION_CONTROL_HPP__

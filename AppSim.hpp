/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file AppSim.hpp
 */
#ifndef __APP_SIM_HPP__
#define __APP_SIM_HPP__
#include "AppMates.hpp"
#include "Config.hpp"
#include "vis/Visualizer.hpp"
#include <getopt.h>
#include <memory>

//##############################################################################
/**
 * @~japanese advmates-simアプリケーションクラス
 * @~english  Application class of advmates-sim
 * @~ @ingroup   Initialization
 */
class AppSim : public AppMates
{
public:
    AppSim() : AppMates(DEFAULT_MAX_TIME) {};
    virtual ~AppSim() {};

    /**
     * @~japanese アプリケーションの初期化とコマンドライン引数の処理
     * @param loopNum ループ回数 (出力ディレクトリの切り替えに用いる)
     *
     * @~english  Initialize application and handle command-line options
     * @param loopNum Loop count (used for switching output directory)
     */
    virtual void initialize(
        int argc, char** argv, unsigned int loopNum) override;

protected:
    /**
     * @~japanese コマンドライン引数の処理の実体
     * @~english  Substance of command-line option processing
     */
    virtual void _parseArgument(int argc, char** argv);

    /**
     * @~japanese 説明を出力する
     * @~english  Print usage to stdout
     */
    virtual void _printUsage();

public:
    /**
     * @~japanese シミュレータの初期設定を行う
     * @~english  Set up simulator
     * @~
     * @pre AppMates::initialize()
     * @pre AppMates::getReadySimulator();
     */
    void getReadyVisualizer();

protected:
    /**
     * @~japanese デフォルト計算対象時間
     * @~english  Default simulation target time
     */
    static const ulint DEFAULT_MAX_TIME = 60ul * 1000ul; // 60 sec

public:
    /**
     * @~japanese シミュレーションを開始する
     * @~english  Start simulation
     */
    int run();

private:
    /**
     * @~japanese 可視化とGUI機能を提供するオブジェクト
     * @~english  Object for visualization and GUI functionality
     */
    std::unique_ptr<Visualizer> _vis;
};

#endif //__APP_SIM_HPP__

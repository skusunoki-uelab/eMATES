/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file mainSim.cpp
 */
#include "AppSim.hpp"
#include "CustomMessage.hpp"
#include <autogl.h>
#include <cstdlib>
#include <memory>

//==============================================================================
/**
 * @~japanese アプリケーションオブジェクト
 * @~english  Application object
 */
std::unique_ptr<AppSim> App;

//==============================================================================
/**
 * @~japanese exit時に実行する関数
 * @~english  Function to run on exit
 *
 * @see AppMates
 */
void exitFunc()
{
    App.reset();
}

//==============================================================================
/**
 * @~japanese
 * advmates-sim用のセットアップ関数
 *
 * AppSimを作成して初期化を行い，シミュレーションを開始する．計算と結果の
 * 可視化（アニメーション）を同時に行う．
 *
 * @note
 * AutoGL を利用したコードを作成する場合，AutoGL_SetUp から実質的な処理を
 * 開始する．main 関数は lib/autogl/autogl_gui_*.c に含まれる．
 *
 * @~english
 * Set-up function for advmates-sim
 *
 * Create and initialize AppSim, start the simulation. Calculation and
 * visualization are performed simultaneously.
 *
 * @note
 * When coding with AutoGL, start process with AutoGL_SetUp; the main function
 * is contained in lib/autogl/autogl_gui_*.c.
 */
void AutoGL_SetUp(int argc, char* argv[])
{
    // exit時に明示的にデストラクタを呼び出しメモリリークを回避する
    // Call destructor explicitly before exit to avoid memory leaks.
    atexit(exitFunc);

    App.reset(new AppSim());
    App->initialize(argc, argv, 0);

    App->getReadySimulator();
    App->getReadyVisualizer();
    App->printGlobalSetting(false);

    App->run();
}

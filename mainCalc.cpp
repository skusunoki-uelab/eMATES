/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file mainCalc.cpp
 */
#include "AppCalc.hpp"
#include "GVManager.hpp"
#include <cstdlib>
#include <memory>
#include <iostream>

using namespace std;

//==============================================================================
/**
 * @~japanese アプリケーションオブジェクト
 * @~english  Application object
 */
unique_ptr<AppCalc> App;

//==============================================================================
/**
 * @~japanese exit時に実行する関数
 * @~english Function to run on exit
 *
 * @see AppMates
 */
void exitFunc()
{
    if (App.get())
    {
        App.reset();
    }
}

//==============================================================================
/**
 * @~japanese
 * advmates-calc用のmain関数
 *
 * AppCalcを作成して初期化を行い，シミュレーションを開始する．計算のみ行い，
 * 結果の可視化（アニメーション）を行わない．
 *
 * @~english
 * Main function for advmates-calc
 *
 * Create and initialize AppCalc, start the simulation. Only calculation is
 * performed; no visualization (animation) is performed.
 */
int main(int argc, char* argv[])
{
    // exit時に明示的にデストラクタを呼び出しメモリリークを回避する
    // Call destructor explicitly before exit to avoid memory leaks
    atexit(exitFunc);

    App.reset(new AppCalc());
    App->initialize(argc, argv, 1);
    App->getReadySimulator();
    App->printGlobalSetting(true);
    App->batchRun();

    /*
     * 繰り返し回数の取得
     *   グローバル変数はAppCalc::initialize()の呼び出し後から，デストラクタの
     *   呼び出し前まで有効
     *
     * Get number of repetition
     *   Global variables are valid after calling AppCalc::initialize() and
     *   before calling its destructor.
     */
    unsigned int maxLoop = static_cast<unsigned int>(
        AppMates::getGVManager().getNumeric("NUM_LOOPS"));

    for (unsigned int i = 2; i <= maxLoop; i++)
    {
        App.reset();

        cout << endl << "### LOOP: " << i << "/" << maxLoop << " ###" << endl;
        App.reset(new AppCalc());
        App->initialize(argc, argv, i);
        App->getReadySimulator();
        App->printGlobalSetting(false);
        App->batchRun();
    }

    return EXIT_SUCCESS;
}

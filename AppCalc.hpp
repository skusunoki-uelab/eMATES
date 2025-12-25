/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file AppCalc.hpp
 */
#ifndef __APP_CALC_HPP__
#define __APP_CALC_HPP__
#include "AppMates.hpp"
#include "Config.hpp"
#include <getopt.h>
#include <memory>

//##############################################################################
/**
 * @~japanese advmates-calcアプリケーションクラス
 * @~english  Application class of advmates-calc
 * @ingroup   Initialization
 */
class AppCalc : public AppMates
{
public:
    AppCalc() : AppMates(DEFAULT_MAX_TIME) {};
    virtual ~AppCalc() {};

    /**
     * @~japanese アプリケーションの初期化とコマンドライン引数の処理
     * @param loopNum ループ回数 (出力ディレクトリの切り替えに用いる)
     *
     * @~english  Initialize application and handle command-line options
     * @param loopNum Loop count (used for switching output directory)
     */
    virtual void initialize(
        int argc, char** argv, unsigned int loopNum) override;

    /**
     * @~japanese 計算と結果出力
     * @~english  Calculation and result output
     */
    int batchRun();

protected:
    /**
     * @~japanese コマンドライン引数の処理の実体
     * @~english  Substance of command-line option processing
     */
    virtual void _parseArgument(int argc, char** argv) override;

    /**
     * @~japanese 説明を出力する
     * @~english  Print usage to stdout
     */
    virtual void _printUsage() override;

protected:
    /**
     * @~japanese デフォルト計算対象時間
     * @~english  Default simulation target time
     */
    static const ulint DEFAULT_MAX_TIME = 3600ul * 1000ul; // 1 hr
};

#endif //__APP_CALC_HPP__

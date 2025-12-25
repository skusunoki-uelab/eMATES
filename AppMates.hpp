/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file AppMates.hpp
 */
#ifndef __APP_MATES_HPP__
#define __APP_MATES_HPP__
#include "ManagerBase.hpp"
#include "ManagerPool.hpp"
#include "Simulator.hpp"
#include <memory>
#include <string>
#ifndef USE_MINGW
#include <getopt.h>
#endif //USE_MINGW

class ClockerManager;
class FileManager;
class GVManager;
class LoggerManager;
class ObjectManager;
class RandomSeedManager;
class RouteCacheManager;
class RouterManager;
class ScheduleManager;
class TimeManager;
class VehicleTypeManager;
#ifdef INCLUDE_TRAMS
class TramRouteManager;
#endif //INCLUDE_TRAMS

/**
 * @defgroup  Procedure
 * @~japanese シミュレーション手順を定義するモジュール
 * @~english  Simulation procedure
 */

/**
 * @defgroup  Initialization
 * @ingroup   Procedure
 * @~japanese シミュレーションの初期化と開始
 * @~english  Simulation initialization
 */

//##############################################################################
/**
 * @~japanese アプリケーションの基底クラス
 * 
 * コマンドラインオプションや乱数の種などを管理する
 *
 * @~english  Application base class
 *
 * Manage command-line options and random seeds etc.
 *
 * @~ @ingroup Initialization
 */
class AppMates
{
public:
    AppMates() {};
    explicit AppMates(unsigned long maxTime);
    virtual ~AppMates();

public:
    /**
     * @~japanese アプリケーションの初期化とコマンドライン引数の処理
     * @param loopNum ループ回数 (出力ディレクトリの切り替えに用いる)
     *
     * @~english  Initialize application and handle command-line options
     * @param loopNum Loop count (used for switching output directory)
     */
    virtual void initialize(int argc, char** argv, unsigned int loopNum);

protected:
    /**
     * @~japanese --no-verbose (-q) オプションをチェックする
     *
     * @attention
     * 出力を抑制する必要があるため優先してフラグを設定する．
     *
     * @~english  Check --no-verbose (-q) option
     *
     * @attention
     * Set the flag before other settings because it is necessary to suppress
     * output.
     */
    bool _checkQuietOption(int argc, char** argv);

    /**
     * @~japanese データディレクトリのパスを指定する
     *
     * @attention
     * 指定がない場合はカレントディレクトリをデータディレクトリとみなす．
     * GV_INIT_FILEファイルの場所を指定する必要があるため，優先して設定．
     *
     * @~english  Set the path to the data directory
     *
     * @attention
     * The current directory is assumed to be the data directory if it is not
     * specified. Set the data directory before other settings because it is
     * necessary to specify the location of GV_INIT_FILE file.
     */
    bool _setDataPath(int argc, char** argv);

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

    /**
     * @~japanese 乱数の種を指定する
     * @~english  Set random seed
     */
    bool _setRandomSeed(const std::string& arg);

public:
    /**
     * @~japanese シミュレータの初期設定を行う
     * @~english  Set up simulator
     * @~ @pre AppMates::initialize()
     */
    virtual void getReadySimulator();

    /**
     * @~japanese シミュレーション全体の設定を出力する
     * @param output 重要なフラグの設定を表示するか
     *
     * @~english  Print settings for the entire simulation
     * @param output Whether to print only important flags
     *
     * @~ @pre AppMates::initialize()
     */
    virtual void printGlobalSetting(bool isFlagOutput);

protected:
    /**
     * @~japanese マネージャのコンテナ
     *
     * @note
     * マネージャへのアクセッサを静的メンバ関数として提供するため，コンテナを
     * 静的メンバ変数として持つ．デストラクタで明示的にdeleteする．
     *
     * @~english  Manager container
     *
     * @note
     * In order to provide accessors to the manager as static member functions,
     * have this container as a static member variable. Explicitly delete in
     * destructor.
     */
    static ManagerPool* _managerPool;

    /**
     * @~japanese シミュレータ
     * @~english  Simulator
     */
    Simulator* _simulator;

    /**
     * @~japanese データディレクトリへのパス
     * @~english  Path to the data directory
     */
    std::string _dataPath;

    /**
     * @~japanese 乱数の種
     * @~english  Random seed
     */
    unsigned int _key;

    //==========================================================================
    /**
     * @~japanese @name コマンドラインオプションの解析に用いる変数
     * @~english  @name Variables used to parse command-line options
     */
    ///@{
    static int           optionIndex;
    static std::string   shortOptions;
    static struct option longOptions[];
    ///@}

    //==========================================================================
    /**
     * @~japanese @name マネージャへのアクセッサと別名
     * @~english  @name Accessor to managers and their aliases
     */
    ///@{
public:
    static ClockerManager& getClockerManager()
    {
        return ManagerPool::instance()->clocker();
    }

    static FileManager& getFileManager()
    {
        return ManagerPool::instance()->file();
    }

    static GVManager& getGVManager()
    {
        return ManagerPool::instance()->gv();
    }

    static LoggerManager& getLoggerManager()
    {
        return ManagerPool::instance()->logger();
    }

    static ObjectManager& getObjectManager()
    {
        return ManagerPool::instance()->obj();
    }

    static RandomSeedManager& getRandomSeedManager()
    {
        return ManagerPool::instance()->rand();
    }

    static RouteCacheManager& getRouteCacheManager()
    {
        return ManagerPool::instance()->routeCache();
    }

    static RouterManager& getRouterManager()
    {
        return ManagerPool::instance()->router();
    }

    static ScheduleManager& getScheduleManager()
    {
        return ManagerPool::instance()->schedule();
    }

    static TimeManager& getTimeManager()
    {
        return ManagerPool::instance()->time();
    }

    static VehicleTypeManager& getVehicleTypeManager()
    {
        return ManagerPool::instance()->vt();
    }
#ifdef INCLUDE_TRAMS
    static TramRouteManager& getTramRouteManager()
    {
        return ManagerPool::instance()->tramRoute();
    }
#endif //INCLUDE_TRAMS
    ///@}
};

#endif //__APP_MATES_HPP__

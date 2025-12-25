/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LoggerManager.hpp
 */
#ifndef __LOGGER_MANAGER_HPP__
#define __LOGGER_MANAGER_HPP__
#include "ManagerBase.hpp"
#include "io/Logger.hpp"
#include <string>
#include <unordered_map>

//##############################################################################
/**
 * @~japanese シミュレーションのログを管理する
 * @~english  Manage simulation log
 * @~ @ingroup IO Manager
 */
class LoggerManager : public ManagerBase
{
    friend class ManagerPool;

private:
    LoggerManager();
    ~LoggerManager()
    {
        deleteAllLoggers();
    }

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void _finalizeInside() override {}

    //==========================================================================
public:
    /**
     * @~japanese @p name で指定された名前を持つロガーを返す
     * @~english  Return logger with the name given by @p name
     */
    Logger* logger(const std::string& name);

    /**
     * @~japanese タイムステップ終了時にすべてのログをファイル出力する
     * @~english  Output all logs to files at the end of the time-step
     */
    void writeAllLogs();

    /**
     * @~japanese すべてのロガーを消去する
     * @~english  Delete all loggers
     */
    void deleteAllLoggers();

private:
    /**
     * @~japanese ロガーのコンテナ
     * @~english  Container of loggers
     */
    std::unordered_map<std::string, Logger*> _loggers;
};

#endif //__LOGGER_MANAGER_HPP__

/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file Logger.hpp
 */
#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__
#include <string>
#include <unordered_map>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

//######################################################################
/**
 * @~japanese
 * シミュレーション中のログを記録し1ステップごとに出力する
 *
 * @note
 * その場でファイルに出力すると，マルチスレッドで実行中は表示が乱れる．
 * これを防ぐために，マルチスレッドでの実行中はメモリにログを保存し，
 * ステップ終了時にソートして出力する．
 * 
 * @~english
 * Record log during simulation and output at each step
 *
 * @note
 * In the case to output logs to files on the fly, the display may be
 * corrupted when running with multithreading. To prevent this, the log
 * is saved in memory during multithread execution, and sorted and
 * output at the end of the step.
 *
 * @~ @ingroup IO
 */
class Logger
{
public:
    Logger() {};
    explicit Logger(const std::string& name);
    ~Logger();

    /**
     * @~japanese
     * キー @p key を指定して文字列 @p str を保存する
     *
     * キーが存在しない場合は新規作成し，既にキーが存在する場合は
     * 改行後に追記する
     * 
     * @~english
     * Record string @p str given key @p key
     *
     * If the key does not exist, create a new one. If the key
     * already exists, add it after a newline.
     */
    void saveLog(const std::string& key, const std::string& str);

    /**
     * @~japanese 保存内容をファイルに出力する
     * @~english  Output recorded contents to file
     */
    void writeLog();

    /**
     * @~japanese 保存内容を消去する
     * @~english  Clear recorded contents
     */
    void clearLog();

private:
    /**
     * @~japanese 出力ファイル名
     * @~english  Output file name
     */
    std::string _fileName;

    /**
     * @~japanese 記録内容 
     * @~english  Recorded contents
     */
    std::unordered_map<std::string, std::string> _logs;

#ifdef _OPENMP
    /**
     * @~japanese ロック変数
     * @~english  Lock variable
     */
    omp_lock_t _lock;
#endif //_OPENMP
};

#endif //__LOGGER_HPP__

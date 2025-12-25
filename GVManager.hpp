/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file GVManager.hpp
 */
#ifndef __GV_MANAGER_HPP__
#define __GV_MANAGER_HPP__
#include "Config.hpp"
#include "ManagerBase.hpp"
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

//##############################################################################
/**
 * @~japanese  シミュレーション内で大域的に利用する変数を管理する
 * @~english   Manage variables used globally within the simulation
 * @~ @ingroup Manager IO
 */
class GVManager : public ManagerBase
{
    friend class ManagerPool;

private:
    GVManager();
    ~GVManager() {};

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void _finalizeInside() override {}

    //==========================================================================
    /**
     * @~japanese @name 値を返す関数
     * @~english  @name Functions that return values
     */
    ///@{
public:
    /**
     * @~japanese @p key に関連付けられた文字列を返す
     * @~english  Return the string associated with @p key
     */
    std::string getString(const std::string& key) const
    {
        if (!_isStringKeyFound(key))
        {
            std::cerr << "ERROR: GVManager::getString - key[" << key
                      << "] not found." << std::endl;
            exit(EXIT_FAILURE);
        }
        return _strings.at(key);
    }

    /**
     * @~japanese @p keyに関連付けられたdouble型の数値を返す
     * @~english  Return the double-type value  associated with @p key
     */
    double getNumeric(const std::string& key) const
    {
        if (!_isNumericKeyFound(key))
        {
            std::cerr << "ERROR: GVManager::getNumeric - key[" << key
                      << "] not found." << std::endl;
            exit(EXIT_FAILURE);
        }
        return _numerics.at(key);
    }

    /**
     * @~japanese @p keyに関連付けられた真偽値を返す
     * @~english  Return the boolean  associated with @p key
     */
    bool getFlag(const std::string& key) const
    {
        if (!_isFlagKeyFound(key))
        {
            std::cerr << "ERROR: GVManager::getFlag - key[" << key
                      << "] not found." << std::endl;
            exit(EXIT_FAILURE);
        }
        return _flags.at(key);
    }

    /**
     * @~japanese _maxTime を返す 
     * @~english  Return _maxTime
     */
    ulint getMaxTime() const
    {
        return _maxTime;
    }

    ///@}

    //==========================================================================
    /**
     * @~japanese @name 値をセットする関数
     * @attention 既にセットされていたら上書きしない
     *
     * @~english @name Function to set values
     * @attention Not overwrite if already set
     */
    ///@{
public:
    /**
     * @~japanese @p keyに関連付けられた文字列として @p value を設定する
     * @~english  Set @p value as a string associated with @p key
     */
    bool setNewString(const std::string& key, const std::string& value);

    /**
     * @~japanese @p keyに関連付けられた数値として @p value を設定する
     * @~english  Set @p value as double-type number associated with @p key
     */
    bool setNewNumeric(const std::string& key, const double value);

    /**
     * @~japanese @p keyに関連付けられた数値として @p value を設定する
     * @note 内部でdoubleにキャスト
     *
     * @~english Set @p value as double-type number associated with @p key
     * @note Cast the value to double internally
     */
    bool setNewNumeric(const std::string& key, const int value);

    /**
     * @~japanese @p keyに関連付けられた真偽値として @p value を設定する
     * @~english  Set @p value as boolean associated with @p key
     */
    bool setNewFlag(const std::string& key, const bool value);

    ///@}

    //==========================================================================
    /**
     * @~japanese @name 値を再設定する関数
     * @attention 該当するキーが登録されていなければ新たに作成する
     *
     * @~english  @name Function to reset values
     * @attention If corresponding key is not registered, create new one
     */
    ///@{
public:
    /**
     * @~japanese @p keyに関連付けられた文字列として @p value を再設定する
     * @~english  Reset @p value as a string associated with @p key
     */
    bool resetString(const std::string& key, const std::string& value);

    /**
     * @~japanese @p keyに関連付けられた数値として @p value を再設定する
     * @~english  Reset @p value as double-type number associated with @p key
     */
    bool resetNumeric(const std::string& key, const double value);

    /**
     * @~japanese @p keyに関連付けられた数値として @p value を再設定する
     * @note 内部でdoubleにキャストする
     *
     * @~english  Reset @p value as double-type number associated with @p key
     * @note Cast the value to double internally
     */
    bool resetNumeric(const std::string& key, const int value);

    /**
     * @~japanese @p keyに関連付けられた真偽値として @p value を再設定する
     * @~english  Reset @p value as boolean associated with @p key
     */
    bool resetFlag(const std::string& key, const bool value);

    /**
     * @~japanese _maxTimeを @p maxTime に再設定する
     * @note _maxTimeは初期値0が与えられているため，set関数は不要
     *
     * @~english  Reset _maxTime back to @p maxTime
     * @note Since the initial value is given, the set function is unnecessary.
     */
    bool resetMaxTime(const unsigned long maxTime);

    ///@}

    //==========================================================================
    /**
     * @~japanese ファイルから変数を読み込む
     *
     * 1行につき1変数．形式は "key=value" とする．
     *
     * @~english  Read variables from file
     *
     * One variable per line. The format is "key=value".
     */
    bool setVariablesFromFile();

    /**
     * @~japanese
     * @p key に関連付けられたファイル名からデータディレクトリのパスを
     * 削除したものを戻す
     *
     * @note
     * basenameコマンドに似ているが，サブディレクトリを残す点が異なる．
     *
     * @~english
     * Return the file name associated with @p key with data directory
     * path removed
     *
     * @note
     * Similar to the basename command, except that subdirectories are
     * left behind.
     */
    std::string stripDataDir(const std::string& key) const;

    /**
     * @~japanese すべての変数を @p out に出力する
     * @~english  Output all variables to @p out
     */
    void print(std::ostream& out);

protected:
    /**
     * @~japanese 文字列として保持される変数
     * @~english  Variables held as strings
     */
    std::unordered_map<std::string, std::string> _strings;

    /**
     * @~japanese double型数値として保持される変数
     * @~english  Variables held as double-type numbers
     */
    std::unordered_map<std::string, double> _numerics;

    /**
     * @~japanese 真偽値として保持される変数
     * @~english  Variables held as boolean
     */
    std::unordered_map<std::string, bool> _flags;

    /**
     * @~japanese シミュレーション対象時間 [ms]
     * @~english  Simulation target time [ms]
     */
    unsigned long _maxTime;

private:
    /**
     * @~japanese @p key を持つ文字列が登録されているかどうかチェックする
     * @~english  Check if the string with @p key is registered
     */
    bool _isStringKeyFound(const std::string& key) const;

    /**
     * @~japanese @p key を持つ数値が登録されているかどうかチェックする
     * @~english  Check if the double-type value with @p key is registered
     */
    bool _isNumericKeyFound(const std::string& key) const;

    /**
     * @~japanese @p key を持つ真偽値が登録されているかどうかチェックする
     * @~english  Check if the boolean with @p key is registered
     */
    bool _isFlagKeyFound(const std::string& key) const;
};

using GV = GVManager;

#endif //__GV_MANAGER_HPP__

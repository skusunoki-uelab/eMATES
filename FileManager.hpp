/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file FileManager.hpp
 */
#ifndef __FILE_MANAGER_HPP__
#define __FILE_MANAGER_HPP__
#include "ManagerBase.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

//#############################################################################
/**
 * @~japanese シミュレーション内で常に利用するファイルを管理する
 * 
 * 計算の開始から終了まで開いておいて追記するファイルを対象とする
 *
 * @~english  Manage files always used in simulation
 *
 * Target files are ones to be opened from the start to the end of the
 * simulation and to be appended contents
 *
 * @~ @ingroup IO Manager
 */
class FileManager : public ManagerBase
{
    friend class ManagerPool;

private:
    FileManager();
    ~FileManager()
    {
        deleteAllOFStreams();
    }

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void _finalizeInside() override {};

    //==========================================================================
public:
    /**
     * @~japanese ファイル名が @p filename であるファイルのストリームを返す
     * @return 該当するファイルストリーム
     *
     * @~english  Return a stream of files whose filename is @p filename
     * @return Corresponding file stream
     */
    std::ofstream* getOFStream(const std::string& filename);

    /**
     * @~japanese すべてのファイルストリームを閉じる
     * @~english  Close all file streams
     */
    void deleteAllOFStreams();

private:
    /**
     * @~japanese
     * ファイル名と書き込み用ファイルオブジェクトを対応付けたコンテナ
     *
     * @~english
     * Container associating a file name and a file object for writing
     */
    std::unordered_map<std::string, std::ofstream*> _ofstreams;

    //==========================================================================
    /**
     * @~japanese @name 静的関数
     * @~english  @name Static functions
     */
    ///@{
public:
    /**
     * @~japanese 出力用ディレクトリを作成する
     *
     * @note
     * 1コマンドでシミュレーション（advmates-calcのみ対応）を繰り返すために作成．
     *
     * @~english  Prepare output directories
     *
     * @note
     * Implement to repeat simulation (applicable for only advmates-calc) with
     * one command.
     */
    static bool prepareResultDirectories();

    /**
     * @~japanese ディレクトリを階層的に作成する
     *
     * @p pathsに格納された名前のディレクトリを ./paths[0]/paths[1]/... の順に
     * 作成する．
     *
     * @~english  Create directories hierarchically
     *
     * Create directories with names stored in @p paths in the order of
     * ./paths[0]/paths[1]//....
     */
    static bool makeDirectories(std::vector<std::string>& paths);

    /**
     * @~japanese ファイル @p name が存在するかどうかを調べて戻す
     * @~english  Check if file @p name exists and return it
     */
    static bool exists(const std::string& name);
    ///@}
};

#endif //__FILE_MANAGER_HPP__

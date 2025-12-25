/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ManagerBase.hpp
 */
#ifndef __MANAGER_BASE_HPP__
#define __MANAGER_BASE_HPP__
#include <string>
#include <vector>

/**
 * @defgroup Manager
 * @~japanese
 * シミュレータ全般から参照される情報を管理する
 *
 * マネージャはManagerBaseクラスを継承し，すべてシングルトンとする．
 * 
 * @attention
 * 必ずfinalize()を持ち，ManagerBase.cppで定義されるグローバルコンテナに
 * インスタンスのポインタを登録する (ManagerBaseのコンストラクタにより
 * 自動登録される)．これにより，デストラクタを呼び出す前にそのクラスの
 * finalize()を ManagerFinalizer::finalizerAll() を通じて確実に呼び出す．
 *
 * @~english
 * Manage information referenced by simulator in general
 *
 * Managers inherit the Manager class and are all singletons.
 *
 * @attention
 * Always have finalize() and register the instance pointer in global
 * container defined in ManagerBase.cpp (Automatically registered in
 * ManagerBase constructor). This ensures that the class's finalize()
 * is called through ManagerFinalizer::finalizerAll() before being
 * called their destructors.
 *
 * @~
 * @see ManagerBase ManagerFinalizer
 */

//######################################################################
/**
 * @~japanese  マネージャの基底クラス
 * @~english   Base manager class
 * @~ @ingroup Manager
 */
class ManagerBase
{
    friend class ManagerPool;

protected:
    ManagerBase();
    virtual ~ManagerBase();

public:
    /**
     * @~japanese このマネージャに依存する別のマネージャ @p mng を追加
     * @~english  Add another manager @p mng depending on this manager
     */
    virtual void addDependee(ManagerBase* mng) final;

    /**
     * @~japanese 保持する _dependees への参照を返す
     * @~english  Return reference to _dependees it holds
     */
    const std::vector<ManagerBase*>& dependees() const
    {
        return _dependees;
    }

protected:
    /**
     * @~japanese 
     * このマネージャに依存するマネージャの集合
     *
     * @note
     * これに登録されたマネージャのfinalzie()を先に呼び出す
     *
     * @~english
     * Collections of managers depending on this manger
     *
     * @note
     * Call finalzie() of the manager registered with this first
     */
    std::vector<ManagerBase*> _dependees;

public:
    /**
     * @~japanese シミュレーション正常終了時の処理を定義
     * @~english  Define the process at the simulation normal end
     */
    virtual void finalize() final;

protected:
    /**
     * @~japanese
     * シミュレーション正常終了時の自身の処理の実体
     *
     * @~english
     * Substance to process this manager at the simulation normal end 
     */
    virtual void _finalizeInside() = 0;

private:
    /**
     * @~japanese 
     * finalize()が呼び出されたかどうかをあらわすフラグ
     *
     * @note
     * finalize()が複数回呼ばれることを防ぐ
     * 
     * @~english
     * Flag indicating whether finalize() was called
     *
     * @note
     * Prevent finalize() from being called multiple times 
     */
    bool _isFinalized;

protected:
    /**
     * @~japanese クラス名
     * @~english  Class name
     */
    std::string _className;

public:
    /**
     * @~japanese クラス名を返す 
     * @~english  Return class name
     */
    std::string className() const
    {
        return _className;
    }
};

//#define MANAGER_DEBUG

#endif //__MANAGER_BASE_HPP__

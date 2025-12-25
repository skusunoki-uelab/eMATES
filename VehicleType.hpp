/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTypeCategory.hpp
 */
#ifndef __VEHICLE_TYPE_HPP__
#define __VEHICLE_TYPE_HPP__
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

//######################################################################
/**
 * @~japanese 車両カテゴリ
 *
 * @attention
 * 新しい車両カテゴリを追加する場合は，VehicleType::isvalidCategory() も
 * かならず変更すること
 *
 * @~english  Vehicle category
 *
 * @attention
 * Be sure to change VehicleType::isValidCategory() , when add vehicle
 * categories,
 *
 * @~ @ingroup Vehicle
 */
enum class VehicleCategory : unsigned int
{
    DEFAULT        = 0,
    MINI_PASSENGER = 1,
    PASSENGER      = 2,
    BUS            = 3,
    MINI_TRUCK     = 4,
    TRUCK          = 5,
    EV             = 8, // [eMATES]
    TRAM           = 999,
};

//######################################################################
/**
 * @~japanese 車種
 *
 * 上位分類の車両カテゴリと下位分類のモデルIDにより構成される
 *
 * @note
 * 以前は車両カテゴリを表す数字とモデルIDを表す数字を結合した2桁以上の
 * 数字により車種を表していたが，新フォーマットでは"c-m"の形式で
 * 車両カテゴリ c とモデルID m の車種を表す．
 *
 * @~english  Vehicle type
 *
 * Configured by vehicle category of upper class and model ID of
 * lower class
 *
 * @note
 * Previously, vehicle type was represented by two or more digits
 * combining the number representing vehicle category and the number
 * representing the model ID. In the new format, the vehicle type of
 * vehicle category c and model ID m is represented as "c-m".
 */
struct VehicleType
{
public:
    /**
     * @~japanese 出力演算子のオーバーロード
     * @~english  Overloaded output operator
     */
    friend std::ostream& operator<<(std::ostream& out, const VehicleType& type)
    {
        out << type.toString();
        return out;
    }

    /**
     * @~japanese ソートに用いる比較演算子
     * @~english  Comparison operator for sorting
     */
    friend bool operator<(const VehicleType& lt, const VehicleType& rt)
    {
        return std::tie(lt._category, lt._modelId)
            < std::tie(rt._category, rt._modelId);
    }

    //============================================================================
    /**
     * @~japanese
     * VehicleCategoryにキャストされた数値の正当性をチェックする
     *
     * @~english
     * Check validity of the number cast to VehicleCategory
     */
    static bool isValidCategory(VehicleCategory& c)
    {
        switch (c)
        {
        case VehicleCategory::DEFAULT:
        case VehicleCategory::MINI_PASSENGER:
        case VehicleCategory::PASSENGER:
        case VehicleCategory::BUS:
        case VehicleCategory::MINI_TRUCK:
        case VehicleCategory::TRUCK:
        case VehicleCategory::EV: // [eMATES]
        case VehicleCategory::TRAM:
            return true;
        default:
            return false;
        }
    }

    //============================================================================
public:
    /**
     * @~japanese デフォルトコンストラクタ
     * @~english  Default constructor
     */
    VehicleType()
    {
        _category = VehicleCategory::DEFAULT;
        _modelId  = 0;
    }

    /**
     * @~japanese
     * 車両カテゴリ @p c とモデルID @p m を指定したコンストラクタ
     *
     * @~english
     * Constructor with vehicle category @p c and model ID @p m
     */
    VehicleType(VehicleCategory c, unsigned int m)
    {
        _category = c;
        _modelId  = m;
    }

    /**
     * @~japanese
     * 文字列 @p str から車両カテゴリとモデルIDを抽出するコンストラクタ
     *
     * @~english
     * Constructor extracting vehicle category and model ID from
     * given string @p str
     */
    VehicleType(const std::string& str)
    {
        if (str.find('-') == std::string::npos)
        {
            /*
             * 旧フォーマット
             *   下1桁がモデルID, それ以外が車両カテゴリ．ただし1桁で
             *   あっても0のみ許される．
             *
             * Old format
             *   The last digit indicates model ID, the others indicates
             *   vehicle category. However, only 0 is allowed even if
             *   it is a single digit.
             */
            VehicleCategory c;
            if (str == "0")
            {
                c = VehicleCategory::DEFAULT;
            }
            else
            {
                c = static_cast<VehicleCategory>(
                    std::stoi(str.substr(0, str.size() - 1)));
            }
            if (isValidCategory(c))
            {
                _category = c;
            }
            else
            {
                std::cerr << "WARNING: " << "invalid vehicle category - "
                          << str.substr(0, str.size() - 1) << std::endl;
                _category = VehicleCategory::PASSENGER;
            }
            _modelId = static_cast<unsigned int>(str[str.size() - 1] - '0');
        }
        else
        {
            /*
             * 新フォーマット
             * '-'の前が車両カテゴリ，'-'の後ろがモデルID
             *
             * New format
             * Before '-' indicates vehicle category, and after '-'
             * indicates model ID
             */
            std::vector<std::string> tokens;
            amu::string_operator::getTokens(&tokens, str, '-');
            if (tokens.size() != 2)
            {
                std::cerr << "WARNING: "
                          << "invalid (new style) vehicle type - " << str
                          << std::endl;
                _category = VehicleCategory::DEFAULT;
                _modelId  = 0;
            }
            VehicleCategory c = static_cast<VehicleCategory>(stoi(tokens[0]));
            if (isValidCategory(c))
            {
                _category = c;
            }
            else
            {
                std::cerr << "WARNING: " << "invalid vehicle category - "
                          << tokens[0] << std::endl;
                _category = VehicleCategory::PASSENGER;
            }
            _modelId = static_cast<unsigned int>(stoul(tokens[1]));
        }
    }

    /**
     * @~japanese コピーコンストラクタ
     * @~english  Copy constructor
     */
    VehicleType(const VehicleType& other)
    {
        _category = other._category;
        _modelId  = other._modelId;
    }

    //==========================================================================
    /**
     * @~japanese 文字列への変換
     * @~english  Convert to string
     */
    std::string toString() const
    {
        std::stringstream ss;
        ss << amu::converter::toUnderlying(_category) << "-" << _modelId;
        return ss.str();
    }

    //==========================================================================
    /**
     * @~japanese 代入演算子
     * @~english  Assignment operator
     */
    VehicleType operator=(const VehicleType& other)
    {
        if (this == &other)
        {
            std::cerr << "ERROR: assignment to self" << std::endl;
            exit(EXIT_FAILURE);
        }
        _category = other.category();
        _modelId  = other.modelId();
        return *this;
    }

    //==========================================================================
    /**
     * @~japanese @name 比較・等価演算子
     * @~english  @name Comparison and equality operators
     */
    ///@{
    bool operator<(const VehicleType& type)
    {
        return (*this < type);
    }

    bool operator==(const VehicleType& type) const
    {
        return (_category == type.category() && _modelId == type.modelId());
    }

    bool operator!=(const VehicleType& type) const
    {
        return !(*this == type);
    }

    ///@}

    //==========================================================================
private:
    /**
     * @~japanese 車両カテゴリ
     * @~english  Vehicle category
     */
    VehicleCategory _category;

    /**
     * @~japanese モデルID
     * @~english  Model Id
     */
    unsigned int _modelId;

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    VehicleCategory category() const
    {
        return _category;
    }

    unsigned int modelId() const
    {
        return _modelId;
    }

    ///@}
};

#endif //__VEHICLE_TYPE_CATEGORY_HPP__

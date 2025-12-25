/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file CustomMessage.hpp
 */
#ifndef __CUSTOM_MESSAGE_HPP__
#define __CUSTOM_MESSAGE_HPP__
#include <string>
#include <iostream>

/**
 * @~ Namespace for AdvMates utilities
 */
namespace amu
{
    /**
     * @~ Namespace for custom message in AdvMates 
     */
    namespace msg
    {
        /**
         * @~japanese セクションタイトル @p msg を @p out に出力する
         * @~english  Print section title @p msg to stdout
         */
        void title(std::ostream& out, const std::string& msg);

        /**
         * @~japanese ステータスメッセージ @p msg を @p out に出力する
         * @~english  Print status message @p msg to @p out
         */
        void status(std::ostream& out, const std::string& msg);

        /**
         * @~japanese 一般メッセージ @p msg を @p out に出力する
         * @~english  Print general message @p msg to @p out
         */
        void message(std::ostream& out, const std::string& msg);

        /**
         * @~japanese 警告メッセージ @p msg を標準エラー出力に出力する
         * @~english  Print warning message @p msg to stderr
         */
        void warn(const std::string& msg);

        /**
         * @~japanese エラーメッセージ @p msg を標準エラー出力に出力する
         * @~english  Print error message @p msg to stderr
         */
        void error(const std::string& msg);
    }
}

//##############################################################################
#ifndef NDEBUG
#include <cstdlib>
#include <cstring>

namespace custom_assert
{
    /**
     * @~japanese ASSERT_MSG用の戻り値つき::exit()
     * @~english  ::exit() with return value for ASSERT_MSG
     */
    int exit();
}

/**
 * @~japanese __FILE__がフルパスである場合にファイル名のみを抽出するマクロ
 * @note gcc(>=12)やclangにおける__FILE_NAME__の代替
 * 
 * @~english  Macro to extract only the file name if __FILE__ is full path
 * @note Alternative to __FILE_NAME__ in gcc(>=12) and clang
 */
#define _FILE_NAME \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

/**
 * @~japanese ファイル名と行数付きでassertionメッセージを出力するマクロ
 * @~english  Macro to output assertion message with file name and line number
 */
#define ASSERT_MSG(expr)                                                      \
    (!(expr)                                                                  \
     && printf(                                                               \
         "%s(%d)\033[1;31m [FAILED] \033[m" #expr "\n", _FILE_NAME, __LINE__) \
     && custom_assert::exit())

#else // if NDEBUG defined

#define ASSERT_MSG(expr)

#endif // NDEBUG

#endif //__CUSTOM__MESSAGE_HPP__

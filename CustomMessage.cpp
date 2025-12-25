/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file CustomMessage.cpp
 */
#include "CustomMessage.hpp"
#include "AppMates.hpp"
#include "GVManager.hpp"
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP
#include <AmuStringOperator.hpp>

using namespace std;
using namespace amu::string_operator;

#ifndef NDEBUG
//==============================================================================
int custom_assert::exit()
{
    ::exit(EXIT_FAILURE);
    return 1;
}
#endif // NDEBUG

//==============================================================================
namespace
{
    // 標準出力に色付けするための列挙子と関数
    // Enumerator and function for coloring standard output
    enum class ColorName
    {
        black  = 30,
        red    = 31,
        green  = 32,
        yellow = 33,
        blue   = 34,
        purple = 35,
        cyan   = 36,
        white  = 37,
    };
    string colorBold(const ColorName color, const string& text)
    {
        ostringstream oss;
        oss << "\033[1;" << static_cast<int>(color) << "m" << text << "\033[m";
        return oss.str();
    }
}

//==============================================================================
void amu::msg::title(ostream& out, const std::string& msg)
{
    if (!(AppMates::getGVManager().getFlag("FLAG_VERBOSE")))
    {
        return;
    }

    string cmsg(msg);
    getStrippedStringTail(&cmsg, '\n');
#pragma omp critical(out_critical)
    out << "*** " << cmsg << " ***" << endl;
}

//==============================================================================
void amu::msg::status(ostream& out, const std::string& msg)
{
    if (!(AppMates::getGVManager().getFlag("FLAG_VERBOSE")))
    {
        return;
    }

    string cmsg(msg);
    getStrippedStringTail(&cmsg, '\n');
#pragma omp critical(out_critical)
    out << "- " << cmsg << endl;
}

//==============================================================================
void amu::msg::message(ostream& out, const std::string& msg)
{
    if (!(AppMates::getGVManager().getFlag("FLAG_VERBOSE")))
    {
        return;
    }

    string cmsg(msg);
    getStrippedStringTail(&cmsg, '\n');
#pragma omp critical(out_critical)
    out << cmsg << endl;
}

//==============================================================================
void amu::msg::warn(const std::string& msg)
{
    if (!(AppMates::getGVManager().getFlag("FLAG_VERBOSE")))
    {
        return;
    }

    string cmsg(msg);
    getStrippedStringTail(&cmsg, '\n');
#pragma omp critical(out_critical)
    // 緑太字で [warning] を表示
    // Display [warning] in bold green text
    cerr << colorBold(ColorName::green, "[warning] ") << msg << endl;
}

//==============================================================================
void amu::msg::error(const std::string& msg)
{
    string cmsg(msg);
    getStrippedStringTail(&cmsg, '\n');
#pragma omp critical(out_critical)
    // 赤太字で [error] を表示
    // Display [error] in bold red text
    std::cerr << colorBold(ColorName::red, "[error] ") << msg << endl;
}

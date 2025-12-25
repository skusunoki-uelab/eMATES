/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiViewControl.cpp
 */
#include "GuiViewControl.hpp"
#include "Visualizer.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include <autogl.h>

using namespace std;

double GuiViewControl::_svpSize = 100;

double GuiViewControl::_svpPositionX = 0.0;
double GuiViewControl::_svpPositionY = 0.0;
double GuiViewControl::_svpPositionZ = 0.0;

double GuiViewControl::_svpDirectionX = 0.0;
double GuiViewControl::_svpDirectionY = 0.0;
double GuiViewControl::_svpDirectionZ = 1.0;

double GuiViewControl::_svpUpVectorX = 0.0;
double GuiViewControl::_svpUpVectorY = 1.0;
double GuiViewControl::_svpUpVectorZ = 0.0;

//======================================================================
GuiViewControl::GuiViewControl()
{
    GVManager& gv = AppMates::getGVManager();

    _svpSize = gv.getNumeric("VIS_VIEW_SIZE");

    _svpPositionX = gv.getNumeric("VIS_VIEW_CENTER_X");
    _svpPositionY = gv.getNumeric("VIS_VIEW_CENTER_Y");
    _svpPositionZ = gv.getNumeric("VIS_VIEW_CENTER_Z");

    _svpDirectionX = gv.getNumeric("VIS_VIEW_DIRECTION_X");
    _svpDirectionY = gv.getNumeric("VIS_VIEW_DIRECTION_Y");
    _svpDirectionZ = gv.getNumeric("VIS_VIEW_DIRECTION_Z");

    _svpUpVectorX = gv.getNumeric("VIS_VIEW_UPVECTOR_X");
    _svpUpVectorY = gv.getNumeric("VIS_VIEW_UPVECTOR_Y");
    _svpUpVectorZ = gv.getNumeric("VIS_VIEW_UPVECTOR_Z");
}

//======================================================================
void GuiViewControl::makePanel()
{
    AutoGL_AddGroup(" View Control ");

    // 移動・回転などの事前定義されたボタンを用意する
    // Prepare predefined buttons for translating, rotating, etc.
    AutoGL_SetPanelInMode3D();
    AutoGL_SetMode3D(AUTOGL_MODE_3D_TRANSLATE);

    AutoGL_AddComment();
    AutoGL_SetLabel("Viewing Parameters");
    AutoGL_AddCallback(
        printViewingParamsButtonCallback,
        "printViewingParamsButtonCallback");
    AutoGL_SetLabel("Print View Parameters");
    AutoGL_AddComment();
    AutoGL_SetLabel("Set Parameters");
    AutoGL_AddReal(&_svpSize, "_svpSize");
    AutoGL_SetLabel("ViewSize");
    AutoGL_AddReal(&_svpPositionX, "_svpPositionX");
    AutoGL_SetLabel("ViewCenter_X");
    AutoGL_AddReal(&_svpPositionY, "_svpPositionY");
    AutoGL_SetLabel("ViewCenter_Y");
    AutoGL_AddReal(&_svpPositionZ, "_svpPositionZ");
    AutoGL_SetLabel("ViewCenter_Z");
    AutoGL_AddReal(&_svpDirectionX, "_svpDirectionX");
    AutoGL_SetLabel("ViewDirection_X");
    AutoGL_AddReal(&_svpDirectionY, "_svpDirectionY");
    AutoGL_SetLabel("ViewDirection_Y");
    AutoGL_AddReal(&_svpDirectionZ, "_svpDirectionZ");
    AutoGL_SetLabel("ViewDirection_Z");
    AutoGL_AddReal(&_svpUpVectorX, "_svpUpVectorX");
    AutoGL_SetLabel("ViewUpVector_X");
    AutoGL_AddReal(&_svpUpVectorY, "_svpUpVectorY");
    AutoGL_SetLabel("ViewUpVector_Y");
    AutoGL_AddReal(&_svpUpVectorZ, "_svpUpVectorZ");
    AutoGL_SetLabel("ViewUpVector_Z");
    AutoGL_AddCallback(
        setViewingParamsButtonCallback,
        "setViewingParamsButtonCallback");
    AutoGL_SetLabel("Set View Parameters");

    AutoGL_AddComment();
    AutoGL_AddCallback(
        Visualizer::drawButtonCallback,
        "Visualizer::drawButtonCallback");
    AutoGL_SetLabel("Draw");
    AutoGL_AddCallback(
        Visualizer::quitButtonCallback,
        "Visualizer::quitButtonCallback");
    AutoGL_SetLabel("Quit");
}

//======================================================================
void GuiViewControl::printViewingParamsButtonCallback()
{
    double size;
    double cx, cy, cz;
    double dx, dy, dz;
    double ux, uy, uz;

    size = AutoGL_GetViewSize();
    AutoGL_GetViewCenter(&cx, &cy, &cz);
    AutoGL_GetViewDirection(&dx, &dy, &dz);
    AutoGL_GetViewUpVector(&ux, &uy, &uz);

    cout << "*** Viewing Params ***" << endl;
    cout << "ViewSize: " << size << endl;
    cout << "ViewCenter: (" << cx << ", " << cy << ", " << cz << ")"
         << endl;
    cout << "ViewDirection: (" << dx << ", " << dy << ", " << dz << ")"
         << endl;
    cout << "ViewUpVector: (" << ux << ", " << uy << ", " << uz << ")"
         << endl;
}

//======================================================================
void GuiViewControl::setViewingParamsButtonCallback()
{
    Visualizer::setViewSize(_svpSize);
    Visualizer::setViewPosition(
        _svpPositionX, _svpPositionY, _svpPositionZ);
    Visualizer::setViewDirection(
        _svpDirectionX, _svpDirectionY, _svpDirectionZ);
    Visualizer::setViewUpVector(
        _svpUpVectorX, _svpUpVectorY, _svpUpVectorZ);

    // 更新されたパラメータでビューを再描画する
    // Redraw the view with the updated parameters
    AutoGL_SetViewSize(_svpSize);
    AutoGL_SetViewCenter(_svpPositionX, _svpPositionY, _svpPositionZ);
    AutoGL_SetViewDirection(
        _svpDirectionX, _svpDirectionY, _svpDirectionZ);
    AutoGL_SetViewUpVector(_svpUpVectorX, _svpUpVectorY, _svpUpVectorY);

    Visualizer::renewFlags();
    Visualizer::clearTemporaryFlags();
    AutoGL_DrawView();
}

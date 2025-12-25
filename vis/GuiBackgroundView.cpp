/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiBackgroundView.cpp
 */
#include "GuiBackgroundView.hpp"
#include "GLColor.hpp"
#include "Visualizer.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include <cstring>
#include <autogl.h>

using namespace std;

#ifdef USE_OPENCV
BgImage GuiBackgroundView::_bgImage;
#endif //USE_OPENCV

double GuiBackgroundView::_bgXmin = -100.0;
double GuiBackgroundView::_bgXmax = 100.0;
double GuiBackgroundView::_bgYmin = -100.0;
double GuiBackgroundView::_bgYmax = 100.0;

char GuiBackgroundView::_fileName[64];
bool GuiBackgroundView::_showsBgImage;
int  GuiBackgroundView::_showsBgGrayscale;
int  GuiBackgroundView::_keepsAspectRatio;
char GuiBackgroundView::_creditString[128];

//======================================================================
GuiBackgroundView::GuiBackgroundView()
{
    const GVManager& gv = AppMates::getGVManager();

    _bgXmin = gv.getNumeric("VIS_BACKGROUND_TEXTURE_XMIN");
    _bgXmax = gv.getNumeric("VIS_BACKGROUND_TEXTURE_XMAX");
    _bgYmin = gv.getNumeric("VIS_BACKGROUND_TEXTURE_YMIN");
    _bgYmax = gv.getNumeric("VIS_BACKGROUND_TEXTURE_YMAX");

    strcpy(_fileName, gv.getString("VIS_BACKGROUND_TEXTURE_FILE").c_str());
    strcpy(_creditString, gv.getString("VIS_BACKGROUND_CREDIT_STRING").c_str());
}

//======================================================================
void GuiBackgroundView::makePanel()
{
#ifdef USE_OPENCV
    AutoGL_AddGroup(" Image Control ");
    AutoGL_AddComment();
    AutoGL_SetLabel(" Background Image ");
    AutoGL_AddString(_fileName, "_fileName", 64);
    AutoGL_SetLabel("Filename");

    AutoGL_AddReal(&_bgXmin, "_bgXmin");
    AutoGL_SetLabel("Xmin");
    AutoGL_AddReal(&_bgXmax, "_bgXmax");
    AutoGL_SetLabel("Xmax");
    AutoGL_AddReal(&_bgYmin, "_bgYmin");
    AutoGL_SetLabel("Ymin");
    AutoGL_AddReal(&_bgYmax, "_bgYmax");
    AutoGL_SetLabel("Ymax");
    AutoGL_AddBoolean(&_showsBgGrayscale, "_showsBgGrayscale");
    AutoGL_SetLabel("Grayscale");
    AutoGL_AddBoolean(&_keepsAspectRatio, "_keepsAspectRatio");
    AutoGL_SetLabel("Keep Aspect Ratio");

    AutoGL_AddString(_creditString, "_creditString", 128);
    AutoGL_SetLabel("Credit");

    AutoGL_AddCallback(
        updateBackgroundImageButtonCallback,
        "updateBackgroundImageButtonCallback");
    AutoGL_SetLabel("Update Background Image");
    AutoGL_AddCallback(
        clearBackgroundImageButtonCallback,
        "clearBackgroundImageButtonCallback");
    AutoGL_SetLabel("Clear Background Image");
#endif //USE_OPENCV
}

//======================================================================
void GuiBackgroundView::redrawView()
{
    if (!(AppMates::getGVManager().getFlag("VIS_SIMPLE_MAP"))
#ifdef USE_OPENCV
        && !_showsBgImage
#endif //USE_OPENCV
    )
    {
        /*
         * 背景画像を表示せず詳細な道路地図を表示する場合は地面を
         * 単色で描画する．
         *
         * Draw ground in a single color to show detail road map
         * without background image.
         */
        double xmin, xmax, ymin, ymax;
        Visualizer::getRegion(xmin, xmax, ymin, ymax);

        GLColor::setGround();
        AutoGL_DrawQuadrangle(
            xmin, ymin, -2.0, xmax, ymin, -2.0, xmax, ymax, -2.0, xmin, ymax,
            -2.0);
        return;
    }

    if (_showsBgImage)
    {
#ifdef USE_OPENCV
        AutoGL_BeginNativeCall();
        {
            glEnable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);

            glTexCoord2d(0.0, 1.0);
            glVertex3d(_bgXmin, _bgYmax, -10.0);

            glTexCoord2d(0.0, 0.0);
            glVertex3d(_bgXmin, _bgYmin, -10.0);

            glTexCoord2d(1.0, 0.0);
            glVertex3d(_bgXmax, _bgYmin, -10.0);

            glTexCoord2d(1, 1);
            glVertex3d(_bgXmax, _bgYmax, -10.0);

            glEnd();
            glDisable(GL_TEXTURE_2D);
        }
        AutoGL_EndNativeCall();

        // 著作権表記
        // Copyright notice
        if (strcmp(_creditString, "") != 0)
        {
            AutoGL_SetColor(0, 0, 0);
            AutoGL_DrawStringAtScaledPosition(0.01, 0.01, _creditString);
        }
#endif //USE_OPENCV
    }
}

//======================================================================
void GuiBackgroundView::updateBackgroundImageButtonCallback()
{
#ifdef USE_OPENCV
    string fname = AppMates::getGVManager().getString("VIS_TEXTURE_DIR");
    fname += _fileName;

    bool result = false;

    // 既に作成されたテクスチャがあればクリア
    // Clear any textures that have already been created
    _bgImage.clearImage();
    _showsBgImage = false;

    // 画像の読み込み
    // Load image
    result = _bgImage.loadImage(fname.c_str(), _showsBgGrayscale);
    if (!result)
    {
        return;
    }

    // テクスチャの用意
    // Prepare texture
    result = _bgImage.prepareTexture();
    if (!result)
    {
        return;
    }

    // 描画領域の設定
    // Set drawing area
    result = _bgImage.setRegion(
        _bgXmin, _bgXmax, _bgYmin, _bgYmax, _keepsAspectRatio);
    if (!result)
    {
        return;
    }

    _showsBgImage = true;
    AutoGL_DrawView();
#endif //USE_OPENCV
}

//======================================================================
void GuiBackgroundView::clearBackgroundImageButtonCallback()
{
#ifdef USE_OPENCV
    _showsBgImage = false;
    _bgImage.clearImage();
    Visualizer::clearTemporaryFlags();
    AutoGL_DrawView();
#endif //USE_OPENCV
}

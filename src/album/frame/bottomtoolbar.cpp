/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZhangYong <zhangyong@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "bottomtoolbar.h"
#include "application.h"

namespace {
const QColor DARK_COVERCOLOR = QColor(26, 26, 26, 204);
const QColor LIGHT_COVERCOLOR = QColor(255, 255, 255, 230);
const int BOTTOM_TOOLBAR_HEIGHT = 70;
const int BOTTOM_TOOLBAR_WIDTH_1 = 532;
//const int BOTTOM_TOOLBAR_WIDTH_2 = 782;
}

BottomToolbar::BottomToolbar(QWidget *parent)
    : DFloatingWidget(parent)
{
//    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    DWidget *dwidget = new DWidget(this);
    m_mainLayout = new QHBoxLayout(dwidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    setWidget(dwidget);
    setBlurBackgroundEnabled(true);
    setFixedWidth(BOTTOM_TOOLBAR_WIDTH_1);
    setFixedHeight(BOTTOM_TOOLBAR_HEIGHT);
}

void BottomToolbar::setContent(QWidget *content)
{
    QLayoutItem *child;
    while ((child = m_mainLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    m_mainLayout->addWidget(content);
}

void BottomToolbar::mouseMoveEvent(QMouseEvent *)
{
}

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
#include "viewpanel.h"
#include "navigationwidget.h"
#include "scen/imageview.h"
#include "dimagebutton.h"

#include "utils/baseutils.h"

#include <QTimer>
#include <DBlurEffectWidget>
#include <DFontSizeManager>
#include <DFloatingWidget>

DWIDGET_USE_NAMESPACE

void ViewPanel::initFloatingComponent()
{
//    initSwitchButtons();
    initScaleLabel();
#ifndef tablet_PC
    initNavigation();
#endif
}

void ViewPanel::initScaleLabel()
{
    using namespace utils::base;
    DAnchors<DFloatingWidget> scalePerc = new DFloatingWidget(this);
    scalePerc->setBlurBackgroundEnabled(true);

    QHBoxLayout *layout = new QHBoxLayout();
    scalePerc->setLayout(layout);
    QLabel *label = new QLabel();
    layout->addWidget(label);
    scalePerc->setAttribute(Qt::WA_TransparentForMouseEvents);
    scalePerc.setAnchor(Qt::AnchorHorizontalCenter, this, Qt::AnchorHorizontalCenter);
    scalePerc.setAnchor(Qt::AnchorBottom, this, Qt::AnchorBottom);
    scalePerc.setBottomMargin(75 + 14);
    label->setAlignment(Qt::AlignCenter);
//    scalePerc->setFixedSize(82, 48);
    scalePerc->setFixedWidth(90 + 10);
    scalePerc->setFixedHeight(40 + 10);
    scalePerc->adjustSize();
    label->setText("100%");
    DFontSizeManager::instance()->bind(label, DFontSizeManager::T6);
    scalePerc->hide();

    QTimer *hideT = new QTimer(this);
    hideT->setSingleShot(true);
    connect(hideT, &QTimer::timeout, scalePerc, &QLabel::hide);

    connect(m_viewB, &ImageView::scaled, this, [ = ](qreal perc) {
        int p = static_cast<int>(perc);
        label->setText(QString("%1%").arg(QString::number(p)));
        if (perc > 100) {
            emit dApp->signalM->enterScaledMode(true);
        } else {
            emit dApp->signalM->enterScaledMode(false);
        }
    });
    connect(m_viewB, &ImageView::showScaleLabel, this, [ = ]() {
        scalePerc->show();
        hideT->start(1000);
    });
}

void ViewPanel::initNavigation()
{
    m_nav = new NavigationWidget(this);
    m_nav.setBottomMargin(100);
    m_nav.setLeftMargin(10);
    m_nav.setAnchor(Qt::AnchorLeft, this, Qt::AnchorLeft);
    m_nav.setAnchor(Qt::AnchorBottom, this, Qt::AnchorBottom);

    connect(this, &ViewPanel::imageChanged, this, [ = ](const QString & path) {
        if (path.isEmpty()) m_nav->setVisible(false);
        m_nav->setImage(m_viewB->image());
    });
    connect(m_nav, &NavigationWidget::requestMove, [this](int x, int y) {
        m_viewB->centerOn(x, y);
    });
    connect(m_viewB, &ImageView::transformChanged, [this]() {
        m_nav->setVisible(! m_nav->isAlwaysHidden() && ! m_viewB->isWholeImageVisible());
        m_nav->setRectInImage(m_viewB->visibleImageRect());
    });
    connect(m_viewB, &ImageView::hideNavigation, this, [ = ]() {
        m_nav->setVisible(false);
    });

}

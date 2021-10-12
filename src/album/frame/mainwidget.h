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

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "bottomtoolbar.h"
#include "extensionpanel.h"
#include "toptoolbar.h"
#include "controller/signalmanager.h"
#include "module/view/viewpanel.h"

#include <QFrame>
#include <QStackedWidget>

class MainWidget : public QFrame
{
    Q_OBJECT
public:
    MainWidget(bool manager, QWidget *parent = nullptr);
    ~MainWidget() override;

protected:
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onGotoPanel(ModulePanel *panel);
    void onShowImageInfo(const QString &path);
    void onBackToMainPanel();
    void onActiveWindow();
    void onShowInFileManager(const QString &path);
    void onMouseMove(bool show);
    void onShowFullScreen();
    void onUpdateBottomToolbar(bool wideMode);
    void onUpdateBottomToolbarContent(QWidget *c, bool wideMode);
    void onShowBottomToolbar();
    void onHideBottomToolbar(bool immediately);
    void onUpdateExtensionPanelContent(QWidget *c);
    void onShowExtensionPanel();

private:
    void initBottomToolbar();
    void initExtensionPanel();
    void initTopToolbar();
    void initConnection();
    void initPanelStack(bool manager);

private:
    QStringList m_infoShowingList;
    QStackedWidget  *m_panelStack;

#ifndef LITE_DIV
    bool m_manager;
#endif
    ExtensionPanel  *m_extensionPanel;
    BottomToolbar   *m_bottomToolbar;
    TopToolbar      *m_topToolbar;
    QLabel          *m_topSeparatorLine;
    QLabel          *m_btmSeparatorLine;
    ViewPanel       *m_viewPanel;
};

#endif // MAINWIDGET_H

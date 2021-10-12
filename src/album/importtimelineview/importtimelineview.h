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
#ifndef IMPORTTIMELINEVIEW_H
#define IMPORTTIMELINEVIEW_H

#include "application.h"
#include "controller/signalmanager.h"
#include "dbmanager/dbmanager.h"
#include "thumbnail/thumbnaillistview.h"
#include "widgets/timelinelist.h"
#include "widgets/timelineitem.h"
#include "importview/importview.h"
#include "widgets/statusbar.h"
#include "allpicview/allpicview.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <DListWidget>
#include <DCommandLinkButton>
#include <DApplicationHelper>
#include <QGraphicsOpacityEffect>

class ImportTimeLineView : public DWidget, public ImageEngineImportObject
{
    Q_OBJECT
public:
    explicit ImportTimeLineView(DWidget *parent);
    ~ImportTimeLineView() override
    {
        void clearAndStop();
    }

    bool imageImported(bool success) override
    {
        Q_UNUSED(success);
        emit dApp->signalM->closeWaitDialog();
        return true;
    }
    void updateStackedWidget();
    int getIBaseHeight();
    int getGoToHeight();

signals:
    void sigResizeTimelineBlock();

public slots:
    void getCurrentSelectPics();
    void on_AddLabel(QString date, QString num);
//    void on_DelLabel();//未使用
#if 1
    void on_MoveLabel(int y, const QString &date, const QString &num, const QString &choseText);
#endif
    void on_KeyEvent(int key);

protected:
    void resizeEvent(QResizeEvent *ev) override;
    void showEvent(QShowEvent *ev) override;

private:
    void initTimeLineViewWidget();
    void initConnections();
    void sigImprotPicsIntoThumbnailView();
    void getImageInfos();
    void initMainStackWidget();

    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

public:
    void clearAndStartLayout();
    void addTimelineLayout();
    void getFatherStatusBar(DSlider *s);
    void themeChangeSlot(DGuiApplicationHelper::ColorType themeType);
    void resizeHand();  //手动计算大小
    ThumbnailListView *getFirstListView();
    //tab进入时清除其他所有选中
    void clearAllSelection();
#if 1
    QStringList selectPaths();
    void updateChoseText();
#endif
private slots:
    /**
     * @brief updateSize
     * 调整已导入界面的整体大小
     */
    void updateSize();
    void onNewTime(const QString &date, const QString &num, int index);
    void onRepeatImportingTheSamePhotos(QStringList importPaths, QStringList duplicatePaths, const QString &albumName);
    void onSuspensionChoseBtnClicked();

signals:
    void sigUpdatePicNum();

private:
    void clearAndStop();
    QLayout *m_mainLayout;
    QList<QString> m_timelines;
    DWidget *m_dateItem;
    DWidget *m_TitleItem;                               //title包裹窗口
    DCommandLinkButton *pSuspensionChose;               //悬浮选择按钮
    DWidget *pTimeLineViewWidget;
    ImportView *pImportView;
    DLabel *m_pDate;                                    //已导入悬浮日期
    DLabel *pNum_up;                                    //已导入悬浮数量
    DLabel *pNum_dn;                                    //已导入其他时间线数量
    DLabel *m_pImportTitle;                             //已导入悬浮标题
    DSlider *m_DSlider;
    QList<ThumbnailListView *> m_allThumbnailListView;
    QList<DCommandLinkButton *> m_allChoseButton;
    QGraphicsOpacityEffect *m_oe;
    QGraphicsOpacityEffect *m_oet;

    bool m_ctrlPress;
    int lastClickedIndex;
    int lastRow;
    int m_lastShiftRow;
    int m_lastShiftClickedIndex;
    bool lastChanged;
    bool m_bshow = false;

public:
    int m_index;
    TimelineListWidget *m_mainListWidget;
    int currentTimeLineLoad;
    QString selectPrePaths = "";//跳转的上一图片位置
    int hasPicViewNum = -1;//包含跳转图片的view索引
    bool isFindPic = false;
};

#endif // IMPORTTIMELINEVIEW_H

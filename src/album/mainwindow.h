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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "albumview/albumview.h"
#include "allpicview/allpicview.h"
#include "timelineview/timelineview.h"
#include "dbmanager/dbmanager.h"
#include "searchview/searchview.h"
#include "controller/commandline.h"
#include "controller/exporter.h"
#include "controller/dbusclient.h"
#include "widgets/dialogs/imginfodialog.h"
#include "module/slideshow/slideshowpanel.h"
#include "fileinotify.h"

#include <DMainWindow>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
//#include <DSegmentedControl>
#include <DTitlebar>
#include <QStackedWidget>
#include <DSearchEdit>
#include <DLabel>
#include <QStatusBar>
#include <DTabBar>
#include <QButtonGroup>
#include <DSuggestButton>
#include <DProgressBar>

#define DEFAULT_WINDOWS_WIDTH   1300
#define DEFAULT_WINDOWS_HEIGHT  640
#define MIX_WINDOWS_WIDTH       1300
#define MIX_WINDOWS_HEIGHT      848
//#define TITLEBAR_BLANK_WIDTH    235
#define TITLEBAR_BLANK_WIDTH 365
#define TITLEBAR_BTNWIDGET_WIDTH 240
//#define TITLEBAR_BTNWIDGET_WIDTH 280
#define TITLEBAR_ICON_WIDTH 50

DWIDGET_USE_NAMESPACE

extern bool bfirstopen;
extern bool bfirstandviewimage;
//namespace Ui {
//class MainWindow;
//}
class MainWindow : public DMainWindow, public ImageEngineImportObject
{
    Q_OBJECT

public:
    //???????????????????????????closeFromMenu??????
    static MainWindow &instance()
    {
        static MainWindow w;
        return w;
    }

    ~MainWindow() override;

    bool imageImported(bool success) override;
    void initConnections();
    void initDBus() ;//lmh0407
    //?????????UI
    void initUI();
    void initWaitDialog();
    void initShortcut();
    void initShortcutKey();
    void initTitleBar();
    void initCentralWidget();
    // install filter
    void initInstallFilter();
    // ??????????????????,??????????????????allpicview tab????????????
    void initNoPhotoNormalTabOrder();
    // ?????????????????????allpicview tab tab????????????
    void initAllpicViewTabOrder();
    // ?????????????????????allpicview tab tab????????????
    void initTimeLineViewTabOrder();
    // ?????????????????????AlbumView tab tab????????????
    void initAlbumViewTabOrder();
    // enter key
    void initEnterkeyAction(QObject *obj);
    // right key
    bool initRightKeyOrder(QObject *obj);
    // left key
    bool initLeftKeyOrder(QObject *obj);
    // down key
    bool initDownKeyOrder();
    // up key
    bool initUpKeyOrder();
    // all view tab key order
    bool initAllViewTabKeyOrder(QObject *obj);

    void setWaitDialogColor();
    int getCurrentViewType();
    void showCreateDialog(QStringList imgpaths);
    void onShowImageInfo(const QString &path);
    void loadZoomRatio();
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    QJsonObject createShorcutJson();
    //????????????
    void startMonitor();
private:
    explicit MainWindow(); //?????????????????????????????????????????????
    void thumbnailZoomIn();
    void thumbnailZoomOut();
    void saveWindowState();
    void loadWindowState();
    void saveZoomRatio();
    bool compareVersion();
//    void viewImageClose();
    void floatMessage(const QString &str, const QIcon &icon);
protected:
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

signals:
    void sigTitleMenuImportClicked();
    bool sigImageImported(bool success);

public slots:
    void allPicBtnClicked();
    void timeLineBtnClicked();
    void albumBtnClicked();
    void onCreateAlbum(QStringList imagepaths);
    void closeFromMenu();
    void onButtonClicked(int id);
    void onImageImported(bool success);
    void onSearchEditTextChanged(QString text);
    void onImagesInserted();
    void onStartImprot();
    void onProgressOfWaitDialog(int allfiles, int completefiles);
    void onPopupWaitDialog(QString waittext, bool bneedprogress);
    void onCloseWaitDialog();
    void onImagesRemoved();
    void onHideImageView();
    void onShowSlidePanel(int index);
    void onHideSlidePanel();
    void onExportImage(QStringList paths);
    void onMainwindowSliderValueChg(int step);
    void onAlbDelToast(QString str1);
    void onAddDuplicatePhotos();
    void onAddToAlbToast(QString album);
    void onImportSuccess();
    void onSearchEditClear();
    void onImportFailed();
    void onImportSomeFailed(int successful, int failed);
    void onImgExportFailed();
    void onImgExportSuccess();
    void onAlbExportFailed();
    void onAlbExportSuccess();
    void onEscShortcutActivated();
    void onDelShortcutActivated();
    void onKeyF2ShortcutActivated();
    void onCtrlShiftSlashShortcutActivated();
    void onCtrlFShortcutActivated();
    void onShowImageView(int index);
    void onSearchEditIsDisplay(bool bIsDisp);


#if 1
    void onViewCreateAlbum(QString imgpath, bool bmodel = true);
#endif
    void onSearchEditFinished();
    void onTitleBarMenuClicked(QAction *action);
    void onImprotBtnClicked();
//    void onUpdateCentralWidget();
    void onNewAPPOpen(qint64 pid, const QStringList &arguments);
    void onLoadingFinished();
private:
    int m_iCurrentView;
    bool m_bTitleMenuImportClicked;
    bool m_bImport = false;
    QWidget *m_titleBtnWidget;
    DMenu *m_pTitleBarMenu;
    QStackedWidget *m_pCenterWidget = nullptr;
public:
    AlbumView *m_pAlbumview;                    //????????????????????????
    QWidget *m_pAlbumWidget;
    AllPicView *m_pAllPicView;                  //????????????????????????
    TimeLineView *m_pTimeLineView;              //?????????????????????
    QWidget *m_pTimeLineWidget;
    SearchView *m_pSearchView;                  //??????????????????
    QWidget *m_pSearchViewWidget = nullptr;
    SlideShowPanel *m_slidePanel;               //?????????????????????
    DSearchEdit *m_pSearchEdit;
    CommandLine *m_commandLine;
private:
    DBManager *m_pDBManager;
    QMap<QString, ImgInfoDialog *> m_propertyDialogs{};
    int m_backIndex;
    int m_backIndex_fromSlide;
    int m_pSliderPos;       //???????????????

    QButtonGroup *btnGroup;
    DPushButton *m_pAllPicBtn;
    DPushButton *m_pTimeBtn;
    DPushButton *m_pAlbumBtn;

    DDialog  *m_waitdailog;
    DProgressBar *m_importBar;
    DLabel *m_waitlabel;
    DLabel *m_countLabel;

    QString       m_SearchKey;      //?????????????????????

    dbusclient *m_pDBus;//LMH0407DBus
    bool m_isFirstStart = true;
    bool m_processOptionIsEmpty = false;
    QSettings *m_settings;
    // ??????????????????????????????taborder
    QList<QWidget *> m_emptyAllViewTabOrder;
    // ??????????????????????????????
    QList<QWidget *> m_AllpicViewTabOrder;
    // ???????????????????????????
    QList<QWidget *> m_TimelineViewTabOrder;
    // ????????????????????????
    QList<QWidget *> m_AlbumViewTabOrder;
    // isfirst init
    QVector<bool> m_bVector;
    FileInotify *m_fileInotify;//?????????????????????

public:
    QButtonGroup *getButG();
};

#endif // MAINWINDOW_H

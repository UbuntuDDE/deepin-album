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
#include "allpicview.h"
#include <QMimeData>
#include "imageengine/imageengineapi.h"
#include "mainwindow.h"
#include <dgiovolumemanager.h>
#include <dgiofile.h>
#include <dgiofileinfo.h>
#include <dgiovolume.h>
#include "ac-desktop-define.h"

namespace {
struct MetaData {
    QString key;
    const char *name;
};

static MetaData MetaDataBasics[] = {
    {"FileName",            QT_TRANSLATE_NOOP("MetadataName", "Photo name")},
    {"FileFormat",          QT_TRANSLATE_NOOP("MetadataName", "Type")},
    {"FileSize",            QT_TRANSLATE_NOOP("MetadataName", "File size")},
    {"Dimension",           QT_TRANSLATE_NOOP("MetadataName", "Dimensions")},
    {"DateTimeOriginal",    QT_TRANSLATE_NOOP("MetadataName", "Date captured")},
    {"DateTimeDigitized",   QT_TRANSLATE_NOOP("MetadataName", "Date modified")},
    {"Tag",                 QT_TRANSLATE_NOOP("MetadataName", "Tag")},
    {"", ""}
};
};

namespace {
const int VIEW_IMPORT = 0;
const int VIEW_ALLPICS = 1;
const int VIEW_SEARCH = 2;
const int VIEW_MAINWINDOW_ALLPIC = 0;
}  //namespace

AllPicView::AllPicView()
    : m_pStackedWidget(nullptr), m_pStatusBar(nullptr), m_pwidget(nullptr)
    , m_pImportView(nullptr), step(0), m_pThumbnailListView(nullptr)
    , m_pSearchView(nullptr), /*m_spinner(nullptr),*/ fatherwidget(nullptr)
{
    setAcceptDrops(true);
    fatherwidget = new DWidget(this);
    fatherwidget->setFixedSize(this->size());
    m_pStackedWidget = new DStackedWidget(this);
    AC_SET_OBJECT_NAME(m_pStackedWidget, All_Picture_StackedWidget);
    AC_SET_ACCESSIBLE_NAME(m_pStackedWidget, All_Picture_StackedWidget);
    m_pImportView = new ImportView();
    m_pThumbnailListView = new ThumbnailListView(ThumbnailDelegate::AllPicViewType);
    AC_SET_OBJECT_NAME(m_pThumbnailListView, All_Picture_Thembnail);
    AC_SET_ACCESSIBLE_NAME(m_pThumbnailListView, All_Picture_Thembnail);
    DWidget *pThumbnailListView = new DWidget();
    QLayout *m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(8, 0, 0, 27);
    m_mainLayout->addWidget(m_pThumbnailListView);
    pThumbnailListView->setLayout(m_mainLayout);
    m_pSearchView = new SearchView();
    m_pStackedWidget->addWidget(m_pImportView);
    m_pStackedWidget->setCurrentIndex(VIEW_IMPORT);
    m_pStackedWidget->addWidget(pThumbnailListView);
    m_pStackedWidget->addWidget(m_pSearchView);
    m_pStackedWidget->setCurrentIndex(VIEW_ALLPICS);
    m_pStatusBar = new StatusBar(this);
    m_pStatusBar->raise();
    m_pStatusBar->setFixedWidth(this->width());
    m_pStatusBar->move(0, this->height() - m_pStatusBar->height());
    QVBoxLayout *pVBoxLayout = new QVBoxLayout();
    pVBoxLayout->setContentsMargins(2, 0, 0, 0);
    pVBoxLayout->addWidget(m_pStackedWidget);
    fatherwidget->setLayout(pVBoxLayout);
    initConnections();
//    m_spinner = new DSpinner(this);
//    m_spinner->setFixedSize(40, 40);
//    m_spinner->hide();
    connect(m_pThumbnailListView, &ThumbnailListView::sigLoad80ThumbnailsFinish, this, &AllPicView::updatePicsIntoThumbnailViewWithCache);
    connect(m_pThumbnailListView, &ThumbnailListView::sigDBImageLoaded, this, &AllPicView::updateStackedWidget);
    m_pwidget = new QWidget(this);
    m_pwidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    if (ImageEngineApi::instance()->m_AllImageData.size() > 0) {
        m_pThumbnailListView->slotLoad80ThumbnailsFinish();
    } else {
        if (ImageEngineApi::instance()->m_80isLoaded) {
            //相册为空的情况。
            m_pThumbnailListView->slotLoad80ThumbnailsFinish();
        } else {
            connect(ImageEngineApi::instance(), &ImageEngineApi::sigLoad80ThumbnailsToView,
                    m_pThumbnailListView, &ThumbnailListView::slotLoad80ThumbnailsFinish);
        }
    }
    AC_SET_OBJECT_NAME(this, All_Picture_View);
    AC_SET_ACCESSIBLE_NAME(this, All_Picture_View);
}

void AllPicView::initConnections()
{
    qRegisterMetaType<DBImgInfoList>("DBImgInfoList &");
    connect(dApp->signalM, &SignalManager::imagesInserted, this, &AllPicView::updatePicsIntoThumbnailView);
    connect(dApp->signalM, &SignalManager::imagesRemoved, this, &AllPicView::updatePicsIntoThumbnailView);
    connect(dApp, &Application::sigFinishLoad, this, &AllPicView::onFinishLoad);
    // 添加重复照片提示
    connect(dApp->signalM, &SignalManager::RepeatImportingTheSamePhotos, this, &AllPicView::onRepeatImportingTheSamePhotos);
    connect(m_pThumbnailListView, &ThumbnailListView::openImage, this, &AllPicView::onOpenImage);
    connect(m_pThumbnailListView, &ThumbnailListView::menuOpenImage, this, &AllPicView::onMenuOpenImage);
    connect(dApp->signalM, &SignalManager::sigUpdateImageLoader, this, &AllPicView::updatePicsThumbnailView);
    connect(m_pStatusBar->m_pSlider, &DSlider::valueChanged, dApp->signalM, &SignalManager::sigMainwindowSliderValueChg);
    connect(m_pThumbnailListView, &ThumbnailListView::sigMouseRelease, this, &AllPicView::updatePicNum);
    connect(m_pThumbnailListView, &ThumbnailListView::customContextMenuRequested, this, &AllPicView::updatePicNum);
    connect(m_pSearchView->m_pThumbnailListView, &ThumbnailListView::sigMouseRelease, this, &AllPicView::updatePicNum);
    connect(m_pSearchView->m_pThumbnailListView, &ThumbnailListView::customContextMenuRequested, this, &AllPicView::updatePicNum);
    connect(m_pImportView->m_pImportBtn, &DPushButton::clicked, this, &AllPicView::onImportViewImportBtnClicked);
    connect(dApp->signalM, &SignalManager::sigImportFailedToView, this, &AllPicView::onImportFailedToView);
    connect(m_pThumbnailListView, &ThumbnailListView::sigSelectAll, this, &AllPicView::updatePicNum);
    connect(dApp->signalM, &SignalManager::sigShortcutKeyDelete, this, &AllPicView::onKeyDelete);
    connect(dApp->signalM, &SignalManager::sigMonitorChanged, this, &AllPicView::monitorHaveNewFile);
}

void AllPicView::updateStackedWidget()
{
    if (0 < DBManager::instance()->getImgsCount()) {
        m_pStackedWidget->setCurrentIndex(VIEW_ALLPICS);
        m_pStatusBar->setVisible(true);
    } else {
        m_pStackedWidget->setCurrentIndex(VIEW_IMPORT);
        m_pStatusBar->setVisible(false);
    }
    updatePicNum();
}

void AllPicView::monitorHaveNewFile(QStringList list)
{
    ImageEngineApi::instance()->ImportImagesFromFileList(list, "", this, true);
}

void AllPicView::updatePicsIntoThumbnailView()
{
//    m_spinner->hide();
//    m_spinner->stop();
    m_pThumbnailListView->stopLoadAndClear();
    m_pThumbnailListView->loadFilesFromDB("NOCache");
    if (VIEW_SEARCH == m_pStackedWidget->currentIndex()) {
        //donothing
    } else {
        updateStackedWidget();
    }
    restorePicNum();
}

void AllPicView::updatePicsIntoThumbnailViewWithCache()
{
//    m_spinner->hide();
//    m_spinner->stop();
    m_pThumbnailListView->stopLoadAndClear(false);
    m_pThumbnailListView->loadFilesFromDB();
    if (VIEW_SEARCH == m_pStackedWidget->currentIndex()) {
        //donothing
    } else {
        updateStackedWidget();
    }
    restorePicNum();
}

void AllPicView::onFinishLoad()
{
    m_pThumbnailListView->update();
}

void AllPicView::onRepeatImportingTheSamePhotos(QStringList importPaths, QStringList duplicatePaths, const QString &albumName)
{
    Q_UNUSED(importPaths)
    if (albumName.length() == 0 && dApp->getMainWindow()->getCurrentViewType() == 0) {
        m_pThumbnailListView->selectDuplicatePhotos(duplicatePaths);
    }
}

void AllPicView::onOpenImage(int index)
{
    SignalManager::ViewInfo info;
    info.album = "";
    info.lastPanel = nullptr;
    auto imagelist = m_pThumbnailListView->getAllFileList();
    if (imagelist.size() > 0) {
        info.paths << imagelist;
        info.path = imagelist[index];
    } else {
        info.paths.clear();
    }
    info.viewType = utils::common::VIEW_ALLPIC_SRN;
    info.viewMainWindowID = VIEW_MAINWINDOW_ALLPIC;

    emit dApp->signalM->viewImage(info);
    emit dApp->signalM->showImageView(VIEW_MAINWINDOW_ALLPIC);
}

void AllPicView::onMenuOpenImage(const QString &path, QStringList paths, bool isFullScreen, bool isSlideShow)
{
    SignalManager::ViewInfo info;
    info.album = "";
    info.lastPanel = nullptr;
    auto imagelist = m_pThumbnailListView->getAllFileList();
    if (paths.size() > 1) {
        info.paths = paths;
    } else {
        if (imagelist.size() > 0) {
            info.paths << imagelist;
        } else {
            info.paths.clear();
        }
    }
    info.path = path;
    info.fullScreen = isFullScreen;
    info.slideShow = isSlideShow;
    info.viewType = utils::common::VIEW_ALLPIC_SRN;
    info.viewMainWindowID = VIEW_MAINWINDOW_ALLPIC;
    if (info.slideShow) {
        //lmh0427幻灯片播放选中地址
        if (paths.count() == 1) {
            info.paths = imagelist;
        } else {
            info.paths = paths;
        }
        emit dApp->signalM->startSlideShow(info);
        emit dApp->signalM->showSlidePanel(VIEW_MAINWINDOW_ALLPIC);
    } else {
        emit dApp->signalM->viewImage(info);
        emit dApp->signalM->showImageView(VIEW_MAINWINDOW_ALLPIC);
    }
}

void AllPicView::onImportViewImportBtnClicked()
{
    m_pStackedWidget->setCurrentIndex(VIEW_ALLPICS);
    emit dApp->signalM->startImprot();
    m_pImportView->onImprotBtnClicked();
}

void AllPicView::onImportFailedToView()
{
    if (isVisible()) {
//        m_spinner->hide();
//        m_spinner->stop();
        updateStackedWidget();
    }
}

ThumbnailListView *AllPicView::getThumbnailListView()
{
    return m_pThumbnailListView;
}

void AllPicView::updatePicsThumbnailView(QStringList strpath)
{
    if (strpath.empty()) {
        updatePicsIntoThumbnailViewWithCache();
    } else {
        updatePicsIntoThumbnailView();
    }
}

void AllPicView::dragEnterEvent(QDragEnterEvent *e)
{
    const QMimeData *mimeData = e->mimeData();
    if (!utils::base::checkMimeData(mimeData)) {
        return;
    }
    e->setDropAction(Qt::CopyAction);
    e->accept();
}

void AllPicView::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }
    ImageEngineApi::instance()->ImportImagesFromUrlList(urls, nullptr, this);
    event->accept();
}

void AllPicView::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void AllPicView::dragLeaveEvent(QDragLeaveEvent *e)
{
    Q_UNUSED(e);
}

void AllPicView::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
//    m_spinner->move(width() / 2 - 20, (height() - 50) / 2 - 20);
    m_pwidget->setFixedHeight(this->height() - 23);
    m_pwidget->setFixedWidth(this->width());
    m_pwidget->move(0, 0);
    m_pStatusBar->setFixedWidth(this->width());
    m_pStatusBar->move(0, this->height() - m_pStatusBar->height());
    fatherwidget->setFixedSize(this->size());
}

void AllPicView::updatePicNum()
{
    QString str = tr("%1 photo(s) selected");
    int selPicNum = 0;
    if (VIEW_ALLPICS == m_pStackedWidget->currentIndex()) {
        QStringList paths = m_pThumbnailListView->selectedPaths();
        selPicNum = paths.length();
    } else if (VIEW_SEARCH == m_pStackedWidget->currentIndex()) {
        QStringList paths = m_pSearchView->m_pThumbnailListView->selectedPaths();
        selPicNum = paths.length();
    }
    if (0 < selPicNum) {
        m_pStatusBar->m_pAllPicNumLabel->setText(str.arg(QString::number(selPicNum)));
    } else {
        restorePicNum();
    }
}

const ThumbnailListView *AllPicView::getAllPicThumbnailListViewModel()
{
    return m_pThumbnailListView;
}

void AllPicView::restorePicNum()
{
    QString str = tr("%1 photo(s)");
    int selPicNum = 0;
    if (VIEW_ALLPICS == m_pStackedWidget->currentIndex()) {
        selPicNum = DBManager::instance()->getImgsCount();
    } else if (VIEW_SEARCH == m_pStackedWidget->currentIndex()) {
        selPicNum = m_pSearchView->m_searchPicNum;
    }
    m_pStatusBar->m_pAllPicNumLabel->setText(str.arg(QString::number(selPicNum)));
}

void AllPicView::onKeyDelete()
{
    if (!isVisible()) return;
    if (VIEW_SEARCH == m_pStackedWidget->currentIndex()) return;
    QStringList paths;
    paths.clear();
    paths = m_pThumbnailListView->selectedPaths();
    if (0 >= paths.length()) {
        return;
    }
    m_pThumbnailListView->setCurrentSelectPath();
    ImageEngineApi::instance()->moveImagesToTrash(paths);
}

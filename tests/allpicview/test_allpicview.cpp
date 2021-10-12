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
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTestEventList>

#define private public
#define protected public

#include "application.h"
#include "mainwindow.h"
#include "allpicview.h"
#include "imgdeletedialog.h"
#include "wallpapersetter.h"
#include "ttbcontent.h"
#include "viewpanel.h"
#include "mainwidget.h"
#include "signalmanager.h"
#include "../test_qtestDefine.h"
#include "ac-desktop-define.h"


TEST(allpicview, test_ini)
{
    TEST_CASE_NAME("test_ini")
    MainWindow *w = dApp->getMainWindow();

    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos);
    event.simulate(w->getButG()->button(0));
    AllPicView *a = w->m_pAllPicView;

    int width = a->m_pStatusBar->m_pSlider->slider()->width();
    pos = QPoint(width / 10 * a->m_pStatusBar->m_pSlider->slider()->sliderPosition(), 10);
    event.addMousePress(Qt::LeftButton, Qt::NoModifier, pos);
    pos = pos - QPoint(200, 0);
    event.addMouseMove(pos);
    QTest::qWait(500);
    event.simulate(a->m_pStatusBar->m_pSlider->slider());
    pos = QPoint(width / 10 * a->m_pStatusBar->m_pSlider->slider()->sliderPosition(), 10);
    for (int i = 0; i < 5; i++) {
        event.addMousePress(Qt::LeftButton, Qt::NoModifier, pos);
        pos = pos + QPoint(20, 0);
        event.addMouseMove(pos);
        event.simulate(a->m_pStatusBar->m_pSlider->slider());
        event.clear();
        QTest::qWait(500);
    }
    pos = QPoint(width / 10 * a->m_pStatusBar->m_pSlider->slider()->sliderPosition(), 10);
    for (int i = 0; i < 5; i++) {
        event.addMousePress(Qt::LeftButton, Qt::NoModifier, pos);
        pos = pos - QPoint(20, 0);
        event.addMouseMove(pos);
        event.simulate(a->m_pStatusBar->m_pSlider->slider());
        event.clear();
        QTest::qWait(500);
    }
    event.addMousePress(Qt::LeftButton, Qt::NoModifier, pos);
    pos = pos - QPoint(20, 0);
    event.addMouseMove(pos);
    event.addMouseRelease(Qt::LeftButton, Qt::NoModifier, pos);
    event.simulate(a->m_pStatusBar->m_pSlider->slider());
    event.clear();
}

TEST(allpicview, resize)
{
    TEST_CASE_NAME("resize")
    MainWindow *w = dApp->getMainWindow();

    QTest::qWait(200);
    w->resize(w->size() + QSize(300, 300));
    QTest::qWait(200);
    QResizeEvent resize2(w->size() - QSize(100, 100), w->size());
    w->resize(w->size() - QSize(300, 300));
    QTest::qWait(200);
}

TEST(allpicview, dragevent)
{
    TEST_CASE_NAME("dragevent")
    QTest::qWait(200);
    MainWindow *w = dApp->getMainWindow();
    AllPicView *a = w->m_pAllPicView;
    w->show();

    // 打开保存绘制的 ddf
    QString jpgItemPath = testPath_test + "/2e5y8y.jpg";

    QString text = "xxxxxxxxxxxxxx";
    QIcon icon = QIcon(":/resources/images/other/deepin-album.svg");
    QIcon icon_hover = QIcon(":/resources/images/other/deepin-album.svg");
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << text << icon << icon_hover;
//    QMimeData *mimeData = new QMimeData;
    QMimeData mimedata;
    mimedata.setData(QStringLiteral("TestListView/text-icon-icon_hover"), itemData);
    QList<QUrl> li;
    li.append(QUrl::fromLocalFile(jpgItemPath));
    mimedata.setUrls(li);

    const QPoint pos = a->getThumbnailListView()->viewport()->rect().center();
    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->getDAppNew()->sendEvent(a->getThumbnailListView()->viewport(), &eEnter);
    QTest::qWait(200);

    QDragMoveEvent eMove(pos + QPoint(100, 100), Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->getDAppNew()->sendEvent(a->getThumbnailListView()->viewport(), &eMove);
    QTest::qWait(200);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->getDAppNew()->sendEvent(a->getThumbnailListView()->viewport(), &e);
    QTest::qWait(200);

    dApp->getDAppNew()->sendEvent(a->getThumbnailListView()->viewport(), &eEnter);
    QTest::qWait(200);

    QDragLeaveEvent eLeave;
    dApp->getDAppNew()->sendEvent(a->getThumbnailListView()->viewport(), &eLeave);
    QTest::qWait(2000);
}

TEST(allpicview, mousePress)
{
    TEST_CASE_NAME("mousePress")
    MainWindow *w = dApp->getMainWindow();

    QPoint pos(40, 60);
    AllPicView *a = w->m_pAllPicView;
    QTestEventList event;
    event.addMousePress(Qt::LeftButton, Qt::NoModifier, pos);
    event.addMouseMove(pos + QPoint(200, 200));
    event.addMouseRelease(Qt::LeftButton, Qt::NoModifier, pos + QPoint(200, 200));
    event.simulate(a->getThumbnailListView()->viewport());
    event.clear();
    QTest::qWait(500);
}

TEST(allpicview, test_open)
{
    TEST_CASE_NAME("test_open");

    QString publicTestPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + QDir::separator() + "test";
    QString testPath = publicTestPath;
    MainWindow *w = dApp->getMainWindow();
    QStringList testPathlist = ImageEngineApi::instance()->get_AllImagePath();
    if (!testPathlist.isEmpty()) {
        ImageEngineApi::instance()->moveImagesToTrash(testPathlist);
        QTest::qWait(500);
    }
    AllPicView *a = w->m_pAllPicView;
    QTestEventList tl;
    tl.addMouseMove(QPoint(0, 0), 100);
    tl.addMousePress(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0), 100);
    tl.addMouseMove(a->m_pImportView->geometry().center());
    tl.simulate(a->m_pImportView);
    ImageEngineApi::instance()->ImportImagesFromFileList((QStringList() << testPath), "", a, false);
    QTest::qWait(500);

    testPathlist = ImageEngineApi::instance()->get_AllImagePath();
    if (!testPathlist.isEmpty()) {
        qDebug() << "test ImageView Success ";
        w->m_pSearchEdit->setText("1");
        w->m_pSearchEdit->editingFinished();
        QTest::qWait(300);
        emit a->getThumbnailListView()->menuOpenImage(testPathlist.first(), testPathlist, false);
        QTest::qWait(300);
        QTestEventList e;
        e.addKeyClick(Qt::Key_Minus, Qt::ControlModifier, 100);
        e.addKeyClick(Qt::Key_Minus, Qt::ControlModifier, 100);
        e.addKeyClick(Qt::Key_Plus, Qt::ControlModifier, 100);
        e.addKeyClick(Qt::Key_Plus, Qt::ControlModifier, 100);
        e.addKeyClick(Qt::Key_Plus, Qt::ControlModifier, 100);
        e.addKeyClick(Qt::Key_Plus, Qt::ControlModifier, 100);
        e.addKeyClick(Qt::Key_Plus, Qt::ControlModifier, 100);
        e.simulate(w);
        e.clear();

        emit dApp->signalM->showImageInfo(testPathlist.first());

        CommandLine *commandline = CommandLine::instance();
        QString jpgItemPath = testPath_test + "/2e5y8y.jpg";
        QStringList list;
        list << jpgItemPath;
        commandline->viewImage(jpgItemPath, list);

        MainWidget *mw = CommandLine::instance()->findChild<MainWidget *>("MainWidget");
        if (mw) {
            TTBContent *t = mw->findChild<TTBContent *>("TTBContent");
            if (t) {
                MyImageListWidget *ml = t->findChild<MyImageListWidget *>("MyImageListWidget");
                if (ml) {
                    ml->getObj();
                    ml->findSelectItem();
                    ml->thumbnailIsMoving();
                    ml->stopAnimation();
                    ml->deleteLater();

                    QMouseEvent e1(QEvent::MouseButtonPress,
                                   QPointF(100, 100),
                                   Qt::MouseButton::AllButtons,
                                   Qt::MouseButtons(),
                                   Qt::KeyboardModifier::NoModifier);
                    ml->eventFilter(nullptr, &e1);

                    QEvent e2(QEvent::MouseButtonRelease);
                    ml->eventFilter(nullptr, &e2);

                    QEvent e3(QEvent::Leave);
                    ml->eventFilter(ml->getObj(), &e3);

                    QMouseEvent e4(QEvent::MouseButtonPress,
                                   QPointF(100, 100),
                                   Qt::MouseButton::AllButtons,
                                   Qt::MouseButtons(),
                                   Qt::KeyboardModifier::NoModifier);
                    ml->eventFilter(nullptr, &e4);

                    QMouseEvent e5(QEvent::MouseMove,
                                   QPointF(100, 100),
                                   Qt::MouseButton::AllButtons,
                                   Qt::MouseButtons(),
                                   Qt::KeyboardModifier::NoModifier);
                    ml->eventFilter(nullptr, &e5);
                }
            }
        }
    }
    QTest::qWait(500);
    QTestEventList e;
    e.addKeyClick(Qt::Key_Escape);
    e.simulate(w);

    ImgDeleteDialog *dialog = new ImgDeleteDialog(a->getThumbnailListView(), testPathlist.length());
    ImageEngineApi::instance()->moveImagesToTrash(testPathlist, true, false);
    dialog->deleteLater();
}

TEST(allpicview, test_select)
{
    TEST_CASE_NAME("test_select")
    MainWindow *w = dApp->getMainWindow();
    AllPicView *a = w->m_pAllPicView;
    QTestEventList e;
    QPoint p = a->getThumbnailListView()->pos();
    p.setX(p.x() + 10);
    p.setY(p.y() + 10);
    e.addMouseMove(p);
    e.addKeyPress(Qt::LeftButton);
    e.addMouseMove(QPoint(p.x() + 400, p.y() + 200));
    e.clear();
    e.simulate(a->getThumbnailListView());

    dApp->signalM->sigShortcutKeyDelete();
    QTest::qWait(100);
    a->getThumbnailListView()->sigLoad80ThumbnailsFinish();
    QTest::qWait(100);
    a->updateStackedWidget();
    a->restorePicNum();
    a->updatePicNum();
    dApp->signalM->imagesRemoved();
    QTest::qWait(100);
    a->getThumbnailListView()->openImage(0);
    QTest::qWait(100);
}

TEST(allpicview, test_shortCut)
{
    TEST_CASE_NAME("test_shortCut")
    MainWindow *w = dApp->getMainWindow();
    w->createShorcutJson();
    QTestEventList e;
    e.addKeyClick(Qt::Key_Question, Qt::ControlModifier | Qt::ShiftModifier);
    e.simulate(w);
    QTest::qWait(500);
}

TEST(allpicview, test_showInFileManagerAndBackGrond)
{
//    TEST_CASE_NAME("test_showInFileManagerAndBackGrond")
//    using namespace utils::base;
//    using namespace utils::image;
//    MainWindow *w = dApp->getMainWindow();
//    AllPicView *a = w->m_pAllPicView;
//    QString testPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + QDir::separator() + "test";
//    ImageEngineApi::instance()->ImportImagesFromFileList((QStringList() << testPath), "", a, false);
////    QTime t;
//    QTest::qWait(500);
//    QStringList paths = ImageEngineApi::instance()->get_AllImagePath();
//    if (!paths.isEmpty()) {
//        QString testImage = paths.first();
////        showInFileManager(testImage);//roc
////        imageSupportSave(testImage);
////        checkFileType(testImage);
//        copyImageToClipboard(paths);
////        copyOneImageToClipboard(testImage);
//        WallpaperSetter::instance()->setBackground(testImage);
//        QTest::qWait(500);
//        QMimeData *newMimeData = new QMimeData();
//        QByteArray gnomeFormat = QByteArray("copy\n");
//        QString text;
//        QList<QUrl> dataUrls;

//        for (QString path : paths) {
//            if (!path.isEmpty())
//                text += path + '\n';
//            dataUrls << QUrl::fromLocalFile(path);
//            gnomeFormat.append(QUrl::fromLocalFile(path).toEncoded()).append("\n");
//        }
//        newMimeData->setUrls(dataUrls);
//        checkMimeData(newMimeData);
//        QTest::qWait(500);
//    }
}

TEST(allpicview, viewpaneltest)
{
    TEST_CASE_NAME("viewpaneltest")
    QString publicTestPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + QDir::separator() + "test";
    QString testPath = publicTestPath;
    MainWindow *w = dApp->getMainWindow();
    AllPicView *a = w->m_pAllPicView;
    QTestEventList tl;
    tl.addMouseMove(QPoint(0, 0), 100);
    tl.addMousePress(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0), 100);
    tl.addMouseMove(a->m_pImportView->geometry().center());
    tl.simulate(a->m_pImportView);
    QTest::qWait(500);
//    ImageEngineApi::instance()->ImportImagesFromFileList((QStringList() << testPath), "", a, false);
//    QTest::qWait(500);

    QStringList testPathlist = ImageEngineApi::instance()->get_AllImagePath();
    if (!testPathlist.isEmpty()) {
        qDebug() << "test ImageView Success ";
        w->m_pSearchEdit->setText("1");
        w->m_pSearchEdit->editingFinished();
        QTest::qWait(300);

        emit a->getThumbnailListView()->menuOpenImage(testPathlist.first(), testPathlist, false);
        QTest::qWait(300);

        QTestEventList e;
        e.addKeyClick(Qt::Key_Minus, Qt::ControlModifier, 100);
        e.addKeyClick(Qt::Key_Minus, Qt::ControlModifier, 100);
        e.addKeyClick(Qt::Key_Plus, Qt::ControlModifier, 100);
        e.addKeyClick(Qt::Key_Plus, Qt::ControlModifier, 100);
        e.addKeyClick(Qt::Key_Plus, Qt::ControlModifier, 100);
        e.addKeyClick(Qt::Key_Plus, Qt::ControlModifier, 100);
        e.addKeyClick(Qt::Key_Plus, Qt::ControlModifier, 100);
        e.simulate(w);
        e.clear();

        emit dApp->signalM->showImageInfo(testPathlist.first());

        QTest::qWait(500);
//        ImgInfoDialog *dialog = w->findChild<ImgInfoDialog *>("ImgInfoDialog");
//        dialog->height();
//        dialog->setImagePath(testPath_test + "2k9o1m.png");
//        dialog->updateInfo();
//        QPoint p = dialog->pos();
//        e.addMouseMove(p, 10);
//        e.addKeyClick(Qt::Key_Escape, Qt::NoModifier, 10);
//        e.simulate(dialog);
//        e.clear();
//        dialog->deleteLater();
//        QTest::qWait(500);
//        ViewPanel *viewPanel = w->findChild<ViewPanel *>("ViewPanel");

        QList<QWidget *> widgets = w->findChildren<QWidget *>();
        foreach (auto widget, widgets) {
            if (!strcmp(widget->metaObject()->className(), "ViewPanel")) {
                ViewPanel *viewPanel = dynamic_cast<ViewPanel *>(widget);

                viewPanel->onDeleteByMenu();
                QTest::qWait(100);
                viewPanel->onShowExtensionPanel();
                QTest::qWait(100);
                viewPanel->onHideExtensionPanel();
                QTest::qWait(100);
                viewPanel->onResetTransform(false);
                QTest::qWait(100);
                viewPanel->onResetTransform(true);
                QTest::qWait(100);
                viewPanel->showNormal();
                QTest::qWait(100);
                viewPanel->onHideImageView();
                QTest::qWait(500);
                emit a->getThumbnailListView()->menuOpenImage(testPathlist.first(), testPathlist, false);
                QTest::qWait(500);
//                viewPanel->onESCKeyActivated();
//                QTest::qWait(500);
                break;
            }
        }

        MainWidget *mw = CommandLine::instance()->findChild<MainWidget *>("MainWidget");
        if (mw) {
            TTBContent *t = mw->findChild<TTBContent *>("TTBContent");
            if (t == nullptr) {
                return;
            }
            t->onNextButton();
            QTest::qWait(500);
            t->onPreButton();
            QTest::qWait(500);
            t->updateFilenameLayout();
            QTest::qWait(500);

            DWidget *imgList = t->findChild<DWidget *>("imageListObj");
            if (imgList) {
                QObjectList list = dynamic_cast<DWidget *>(imgList)->children();
                for (int i = 0; i < list.size(); i++) {
                    QList<ImageItem *> labelList = dynamic_cast<DWidget *>(imgList)->findChildren<ImageItem *>(QString("%1").arg(i));
                    if (labelList.size() <= 0) {
                        continue;
                    }
                    ImageItem *img = labelList.at(0);
                    if (nullptr == img) {
                        continue;
                    }
                    img->emitClickSig();
                    QTest::qWait(400);
                }
            }

            MyImageListWidget *imgListView = t->findChild<MyImageListWidget *>("MyImageListWidget");
            if (imgListView) {
                imgListView->animationFinished();
                imgListView->findSelectItem();
                imgListView->animationStart(true, 0, 300);
                imgListView->stopAnimation();
//                imgListView->isAnimationStart();
//                imgListView->animationTimerTimeOut();
            }

            QTestEventList el;
            QPoint p(150, 20);
            el.addMouseMove(p);
            el.addMousePress(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, p, 100);
            p = p + QPoint(-20, 0);
            el.addMouseMove(p, 100);
//            QTest::qWait(300);
            p = p + QPoint(-20, 0);
            el.addMouseMove(p, 100);
//            QTest::qWait(300);
            p = p + QPoint(-20, 0);
            el.addMouseMove(p, 100);
//            QTest::qWait(300);
            p = p + QPoint(-20, 0);
            el.addMouseMove(p, 100);
//            QTest::qWait(300);
            p = p + QPoint(-20, 0);
            el.addMouseMove(p, 100);
//            QTest::qWait(300);
            el.addMouseRelease(Qt::LeftButton, Qt::NoModifier, p, 100);
            el.simulate(imgList);
            el.clear();

            if (imgListView) {
                QPoint p2(imgListView->pos().x() - 50, imgListView->pos().y() + 10);
                el.addMouseMove(p2);
                el.addMousePress(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, p2, 100);
                el.addMouseMove(QPoint(p2.x() + 100, p2.y()), 100);
                el.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(p2.x() + 100, p2.y()), 100);
                el.simulate(imgListView);
            }
        }
    }

    QTestEventList e;
    e.addKeyClick(Qt::Key_Escape);
    e.simulate(w);
}

TEST(allpicview, deleteTips)
{
    TEST_CASE_NAME("deleteTips")
    MainWindow *w = dApp->getMainWindow();
    w->allPicBtnClicked();
    QTest::qWait(500);
    AllPicView *a = w->m_pAllPicView;
    QStringList testPathlist = ImageEngineApi::instance()->get_AllImagePath();
    if (testPathlist.count() > 0) {
        QStringList tempDel;
        tempDel << testPathlist.last();
        ImgDeleteDialog *delDlg = new ImgDeleteDialog(a->getThumbnailListView(), tempDel.length());
        delDlg->show();
        QTest::qWait(500);
        ImageEngineApi::instance()->moveImagesToTrash(tempDel);
        delDlg->deleteLater();
        QTest::qWait(500);
    }
}

TEST(allpicview, allpicview_other_test)
{
    TEST_CASE_NAME("allpicview_other_test")
    MainWindow *w = dApp->getMainWindow();

    w->m_pAllPicView->onFinishLoad();
    w->m_pAllPicView->onMenuOpenImage("", QStringList(), false, false);
//    w->m_pAllPicView->onImportViewImportBtnClicked();


    QMimeData mimedata;
    QList<QUrl> li;
    QString lastImportPath =  QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
    lastImportPath += "/test/4l6r5y.png";
    li.append(QUrl(lastImportPath));
    mimedata.setUrls(li);

    QPoint pos = QPoint(20, 20);
    QDragEnterEvent dee(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    w->m_pAllPicView->dragEnterEvent(&dee);

    QDragMoveEvent dme(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    w->m_pAllPicView->dragMoveEvent(&dme);

    QDropEvent de(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    w->m_pAllPicView->dropEvent(&de);
}

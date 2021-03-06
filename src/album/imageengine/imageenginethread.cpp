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
#include "imageenginethread.h"
#include "imageengineapi.h"
#include <dgiovolumemanager.h>
#include <dgiofile.h>
#include <dgiofileinfo.h>
#include <dgiovolume.h>
#include <QImage>
#include <QImageReader>
#include <QDebug>
#include <QDir>
#include <DApplication>
#include <DApplicationHelper>
#include <QStandardPaths>
#include <QDirIterator>
#include <QSvgGenerator>
#include "utils/imageutils.h"
#include "utils/unionimage.h"
#include "dbmanager/dbmanager.h"
#include "application.h"
#include "controller/signalmanager.h"

DBImgInfo getDBInfo(const QString &srcpath)
{
    using namespace utils::base;
    using namespace UnionImage_NameSpace;
    QFileInfo srcfi(srcpath);
    DBImgInfo dbi;
    auto mds = getAllMetaData(srcpath);
    QString value = mds.value("DateTimeOriginal");
    dbi.fileName = srcfi.fileName();
    dbi.filePath = srcpath;
    dbi.dirHash = utils::base::hash(QString());
    if (!value.isEmpty()) {
        dbi.time = QDateTime::fromString(value, "yyyy/MM/dd hh:mm");
    } else if (!srcfi.birthTime().isValid()) {
        dbi.time = srcfi.birthTime();
    } else if (!srcfi.metadataChangeTime().isValid()) {
        dbi.time = srcfi.metadataChangeTime();
    } else {
        dbi.time = QDateTime::currentDateTime();
    }
    dbi.changeTime = QDateTime::fromString(mds.value("DateTimeDigitized"), "yyyy/MM/dd hh:mm");
    dbi.importTime = QDateTime::currentDateTime();
    return dbi;
}

namespace {
const QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + "deepin" + QDir::separator() + "deepin-album";
}

ImportImagesThread::ImportImagesThread()
{
    m_paths.clear();
    //setAutoDelete(false); ??????auto delete???????????????
}

ImportImagesThread::~ImportImagesThread()
{
}

void ImportImagesThread::setData(QList<QUrl> &paths, QString &albumname, ImageEngineImportObject *obj, bool bdialogselect)
{
    m_urls = paths;
    m_albumname = albumname;
    m_obj = obj;
    m_bdialogselect = bdialogselect;
    m_type = DataType_UrlList;
}

void ImportImagesThread::setData(QStringList &paths, QString &albumname, ImageEngineImportObject *obj, bool bdialogselect)
{
    m_paths = paths;
    m_albumname = albumname;
    m_obj = obj;
    m_bdialogselect = bdialogselect;
    m_type = DataType_StringList;
}


bool ImportImagesThread::ifCanStopThread(void *imgobject)
{
    static_cast<ImageEngineImportObject *>(imgobject)->removeThread(this);
    if (imgobject == m_obj) {
        return true;
    }
    return false;
}

void ImportImagesThread::runDetail()
{
    if (bneedstop) {
        m_obj->imageImported(false);
        m_obj->removeThread(this);
        return;
    }
    QStringList image_list;
    QStringList curAlbumImgPathList;
    if (m_albumname.length() > 0) {
        // ?????????????????????,
        curAlbumImgPathList = DBManager::instance()->getPathsByAlbum(m_albumname);
    } else {
        // ???????????????????????????,allpic timeline .etc
        curAlbumImgPathList = DBManager::instance()->getAllPaths();
    }
    QStringList curAlbumImportedPathList;
    // ???????????? url
    if (m_type == DataType_UrlList) {
        QStringList urlLocalPathList;
        for (QUrl url : m_urls) {
            const QString path = url.toLocalFile();
            urlLocalPathList << path;
            if (QFileInfo(path).isDir()) {
                auto finfos =  utils::image::getImagesInfo(path, true);
                for (auto finfo : finfos) {
                    if (utils::image::imageSupportRead(finfo.absoluteFilePath())) {
                        // if path imported album
                        if (curAlbumImgPathList.contains(finfo.absoluteFilePath())) {
                            curAlbumImportedPathList << finfo.absoluteFilePath();
                        } else {
                            image_list << finfo.absoluteFilePath();
                        }
                    }
                }
            } else if (utils::image::imageSupportRead(path)) {

                // if path imported album
                if (curAlbumImgPathList.contains(path)) {
                    curAlbumImportedPathList << path;
                } else {
                    image_list << path;
                }
            }
        }
    }
    // ?????????????????????
    else if (m_type == DataType_StringList) {
        foreach (QString path, m_paths) {
            if (bneedstop) {
                m_obj->imageImported(false);
                m_obj->removeThread(this);
                return;
            }
            QFileInfo file(path);
            if (file.isDir()) {
                auto finfos =  utils::image::getImagesInfo(path, true);
                for (auto finfo : finfos) {
                    if (utils::image::imageSupportRead(finfo.absoluteFilePath())) {
                        // if path imported album
                        if (curAlbumImgPathList.contains(finfo.absoluteFilePath())) {
                            curAlbumImportedPathList << finfo.absoluteFilePath();
                        } else {
                            image_list << finfo.absoluteFilePath();
                        }
                    }
                }
            } else if (file.exists()) { //????????????

                // if path imported album
                if (curAlbumImgPathList.contains(path)) {
                    curAlbumImportedPathList << path;
                } else {
                    image_list << path;
                }
            }
        }
    }
    // ?????????????????????????????????????????????????????????????????????????????????,????????????
    if (image_list.size() < 1 && curAlbumImportedPathList.size() < 1) {
        emit dApp->signalM->ImportFailed();
        m_obj->imageImported(false);
        m_obj->removeThread(this);
        return;
    } else if (image_list.size() < 1 && curAlbumImportedPathList.size() > 0) {
        // ??????????????????????????????????????????
        // ImportImageLoader() ???????????????????????????????????????????????????????????????????????????????????????????????????????????????
        emit dApp->signalM->RepeatImportingTheSamePhotos(image_list, curAlbumImportedPathList, m_albumname);
        // ????????????????????????
        emit dApp->signalM->sigAddDuplicatePhotos();
        m_obj->imageImported(true);
        m_obj->removeThread(this);
        return;
    } else if (image_list.size() > 0) {
        if (m_bdialogselect) {
            QFileInfo firstFileInfo(image_list.first());
            static QString cfgGroupName = QStringLiteral("General"), cfgLastOpenPath = QStringLiteral("LastOpenPath");
            dApp->setter->setValue(cfgGroupName, cfgLastOpenPath, firstFileInfo.path());
        }
        // ?????????????????????????????????????????????
        int isMountFlag = 0;
        DGioVolumeManager *pvfsManager = new DGioVolumeManager;
        QList<QExplicitlySharedDataPointer<DGioMount>> mounts = pvfsManager->getMounts();
        for (auto mount : mounts) {
            if (bneedstop || ImageEngineApi::instance()->closeFg()) {
                m_obj->imageImported(false);
                m_obj->removeThread(this);
                return;
            }
            QExplicitlySharedDataPointer<DGioFile> LocationFile = mount->getDefaultLocationFile();
            QString strPath = LocationFile->path();
            if (0 == image_list.first().compare(strPath)) {
                isMountFlag = 1;
                break;
            }
        }

        // ??????????????????
        if (isMountFlag) {
            QString strHomePath = QDir::homePath();
            //???????????????????????????
            QString strDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
            QString basePath = QString("%1%2%3").arg(strHomePath, "/Pictures/??????/", strDate);
            QDir dir;
            if (!dir.exists(basePath)) {
                dir.mkpath(basePath);
            }
            QStringList newImagePaths;
            foreach (QString strPath, image_list) {
                if (bneedstop || ImageEngineApi::instance()->closeFg()) {
                    m_obj->imageImported(false);
                    m_obj->removeThread(this);
                    return;
                }
                //??????????????????
                QStringList pathList = strPath.split("/", QString::SkipEmptyParts);
                QStringList nameList = pathList.last().split(".", QString::SkipEmptyParts);
                QString strNewPath = QString("%1%2%3%4%5%6").arg(basePath, "/", nameList.first(), QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()), ".", nameList.last());

                newImagePaths << strNewPath;
                //?????????????????????????????????????????????????????????????????????
                if (dir.exists(strNewPath)) {
                    continue;
                }

                // ?????????????????????????????????
                if (QFile::copy(strPath, strNewPath)) {

                }
            }

            image_list.clear();
            image_list = newImagePaths;
        }

        DBImgInfoList dbInfos;
        using namespace utils::image;
        int noReadCount = 0;
        for (auto imagePath : image_list) {
            if (!imageSupportRead(imagePath)) {
                noReadCount++;
                continue;
            }
            QFileInfo srcfi(imagePath);
            if (!srcfi.exists()) {  //?????????????????????
                continue;
            }
            dbInfos << getDBInfo(imagePath);
            emit dApp->signalM->progressOfWaitDialog(image_list.size(), dbInfos.size());
        }

        if (bneedstop) {
            m_obj->imageImported(false);
            m_obj->removeThread(this);
            return;
        }
        DBImgInfoList tempdbInfos;
        for (auto Info : dbInfos) {
            QFileInfo fi(Info.filePath);
            if (!fi.exists())
                continue;
            tempdbInfos << Info;
        }
        if (image_list.length() == tempdbInfos.length() && !tempdbInfos.isEmpty()) {
            dApp->m_imageloader->ImportImageLoader(tempdbInfos, m_albumname);// ?????????????????????????????????
            m_obj->imageImported(true);
            // ImportImageLoader() ???????????????????????????????????????????????????????????????????????????????????????????????????????????????
            if (curAlbumImportedPathList.count() > 0) {
                emit dApp->signalM->RepeatImportingTheSamePhotos(image_list, curAlbumImportedPathList, m_albumname);
            }
        } else {
            emit dApp->signalM->ImportFailed();
            m_obj->imageImported(false);
        }
        m_obj->removeThread(this);
    }
}

ImageRecoveryImagesFromTrashThread::ImageRecoveryImagesFromTrashThread()
{
    setAutoDelete(false);
}

void ImageRecoveryImagesFromTrashThread::setData(QStringList &paths)
{
    m_paths = paths;
}

void ImageRecoveryImagesFromTrashThread::runDetail()
{
    QStringList paths = m_paths;

    DBImgInfoList infos;
    for (auto path : paths) {
        DBImgInfo info;
        info = DBManager::instance()->getTrashInfoByPath(path);
        QFileInfo fi(info.filePath);
        info.importTime = QDateTime::currentDateTime();
        infos << info;
    }
    DBManager::instance()->insertImgInfos(infos);

    for (auto path : paths) {
        DBImgInfo info;
        info = DBManager::instance()->getTrashInfoByPath(path);
        QStringList namelist = info.albumname.split(",");
        for (auto eachname : namelist) {
            if (DBManager::instance()->isAlbumExistInDB(eachname)) {
                DBManager::instance()->insertIntoAlbum(eachname, QStringList(path));
            }
        }
    }
    DBManager::instance()->removeTrashImgInfos(paths);
    emit dApp->signalM->closeWaitDialog();
}

ImageMoveImagesToTrashThread::ImageMoveImagesToTrashThread()
{
    setAutoDelete(false);
}

void ImageMoveImagesToTrashThread::setData(QStringList &paths, bool typetrash)
{
    m_paths = paths;
    btypetrash = typetrash;
}

void ImageMoveImagesToTrashThread::runDetail()
{
    QStringList paths = m_paths;
    if (btypetrash) {
        DBManager::instance()->removeTrashImgInfos(paths);
        emit dApp->signalM->sigDeletePhotos(paths.length());
    } else {
        DBImgInfoList infos;
        int pathsCount = paths.size();
        int remoneOffset = pathsCount / 200;//???100?????????
        int removedCount = 0;
        QStringList removedPaths;
        emit dApp->signalM->progressOfWaitDialog(paths.size(), 0);
        for (auto path : paths) {
            DBImgInfo info;
            info = DBManager::instance()->getInfoByPath(path);
            info.importTime = QDateTime::currentDateTime();
            QStringList allalbumnames = DBManager::instance()->getAllAlbumNames();
            for (auto eachname : allalbumnames) {
                if (DBManager::instance()->isImgExistInAlbum(eachname, path)) {
                    info.albumname += (eachname + ",");
                }
            }
            infos << info;
            removedPaths << path;
            removedCount++;
            if (removedCount == remoneOffset) {
                DBManager::instance()->insertTrashImgInfos(infos);
                DBManager::instance()->removeImgInfos(removedPaths);
                emit dApp->signalM->progressOfWaitDialog(paths.size(), removedCount);
                remoneOffset += remoneOffset;
                removedPaths.clear();
                infos.clear();
            }
//            emit dApp->signalM->progressOfWaitDialog(paths.size(), infos.size());
        }
        DBManager::instance()->insertTrashImgInfos(infos);
        DBManager::instance()->removeImgInfos(removedPaths);
        emit dApp->signalM->progressOfWaitDialog(paths.size(), removedCount);
//        DBImgInfoList infos;
//        //??????????????????
//        DBImgInfoList infosAll = DBManager::instance()->getAllInfos(0);
//        QStringList allalbumnames = DBManager::instance()->getAllAlbumNames();
//        int pathsCount = paths.size();
//        int remoneOffset = pathsCount / 10;//???10?????????
//        int removedCount = 0;
//        QStringList removedPaths;
//        emit dApp->signalM->progressOfWaitDialog(paths.size(), removedCount);
//        for (auto path : paths) {
//            removedPaths << path;
//            DBImgInfo info;
//            for (auto infoCompare : infosAll) {
//                if (infoCompare.filePath == path) {
//                    info = infoCompare;
//                    infosAll.removeOne(infoCompare);
//                    break;
//                }
//            }

//            info.changeTime = QDateTime::currentDateTime();
//            for (auto eachname : allalbumnames) {
//                if (DBManager::instance()->isImgExistInAlbum(eachname, path)) {
//                    info.albumname += (eachname + ",");
//                }
//            }
//            infos << info;
//            removedCount++;
//            if (removedCount == remoneOffset) {
//                DBManager::instance()->insertTrashImgInfos(infos);
//                DBManager::instance()->removeImgInfos(removedPaths);
//                emit dApp->signalM->progressOfWaitDialog(paths.size(), removedCount);
//                remoneOffset += remoneOffset;
//                removedPaths.clear();
//                infos.clear();
//            }
//        }
//        emit dApp->signalM->progressOfWaitDialog(paths.size(), removedCount);
    }
    emit dApp->signalM->closeWaitDialog();
}

ImageImportFilesFromMountThread::ImageImportFilesFromMountThread()
{
    setAutoDelete(false);
}

ImageImportFilesFromMountThread::~ImageImportFilesFromMountThread()
{
}

void ImageImportFilesFromMountThread::setData(QString &albumname, QStringList &paths, ImageMountImportPathsObject *imgobject)
{
    m_paths = paths;
    m_imgobject = imgobject;
    m_albumname = albumname;
}

bool ImageImportFilesFromMountThread::ifCanStopThread(void *imgobject)
{
    static_cast<ImageMountImportPathsObject *>(imgobject)->removeThread(this);
    if (imgobject == m_imgobject) {
        return true;
    }
    return false;
}

void ImageImportFilesFromMountThread::runDetail()
{
    if (bneedstop) {
        return;
    }
    QStringList newPathList;
    DBImgInfoList dbInfos;
    QString strHomePath = QDir::homePath();
    //???????????????????????????
    QString strDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString basePath = QString("%1%2%3").arg(strHomePath, "/Pictures/??????/", strDate);
    QDir dir;
    if (!dir.exists(basePath)) {
        dir.mkpath(basePath);
    }

    foreach (QString strPath, m_paths) {
        if (bneedstop || ImageEngineApi::instance()->closeFg()) {
            return;
        }
        //??????????????????
        QStringList pathList = strPath.split("/", QString::SkipEmptyParts);
        QStringList nameList = pathList.last().split(".", QString::SkipEmptyParts);
        QString strNewPath = QString("%1%2%3%4%5%6").arg(basePath, "/", nameList.first(),
                                                         QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()), ".", nameList.last());

        //????????????????????????????????????????????????????????????????????????
        if (!dir.exists(strPath)) {
            continue;
        }
        //?????????????????????????????????????????????????????????????????????
        if (dir.exists(strNewPath)) {
            dir.remove(strNewPath);
        }
        newPathList << strNewPath;
        QFileInfo fi(strNewPath);
        using namespace utils::image;
        using namespace utils::base;
        if (QFile::copy(strPath, strNewPath)) {
            qDebug() << "onCopyPhotoFromPhone()";
        }
//        auto mds = getAllMetaData(strNewPath);
//        QString value = mds.value("DateTimeOriginal");
//        DBImgInfo dbi;
//        dbi.fileName = fi.fileName();
//        dbi.filePath = strNewPath;
//        dbi.dirHash = utils::base::hash(QString());
//        if ("" != value) {
//            dbi.time = QDateTime::fromString(value, "yyyy/MM/dd hh:mm:ss");
//        } else if (fi.birthTime().isValid()) {
//            dbi.time = fi.birthTime();
//        } else if (fi.metadataChangeTime().isValid()) {
//            dbi.time = fi.metadataChangeTime();
//        } else {
//            dbi.time = QDateTime::currentDateTime();
//        }

//        dbi.changeTime = QDateTime::currentDateTime();
        dbInfos << getDBInfo(strNewPath);

        emit dApp->signalM->progressOfWaitDialog(m_paths.size(), dbInfos.size());
    }
    if (!dbInfos.isEmpty()) {
        DBImgInfoList dbInfoList;
        QStringList pathslist;

        for (int i = 0; i < dbInfos.length(); i++) {
            if (bneedstop) {
                return;
            }
            pathslist << dbInfos[i].filePath;
            dbInfoList << dbInfos[i];
        }

        if (m_albumname.length() > 0) {
            DBManager::instance()->insertIntoAlbumNoSignal(m_albumname, pathslist);
        }
        DBManager::instance()->insertImgInfos(dbInfoList);

        if (bneedstop) {
            return;
        }
        if (dbInfoList.length() != m_paths.length()) {
            int successful = dbInfoList.length();
            int failed = m_paths.length() - dbInfoList.length();
            emit dApp->signalM->ImportSomeFailed(successful, failed);
        } else {
            emit dApp->signalM->ImportSuccess();
        }
    } else {
        emit dApp->signalM->ImportFailed();
    }
    emit sigImageFilesImported(m_imgobject, newPathList);
    m_imgobject->removeThread(this);
}

ImageGetFilesFromMountThread::ImageGetFilesFromMountThread()
{
    setAutoDelete(false);
}

ImageGetFilesFromMountThread::~ImageGetFilesFromMountThread()
{
}

void ImageGetFilesFromMountThread::setData(QString &mountname, QString &path, ImageMountGetPathsObject *imgobject)
{
    m_mountname = mountname;
    m_path = path;
    m_imgobject = imgobject;
}

bool ImageGetFilesFromMountThread::ifCanStopThread(void *imgobject)
{
    static_cast<ImageMountGetPathsObject *>(imgobject)->removeThread(this);
    if (imgobject == m_imgobject) {
        return true;
    }
    return false;
}

//???????????????????????????????????????????????????????????????????????????????????????"DCIM"????????????
//??????????????????????????????path/??????????????????/DCIM??????iPhone???patn/DCIM???
bool ImageGetFilesFromMountThread::findPicturePathByPhone(QString &path)
{
    QDir dir(path);
    if (!dir.exists()) return false;
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfo tempFileInfo;
    foreach (tempFileInfo, fileInfoList) {
        if (tempFileInfo.fileName().compare(ALBUM_PATHNAME_BY_PHONE) == 0) {
            path = tempFileInfo.absoluteFilePath();
            return true;
        } else {
            QDir subDir;
            subDir.setPath(tempFileInfo.absoluteFilePath());
            QFileInfoList subFileInfoList = subDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
            QFileInfo subTempFileInfo;
            foreach (subTempFileInfo, subFileInfoList) {
                if (subTempFileInfo.fileName().compare(ALBUM_PATHNAME_BY_PHONE) == 0) {
                    path = subTempFileInfo.absoluteFilePath();
                    return true;
                }
            }
            return false;
        }
    }
    return false;
}

void ImageGetFilesFromMountThread::runDetail()
{
    if (bneedstop) {
        return;
    }
    QString strPath = m_path;
    //?????????????????????????????????
    QStringList filters;
    for (QString i : UnionImage_NameSpace::unionImageSupportFormat()) {
        filters << "*." + i;
    }
    //??????????????????????????????????????????????????????QDirIterator::Subdirectories
    QDirIterator dir_iterator(m_path,
                              filters,
                              QDir::Files | QDir::NoSymLinks,
                              QDirIterator::Subdirectories);
    QStringList allfiles;
    while (dir_iterator.hasNext()) {
        if (bneedstop || ImageEngineApi::instance()->closeFg()) {
            return;
        }
        dir_iterator.next();
        QFileInfo fileInfo = dir_iterator.fileInfo();
        allfiles << fileInfo.filePath();
    }
    if (bneedstop) {
        return;
    }
    emit sigImageFilesGeted(m_imgobject, allfiles, m_path);
    m_imgobject->removeThread(this);
    emit dApp->signalM->sigLoadMountImagesEnd(m_mountname);
}

ImageLoadFromDBThread::ImageLoadFromDBThread(int loadCount)
    : m_loadCount(loadCount)
{
//    m_loadCount = loadCount;
    setAutoDelete(false);
}

ImageLoadFromDBThread::~ImageLoadFromDBThread()
{
}

void ImageLoadFromDBThread::setData(ThumbnailDelegate::DelegateType type, ImageEngineObject *imgobject, QString nametype)
{
    m_type = type;
    m_imgobject = imgobject;
    m_nametype = nametype;
}

bool ImageLoadFromDBThread::ifCanStopThread(void *imgobject)
{
    static_cast<ImageEngineObject *>(imgobject)->removeThread(this, false);
    if (imgobject == m_imgobject) {
        return true;
    }
    return false;
}

void ImageLoadFromDBThread::runDetail()
{
    if (bneedstop) {
        return;
    }
    QStringList image_list;
    QStringList fail_image_list;
    if (ThumbnailDelegate::AllPicViewType == m_type) {
        auto infos = DBManager::instance()->getAllInfos(m_loadCount);
        for (auto info : infos) {
            //?????????????????????????????????
            if (!QFileInfo(info.filePath).exists()) {
                fail_image_list << info.filePath;
                emit dApp->signalM->updatePicView(0);
                continue;
            }
            image_list << info.filePath;
            if (bneedstop || ImageEngineApi::instance()->closeFg()) {
                return;
            }
            emit sigInsert(info.filePath);
        }
        if (m_nametype.isEmpty())
            ImageEngineApi::instance()->SaveImagesCache(image_list);
    }
    if (bneedstop) {
        return;
    }

    //??????????????????????????????
    DBManager::instance()->removeImgInfosNoSignal(fail_image_list);
    // ??????????????????????????????????????????????????????,??????listview???setdata userrole + 2
    ImageEngineApi::instance()->setImgPathAndAlbumNames(DBManager::instance()->getAllPathAlbumNames());

    //??????????????????????????????
    emit sigImageLoaded(m_imgobject, image_list);

    m_imgobject->removeThread(this);
}

ImageLoadFromLocalThread::ImageLoadFromLocalThread()
{
    setAutoDelete(false);
}

ImageLoadFromLocalThread::~ImageLoadFromLocalThread()
{
}

void ImageLoadFromLocalThread::setData(QStringList &filelist, ImageEngineObject *imgobject, bool needcheck, DataType type)
{
    m_filelist = filelist;
    m_imgobject = imgobject;
    bneedcheck = needcheck;
    if (type == DataType_NULL)
        m_type = DataType_StrList;
    else
        m_type = type;
}

bool ImageLoadFromLocalThread::ifCanStopThread(void *imgobject)
{
    static_cast<ImageEngineObject *>(imgobject)->removeThread(this, false);
    if (imgobject == m_imgobject) {
        return true;
    }
    return false;
}

void ImageLoadFromLocalThread::setData(DBImgInfoList filelist, ImageEngineObject *imgobject, bool needcheck, DataType type)
{
    m_fileinfolist = filelist;
    m_imgobject = imgobject;
    bneedcheck = needcheck;
    if (type == DataType_NULL)
        m_type = DataType_InfoList;
    else
        m_type = type;
}

void ImageLoadFromLocalThread::runDetail()
{
    if (bneedstop) {
        return;
    }
    QStringList image_list;
    switch (m_type) {
    case DataType_StrList:
        if (!m_filelist.isEmpty()) {
            for (const QString &path : m_filelist) {
                image_list << path;
                emit sigInsert(path);
            }
        }
        break;
    case DataType_InfoList:
        if (!m_fileinfolist.isEmpty()) {
            for (auto info : m_fileinfolist) {
                if (bneedstop) {
                    return;
                }
                image_list << info.filePath;
                emit sigInsert(info.filePath);
            }
        }
        break;
    case DataType_TrashList:
        if (!m_fileinfolist.isEmpty()) {
            QStringList removepaths;
            int idaysec = 24 * 60 * 60;
            for (auto info : m_fileinfolist) {
                if (bneedstop) {
                    return;
                }
                QDateTime start = QDateTime::currentDateTime();
                QDateTime end = info.importTime;
                int etime = static_cast<int>(start.toTime_t());
                int stime = static_cast<int>(end.toTime_t());
                int Day = (etime - stime) / (idaysec) + ((etime - stime) % (idaysec) + (idaysec - 1)) / (idaysec) - 1;
                if (30 <= Day) {
                    removepaths << info.filePath;
                } else {
                    QString remainDay = QString::number(30 - Day) + tr("days");
                    image_list << info.filePath;
                    emit sigInsert(info.filePath, remainDay);
                }
            }
            if (0 < removepaths.length()) {
                DBManager::instance()->removeTrashImgInfosNoSignal(removepaths);
            }
        }
        break;
    default:
        break;
    }

    if (bneedstop) {
        return;
    }
    if (nullptr != m_imgobject) {
        m_imgobject->removeThread(this);
        emit sigImageLoaded(m_imgobject, image_list);
    }
}

ImageEngineThread::ImageEngineThread()
{
    m_imgobject.clear();
    setAutoDelete(false);
}

ImageEngineThread::~ImageEngineThread()
{
}

void ImageEngineThread::setData(QString &path, ImageEngineObject *imgobject, ImageDataSt &data, bool needcache)
{
    m_path = path;
    m_imgobject << imgobject;
    m_data = data;
    bneedcache = needcache;
}

bool ImageEngineThread::ifCanStopThread(void *imgobject)
{
    if (nullptr != imgobject && ImageEngineApi::instance()->ifObjectExist(imgobject))
        static_cast<ImageEngineObject *>(imgobject)->removeThread(this, false);
    m_imgobject.removeOne(static_cast<ImageEngineObject *>(imgobject));
    if (m_imgobject.size() < 1) {
        bneedstop = true;
        return true;
    }
    return false;
}

bool ImageEngineThread::getNeedStop()
{
    return bneedstop;
//    if (!bneedstop) {
//        return false;
//    }
//    baborted = true;
//    bneedstop = false;
//    emit sigAborted(m_path);
//    while (!bneedstop) {
//        QThread::msleep(50);
//    }
//    if (!baborted) {
//        bneedstop = false;
//        return false;
//    }
//    return true;
}

bool ImageEngineThread::addObject(ImageEngineObject *imgobject)
{
    if (baborted) {
        baborted = false;
        QMutexLocker mutex(&m_mutex);
        m_imgobject << imgobject;
        return false;
    }
    if (bneedstop) {
        bneedstop = false;
    }
    if (!bwaitstop) {
        QMutexLocker mutex(&m_mutex);
        m_imgobject << imgobject;
    } else {
        imgobject->removeThread(this);
        emit sigImageLoaded(imgobject, m_path, m_data);
    }
    return true;
}

//??????QPixmap
void ImageEngineThread::runDetail()
{
    if (getNeedStop())
        return;
    if (!QFileInfo(m_path).exists()) {
        emit sigAborted(m_path);
        return;
    }

    using namespace UnionImage_NameSpace;
    QImage tImg;
    QString path = m_path;
    QFileInfo file(CACHE_PATH + m_path);
    QString errMsg;
    QString dimension;
    QFileInfo srcfi(m_path);
    if (m_data.imgpixmap.isNull()) {
        bool cache_exist = false;
        if (file.exists()) {
            QDateTime cachetime = file.metadataChangeTime();    //??????????????????
            QDateTime srctime = srcfi.metadataChangeTime();     //?????????????????????
            if (srctime.toTime_t() > cachetime.toTime_t()) {  //???????????????????????????????????????????????????
                cache_exist = false;
                breloadCache = true;
                path = m_path;
                if (!loadStaticImageFromFile(path, tImg, errMsg)) {
                    qDebug() << errMsg;
                }
                dimension = QString::number(tImg.width()) + "x" + QString::number(tImg.height());
            } else {
                cache_exist = true;
                path = CACHE_PATH + m_path;
                if (!loadStaticImageFromFile(path, tImg, errMsg, "PNG")) {
                    qDebug() << errMsg;
                }
            }
        } else {
            if (!loadStaticImageFromFile(path, tImg, errMsg)) {
                qDebug() << errMsg;
            }
            dimension = QString::number(tImg.width()) + "x" + QString::number(tImg.height());
        }
        if (getNeedStop())
            return;
        QPixmap pixmap = QPixmap::fromImage(tImg);
        if (0 != pixmap.height() && 0 != pixmap.width() && (pixmap.height() / pixmap.width()) < 10 && (pixmap.width() / pixmap.height()) < 10) {
            if (pixmap.height() != 140 && pixmap.width() != 140) {
                if (pixmap.height() >= pixmap.width()) {
                    cache_exist = true;
                    pixmap = pixmap.scaledToWidth(140,  Qt::FastTransformation);
                } else if (pixmap.height() <= pixmap.width()) {
                    cache_exist = true;
                    pixmap = pixmap.scaledToHeight(140,  Qt::FastTransformation);
                }
            }
            if (!cache_exist) {
                if ((static_cast<float>(pixmap.height()) / (static_cast<float>(pixmap.width()))) > 3) {
                    pixmap = pixmap.scaledToWidth(140,  Qt::FastTransformation);
                } else {
                    pixmap = pixmap.scaledToHeight(140,  Qt::FastTransformation);
                }
            }
        }
        if (pixmap.isNull()) {
            qDebug() << "null pixmap" << tImg;
            pixmap = QPixmap::fromImage(tImg);
        }
        if (breloadCache) { //??????????????????
            QString spath = CACHE_PATH + m_path;
            utils::base::mkMutiDir(spath.mid(0, spath.lastIndexOf('/')));
            pixmap.save(spath, "PNG");
        }
        m_data.imgpixmap = pixmap;
    }
    DBImgInfo dbi = getDBInfo(m_path);
    if (!dimension.isEmpty()) {
        dbi.albumSize = dimension;
    }
    m_data.dbi = dbi;
    m_data.loaded = ImageLoadStatu_Loaded;
    if (getNeedStop()) {
        return;
    }
    bwaitstop = true;
    QMutexLocker mutex(&m_mutex);
    for (ImageEngineObject *imgobject : m_imgobject) {
        imgobject->removeThread(this);
        emit sigImageLoaded(imgobject, m_path, m_data);
    }
    //???????????????????????????????????????????????????????????????????????????????????????????????????????????????
//    while (!bneedstop && !ImageEngineApi::instance()->closeFg()) {
//        QThread::msleep(100);
//    }
}

ImageFromNewAppThread::ImageFromNewAppThread()
{
    setAutoDelete(false);
}

ImageFromNewAppThread::~ImageFromNewAppThread()
{
}

void ImageFromNewAppThread::setDate(QStringList &files, ImageEngineImportObject *obj)
{
    paths = files;
    m_imgobj = obj;
}

bool ImageFromNewAppThread::ifCanStopThread(void *imgobject)
{
    static_cast<ImageEngineImportObject *>(imgobject)->removeThread(this);
    if (imgobject == m_imgobj) {
        return true;
    }
    return  false;
}

void ImageFromNewAppThread::runDetail()
{
    if (bneedstop) {
        m_imgobj->imageImported(false);
        m_imgobj->removeThread(this);
        return;
    }
    DBImgInfoList dbInfos;
    using namespace utils::image;
    for (auto path : paths) {
        if (bneedstop) {
            m_imgobj->imageImported(false);
            m_imgobj->removeThread(this);
            return;
        }
        if (!imageSupportRead(path)) continue;
//        QFileInfo fi(path);
//        using namespace utils::image;
//        using namespace utils::base;
//        auto mds = getAllMetaData(path);
//        QString value = mds.value("DateTimeOriginal");
//        DBImgInfo dbi;
//        dbi.fileName = fi.fileName();
//        dbi.filePath = path;
//        dbi.dirHash = utils::base::hash(QString());
//        if ("" != value) {
//            dbi.time = QDateTime::fromString(value, "yyyy/MM/dd hh:mm:ss");
//        } else if (fi.birthTime().isValid()) {
//            dbi.time = fi.birthTime();
//        } else if (fi.metadataChangeTime().isValid()) {
//            dbi.time = fi.metadataChangeTime();
//        } else {
//            dbi.time = QDateTime::currentDateTime();
//        }
//        dbi.changeTime = QDateTime::currentDateTime();
        dbInfos << getDBInfo(path);
    }
    if (! dbInfos.isEmpty()) {
        if (bneedstop) {
            m_imgobj->imageImported(false);
            m_imgobj->removeThread(this);
            return;
        }
        dApp->m_imageloader->ImportImageLoader(dbInfos);
    }
    m_imgobj->removeThread(this);
}

ImageCacheQueuePopThread::ImageCacheQueuePopThread()
{
    setAutoDelete(false);
}

ImageCacheQueuePopThread::~ImageCacheQueuePopThread()
{

}

void ImageCacheQueuePopThread::saveCache(QString m_path)
{
    if (needStop || m_path.isEmpty()) {
        qDebug() << "m_path empty";
        return;
    }
    QImage tImg;
    QString path = m_path;
    QFileInfo file(CACHE_PATH + path);
    if (needStop)
        return;
    if (file.exists()) {
        return;
    }
    if (needStop)
        return;
    QString errMsg;
    if (!UnionImage_NameSpace::loadStaticImageFromFile(path, tImg, errMsg)) {
        qDebug() << errMsg;
        return;
    }
    if (needStop)
        return;
    QPixmap pixmap = QPixmap::fromImage(tImg);
    if (0 != pixmap.height() && 0 != pixmap.width() && (pixmap.height() / pixmap.width()) < 10 && (pixmap.width() / pixmap.height()) < 10) {
        bool cache_exist = false;
        if (pixmap.height() != 200 && pixmap.width() != 200) {
            if (pixmap.height() >= pixmap.width()) {
                cache_exist = true;
                pixmap = pixmap.scaledToWidth(200,  Qt::FastTransformation);
            } else if (pixmap.height() <= pixmap.width()) {
                cache_exist = true;
                pixmap = pixmap.scaledToHeight(200,  Qt::FastTransformation);
            }
        }
        if (!cache_exist) {
            if (static_cast<float>(pixmap.height()) / static_cast<float>(pixmap.width()) > 3) {
                pixmap = pixmap.scaledToWidth(200,  Qt::FastTransformation);
            } else {
                pixmap = pixmap.scaledToHeight(200,  Qt::FastTransformation);
            }
        }
    }
    QString spath = CACHE_PATH + m_path;
    if (needStop)
        return;
    utils::base::mkMutiDir(spath.mid(0, spath.lastIndexOf('/')));
    pixmap.save(spath, "PNG");
}

void ImageCacheQueuePopThread::run()
{
    while (!m_obj->isEmpty() && !needStop && !ImageEngineApi::instance()->closeFg()) {
        QString res = m_obj->pop();
        if (!res.isEmpty()) {
            saveCache(res);
        }
    }
    qDebug() << "Cachethread end,there threads:" << ImageEngineApi::instance()->CacheThreadNum() - 1;
}

ImageEngineBackThread::ImageEngineBackThread(): m_bpause(false)
{
    setAutoDelete(false);
    connect(dApp->signalM, &SignalManager::sigDevStop, this, [ = ](QString devName) {
        if (devName == m_devName || devName.isEmpty()) {
            bbackstop = true;
        }
    });

    connect(dApp->signalM, &SignalManager::sigPauseOrStart, this, &ImageEngineBackThread::onStartOrPause);
}

void ImageEngineBackThread::setData(QStringList pathlist, QString devName)
{
    m_pathlist = pathlist;
    m_devName = devName;
}

void ImageEngineBackThread::runDetail()
{
    using namespace UnionImage_NameSpace;
    for (auto temppath : m_pathlist) {
        QImage tImg;
        QString path = temppath;
        QString errMsg;
        if (!loadStaticImageFromFile(path, tImg, errMsg)) {
            qDebug() << errMsg;
            break;
        }
        if (bbackstop || ImageEngineApi::instance()->closeFg())
            return;

        QPixmap pixmap = QPixmap::fromImage(tImg);
        if (0 != pixmap.height() && 0 != pixmap.width() && (pixmap.height() / pixmap.width()) < 10 && (pixmap.width() / pixmap.height()) < 10) {
            bool cache_exist = false;
            if (pixmap.height() != 100 && pixmap.width() != 100) {
                if (pixmap.height() >= pixmap.width()) {
                    cache_exist = true;
                    pixmap = pixmap.scaledToWidth(100,  Qt::FastTransformation);
                } else if (pixmap.height() <= pixmap.width()) {
                    cache_exist = true;
                    pixmap = pixmap.scaledToHeight(100,  Qt::FastTransformation);
                }
            }
            if (!cache_exist) {
                if ((static_cast<float>(pixmap.height()) / (static_cast<float>(pixmap.width()))) > 3) {
                    pixmap = pixmap.scaledToWidth(100,  Qt::FastTransformation);
                } else {
                    pixmap = pixmap.scaledToHeight(100,  Qt::FastTransformation);
                }
            }
        }
        if (pixmap.isNull()) {
            qDebug() << "[ImageEngineBackThread]:null pixmap!" << tImg;
            pixmap = QPixmap::fromImage(tImg);
        }
        m_data.imgpixmap = pixmap;
        if (bbackstop || ImageEngineApi::instance()->closeFg())
            return;
        m_data.dbi = getDBInfo(temppath);
        m_data.loaded = ImageLoadStatu_Loaded;

        if (bbackstop || ImageEngineApi::instance()->closeFg()) {
            return;
        }

        if (m_bpause) {
            m_WatiCondition.wait(&m_mutex);     //??????
        }
        emit sigImageBackLoaded(temppath, m_data);
    }
}

void ImageEngineBackThread::onStartOrPause(bool pause)
{
    if (pause)
        m_bpause = true;
    else {
        m_WatiCondition.wakeOne();      //????????????
        m_bpause = false;
    }
}

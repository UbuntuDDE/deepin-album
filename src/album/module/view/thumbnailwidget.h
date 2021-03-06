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
#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>

#include <DLabel>

#include "controller/viewerthememanager.h"
#include "widgets/themewidget.h"

class ThumbnailWidget : public ThemeWidget
{
    Q_OBJECT
public:
    ThumbnailWidget(const QString &darkFile, const QString
                    &lightFile, QWidget *parent = nullptr);
    ~ThumbnailWidget() override;
signals:
    void mouseHoverMoved();
#ifdef LITE_DIV
    void openImageInDialog();
#endif

public slots:
    void setThumbnailImage(const QPixmap thumbnail);
    void onThemeTypeChanged();


//    bool isDefaultThumbnail();
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

    bool m_isDefaultThumbnail = false;
    QLabel *m_thumbnailLabel;
    QPixmap m_logo;
#ifndef LITE_DIV
    QLabel *m_tips;
#endif
    QPixmap m_defaultImage;
    QColor m_inBorderColor;
    QString m_picString;
    bool m_theme;
    bool m_usb = false;
};

#endif // THUMBNAILWIDGET_H

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
#include "timelineitem.h"
#include <QApplication>

TimelineItem::TimelineItem(QWidget *parent)
    : QWidget(parent), m_title(nullptr), m_date(nullptr)
    , m_num(nullptr), m_Chose(nullptr)
{

}

void TimelineItem::mousePressEvent(QMouseEvent *e)
{
    if (QApplication::keyboardModifiers() != Qt::ControlModifier && e->button() == Qt::LeftButton) {
        emit sigMousePress();
    }
    QWidget::mousePressEvent(e);
}



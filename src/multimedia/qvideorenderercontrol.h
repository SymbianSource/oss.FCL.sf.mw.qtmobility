/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QVIDEORENDERERCONTROL_H
#define QVIDEORENDERERCONTROL_H

#include "qmediacontrol.h"

QT_BEGIN_NAMESPACE
class QAbstractVideoSurface;
QT_END_NAMESPACE

QT_BEGIN_NAMESPACE


class Q_MULTIMEDIA_EXPORT QVideoRendererControl : public QMediaControl
{
    Q_OBJECT

public:
    ~QVideoRendererControl();

    virtual QAbstractVideoSurface *surface() const = 0;
    virtual void setSurface(QAbstractVideoSurface *surface) = 0;

protected:
    QVideoRendererControl(QObject *parent = 0);
};

#define QVideoRendererControl_iid "com.nokia.Qt.QVideoRendererControl/1.0"
Q_MEDIA_DECLARE_CONTROL(QVideoRendererControl, QVideoRendererControl_iid)

QT_END_NAMESPACE

#endif // QVIDEORENDERERCONTROL_H

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

#ifndef QGEOSATELLITEINFOSOURCE_WINCE_P_H
#define QGEOSATELLITEINFOSOURCE_WINCE_P_H

#include <qgeosatelliteinfosource.h>

#include "qgeoinfothread_wince_p.h"

QTM_BEGIN_NAMESPACE

class QGeoSatelliteInfoValidator : public QGeoInfoValidator
{
public:
    QGeoSatelliteInfoValidator();
    ~QGeoSatelliteInfoValidator();

    bool valid(const GPS_POSITION &data) const;
};

class QGeoSatelliteInfoSourceWinCE : public QGeoSatelliteInfoSource
{
    Q_OBJECT

public:
    enum {
        // The interval at which the periodic updates will occur.
        DefaultUpdateInterval = 5000
    };

    explicit QGeoSatelliteInfoSourceWinCE(QObject *parent = 0);
    ~QGeoSatelliteInfoSourceWinCE();

public slots:
    void startUpdates();
    void stopUpdates();
    void requestUpdate(int timeout = 0);

private slots:
    void dataUpdated(GPS_POSITION data);

private:
    QGeoInfoThreadWinCE *infoThread;
};

QTM_END_NAMESPACE

#endif //#ifndef QGEOSATELLITEINFOSOURCE_WINCE_P_H

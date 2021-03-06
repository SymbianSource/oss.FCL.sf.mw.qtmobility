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

#include "maemo6compass.h"

char const * const maemo6compass::id("maemo6.compass");
bool maemo6compass::m_initDone = false;

maemo6compass::maemo6compass(QSensor *sensor)
    : maemo6sensorbase(sensor)
{
    const QString sensorName = "compasssensor";
    initSensor<CompassSensorChannelInterface>(sensorName, m_initDone);

    if (m_sensorInterface){
        if (!(QObject::connect(m_sensorInterface, SIGNAL(dataAvailable(const Compass&)),
                               this, SLOT(slotDataAvailable(const Compass&)))))
            qWarning() << "Unable to connect "<< sensorName;
    }
    else
        qWarning() << "Unable to initialize "<<sensorName;

    setReading<QCompassReading>(&m_reading);
}

void maemo6compass::slotDataAvailable(const Compass& data)
{
    // The scale for level is [0,3], where 3 is the best
    // Qt: Measured as a value from 0 to 1 with higher values being better.
    m_reading.setCalibrationLevel(((float) data.level()) / 3.0);

    // The scale for degrees from sensord is [0,359]
    // Value can be directly used as azimuth
    m_reading.setAzimuth(data.degrees());

    m_reading.setTimestamp(data.data().timestamp_);
    newReadingAvailable();
}

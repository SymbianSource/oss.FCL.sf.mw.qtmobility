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

#include "maemo6magnetometer.h"

char const * const maemo6magnetometer::id("maemo6.magnetometer");
bool maemo6magnetometer::m_initDone = false;

maemo6magnetometer::maemo6magnetometer(QSensor *sensor)
    : maemo6sensorbase(sensor)
{
    const QString sensorName = "magnetometersensor";
    initSensor<MagnetometerSensorChannelInterface>(sensorName, m_initDone);


    if (m_sensorInterface){
        if (!(QObject::connect(m_sensorInterface, SIGNAL(dataAvailable(const MagneticField&)),
                               this, SLOT(slotDataAvailable(const MagneticField&)))))
            qWarning() << "Unable to connect "<< sensorName;
    }
    else
        qWarning() << "Unable to initialize "<< sensorName;

    setReading<QMagnetometerReading>(&m_reading);

}

void maemo6magnetometer::start(){
    maemo6sensorbase::start();
    QVariant v = sensor()->property("returnGeoValues");
    m_isGeoMagnetometer =  v.isValid() && v.toBool()? true: false;
}

void maemo6magnetometer::slotDataAvailable(const MagneticField& data)
{

    if (m_isGeoMagnetometer){
        m_reading.setX( 0.0000003 * data.x() );
        m_reading.setY( 0.0000003 * data.y() );
        m_reading.setZ( 0.0000003 * data.z() );
        m_reading.setCalibrationLevel( ((float) data.level()) / 3.0 );
    } else {
        m_reading.setX( 0.0000003 * data.rx() );
        m_reading.setY( 0.0000003 * data.ry() );
        m_reading.setZ( 0.0000003 * data.rz() );
        m_reading.setCalibrationLevel(1);
    }
    m_reading.setTimestamp(data.timestamp());
    newReadingAvailable();
}

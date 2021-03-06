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

#include "maemo6sensorbase.h"


SensorManagerInterface* maemo6sensorbase::m_remoteSensorManager = 0;


const float maemo6sensorbase::GRAVITY_EARTH = 9.80665;
const float maemo6sensorbase::GRAVITY_EARTH_THOUSANDTH = 0.00980665;

maemo6sensorbase::maemo6sensorbase(QSensor *sensor)
    : QSensorBackend(sensor), m_sensorInterface(0)
{
    if (!m_remoteSensorManager)
        m_remoteSensorManager = &SensorManagerInterface::instance();
}

maemo6sensorbase::~maemo6sensorbase()
{
    if (m_sensorInterface) {
        stop();
        QObject::disconnect(m_sensorInterface);
        delete m_sensorInterface, m_sensorInterface = 0;
    }
}

void maemo6sensorbase::start()
{

    if (m_sensorInterface) {
        int dataRate = sensor()->dataRate();
        if (dataRate > 0) {
            int interval = 1000 / dataRate;
            // for testing maximum speed
            //interval = 1;
            //dataRate = 1000;
            qDebug() << "Setting data rate" << dataRate << "Hz (interval" << interval << "ms) for" << m_sensorInterface->id();
            m_sensorInterface->setInterval(interval);
        } else {
            qDebug() << "Data rate in don't care mode (interval" << m_sensorInterface->interval() << "ms) for" << m_sensorInterface->id();
        }
        int returnCode = m_sensorInterface->start().error().type();
        if (returnCode==0) return;
        qWarning()<<"m_sensorInterface did not start, error code:"<<returnCode;
    }
    sensorStopped();
}

void maemo6sensorbase::stop()
{
    if (m_sensorInterface)
        m_sensorInterface->stop();
}


void maemo6sensorbase::setRanges(qreal correctionFactor){
    int l = m_sensorInterface->getAvailableDataRanges().size();

    for (int i=0; i<l; i++){
        qreal rangeMin = ((DataRange)(m_sensorInterface->getAvailableDataRanges().at(i))).min * correctionFactor;
        qreal rangeMax =((DataRange)(m_sensorInterface->getAvailableDataRanges().at(i))).max * correctionFactor;
        qreal resolution = ((DataRange)(m_sensorInterface->getAvailableDataRanges().at(i))).resolution * correctionFactor;
        addOutputRange(rangeMin, rangeMax, resolution);
    }

}

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

#ifndef MAEMO6MAGNETOMETER_H
#define MAEMO6MAGNETOMETER_H

#include "maemo6sensorbase.h"
#include <qmagnetometer.h>

#include <magnetometersensor_i.h>
#include <magneticfield.h>

QTM_USE_NAMESPACE

class maemo6magnetometer : public maemo6sensorbase
{
    Q_OBJECT

public:
    static char const * const id;
    maemo6magnetometer(QSensor *sensor);

protected:
    virtual void start();

private:
    static const float NANO;
    QMagnetometerReading m_reading;
    static bool m_initDone;
    bool m_isGeoMagnetometer;

private slots:
    void slotDataAvailable(const MagneticField& data);
};

#endif

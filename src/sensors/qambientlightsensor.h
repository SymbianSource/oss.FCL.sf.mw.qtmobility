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

#ifndef QAMBIENTLIGHTSENSOR_H
#define QAMBIENTLIGHTSENSOR_H

#include "qsensor.h"

QTM_BEGIN_NAMESPACE

class QAmbientLightReadingPrivate;

class Q_SENSORS_EXPORT QAmbientLightReading : public QSensorReading
{
    Q_OBJECT
    Q_ENUMS(LightLevel)
    Q_PROPERTY(LightLevel lightLevel READ lightLevel)
    DECLARE_READING(QAmbientLightReading)
public:
    enum LightLevel {
        Undefined = 0,
        Dark,
        Twilight,
        Light,
        Bright,
        Sunny
    };

    LightLevel lightLevel() const;
    void setLightLevel(LightLevel lightLevel);
};

class Q_SENSORS_EXPORT QAmbientLightFilter : public QSensorFilter
{
public:
    virtual bool filter(QAmbientLightReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) { return filter(static_cast<QAmbientLightReading*>(reading)); }
};

class Q_SENSORS_EXPORT QAmbientLightSensor : public QSensor
{
    Q_OBJECT
public:
    explicit QAmbientLightSensor(QObject *parent = 0) : QSensor(QAmbientLightSensor::type, parent) {}
    virtual ~QAmbientLightSensor() {}
    QAmbientLightReading *reading() const { return static_cast<QAmbientLightReading*>(QSensor::reading()); }
    static char const * const type;
};

QTM_END_NAMESPACE

#endif


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

#ifndef ACCELEROMETERSYM_H
#define ACCELEROMETERSYM_H

// QT Mobility Sensor API headers
#include <qsensorbackend.h>
#include <qaccelerometer.h>

// Internal Headers
#include "sensorbackendsym.h"

// Sensor client headers
// Accelerometer Sensor specific header
#include <sensrvaccelerometersensor.h>

QTM_USE_NAMESPACE

class CAccelerometerSensorSym: public CSensorBackendSym
    {
public:
    /**
     * Factory function, this is used to create the accelerometer object
     * @return CAccelerometerSensorSym if successful, leaves on failure
     */
    static CAccelerometerSensorSym* NewL(QSensor *sensor);
    
    /**
     * Destructor
     * Closes the backend resources
     */
    ~CAccelerometerSensorSym();
    
    void start();
    
private:
    /**
     * Default constructor
     */
    CAccelerometerSensorSym(QSensor *sensor);
    
    /*
     * RecvData is used to retrieve the sensor reading from sensor server
     * It is implemented here to handle accelerometer sensor specific
     * reading data and provides conversion and utility code
     */  
    void RecvData(CSensrvChannel &aChannel);
    
    /**
     * Second phase constructor
     * Initialize the backend resources
     */
    void ConstructL();   
    
public:
    /**
     * Holds the id of the accelerometer
     */
    static char const * const id;
    
private:
    QAccelerometerReading iReading;
    TSensrvAccelerometerAxisData iData;
    TInt iScaleRange, iUnit;
    };

#endif //ACCELEROMETERSYM_H

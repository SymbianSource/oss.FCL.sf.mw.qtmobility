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

#ifndef QGEOSATELLITEINFOSOURCE60_H_
#define QGEOSATELLITEINFOSOURCE60_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <lbs.h>
#include "qgeosatelliteinfosource.h"
#include "notificationsatellitecallback_s60_p.h"

#define MAX_SIZE 5

QTM_BEGIN_NAMESPACE

// CLASS DECLARATION
//forward declaration
class CQMLBackendAO;

class CSatMethodInfo
{
public:
    // A unique id for the positioning module
    TPositionModuleId  mUid;

    // current status of the module
    int  mStatus;

    // time to first and next fix from the location server
    TTimeIntervalMicroSeconds  mTimeToFirstFix;
    TTimeIntervalMicroSeconds  mTimeToNextFix;

    // Accuracy
    double mHorizontalAccuracy;

    // Flags whether the positioning technology is currently available or not.
    bool mIsAvailable;
};


/**
 *  CQGeoSatelliteInfoSourceS60
 *
 */
class CQGeoSatelliteInfoSourceS60 : public INotificationSatelliteCallback,
            public QGeoSatelliteInfoSource

{
public:
    // Constructors and destructor
    /**
     * Destructor.
     */
    ~CQGeoSatelliteInfoSourceS60();

    /**
     * Two-phased constructor.
     */
    static CQGeoSatelliteInfoSourceS60* NewL(QObject* aParent);

    /**
     * Two-phased constructor.
     */
    static CQGeoSatelliteInfoSourceS60* NewLC(QObject* aParent);

    /**
     * Notification methods from active object.
     * Notifies device status, position value, and status
     */
    void updateDeviceStatus(void) ;

    /**
     * Update the position info
     */
    void updatePosition(TPositionSatelliteInfo aPosSatInfo, int aError , bool isStartUpdate);


    /**
     * Gets the handle of the PositionServer
     */
    RPositionServer& getPositionServer() {
        return mPositionServer;
    }

    int minimumUpdateInterval() const;

    /*
     * checks whether the object is valid
     */
    inline TBool isValid() {
        if (mDevStatusUpdateAO && mRegUpdateAO)
            return TRUE;
        else
            return FALSE;
    }

    inline TPositionModuleId getCurrentPositonModuleID() {
        return mCurrentModuleId;
    }

    inline TPositionModuleId getRequestUpdateModuleID() {
        return mReqModuleId;
    }

public slots :
    // for request update
    void requestUpdate(int timeout = 5000);

    // starts the regular updates
    virtual void startUpdates();

    // stops the regular updates
    virtual void stopUpdates();


private:

    /**
     * Constructor for performing 1st stage construction
     */
    CQGeoSatelliteInfoSourceS60(QObject* aParent = 0);

    /**
     * EPOC default constructor for performing 2nd stage construction
     */
    void ConstructL();

    void updateStatus(TPositionModuleInfo aModInfo, TInt aStatus);

    //get the index of the module in the List array
    TInt checkModule(TPositionModuleId aId) const;

    //get the index of the position module based on the preferred methods
    TInt getIndexPositionModule(TUint8 aBits) const;

    //get the more accuarte method with time to first fix < than timeout
    TInt getMoreAccurateMethod(TInt aTimeout, TUint8 aBits);

    void TPositionSatelliteInfo2QGeoSatelliteInfo(TPositionSatelliteInfo aSatInfo,
            QList<QGeoSatelliteInfo> &qListSatInView,
            QList<QGeoSatelliteInfo> &qListSatInUse);


protected:
    void connectNotify(const char *aSignal);

    void disconnectNotify(const char *aSignal);

private:
    /**
    *  current module ID
    */
    TPositionModuleId mCurrentModuleId;

    /**
    *prvmoduleID
    */
    TPositionModuleId mReqModuleId;

    /**
    * Active object for device status updates
    */
    CQMLBackendAO * mDevStatusUpdateAO;

    /**
    * Active object for requestUpdate
    */
    CQMLBackendAO * mReqUpdateAO;

    /**
    * Active object for regular updates.
    */
    CQMLBackendAO * mRegUpdateAO;

    /**
     * Positioner server
     */
    RPositionServer mPositionServer;


    /*
     * Number of Satellites in View
     */
    QList<QGeoSatelliteInfo> mqListSatInView;

    /*
     * Numberof Satellites in Use
     */
    QList<QGeoSatelliteInfo> mqListSatInUse;

    /**
    *  list of supported position methods
    */
    CSatMethodInfo mList[MAX_SIZE];

    /**
     * maintaiss the size of thr CPosMethodInfo array
     */
    int mListSize;

    /*
     * query for the status
     */
    TPositionModuleStatusEvent  mStatusEvent;

    /*
     * maintain the startUpdates status
     */
    TBool mStartUpdates;

    /*
     * flags for the modules
     */
    TUint8  mModuleFlags;
};

QTM_END_NAMESPACE

#endif /* QGEOSATELLITEINFOSOURCE60_H_ */

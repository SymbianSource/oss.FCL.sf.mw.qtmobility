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

#include "qnmeapositioninfosourceproxyfactory.h"
#include "../testqgeopositioninfosource_p.h"
#include "../qlocationtestutils_p.h"

#include <qnmeapositioninfosource.h>

#include <QTest>
#include <QDir>
#include <QDebug>
#include <QBuffer>
#include <QSignalSpy>
#include <QMetaType>
#include <QFile>
#include <QTemporaryFile>
#include <QHash>
#include <QTimer>

QTM_USE_NAMESPACE
Q_DECLARE_METATYPE(QNmeaPositionInfoSource::UpdateMode)
Q_DECLARE_METATYPE(QGeoPositionInfo)
Q_DECLARE_METATYPE(QList<QDateTime>)

class tst_QNmeaPositionInfoSource : public QObject
{
    Q_OBJECT

public:
    enum UpdateTriggerMethod
    {
        StartUpdatesMethod,
        RequestUpdatesMethod
    };

    tst_QNmeaPositionInfoSource(QNmeaPositionInfoSource::UpdateMode mode, QObject *parent = 0);

private:
    QList<QDateTime> createDateTimes(int count) const
    {
        QList<QDateTime> dateTimes;
        QDateTime dt = QDateTime::currentDateTime().toUTC();
        int interval = 100;
        for (int i=0; i<count; i++) {
            dateTimes << dt.addMSecs(interval);
            interval += 100;
        }
        return dateTimes;
    }

private slots:
    void initTestCase();

    void constructor();

    void supportedPositioningMethods();

    void minimumUpdateInterval();

    void setUpdateInterval_delayedUpdate();

    void lastKnownPosition();

    void beginWithBufferedData();
    void beginWithBufferedData_data();

    void startUpdates();
    void startUpdates_data();

    void startUpdates_withTimeout();

    void startUpdates_expectLatestUpdateOnly();

    void startUpdates_waitForValidDateTime();
    void startUpdates_waitForValidDateTime_data();

    void requestUpdate_waitForValidDateTime();
    void requestUpdate_waitForValidDateTime_data();

    void requestUpdate();
    void requestUpdate_after_start();

    void testWithBadNmea();
    void testWithBadNmea_data();

private:
    QNmeaPositionInfoSource::UpdateMode m_mode;
};

Q_DECLARE_METATYPE(tst_QNmeaPositionInfoSource::UpdateTriggerMethod)

//---------------------------------------------------

class Feeder : public QObject
{
    Q_OBJECT

public:
    Feeder(QObject *parent)
        : QObject(parent)
    {
    }

    void start(QNmeaPositionInfoSourceProxy *proxy)
    {
        m_proxy = proxy;
        QTimer *timer = new QTimer;
        QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
        timer->setInterval(proxy->source()->minimumUpdateInterval()*2);
        timer->start();
    }

public slots:
    void timeout()
    {
        m_proxy->feedBytes(QLocationTestUtils::createRmcSentence(QDateTime::currentDateTime()).toAscii());
    }

private:
    QNmeaPositionInfoSourceProxy *m_proxy;
};

//---------------------------------------------------


class UnlimitedNmeaStream : public QIODevice
{
    Q_OBJECT

public:
    UnlimitedNmeaStream(QObject *parent) : QIODevice(parent) {}

protected:
    qint64 readData(char *data, qint64 maxSize)
    {
        QByteArray bytes = QLocationTestUtils::createRmcSentence(QDateTime::currentDateTime()).toLatin1();
        qint64 sz = qMin(qint64(bytes.size()), maxSize);
        memcpy(data, bytes.constData(), sz);
        return sz;
    }

    qint64 writeData(const char *, qint64)
    {
        return -1;
    }

    qint64 bytesAvailable() const
    {
        return 1024 + QIODevice::bytesAvailable();
    }
};

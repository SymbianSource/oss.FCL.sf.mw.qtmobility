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

#ifndef QXAMEDIARECORDERCONTROL_H
#define QXAMEDIARECORDERCONTROL_H

#include <qmediarecorder.h>
#include <qmediarecordercontrol.h>

QT_USE_NAMESPACE

/*
 * This class implements QMediaRecorderControl interface.
 */

class QXARecordSession;

class QXAMediaRecoderControl : public QMediaRecorderControl
{
    Q_OBJECT
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)

public:
    QXAMediaRecoderControl(QXARecordSession *session, QObject *parent = 0);
    virtual ~QXAMediaRecoderControl();

    QUrl outputLocation() const;
    bool setOutputLocation(const QUrl &location);

    QMediaRecorder::State state() const;

    qint64 duration() const;
    bool isMuted() const;
    void applySettings();

Q_SIGNALS:
    void durationChanged(qint64 position);

public Q_SLOTS:
    void record();
    void pause();
    void stop();
    void setMuted(bool);

private:
    QXARecordSession *m_session;
};

#endif /* QXAMEDIARECORDERCONTROL_H */

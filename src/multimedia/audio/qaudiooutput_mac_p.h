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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QAUDIOOUTPUT_MAC_P_H
#define QAUDIOOUTPUT_MAC_P_H

#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

#include <QtCore/qobject.h>
#include <QtCore/qmutex.h>
#include <QtCore/qwaitcondition.h>
#include <QtCore/qtimer.h>
#include <QtCore/qatomic.h>

#include <qaudio.h>
#include <qaudioformat.h>
#include <qaudiosystem.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QIODevice;

namespace QtMultimediaKitInternal
{
class QAudioOutputBuffer;
}

class QAudioOutputPrivate : public QAbstractAudioOutput
{
    Q_OBJECT

public:
    bool            isOpen;
    int             internalBufferSize;
    int             periodSizeBytes;
    qint64          totalFrames;
    QAudioFormat    audioFormat;
    QIODevice*      audioIO;
    AudioDeviceID   audioDeviceId;
    AudioUnit       audioUnit;
    Float64         clockFrequency;
    UInt64          startTime;
    AudioStreamBasicDescription deviceFormat;
    AudioStreamBasicDescription streamFormat;
    QtMultimediaKitInternal::QAudioOutputBuffer*   audioBuffer;
    QAtomicInt      audioThreadState;
    QWaitCondition  threadFinished;
    QMutex          mutex;
    QTimer*         intervalTimer;

    QAudio::Error    errorCode;
    QAudio::State    stateCode;

    QAudioOutputPrivate(const QByteArray& device);
    ~QAudioOutputPrivate();

    bool open();
    void close();

    QAudioFormat format() const;
    void setFormat(const QAudioFormat& fmt);

    QIODevice* start();
    void start(QIODevice* device);
    void stop();
    void reset();
    void suspend();
    void resume();

    int bytesFree() const;
    int periodSize() const;

    void setBufferSize(int value);
    int bufferSize() const;

    void setNotifyInterval(int milliSeconds);
    int notifyInterval() const;

    qint64 processedUSecs() const;
    qint64 elapsedUSecs() const;

    QAudio::Error error() const;
    QAudio::State state() const;

    void audioThreadStart();
    void audioThreadStop();
    void audioThreadDrain();

    void audioDeviceStop();
    void audioDeviceIdle();
    void audioDeviceError();

    void startTimers();
    void stopTimers();

private slots:
    void deviceStopped();
    void inputReady();

private:
    enum { Running, Draining, Stopped };

    static OSStatus renderCallback(void* inRefCon,
                                    AudioUnitRenderActionFlags* ioActionFlags,
                                    const AudioTimeStamp* inTimeStamp,
                                    UInt32 inBusNumber,
                                    UInt32 inNumberFrames,
                                    AudioBufferList* ioData);
};

QT_END_NAMESPACE

QT_END_HEADER

#endif

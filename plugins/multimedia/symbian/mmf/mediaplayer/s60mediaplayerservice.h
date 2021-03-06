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

#ifndef S60VIDEOPLAYERSERVICE_H
#define S60VIDEOPLAYERSERVICE_H

#include <QtCore/qobject.h>
#include <qmediaservice.h>

#include "ms60mediaplayerresolver.h"
#include "s60mediaplayeraudioendpointselector.h"

QT_BEGIN_NAMESPACE
class QMediaMetaData;
class QMediaPlayerControl;
class QMediaPlaylist;
QT_END_NAMESPACE

QT_USE_NAMESPACE

class S60VideoPlayerSession;
class S60AudioPlayerSession;
class S60MediaPlayerControl;
class S60MediaMetaDataProvider;
class S60VideoWidgetControl;
class S60MediaRecognizer;
class S60VideoRenderer;
class S60VideoOverlay;
class QMediaPlaylistNavigator;

class S60MediaPlayerService : public QMediaService, public MS60MediaPlayerResolver
{
    Q_OBJECT

public:

    S60MediaPlayerService(QObject *parent = 0);
    ~S60MediaPlayerService();

    QMediaControl *requestControl(const char *name);
    void releaseControl(QMediaControl *control);

protected: // From MS60MediaPlayerResolver
    S60MediaPlayerSession* PlayerSession();
    S60MediaPlayerSession* VideoPlayerSession();
    S60MediaPlayerSession* AudioPlayerSession();

private:
    S60MediaPlayerControl *m_control;
    S60VideoPlayerSession *m_videoPlayerSession;
    S60AudioPlayerSession *m_audioPlayerSession;
    S60MediaMetaDataProvider *m_metaData;
    S60MediaPlayerAudioEndpointSelector *m_audioEndpointSelector;
    QMediaControl *m_videoWidget;
    QMediaControl *m_videoWindow;
    QMediaControl *m_videoRenderer;
    QMediaControl *m_videoOutput;
};

#endif

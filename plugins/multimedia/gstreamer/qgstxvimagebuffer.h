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

#ifndef QGSTXVIMAGEBUFFER_H
#define QGSTXVIMAGEBUFFER_H

#include <qabstractvideobuffer.h>
#include <qvideosurfaceformat.h>
#include <QtCore/qmutex.h>
#include <QtCore/qwaitcondition.h>
#include <QtCore/qqueue.h>

#ifndef QT_NO_XVIDEO

#include <X11/Xlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>


#include <gst/gst.h>
class QGstXvImageBufferPool;

struct QGstXvImageBuffer {
    GstBuffer buffer;
    QGstXvImageBufferPool *pool;
    XvImage *xvImage;
    XShmSegmentInfo shmInfo;
    bool markedForDeletion;

    static GType get_type(void);
    static void class_init(gpointer g_class, gpointer class_data);
    static void buffer_init(QGstXvImageBuffer *xvimage, gpointer g_class);
    static void buffer_finalize(QGstXvImageBuffer * xvimage);
    static GstBufferClass *parent_class;
};

Q_DECLARE_METATYPE(XvImage*)

class QGstXvImageBufferPool : public QObject {
Q_OBJECT
friend class QGstXvImageBuffer;
public:
    QGstXvImageBufferPool(QObject *parent = 0);
    virtual ~QGstXvImageBufferPool();

    bool isFormatSupported(const QVideoSurfaceFormat &format);

    QGstXvImageBuffer *takeBuffer(const QVideoSurfaceFormat &format, GstCaps *caps);
    void clear();

private slots:
    void queuedAlloc();
    void queuedDestroy();

    void doClear();

    void recycleBuffer(QGstXvImageBuffer *);
    void destroyBuffer(QGstXvImageBuffer *);

private:
    struct XvShmImage {
        XvImage *xvImage;
        XShmSegmentInfo shmInfo;
    };

    QMutex m_poolMutex;
    QMutex m_allocMutex;
    QWaitCondition m_allocWaitCondition;
    QMutex m_destroyMutex;
    QVideoSurfaceFormat m_format;
    GstCaps *m_caps;
    QList<QGstXvImageBuffer*> m_pool;
    QList<QGstXvImageBuffer*> m_allBuffers;
    QList<XvShmImage> m_imagesToDestroy;
};

#endif //QT_NO_XVIDEO

#endif

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

#ifndef QMALLOCPOOL_H
#define QMALLOCPOOL_H

#include "qmobilityglobal.h"
#include <cstdlib>
#include <QString>

QTM_BEGIN_NAMESPACE

class QMallocPoolPrivate;

class Q_AUTOTEST_EXPORT QMallocPool
{
public:
    enum PoolType { Owned, NewShared, Shared };
    QMallocPool();
    QMallocPool(void * poolBase, unsigned int poolLength,
                PoolType type = Owned, const QString& name = QString());
    ~QMallocPool();

    size_t size_of(void *);
    void *calloc(size_t nmemb, size_t size);
    void *malloc(size_t size);
    void free(void *ptr);
    void *realloc(void *ptr, size_t size);

    bool isValid() const;

    struct MemoryStats {
        unsigned long poolSize;
        unsigned long maxSystemBytes;
        unsigned long systemBytes;
        unsigned long inuseBytes;
        unsigned long keepCost;
    };
    MemoryStats memoryStatistics() const;

private:
    Q_DISABLE_COPY(QMallocPool)
    QMallocPoolPrivate * d;
};

QTM_END_NAMESPACE

#endif

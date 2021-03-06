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
#include "qmessageaccount.h"
#include "qmessageaccount_p.h"
#include "qmessagemanager.h"
#include "qmtmengine_symbian_p.h"
#ifdef FREESTYLEMAILUSED
#include "qfsengine_symbian_p.h"
#endif

QTM_BEGIN_NAMESPACE

QMessageAccount QMessageAccountPrivate::from(const QMessageAccountId &id, const QString &name, long int service1EntryId, long int service2EntryId, const QMessage::TypeFlags &types)
{
    QMessageAccount result;
    result.d_ptr->_id = id;
    result.d_ptr->_name = name;
    result.d_ptr->_service1EntryId = service1EntryId;
    result.d_ptr->_service2EntryId = service2EntryId;
    result.d_ptr->_types = types;
    return result;
}

QMessageAccount::QMessageAccount()
 : d_ptr(new QMessageAccountPrivate(this))
{
}

QMessageAccount::QMessageAccount(const QMessageAccountId &id)
 : d_ptr(new QMessageAccountPrivate(this))
{
    *this = QMessageManager().account(id);
}

QMessageAccount::QMessageAccount(const QMessageAccount &other)
 : d_ptr(new QMessageAccountPrivate(this))
{
    this->operator=(other);
}

QMessageAccount& QMessageAccount::operator=(const QMessageAccount& other)
{
    if (&other != this) {
        *d_ptr = *other.d_ptr;
    }
    
    return *this;
}

QMessageAccount::~QMessageAccount()
{
    delete d_ptr;
    d_ptr = 0;
}

QMessageAccountId QMessageAccount::id() const
{
    return d_ptr->_id;
}

QString QMessageAccount::name() const
{
    return d_ptr->_name;
}

QMessage::TypeFlags QMessageAccount::messageTypes() const
{
    return d_ptr->_types;
}

QMessageAccountId QMessageAccount::defaultAccount(QMessage::Type type)
{
#ifdef FREESTYLEMAILUSED
    if (type == QMessage::Email) {
        QMessageAccountId id = CFSEngine::instance()->defaultAccount(type);
        if (id.isValid())
            return id;
        else return CMTMEngine::instance()->defaultAccount(type);
    }
#endif // FREESTYLEMAILUSED    
    return CMTMEngine::instance()->defaultAccount(type);
}

QTM_END_NAMESPACE

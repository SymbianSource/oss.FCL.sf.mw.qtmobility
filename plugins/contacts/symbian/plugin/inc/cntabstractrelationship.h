/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef CNTABSTRACTRELATIONSHIP_H
#define CNTABSTRACTRELATIONSHIP_H

#include <QSet>

#include <qtcontacts.h>
#include <cntdb.h>

QTM_USE_NAMESPACE

class CntAbstractRelationship
{
public: 
    explicit CntAbstractRelationship(CContactDatabase *contactDatabase, const QString &managerUri, const QString &relationshipType); 
    virtual ~CntAbstractRelationship();
    
public:
    virtual QList<QContactRelationship> relationshipsL(const QContactId &participantId, QContactRelationship::Role role, QContactManager::Error *error) = 0;
    virtual bool saveRelationshipL(QSet<QContactLocalId> *affectedContactIds, QContactRelationship *relationship, QContactManager::Error *error) = 0;
    virtual bool removeRelationshipL(QSet<QContactLocalId> *affectedContactIds, const QContactRelationship &relationship, QContactManager::Error *error) = 0;
#ifdef SYMBIAN_BACKEND_USE_SQLITE
    virtual bool saveRelationshipsL(QSet<QContactLocalId> *affectedContactIds, QList<QContactRelationship> *relationships, QContactManager::Error *error) = 0;
    virtual bool removeRelationshipsL(QSet<QContactLocalId> *affectedContactIds, const QList<QContactRelationship> &relationships, QContactManager::Error *error) = 0;
#endif
    virtual bool validateRelationship(const QContactRelationship &relationship, QContactManager::Error *error) = 0;
    QString relationshipType() const;

protected:
    CContactDatabase *database();
    QString managerUri();
    
private:
    CContactDatabase *m_contactDatabase;
    QString m_managerUri;
    QString m_relationshipType;
    
};

#endif

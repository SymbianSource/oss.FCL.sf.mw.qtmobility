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

#include "qcontactlocalidfetchrequest.h"
#include "qcontactrequests_p.h"

QTM_BEGIN_NAMESPACE

/*!
  \class QContactLocalIdFetchRequest
  \brief The QContactLocalIdFetchRequest class allows a client to asynchronously
    request a list of contact ids from a contacts store manager.


  For a QContactLocalIdFetchRequest, the resultsAvailable() signal will be emitted when the resultant
  manager-local contact ids (which may be retrieved by calling ids()), are updated, as well as if
  the overall operation error (which may be retrieved by calling error()) is updated.

  Please see the class documentation of QContactAbstractRequest for more information about
  the usage of request classes and ownership semantics.

  \ingroup contacts-requests
 */

/*! Constructs a new contact id fetch request whose parent is the specified \a parent */
QContactLocalIdFetchRequest::QContactLocalIdFetchRequest(QObject* parent)
    : QContactAbstractRequest(new QContactLocalIdFetchRequestPrivate, parent)
{
}

/*! Frees any memory used by this request */
QContactLocalIdFetchRequest::~QContactLocalIdFetchRequest()
{
    QContactAbstractRequestPrivate::notifyEngine(this);
}

/*! Sets the filter which will be used to select the contacts whose ids will be returned to \a filter */
void QContactLocalIdFetchRequest::setFilter(const QContactFilter& filter)
{
    Q_D(QContactLocalIdFetchRequest);
    QMutexLocker ml(&d->m_mutex);
    d->m_filter = filter;
}

/*! Sets the future sort ordering of the result of the request to \a sorting.  This function only has
    effect on the result if called prior to calling \c start() */
void QContactLocalIdFetchRequest::setSorting(const QList<QContactSortOrder>& sorting)
{
    Q_D(QContactLocalIdFetchRequest);
    QMutexLocker ml(&d->m_mutex);
    d->m_sorting = sorting;
}

/*! Returns the filter which will be used to select the contacts whose ids will be returned */
QContactFilter QContactLocalIdFetchRequest::filter() const
{
    Q_D(const QContactLocalIdFetchRequest);
    QMutexLocker ml(&d->m_mutex);
    return d->m_filter;
}

/*! Returns the sort ordering which will be used to sort the result */
QList<QContactSortOrder> QContactLocalIdFetchRequest::sorting() const
{
    Q_D(const QContactLocalIdFetchRequest);
    QMutexLocker ml(&d->m_mutex);
    return d->m_sorting;
}

/*! Returns the list of ids of contacts which matched the request */
QList<QContactLocalId> QContactLocalIdFetchRequest::ids() const
{
    Q_D(const QContactLocalIdFetchRequest);
    QMutexLocker ml(&d->m_mutex);
    return d->m_ids;
}

#include "moc_qcontactlocalidfetchrequest.cpp"

QTM_END_NAMESPACE

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

#ifndef QNETWORKSESSION_H
#define QNETWORKSESSION_H

#include "qmobilityglobal.h"

#include <QtCore/qobject.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>
#include <QtNetwork/qnetworkinterface.h>
#include <QtCore/qvariant.h>
#include "qnetworkconfiguration.h"


QT_BEGIN_HEADER

QTM_BEGIN_NAMESPACE

class QNetworkSessionPrivate;
class Q_BEARER_EXPORT QNetworkSession : public QObject
{
    Q_OBJECT
public:
    enum State {
        Invalid = 0,
        NotAvailable,
        Connecting,
        Connected,
        Closing,
        Disconnected,
        Roaming
    };

    enum SessionError {
        UnknownSessionError = 0,
        SessionAbortedError,
        RoamingError,
        OperationNotSupportedError,
        InvalidConfigurationError
    };

    explicit QNetworkSession(const QNetworkConfiguration& connConfig, QObject* parent =0);
    virtual ~QNetworkSession();

    bool isOpen() const;
    QNetworkConfiguration configuration() const;
    QNetworkInterface interface() const;

    State state() const;
    SessionError error() const;
    QString errorString() const;
    QVariant sessionProperty(const QString& key) const;
    void setSessionProperty(const QString& key, const QVariant& value);

    quint64 bytesWritten() const;
    quint64 bytesReceived() const;
    quint64 activeTime() const;
    
    bool waitForOpened(int msecs = 30000);

public Q_SLOTS:
    void open();
    void close();
    void stop();

    //roaming related slots
    void migrate();
    void ignore();
    void accept();
    void reject();


Q_SIGNALS:
    void stateChanged(QNetworkSession::State);
    void opened();
    void closed();
    void error(QNetworkSession::SessionError);
    void preferredConfigurationChanged(const QNetworkConfiguration& config, bool isSeamless);
    void newConfigurationActivated();

protected:
    virtual void connectNotify(const char *signal);
    virtual void disconnectNotify(const char *signal);

private:
    QNetworkSessionPrivate* d;
    friend class QNetworkSessionPrivate;
    };

QTM_END_NAMESPACE

QT_END_HEADER

#endif //QNETWORKSESSION_H

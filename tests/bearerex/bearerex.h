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
#ifndef ACCESSPOINTMANAGEREX_H
#define ACCESSPOINTMANAGEREX_H

#include <QtGui>

#include "ui_detailedinfodialog.h"

#if defined(Q_WS_MAEMO_5) || defined(Q_WS_MAEMO_6)
#include "ui_bearerex_maemo.h"
#include "ui_sessiondialog_maemo.h"
#else
#include "ui_bearerex.h"
#include "ui_sessiondialog.h"
#endif
#include "qnetworkconfigmanager.h"
#include "qnetworksession.h"
#include "datatransferer.h"
#include "xqlistwidget.h"

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
QT_END_NAMESPACE

class SessionTab;
class DataTransferer;

QTM_USE_NAMESPACE

class BearerEx : public QMainWindow, public Ui::BearerExMainWindow
{
     Q_OBJECT

public:
    BearerEx(QWidget* parent = 0);
    void createMenus();
    void showConfigurations();

private Q_SLOTS:
    void on_updateConfigurationsButton_clicked();
    void on_updateListButton_clicked();
    void on_showDetailsButton_clicked();
    void on_createSessionButton_clicked();
    void on_clearEventListButton_clicked();

    void configurationsUpdateCompleted();
    void configurationAdded(const QNetworkConfiguration& config);
    void configurationRemoved(const QNetworkConfiguration& config);
    void onlineStateChanged(bool isOnline);
    void configurationChanged(const QNetworkConfiguration & config);     

private:
    QNetworkConfigurationManager m_NetworkConfigurationManager;
    QAction* m_openAction;
};

class DetailedInfoDialog : public QDialog, public Ui::DetailedInfoDialog
{
    Q_OBJECT

public:
    DetailedInfoDialog(QNetworkConfiguration* apNetworkConfiguration = 0, QWidget* parent = 0);
};


class SessionTab : public QWidget, public Ui::SessionTab
{
    Q_OBJECT

public:
    SessionTab(QNetworkConfiguration* apNetworkConfiguration = 0, QNetworkConfigurationManager* configManager = 0,
               QListWidget* eventListWidget = 0,  int index = 0, BearerEx* parent = 0);
    ~SessionTab();
    
    QString stateString(QNetworkSession::State state);    

private Q_SLOTS:
    void on_createQNetworkAccessManagerButton_clicked();
    void on_sendRequestButton_clicked();
    void on_openSessionButton_clicked();
    void on_closeSessionButton_clicked();
    void on_stopConnectionButton_clicked();
    void on_deleteSessionButton_clicked();
    void on_dataObjectChanged(const QString& newObjectType);
    void on_alrButton_clicked();
    void finished(quint32 errorCode, qint64 dataReceived, QString errorType);
    
    void newConfigurationActivated();
    void preferredConfigurationChanged(const QNetworkConfiguration& config, bool isSeamless);
    void stateChanged(QNetworkSession::State state);
    void newState(QNetworkSession::State state);
    void opened();
    void closed();
    void error(QNetworkSession::SessionError error);

private: //data
    // QNetworkAccessManager* m_networkAccessManager;
    DataTransferer* m_dataTransferer;
    QNetworkSession* m_NetworkSession;
    QNetworkConfigurationManager* m_ConfigManager;
    QListWidget* m_eventListWidget;
    QNetworkConfiguration m_config;
    int m_index;
    bool m_dataTransferOngoing;
    bool m_alrEnabled;
};

#endif // ACCESSPOINTMANAGEREX_H

// End of file


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

#include "cntsymbiansimengine.h"
#include "cntsymbiansimtransformerror.h"
#include "cntsimstore.h"
#include "cntsimcontactfetchrequest.h"
#include "cntsimcontactlocalidfetchrequest.h"
#include "cntsimcontactsaverequest.h"
#include "cntsimcontactremoverequest.h"
#include "cntsimdetaildefinitionfetchrequest.h"
#include <qtcontacts.h>

#include <QEventLoop>
#include <QTimer>
#include <QDebug>

#include <centralrepository.h>

// Telephony Configuration API
// Keys under this category are used in defining telephony configuration.
const TUid KCRUidTelConfiguration = {0x102828B8};
// Amount of digits to be used in contact matching.
// This allows a customer to variate the amount of digits to be matched.
const TUint32 KTelMatchDigits                               = 0x00000001;
// Default match length
const TInt KDefaultMatchLength(7);


CntSymbianSimEngineData::CntSymbianSimEngineData()
    :m_simStore(0)
{
    
}

CntSymbianSimEngineData::~CntSymbianSimEngineData()
{
    if (ref == 0) {
        // Remove all unfinished requests.
        // If the client has behaved itself then there should be none left.
        while (m_asyncRequests.size()) {
            QMap<QContactAbstractRequest *, CntAbstractSimRequest *>::iterator itr = m_asyncRequests.begin();
            delete itr.value();
            m_asyncRequests.remove(itr.key());
            qWarning("Dangling async request!");
        }        
    }
}

CntSymbianSimEngine::CntSymbianSimEngine(const QMap<QString, QString>& parameters, QContactManager::Error* error)
{
    *error = QContactManager::NoError;

    d = new CntSymbianSimEngineData();
    d->m_simStore = new CntSimStore(this, parameters.value(KParameterKeySimStoreName), error);
    if (*error != QContactManager::NoError) {
        //qDebug() << "Failed to open SIM store" << error;
        return;
    }
    
    // Get phone number match length from cenrep
    d->m_phoneNumberMatchLen = KDefaultMatchLength;
    TRAP_IGNORE(getMatchLengthL(d->m_phoneNumberMatchLen)); // ignore error and use default value

    if(d->m_simStore->storeInfo().m_storeName == KParameterValueSimStoreNameSdn) {
        // In case of SDN store we need to check if any SDN contacts exist to
        // determine if the store is supported or not
        if(d->m_simStore->storeInfo().m_usedEntries == 0)
            *error = QContactManager::NotSupportedError;
    }    
}

CntSymbianSimEngine::CntSymbianSimEngine(const CntSymbianSimEngine &other)
    :d(other.d)
{
    
}

CntSymbianSimEngine::~CntSymbianSimEngine()
{

}

QString CntSymbianSimEngine::managerName() const
{
    return CNT_SYMBIANSIM_MANAGER_NAME;
}

/*!
 * Returns a list of the ids of contacts that match the supplied \a filter, sorted according to the given \a sortOrders.
 * Any error that occurs will be stored in \a error. Uses the generic (slow) filtering of QContactManagerEngine.
 */
QList<QContactLocalId> CntSymbianSimEngine::contactIds(const QContactFilter& filter, const QList<QContactSortOrder>& sortOrders, QContactManager::Error* error) const
{
    QContactLocalIdFetchRequest req;
    req.setFilter(filter);
    req.setSorting(sortOrders);
    executeRequest(&req, error);
    return req.ids();
}

QList<QContact> CntSymbianSimEngine::contacts(const QContactFilter& filter, const QList<QContactSortOrder>& sortOrders, const QContactFetchHint& fetchHint, QContactManager::Error* error) const
{
    QContactFetchRequest req;
    req.setFilter(filter);
    req.setSorting(sortOrders);
    req.setFetchHint(fetchHint);
    executeRequest(&req, error);
    return req.contacts();
}

/*!
 * Reads a contact from the Etel store.
 *
 * \param contactId The Id of the contact to be retrieved.
 * \param definitionRestrictions Definition restrictions.
 * \param error Qt error code.
 * \return A QContact for the requested QContactLocalId value or 0 if the read
 *  operation was unsuccessful (e.g. contact not found).
 */
QContact CntSymbianSimEngine::contact(const QContactLocalId& contactId, const QContactFetchHint& fetchHint, QContactManager::Error* error) const
{
    QContactFetchRequest req;
    QContactLocalIdFilter filter;
    filter.setIds(QList<QContactLocalId>() << contactId);
    req.setFilter(filter);
    req.setFetchHint(fetchHint);
    executeRequest(&req, error);
    if (req.contacts().count() == 0)
        return QContact();
    return req.contacts().at(0); 
}

QString CntSymbianSimEngine::synthesizedDisplayLabel(const QContact& contact, QContactManager::Error* error) const
{
    Q_UNUSED(error);

    QContactName name = contact.detail(QContactName::DefinitionName);
    if(!name.customLabel().isEmpty()) {
        return name.customLabel();
    } else {
        return QString("");
    }
}

/*!
 * Saves the contacts to the Etel store. Only part of the contact's details
 * can be saved, and some fields may be trimmed to fit to the SIM card.
 *
 * \param contacts Contact to be saved.
 * \param qtError Qt error code.
 * \return Error status.
 */
bool CntSymbianSimEngine::saveContacts(QList<QContact>* contacts, QMap<int, QContactManager::Error>* errorMap, QContactManager::Error* error)
{
    if (!contacts) {
        *error = QContactManager::BadArgumentError;
        return false;
    }
    
    QContactSaveRequest req;
    req.setContacts(*contacts);
    executeRequest(&req, error);
    if (errorMap)
        *errorMap = req.errorMap();
    *contacts = req.contacts();
    return (*error == QContactManager::NoError );
}

/*!
 * Removes the specified contact object from the Etel store.
 *
 * \param contactId Id of the contact to be removed.
 * \param qtError Qt error code.
 * \return Error status.
 */
bool CntSymbianSimEngine::removeContacts(const QList<QContactLocalId>& contactIds, QMap<int, QContactManager::Error>* errorMap, QContactManager::Error* error)
{
    QContactRemoveRequest req;
    req.setContactIds(contactIds);
    executeRequest(&req, error);
    if (errorMap)
        *errorMap = req.errorMap();    
    return (*error == QContactManager::NoError);
}

/*!
 * Returns a map of identifier to detail definition which are valid for contacts whose type is the given \a contactType
 * which are valid for the contacts in this store
 */
QMap<QString, QContactDetailDefinition> CntSymbianSimEngine::detailDefinitions(const QString& contactType, QContactManager::Error* error) const
{
    if (!supportedContactTypes().contains(contactType)) {
        // Should never happen
        *error = QContactManager::NotSupportedError;
        return QMap<QString, QContactDetailDefinition>();
    }

    // Get store information
    SimStoreInfo storeInfo = d->m_simStore->storeInfo();

    // the map we will eventually return
    QMap<QString, QContactDetailDefinition> retn;

    // local variables for reuse
    QMap<QString, QContactDetailFieldDefinition> fields;
    QContactDetailFieldDefinition f;
    QContactDetailDefinition def;
    QVariantList subTypes;

    // sync target
    def.setName(QContactSyncTarget::DefinitionName);
    fields.clear();
    f.setDataType(QVariant::String);
    subTypes.clear();
    subTypes << QString(QLatin1String(KSimSyncTarget));
    f.setAllowableValues(subTypes);
    fields.insert(QContactSyncTarget::FieldSyncTarget, f);
    def.setFields(fields);
    def.setUnique(true);
    retn.insert(def.name(), def);

    // type
    def.setName(QContactType::DefinitionName);
    fields.clear();
    f.setDataType(QVariant::String);
    subTypes.clear();
    // groups are not supported
    subTypes << QString(QLatin1String(QContactType::TypeContact));
    f.setAllowableValues(subTypes);
    fields.insert(QContactType::FieldType, f); // note: NO CONTEXT!!
    def.setFields(fields);
    def.setUnique(true);
    retn.insert(def.name(), def);

/* TODO
    // guid
    def.setName(QContactGuid::DefinitionName);
    fields.clear();
    f.setDataType(QVariant::String);
    f.setAllowableValues(QVariantList());
    fields.insert(QContactGuid::FieldGuid, f);
    f.setDataType(QVariant::StringList);
    f.setAllowableValues(contexts);
    fields.insert(QContactDetail::FieldContext, f);
    def.setFields(fields);
    def.setUnique(false);
    def.setAccessConstraint(QContactDetailDefinition::CreateOnly);
    retn.insert(def.name(), def);
*/

    // display label
    def.setName(QContactDisplayLabel::DefinitionName);
    fields.clear();
    f.setDataType(QVariant::String);
    f.setAllowableValues(QVariantList());
    fields.insert(QContactDisplayLabel::FieldLabel, f);
    def.setFields(fields);
    def.setUnique(true);
    retn.insert(def.name(), def);

    // email support needs to be checked run-time, because it is SIM specific
    if (storeInfo.m_emailSupported) {
        def.setName(QContactEmailAddress::DefinitionName);
        fields.clear();
        f.setDataType(QVariant::String);
        f.setAllowableValues(QVariantList());
        fields.insert(QContactEmailAddress::FieldEmailAddress, f);
        def.setFields(fields);
        def.setUnique(true);
        retn.insert(def.name(), def);
    }

    // phone number
    def.setName(QContactPhoneNumber::DefinitionName);
    fields.clear();
    f.setDataType(QVariant::String);
    f.setAllowableValues(QVariantList());
    fields.insert(QContactPhoneNumber::FieldNumber, f);
    // TODO: subtypes supported in case a sim contact can have multiple phone numbers?
    def.setFields(fields);
    if (storeInfo.m_additionalNumberSupported) {
        // multiple numbers supported
        def.setUnique(false);
    } else {
        // only one phone number allowed
        def.setUnique(true);
    }
    retn.insert(def.name(), def);

    // nickname support needs to be checked run-time, because it is SIM specific
    if (storeInfo.m_secondNameSupported) {
        def.setName(QContactNickname::DefinitionName);
        fields.clear();
        f.setDataType(QVariant::String);
        f.setAllowableValues(QVariantList());
        fields.insert(QContactNickname::FieldNickname, f);
        def.setFields(fields);
        def.setUnique(true);
        retn.insert(def.name(), def);
    }

    // name
    def.setName(QContactName::DefinitionName);
    fields.clear();
    f.setDataType(QVariant::String);
    f.setAllowableValues(QVariantList());
    fields.insert(QContactName::FieldCustomLabel, f);
    def.setFields(fields);
    def.setUnique(true);
    retn.insert(def.name(), def);

    return retn;
}

void CntSymbianSimEngine::requestDestroyed(QContactAbstractRequest* req)
{
    if (d->m_asyncRequests.contains(req)) {
        delete d->m_asyncRequests.take(req); 
    }
}

bool CntSymbianSimEngine::startRequest(QContactAbstractRequest* req)
{
    // Don't allow two async requests to be active at the same time.
    // The RMobilePhoneBookStore cannot handle it.
    foreach (QContactAbstractRequest* r, d->m_asyncRequests.keys()) {
        if (r->isActive()) {
            // TODO: Should we set the error for the request also?
            return false;
        }
    }
    
    // Check for existing request and start again
    if (d->m_asyncRequests.contains(req)) {
        return d->m_asyncRequests.value(req)->start();
    }
    
    // Existing request not found. Create a new one.
    CntAbstractSimRequest* simReq = 0;
    switch (req->type()) 
    {
        case QContactAbstractRequest::ContactFetchRequest:
        {
            QContactFetchRequest* r = static_cast<QContactFetchRequest*>(req);
            simReq = new CntSimContactFetchRequest(this, r);
        }
        break;

        case QContactAbstractRequest::ContactLocalIdFetchRequest:
        {
            QContactLocalIdFetchRequest* r = static_cast<QContactLocalIdFetchRequest*>(req);
            simReq = new CntSimContactLocalIdFetchRequest(this, r);
        }
        break;

        case QContactAbstractRequest::ContactSaveRequest:
        {
            QContactSaveRequest* r = static_cast<QContactSaveRequest*>(req);
            simReq = new CntSimContactSaveRequest(this, r);
        }
        break;

        case QContactAbstractRequest::ContactRemoveRequest:
        {
            QContactRemoveRequest* r = static_cast<QContactRemoveRequest*>(req);
            simReq = new CntSimContactRemoveRequest(this, r);
        }
        break;

        case QContactAbstractRequest::DetailDefinitionFetchRequest:
        {
            QContactDetailDefinitionFetchRequest* r = static_cast<QContactDetailDefinitionFetchRequest*>(req);
            simReq = new CntSimDetailDefinitionFetchRequest(this, r);
        }
        break;

        case QContactAbstractRequest::DetailDefinitionSaveRequest:
        case QContactAbstractRequest::DetailDefinitionRemoveRequest:
        case QContactAbstractRequest::RelationshipFetchRequest:
        case QContactAbstractRequest::RelationshipSaveRequest:
        case QContactAbstractRequest::RelationshipRemoveRequest:
        // fall through.
        default: // unknown request type.
        break;
    }
    
    if (simReq) {
        d->m_asyncRequests.insert(req, simReq);
        return simReq->start();
    }
        
    return false;
}

bool CntSymbianSimEngine::cancelRequest(QContactAbstractRequest* req)
{
    if (d->m_asyncRequests.contains(req))
        return d->m_asyncRequests.value(req)->cancel();
    return false;
}

bool CntSymbianSimEngine::waitForRequestFinished(QContactAbstractRequest* req, int msecs)
{
    if (!d->m_asyncRequests.contains(req)) 
        return false;
    
    if (req->state() != QContactAbstractRequest::ActiveState)
        return false;
    
    QEventLoop *loop = new QEventLoop(this);
    QObject::connect(req, SIGNAL(resultsAvailable()), loop, SLOT(quit()));

    // NOTE: zero means wait forever
    if (msecs > 0)
        QTimer::singleShot(msecs, loop, SLOT(quit()));

    loop->exec();
    loop->disconnect();
    loop->deleteLater();

    return (req->state() == QContactAbstractRequest::FinishedState);
}

/*!
 * Returns true if the given feature \a feature is supported by the manager,
 * for the specified type of contact \a contactType
 */
bool CntSymbianSimEngine::hasFeature(QContactManager::ManagerFeature feature, const QString& contactType) const
{
    Q_UNUSED(feature);
    Q_UNUSED(contactType);
    // We don't support anything in the ManagerFeature
    return false;
}

/*!
  Returns a whether the supplied \a filter can be implemented
  natively by this engine.  If not, the base class implementation
  will emulate the functionality.
 */
bool CntSymbianSimEngine::isFilterSupported(const QContactFilter& filter) const
{
    if (filter.type() == QContactFilter::ContactDetailFilter) {
        QContactDetailFilter f(filter);
        if (f.detailDefinitionName() == QContactPhoneNumber::DefinitionName && 
            f.detailFieldName() == QContactPhoneNumber::FieldNumber &&
            f.matchFlags() == QContactFilter::MatchPhoneNumber)
            return true;
    }
    return false;
}

/*!
 * Returns the list of data types supported by the manager
 */
QStringList CntSymbianSimEngine::supportedContactTypes() const
{
    // TODO: groups supported by some USIM cards?
    return QStringList() << QContactType::TypeContact;
}

void CntSymbianSimEngine::updateDisplayLabel(QContact& contact) const
{
    QContactManager::Error error(QContactManager::NoError);
    QString label = synthesizedDisplayLabel(contact, &error);
    if(error == QContactManager::NoError) {
        setContactDisplayLabel(&contact, label);
    }
}

void CntSymbianSimEngine::setReadOnlyAccessConstraint(QContactDetail* detail) const
{
    setDetailAccessConstraints(detail, QContactDetail::ReadOnly); 
}


/*!
  Returns true if the supplied contact \a contact matches the supplied filter \a filter.
 */
bool CntSymbianSimEngine::filter(const QContactFilter &filter, const QContact &contact)
{
    // Special handling for phonenumber matching:
    // Matching is done from the right by using a configurable number of digits.
    // Default number of digits is 7. So for example if we filter with number
    // +358505555555 the filter should match to +358505555555 and 0505555555.
    if (filter.type() == QContactFilter::ContactDetailFilter) 
    {
        QContactDetailFilter f(filter);
        if (f.detailDefinitionName() == QContactPhoneNumber::DefinitionName && 
            f.detailFieldName() == QContactPhoneNumber::FieldNumber &&
            f.matchFlags() == QContactFilter::MatchPhoneNumber) 
        {
            QList<QContactPhoneNumber> pns = contact.details<QContactPhoneNumber>();
            foreach (QContactPhoneNumber pn, pns) {
                // Best matching enabled
                if(d->m_phoneNumberMatchLen == 0) {
                    bool bestMatchFound(false);
                    QString matchingNumber = f.value().toString();
                    QString numberToMatch = pn.number();
                    TRAP_IGNORE(
                            bestMatchFound = CntSymbianSimPhoneNumberMatching::isBestMatchL(numberToMatch,matchingNumber);
                    )
                    if (bestMatchFound)
                        return true;
                }
                else {
                    // Default matching
                    QString matchNumber = f.value().toString().right(d->m_phoneNumberMatchLen);
                    QString number = pn.number().right(d->m_phoneNumberMatchLen);
                    if (number == matchNumber)
                        return true;
                }
            }
            return false;
        }
    }
    return QContactManagerEngine::testFilter(filter, contact);
}

/*!
  Returns whether the supplied \a phonenumber is the best matching number.
 */
TBool CntSymbianSimPhoneNumberMatching::isBestMatchL(const QString& numberToMatch, const QString& matchingNumber)
{
    bool bestMatchFound(false);
    TPtrC numberPtr(reinterpret_cast<const TUint16*>(numberToMatch.utf16()));
    TPtrC matchNumberPtr(reinterpret_cast<const TUint16*>(matchingNumber.utf16()));
    
    if (CntSymbianSimPhoneNumberMatching::validateBestMatchingRulesL(numberPtr,matchNumberPtr)) {
        bestMatchFound = true;
    }
    return bestMatchFound;
}

/*!
 * Executes an asynchronous request so that it will appear synchronous. This is
 * used internally in all synchronous functions. This way we only need to 
 * implement the matching asynchronous request.
 * 
 * \param req Request to be run.
 * \param qtError Qt error code.
 * \return true if succesfull, false if unsuccesfull.
 */
bool CntSymbianSimEngine::executeRequest(QContactAbstractRequest *req, QContactManager::Error* qtError) const
{
    *qtError = QContactManager::NoError;
    
    // TODO:
    // Remove this code when threads-branch is merged to master. Then this code
    // should not be needed because the default implementation at QContactManager
    // is using the asynchronous requests in a similar manner to implement
    // the synchronous functions.
    
    // Create a copy engine to workaround this functions const qualifier
    CntSymbianSimEngine engine(*this);
    
    // Mimic the way how async requests are normally run
    if (!engine.startRequest(req)) {
        *qtError = QContactManager::LockedError;
    } else {
        if (!engine.waitForRequestFinished(req, 0)) // no timeout
            *qtError = QContactManager::UnspecifiedError;
    }
    engine.requestDestroyed(req);
    
    if (req->error())
        *qtError = req->error();
    
    return (*qtError == QContactManager::NoError);
}

/*
 * Get the match length setting used in MatchPhoneNumber type filtering.
 * \a matchLength Phone number digits to be used in matching (counted from
 * right).
 */
void CntSymbianSimEngine::getMatchLengthL(int &matchLength)
{
    //Get number of digits used to match
    CRepository* repository = CRepository::NewL(KCRUidTelConfiguration);
    CleanupStack::PushL(repository);
    User::LeaveIfError(repository->Get(KTelMatchDigits, matchLength));
    CleanupStack::PopAndDestroy(repository);
}

/*!
  Validates the supplied \a phonenumber against best matching rules.
 */
TBool CntSymbianSimPhoneNumberMatching::validateBestMatchingRulesL(const TDesC& phoneNumber, const TDesC& matchNumber)
    {
    RBuf numberA;
    numberA.CleanupClosePushL();
    numberA.CreateL(matchNumber);
    TNumberType numberAType = (TNumberType) CntSymbianSimPhoneNumberMatching::formatAndCheckNumberType(numberA);
    RBuf numberB;
    numberB.CleanupClosePushL();
    numberB.CreateL(phoneNumber);
    TNumberType numberBType = (TNumberType) CntSymbianSimPhoneNumberMatching::formatAndCheckNumberType(numberB);

    TBool match = (!numberB.Compare(numberA) ||
                    CntSymbianSimPhoneNumberMatching::checkBestMatchingRules(numberA, numberAType, numberB, numberBType) ||
                    CntSymbianSimPhoneNumberMatching::checkBestMatchingRules(numberB, numberBType, numberA, numberAType));
    
    // cleanup
    CleanupStack::PopAndDestroy(2);
    return match;
    }

/*!
  Removes non-digit chars except plus form the beginning.
  Checks if number matches to one of defined types.
 */
TInt CntSymbianSimPhoneNumberMatching::formatAndCheckNumberType(TDes& number)
    {
    _LIT( KOneZeroPattern, "0*" );
    _LIT( KTwoZerosPattern, "00*" );
    _LIT( KPlusPattern, "+*" );
    const TChar KPlus = TChar('+');
    const TChar KZero = TChar('0');
    const TChar KAsterisk = TChar('*');
    const TChar KHash = TChar('#');
    
    for( TInt pos = 0; pos < number.Length(); ++pos ) {
        TChar chr = number[pos];
        if ( !chr.IsDigit() && !( pos == 0 && chr == KPlus  )
                && !( chr == KAsterisk ) && !( chr == KHash ) ) {
            number.Delete( pos, 1 );
            --pos;
        }
    }
    
    TInt format;
    
    if (!number.Match(KTwoZerosPattern) && number.Length() > 2 && number[2] != KZero) {
        format = ETwoZeros;
    }
    else if (!number.Match(KOneZeroPattern)&& number.Length() > 1 && number[1] != KZero) {
        format = EOneZero;
    }
    else if (!number.Match(KPlusPattern) && number.Length() > 1 && number[1] != KZero) {
        format = EPlus;
    }
    else if (number.Length() > 0 && number[0] != KZero && ( ( TChar ) number[0] ).IsDigit()) {
        format = EDigit;
    }
    else {
        format = EUnknown;
    }

    return format;
    }

TBool CntSymbianSimPhoneNumberMatching::checkBestMatchingRules(
        const TDesC& numberA, TNumberType numberAType,
        const TDesC& numberB, TNumberType numberBType  )
    {
    TBool result = EFalse;
    
    // Rules for matching not identical numbers
    // Rules details are presented in Best_Number_Matching_Algorithm_Description.doc
    
    // rule International-International 1
    if (!result && numberAType == EPlus && numberBType == ETwoZeros) {
        TPtrC numberAPtr = numberA.Right(numberA.Length() - 1);
        TPtrC numberBPtr = numberB.Right(numberB.Length() - 2);
        result = !(numberAPtr.Compare(numberBPtr));
        if (result) {
            return result;
        }
    }

    // rule International-International 2
    if (numberAType == EPlus && numberBType == EDigit) {
        TPtrC numberAPtr = numberA.Right( numberA.Length() - 1 );
        if (numberAPtr.Length() < numberB.Length()) {
            TPtrC numberBPtr = numberB.Right( numberAPtr.Length() );
            result = !(numberAPtr.Compare(numberBPtr));
            if (result) {
                return result;
            }
        }
    }

    // rule International-International 3
    if (numberAType == ETwoZeros && numberBType == EDigit) {
        TPtrC numberAPtr = numberA.Right(numberA.Length() - 2);
        if (numberAPtr.Length() < numberB.Length()) {
            TPtrC numberBPtr = numberB.Right(numberAPtr.Length());
            result = !(numberAPtr.Compare(numberBPtr));
            if (result) {
                return result;
            }
        }
    }

    // rule International-Operator 1
    if (numberAType == EOneZero && numberBType == EPlus
            || numberAType == EDigit && numberBType == EPlus) {
        TPtrC numberAPtr;
        if (numberAType == EOneZero) {
            numberAPtr.Set(numberA.Right(numberA.Length() - 1));
        }
        else {
            numberAPtr.Set(numberA);
        }
        if (numberAPtr.Length() < numberB.Length() - 1) {
            TPtrC numberBPtr = numberB.Right(numberAPtr.Length());
            result = !(numberAPtr.Compare(numberBPtr));
            if (result) {
                return result;
            }
        }
    }

    // rule International-Operator 2
    if (numberAType == EOneZero && numberBType == ETwoZeros
            || numberAType == EDigit && numberBType == ETwoZeros) {
        TPtrC numberAPtr;
        if (numberAType == EOneZero) {
            numberAPtr.Set(numberA.Right(numberA.Length() - 1));
        }
        else {
            numberAPtr.Set(numberA);
        }
        if (numberAPtr.Length() < numberB.Length() - 2) {
            TPtrC numberBPtr = numberB.Right(numberAPtr.Length());
            result = !(numberAPtr.Compare(numberBPtr));
            if (result) {
                return result;
            }
        }
    }

    // rule International-Operator 3
    if (numberAType == EOneZero && numberBType == EDigit
            || numberAType == EDigit && numberBType == EDigit) {
        TPtrC numberAPtr;
        if (numberAType == EOneZero) {
            numberAPtr.Set(numberA.Right( numberA.Length() - 1));
        }
        else {
            numberAPtr.Set(numberA);
        }
        if (numberAPtr.Length() < numberB.Length()) {
            TPtrC numberBPtr = numberB.Right(numberAPtr.Length());
            result = !(numberAPtr.Compare(numberBPtr));
            if (result) {
                return result;
            }
        }
    }

    // rule Operator-Operator 1
    if (numberAType == EOneZero && numberBType == EDigit) {
        TPtrC numberAPtr = numberA.Right(numberA.Length() - 1);
        result = !(numberAPtr.Compare(numberB));
        if (result) {
            return result;
        }
    }
    
    // rule North America Numbering Plan 1
    if (numberAType == EDigit && numberBType == EPlus) {
        TPtrC numberBPtr = numberB.Right(numberB.Length() - 1);
        result = !(numberA.Compare(numberBPtr));
        if (result) {
            return result;
        }
    }

    // More exceptional acceptance rules can be added here
    // Keep rules updated in the document Best_Number_Matching_Algorithm_Description.doc

    return result;
    }

QContactManagerEngine* CntSymbianSimFactory::engine(const QMap<QString, QString>& parameters, QContactManager::Error* error)
{
    CntSymbianSimEngine *engine = new CntSymbianSimEngine(parameters, error);
    if(*error != QContactManager::NoError) {
        delete engine;
        return 0;
    }
    return engine;
}

QString CntSymbianSimFactory::managerName() const
{
    return CNT_SYMBIANSIM_MANAGER_NAME;
}

Q_EXPORT_PLUGIN2(qtcontacts_symbiansim, CntSymbianSimFactory);

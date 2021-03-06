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

#include "cntsimstoreprivate.h"
#include "cntsymbiansimtransformerror.h"
#include "cntsimstore.h"
#include "cntsimstoreeventlistener.h"

#include <mmtsy_names.h>
#include <qtcontacts.h>
#include <qcontactchangeset.h>
#include <QDebug>

const TInt KOneSimContactBufferSize = 512;
const TInt KDataClientBuf  = 128;
const TInt KEtsiTonPosition = 0x70;

CntSimStorePrivate* CntSimStorePrivate::NewL(CntSymbianSimEngine &engine, CntSimStore &simStore, const QString &storeName)
{
    CntSimStorePrivate* self = new (ELeave) CntSimStorePrivate(engine, simStore, storeName);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
}

CntSimStorePrivate::CntSimStorePrivate(CntSymbianSimEngine &engine, CntSimStore &simStore, const QString &storeName)
    :CActive(CActive::EPriorityStandard),
     m_state(InactiveState),
     m_engine(engine),
     m_simStore(simStore),
     m_listener(0),
     m_extraDetailsChecked(false)
{
    CActiveScheduler::Add(this);
    m_managerUri = engine.managerUri();
    
    // Initialize store info
    m_storeInfo.m_storeName                 = storeName;
    m_storeInfo.m_totalEntries              = -1;
    m_storeInfo.m_usedEntries               = -1;
    m_storeInfo.m_readOnlyAccess            = false; 
    m_storeInfo.m_numberSupported           = true; // allways supported
    m_storeInfo.m_nameSupported             = true; // allways supported
    m_storeInfo.m_secondNameSupported       = false;
    m_storeInfo.m_additionalNumberSupported = false;
    m_storeInfo.m_emailSupported            = false;
    
    // SDN store is allways read only
    if (m_storeInfo.m_storeName == KParameterValueSimStoreNameSdn ||
        m_storeInfo.m_storeName == KParameterValueSimStoreNameOn)
        m_storeInfo.m_readOnlyAccess = true;
}

void CntSimStorePrivate::ConstructL()
{
    TBuf<RMobilePhoneBookStore::KMaxPBIDSize> storeName;
    convertStoreNameL(storeName);
    
    // Open etel server
    User::LeaveIfError(m_etelServer.Connect());
    User::LeaveIfError(m_etelServer.LoadPhoneModule(KMmTsyModuleName));

    // Open etel phone
    RTelServer::TPhoneInfo info;
    User::LeaveIfError(m_etelServer.GetPhoneInfo(0, info));
    User::LeaveIfError(m_etelPhone.Open(m_etelServer, info.iName));
    
    // Open Etel store
    if (m_storeInfo.m_storeName == KParameterValueSimStoreNameOn) {
        User::LeaveIfError(m_etelOnStore.Open(m_etelPhone));
    }
    else {
        User::LeaveIfError(m_etelStore.Open(m_etelPhone, storeName));        
    }

    // Update store info
    updateStoreInfoL();
    
    // Start listening for events
    if (m_storeInfo.m_storeName == KParameterValueSimStoreNameOn) {
        m_listener = new (ELeave) CntSimStoreEventListener(m_engine, m_etelOnStore);
    }
    else {
        m_listener = new (ELeave) CntSimStoreEventListener(m_engine, m_etelStore);
    }
    m_listener->start();
}

CntSimStorePrivate::~CntSimStorePrivate()
{
    Cancel();
    delete m_listener;
    m_etelStore.Close();
    m_etelOnStore.Close();
    m_etelPhone.Close();
    m_etelServer.Close();
}

void CntSimStorePrivate::convertStoreNameL(TDes &storeName)
{
    if(storeName.MaxLength() < RMobilePhoneBookStore::KMaxPBIDSize) {
        User::Leave(KErrArgument);
    }

    if (m_storeInfo.m_storeName.isEmpty()) {
        // Default to ADN store
        m_storeInfo.m_storeName = (QLatin1String) KParameterValueSimStoreNameAdn;
        storeName.Copy(KETelIccAdnPhoneBook);
    } else if (m_storeInfo.m_storeName == KParameterValueSimStoreNameFdn) {
        storeName.Copy(KETelIccFdnPhoneBook);
    } else if (m_storeInfo.m_storeName == KParameterValueSimStoreNameAdn) {
        storeName.Copy(KETelIccAdnPhoneBook);
    } else if (m_storeInfo.m_storeName == KParameterValueSimStoreNameSdn) {
        storeName.Copy(KETelIccSdnPhoneBook);
    } else if (m_storeInfo.m_storeName == KParameterValueSimStoreNameOn) {
        storeName.Copy(KETelOwnNumberStore);
    }

    // Check that we got a valid store name
    if(storeName.Length() == 0) {
        User::Leave(KErrArgument);
    }
}

bool CntSimStorePrivate::read(int index, int numSlots, QContactManager::Error *error)
{
    if (IsActive()) {
        *error = QContactManager::LockedError;
        return false;
    }
    
    // ON store requires different read approach.
    // fetch ON contacts synchronously since there are usually only couple of them  
    if (m_storeInfo.m_storeName == KParameterValueSimStoreNameOn) {

        TRequestStatus status;
        QList<QContact> fetchedContacts; 
        for (int i = index; i <= numSlots; i++) {
            RMobileONStore::TMobileONEntryV1 onEntry;
            onEntry.iIndex = i;         
            RMobileONStore::TMobileONEntryV1Pckg onEntryPkg(onEntry);
            m_etelOnStore.Read(status, onEntryPkg);
            User::WaitForRequest(status);
            if (status.Int() == KErrNone) {
                QContact c;
                c.setType(QContactType::TypeContact);
                QContactName name;
                name.setCustomLabel(QString::fromUtf16(onEntry.iText.Ptr(),
                        onEntry.iText.Length()));
                c.saveDetail(&name);
                
                QContactPhoneNumber number;
                number.setNumber(QString::fromUtf16(onEntry.iNumber.iTelNumber.Ptr(),
                        onEntry.iNumber.iTelNumber.Length()));
                c.saveDetail(&number);
                
                QScopedPointer<QContactId> contactId(new QContactId());
                contactId->setLocalId(i);
                contactId->setManagerUri(m_managerUri);
                c.setId(*contactId);
                fetchedContacts.append(c);
            }
        }
        emit m_simStore.readComplete(fetchedContacts, QContactManager::NoError);
        *error = QContactManager::NoError;
        return true;
    }        
    
    // start reading
    m_buffer.Zero();
    m_buffer.ReAlloc(KOneSimContactBufferSize*numSlots);
    m_etelStore.Read(iStatus, index, numSlots, m_buffer);
    
    SetActive();
    m_state = ReadState;
    
    *error = QContactManager::NoError;
    return true;
}

bool CntSimStorePrivate::write(const QContact &contact, QContactManager::Error *error)
{
    if (IsActive()) {
        *error = QContactManager::LockedError;
        return false;
    }
    
    if (m_storeInfo.m_readOnlyAccess) {
        *error = QContactManager::NotSupportedError;
        return false;    
    }
    
    // get index
    m_writeIndex = KErrNotFound;
    if (contact.id().managerUri() == m_managerUri &&
        contact.localId() > 0) {
        m_writeIndex = contact.localId();

        // TODO: check that the contact exist in the sim 
    }
    
    // encode
    m_buffer.Zero();
    m_buffer.ReAlloc(KOneSimContactBufferSize);
    m_convertedContact = QContact(contact);

    TRAPD(err, encodeSimContactL(&m_convertedContact, m_buffer));
    if (err != KErrNone) {
        CntSymbianSimTransformError::transformError(err, error);
        return false;
    }

    // start writing
    m_etelStore.Write(iStatus, m_buffer, m_writeIndex);
    SetActive();
    m_state = WriteState;
    
    *error = QContactManager::NoError;
    return true;
}

bool CntSimStorePrivate::remove(int index, QContactManager::Error *error)
{
    if (IsActive()) {
        *error = QContactManager::LockedError;
        return false;
    }
    
    if (m_storeInfo.m_readOnlyAccess) {
        *error = QContactManager::NotSupportedError;
        return false;    
    }
    
    // NOTE:
    // If index points to an empty slot and running in hardware the 
    // delete operation will not return any error.
    
    m_etelStore.Delete(iStatus, index);
    SetActive();
    m_state = DeleteState;
    
    *error = QContactManager::NoError;
    return true;
}

bool CntSimStorePrivate::getReservedSlots(QContactManager::Error *error)
{
    if (IsActive()) {
        *error = QContactManager::LockedError;
        return false;
    }
    
    // reserved slots are checked for sim contacts removing, 
    // this operation is not supported for read only stores
    if (m_storeInfo.m_readOnlyAccess) {
        *error = QContactManager::NotSupportedError;
        return false;    
    }
    
    // start reading
    m_buffer.Zero();
    m_buffer.ReAlloc(KOneSimContactBufferSize*m_storeInfo.m_totalEntries);
    m_etelStore.Read(iStatus, 1, m_storeInfo.m_totalEntries, m_buffer);
    SetActive();
    m_state = ReadReservedSlotsState;
    
    *error = QContactManager::NoError;
    return true;  
}

void CntSimStorePrivate::DoCancel()
{
    if (m_state == ReadState)
        m_etelStore.CancelAsyncRequest(EMobilePhoneStoreRead);
    if (m_state == WriteState)
        m_etelStore.CancelAsyncRequest(EMobilePhoneStoreWrite);
    if (m_state == DeleteState)
        m_etelStore.CancelAsyncRequest(EMobilePhoneStoreDelete);
    if (m_state == ReadReservedSlotsState)
        m_etelStore.CancelAsyncRequest(EMobilePhoneStoreRead);
    
    m_state = InactiveState;
}

void CntSimStorePrivate::RunL()
{
    //qDebug() << "CntSimStorePrivate::RunL()" << m_state << iStatus.Int();
    
    m_asyncError = iStatus.Int();
    User::LeaveIfError(iStatus.Int());
    
    // NOTE: It is assumed that emitting signals is queued
    
    switch (m_state)
    {
        case ReadState:
        {
            QList<QContact> contacts = decodeSimContactsL(m_buffer);

            // set sync target and read only access constraint and display label
            QList<QContact>::iterator i;
            for (i = contacts.begin(); i != contacts.end(); ++i) {
                QContactSyncTarget syncTarget;
                syncTarget.setSyncTarget(KSimSyncTarget);
                m_engine.setReadOnlyAccessConstraint(&syncTarget);
                i->saveDetail(&syncTarget);
                QContactType contactType = i->detail(QContactType::DefinitionName);
                m_engine.setReadOnlyAccessConstraint(&contactType);
                i->saveDetail(&contactType);
                m_engine.updateDisplayLabel(*i);
            }
            
            emit m_simStore.readComplete(contacts, QContactManager::NoError);
        }
        break;
        
        case WriteState:
        {
            // save id
            QContactId contactId;
            contactId.setLocalId(m_writeIndex);
            contactId.setManagerUri(m_managerUri);
            m_convertedContact.setId(contactId);  
            
            // set sync target
            if(m_convertedContact.detail(QContactSyncTarget::DefinitionName).isEmpty()) {
                QContactSyncTarget syncTarget = m_convertedContact.detail(QContactSyncTarget::DefinitionName);
                syncTarget.setSyncTarget(KSimSyncTarget);
                m_engine.setReadOnlyAccessConstraint(&syncTarget);
                m_convertedContact.saveDetail(&syncTarget);
            }

            // set type as read only
            QContactType contactType = m_convertedContact.detail(QContactType::DefinitionName);
            m_engine.setReadOnlyAccessConstraint(&contactType);
            m_convertedContact.saveDetail(&contactType);

            emit m_simStore.writeComplete(m_convertedContact, QContactManager::NoError);
        }
        break;
        
        case DeleteState:
        {
            emit m_simStore.removeComplete(QContactManager::NoError);
        }
        break;
        
        case ReadReservedSlotsState:
        {
            QList<int> reservedSlots = decodeReservedSlotsL(m_buffer);
            emit m_simStore.getReservedSlotsComplete(reservedSlots, QContactManager::NoError);
        }
        break;
        
        default:
        {
            User::Leave(KErrUnknown);
        }
        break;
    }
    m_state = InactiveState;
}

TInt CntSimStorePrivate::RunError(TInt aError)
{
    QContactManager::Error qtError = QContactManager::NoError;
    CntSymbianSimTransformError::transformError(aError, &qtError);
    
    // NOTE: It is assumed that emitting signals is queued
        
    if (m_state == ReadState) 
        emit m_simStore.readComplete(QList<QContact>(), qtError);            
    else if (m_state == WriteState)
        emit m_simStore.writeComplete(m_convertedContact, qtError);  
     else if (m_state == DeleteState)
        emit m_simStore.removeComplete(qtError);
    else if (m_state == ReadReservedSlotsState)
        emit m_simStore.getReservedSlotsComplete(QList<int>(), qtError);
        
    m_state = InactiveState;
    
    return KErrNone;
}

/*! Parses SIM contacts in TLV format.
 *
 * \param rawData SIM contacts in TLV format.
 * \return List of contacts.
*/
QList<QContact> CntSimStorePrivate::decodeSimContactsL(TDes8& rawData) const
{
    PbkPrintToLog(_L("CntSymbianSimEngine::decodeSimContactsL() - IN"));
    QList<QContact> fetchedContacts;
    QContact currentContact;

    TBuf16<KDataClientBuf> buffer;
    TPtrC16 bufPtr(buffer);

    TUint8 tagValue(0);
    CPhoneBookBuffer::TPhBkTagType dataType;

    bool isAdditionalNumber = false;

    CPhoneBookBuffer* pbBuffer = new(ELeave) CPhoneBookBuffer();
    CleanupStack::PushL(pbBuffer);
    pbBuffer->Set(&rawData);
    pbBuffer->StartRead();

    while (pbBuffer->GetTagAndType(tagValue, dataType) == KErrNone) {
        switch (tagValue)
        {
            case RMobilePhoneBookStore::ETagPBAdnIndex:
            {
                //save contact's id (SIM card index) and manager's name
                TUint16  index;
                if (pbBuffer->GetValue(index) == KErrNone) {
                    QScopedPointer<QContactId> contactId(new QContactId());
                    contactId->setLocalId(index);
                    contactId->setManagerUri(m_managerUri);
                    currentContact.setId(*contactId);
                }
                isAdditionalNumber = false;
                break;
            }
            case RMobilePhoneBookStore::ETagPBTonNpi:
            {
                // Note, that TON info can be incorporated into the phone number by Etel
                // implementation (TSY). E.g. this is the case with Nokia TSY.
                // Here general case is implemented.

                // Check number type, we are only interested if it's international or not.
                // We assume here that ETagPBTonNpi always comes after ETagPBNumber, not before.
                TUint8  tonNpi;
                if (pbBuffer->GetValue(tonNpi) == KErrNone) {
                    TUint8  intFlag = (tonNpi & KEtsiTonPosition) >> 4;
                    if (intFlag == 1) {
                        //international number format, append "+" to the last
                        //saved number
                        QList<QContactDetail> phoneNumbers = currentContact.details(
                                QContactPhoneNumber::DefinitionName);
                        if (phoneNumbers.count() > 0) {
                            QContactPhoneNumber lastNumber = static_cast<QContactPhoneNumber>(
                                phoneNumbers.at(phoneNumbers.count() - 1));
                            QString number = lastNumber.number();
                            number.insert(0, "+");
                            lastNumber.setNumber(number);
                            if (m_storeInfo.m_readOnlyAccess)
                                m_engine.setReadOnlyAccessConstraint(&lastNumber);
                            currentContact.saveDetail(&lastNumber);
                        }
                    }
                }

                // We have rearched to the end of the number,
                // invalidate additional number flag.
                isAdditionalNumber = false;
                break;
            }
            case RMobilePhoneBookStore::ETagPBText:
            {
                if (pbBuffer->GetValue(bufPtr) == KErrNone) {
                    if (isAdditionalNumber) {
                        // For additional number bufPtr contains number alpha string,
                        // this is ignored currently
                    }
                    else {
                        // Contact name otherwise
                        QContactName name;
                        QString nameString = QString::fromUtf16(bufPtr.Ptr(), bufPtr.Length());
                        name.setCustomLabel(nameString);
                        if (m_storeInfo.m_readOnlyAccess)
                            m_engine.setReadOnlyAccessConstraint(&name);
                        currentContact.saveDetail(&name);
                        QContactManager::Error error(QContactManager::NoError);
                        m_engine.setContactDisplayLabel(&currentContact, m_engine.synthesizedDisplayLabel(currentContact, &error));
                    }
                }
                break;
            }
            case RMobilePhoneBookStore::ETagPBSecondName:
            {
                if (pbBuffer->GetValue(bufPtr) == KErrNone) {
                    QContactNickname nickName;
                    QString name = QString::fromUtf16(bufPtr.Ptr(), bufPtr.Length());
                    nickName.setNickname(name);
                    if (m_storeInfo.m_readOnlyAccess)
                        m_engine.setReadOnlyAccessConstraint(&nickName);
                    currentContact.saveDetail(&nickName);
                }
                break;
            }
            case RMobilePhoneBookStore::ETagPBNumber:
            {
                if (pbBuffer->GetValue(bufPtr) == KErrNone) {
                    QContactPhoneNumber phoneNumber;
                    QString number = QString::fromUtf16(bufPtr.Ptr(), bufPtr.Length());
                    phoneNumber.setNumber(number);
                    if (m_storeInfo.m_readOnlyAccess)
                        m_engine.setReadOnlyAccessConstraint(&phoneNumber);
                    currentContact.saveDetail(&phoneNumber);
                }
                break;
            }
            case RMobilePhoneBookStore::ETagPBAnrStart:
            {
                // This tag should precede every additional number entry
                isAdditionalNumber = true;
                break;
            }
            case RMobilePhoneBookStore::ETagPBEmailAddress:
            {
                if (pbBuffer->GetValue(bufPtr) == KErrNone) {
                    QContactEmailAddress email;
                    QString emailAddress = QString::fromUtf16(bufPtr.Ptr(), bufPtr.Length());
                    email.setEmailAddress(emailAddress);
                    if (m_storeInfo.m_readOnlyAccess)
                        m_engine.setReadOnlyAccessConstraint(&email);
                    currentContact.saveDetail(&email);
                }
                break;
            }
            case RMobilePhoneBookStore::ETagPBNewEntry:
            {
                // This signals the end of the entry and is a special case
                // which will be handled below.
                break;
            }
            default:
            {
                // An unsupported field type - just skip this value
                pbBuffer->SkipValue(dataType);
                break;
            }
        } //switch

        // save contact to the array of contact to be returned if the whole entry was extracted
        if ((tagValue == RMobilePhoneBookStore::ETagPBNewEntry && currentContact.localId() > 0) ||
            (pbBuffer->RemainingReadLength() == 0 && currentContact.localId() > 0)) {
            fetchedContacts.append(currentContact);
            //clear current contact
            currentContact.clearDetails();
            QScopedPointer<QContactId> contactId(new QContactId());
            contactId->setLocalId(0);
            contactId->setManagerUri(QString());
            currentContact.setId(*contactId);
        }
    } //while

    CleanupStack::PopAndDestroy(pbBuffer);
    PbkPrintToLog(_L("CntSymbianSimEngine::decodeSimContactsL() - OUT"));
    return fetchedContacts;
}

/*! Converts QContact to the TLV format which is used to save it to the Etel store.
 *
 * \param contact QContact to be converted
 * \param rawData Contact in TLV format on return.
*/
void CntSimStorePrivate::encodeSimContactL(QContact* contact, TDes8& rawData) const
{
    // Keep track of the count of phone numbers added
    int phoneNumberCount(0);

    // Create buffer
    CPhoneBookBuffer* pbBuffer = new(ELeave) CPhoneBookBuffer();
    CleanupStack::PushL(pbBuffer);
    pbBuffer->Set(&rawData);
    User::LeaveIfError(pbBuffer->AddNewEntryTag());

    // Loop through details
    foreach(QContactDetail detail, contact->details()) 
    {
        QString definitionName = detail.definitionName();
        
        // NOTE: If the detail is too long let the etel store return
        // an error about it. We could check the maximum lenghts for each
        // detail but then we would need to read it before every write operation
        // bacause it seems to change depending on how full the sim card is.

        // Name
        if (definitionName == QContactName::DefinitionName) 
        {
            QContactName nameDetail = static_cast<QContactName>(detail);
            QString name = nameDetail.customLabel();
            putTagAndValueL(pbBuffer, RMobilePhoneBookStore::ETagPBText, name);
        }
        // Phone number
        else if (definitionName == QContactPhoneNumber::DefinitionName)
        {
            if (m_storeInfo.m_additionalNumberSupported == false && phoneNumberCount>0)
                User::Leave(KErrNotSupported);

            phoneNumberCount++;
            QString number = static_cast<QContactPhoneNumber>(detail).number();
            if (number.isEmpty())
                continue;

            // Verify the number only contains legal digits
            foreach (const QChar character, number) {
                if(!character.isDigit()) {
                    if(character != QChar('+')
                        && character != QChar('*')
                        && character != QChar('#')
                        && character != QChar('p')
                        && character != QChar('w')) {
                        User::Leave(KErrArgument);
                    }
                }
            }

            if (phoneNumberCount > 1) {
                // Mark the beginning of an additional number
                User::LeaveIfError(pbBuffer->AddNewNumberTag());
            }

            // The number itself
            putTagAndValueL(pbBuffer, RMobilePhoneBookStore::ETagPBNumber, number);
        }
        // Nickname
        else if (definitionName == QContactNickname::DefinitionName)
        {
            if (m_storeInfo.m_secondNameSupported == false)
                User::Leave(KErrNotSupported);
            
            QString nickname = static_cast<QContactNickname>(detail).nickname();
            if (nickname.isEmpty())
                continue;
            
            putTagAndValueL(pbBuffer, RMobilePhoneBookStore::ETagPBSecondName, nickname);
        } 
        // email
        else if (definitionName == QContactEmailAddress::DefinitionName)
        {
            if (m_storeInfo.m_emailSupported == false)
                User::Leave(KErrNotSupported);
            
            QString email = static_cast<QContactEmailAddress>(detail).emailAddress();
            if (email.isEmpty())
                continue;            
            
            putTagAndValueL(pbBuffer, RMobilePhoneBookStore::ETagPBEmailAddress, email);
        }
        // These are ignored in the conversion
        else if (definitionName == QContactSyncTarget::DefinitionName
            || definitionName == QContactDisplayLabel::DefinitionName
            || definitionName == QContactType::DefinitionName) 
        {
            // Do nothing
        } 
        else 
        {
            // Unknown detail
            User::Leave(KErrArgument);
        }
    }
    CleanupStack::PopAndDestroy(pbBuffer);
}

void CntSimStorePrivate::putTagAndValueL(CPhoneBookBuffer* pbBuffer, TUint8 tag, QString data) const
{
    TPtrC value(reinterpret_cast<const TUint16*>(data.utf16()));
    User::LeaveIfError(pbBuffer->PutTagAndValue(tag, value));
}

QList<int> CntSimStorePrivate::decodeReservedSlotsL(TDes8& rawData) const
{
    QList<int> reservedSlots;

    TUint8 tagValue(0);
    CPhoneBookBuffer::TPhBkTagType dataType;

    CPhoneBookBuffer* pbBuffer = new (ELeave) CPhoneBookBuffer();
    CleanupStack::PushL(pbBuffer);
    pbBuffer->Set(&rawData);
    pbBuffer->StartRead();

    while (pbBuffer->GetTagAndType(tagValue, dataType) == KErrNone)
    {
        if (tagValue == RMobilePhoneBookStore::ETagPBAdnIndex) 
        {
            TUint16 index;
            if (pbBuffer->GetValue(index) == KErrNone)
                reservedSlots.append(index);
        } else
            pbBuffer->SkipValue(dataType);
    } //while
    
    CleanupStack::PopAndDestroy(pbBuffer);
    return reservedSlots;
}

void CntSimStorePrivate::writeL(QContact *contact)
{
    if (IsActive())
        User::Leave(KErrLocked);
    
    if (m_storeInfo.m_readOnlyAccess) {
        User::Leave(KErrNotSupported);
    }
    
    // get index
    int index = KErrNotFound;
    if (contact->id().managerUri() == m_managerUri &&
        contact->localId() > 0) {
        index = contact->localId();
    }
    
    // encode
    m_buffer.Zero();
    m_buffer.ReAlloc(KOneSimContactBufferSize);
    encodeSimContactL(contact, m_buffer);

    // write
    TRequestStatus status;
    m_etelStore.Write(status, m_buffer, index);
    User::WaitForRequest(status);
    User::LeaveIfError(status.Int());
    
    // update id
    QContactId id;
    id.setLocalId(index);
    id.setManagerUri(m_managerUri);
    contact->setId(id);
}

void CntSimStorePrivate::removeL(int index)
{
    if (IsActive())
        User::Leave(KErrLocked);
    
    if (m_storeInfo.m_readOnlyAccess) {
        User::Leave(KErrNotSupported);
    }
    
    // NOTE:
    // If index points to an empty slot and running in hardware the 
    // delete operation will not return any error.
    
    TRequestStatus status;
    m_etelStore.Delete(status, index);
    User::WaitForRequest(status);
    User::LeaveIfError(status.Int());
}

void CntSimStorePrivate::updateStoreInfoL()
{
#ifdef SYMBIANSIM_BACKEND_PHONEBOOKINFOV1
    TRequestStatus status;
    RMobilePhoneBookStore::TMobilePhoneBookInfoV1 info;
    RMobilePhoneBookStore::TMobilePhoneBookInfoV1Pckg infoPckg(info);
    m_etelStore.GetInfo(status, infoPckg);
    User::WaitForRequest(status);
    User::LeaveIfError(status.Int());
    m_storeInfo.m_totalEntries = info.iTotalEntries;
    m_storeInfo.m_usedEntries  = info.iUsedEntries;
#else
    // Get info
    TRequestStatus status;
    if (m_storeInfo.m_storeName == KParameterValueSimStoreNameOn) {
        RMobileONStore::TMobileONStoreInfoV1 onInfo;
        RMobileONStore::TMobileONStoreInfoV1Pckg onInfoPckg(onInfo);
        m_etelOnStore.GetInfo(status, onInfoPckg);
        User::WaitForRequest(status);
        User::LeaveIfError(status.Int());

        // Update entry counts
        m_storeInfo.m_totalEntries = onInfo.iTotalEntries;
        m_storeInfo.m_usedEntries  = onInfo.iUsedEntries;
    }
    else {
        RMobilePhoneBookStore::TMobilePhoneBookInfoV5 info;
        RMobilePhoneBookStore::TMobilePhoneBookInfoV5Pckg infoPckg(info);
        m_etelStore.GetInfo(status, infoPckg);
        User::WaitForRequest(status);
        User::LeaveIfError(status.Int());

        // Update entry counts
        m_storeInfo.m_totalEntries = info.iTotalEntries;
        m_storeInfo.m_usedEntries  = info.iUsedEntries;
    }
#endif

#ifdef SYMBIANSIM_BACKEND_TEST_EXTRADETAILS
    // Check if store supports the extra details
    //
    // NOTE: 
    // We cannot rely on TMobilePhoneBookInfoV5 to check if we support
    // these details. For example iMaxSecondNames is allways -1 even if the sim
    // card supports a second name.
    //
    // There is an API for checking these but it's Nokia internal so we must
    // do it this way - by checking if saving these details is possible.

    // Have we checked these already?
    if (m_extraDetailsChecked == false)
    {
        // Cannot test extra details if sim card is full 
        if (m_storeInfo.m_usedEntries == m_storeInfo.m_totalEntries)
            return;

        // Cancel store event listener temporarily
        if (m_listener)
            m_listener->Cancel();

        // Test writing nickname
        QContact contact;
        QContactNickname nick;
        nick.setNickname("simbackend test");
        contact.saveDetail(&nick);
        TRAPD(err, {
            m_storeInfo.m_secondNameSupported = true; // enable to pass encodeSimContactL()
            writeL(&contact);
            removeL(contact.localId());
        } );
        if (err)
            m_storeInfo.m_secondNameSupported = false;

        // Test writing additional number
        contact = QContact();
        QContactPhoneNumber num1;
        num1.setNumber("1111111111");
        contact.saveDetail(&num1);
        QContactPhoneNumber num2;
        num2.setNumber("2222222222");
        contact.saveDetail(&num2);
        TRAP(err, {
            m_storeInfo.m_additionalNumberSupported = true; // enable to pass encodeSimContactL()
            writeL(&contact); 
            removeL(contact.localId()); 
        } );
        if (err)
            m_storeInfo.m_additionalNumberSupported = false;

        // Test writing email
        contact = QContact();
        QContactEmailAddress email;
        email.setEmailAddress("simbackend@test.com");
        contact.saveDetail(&email);
        TRAP(err, {
            m_storeInfo.m_emailSupported = true; // enable to pass encodeSimContactL()
            writeL(&contact); 
            removeL(contact.localId()); 
        } );
        if (err)
            m_storeInfo.m_emailSupported = false;

        // Start store event listener again
        if (m_listener)
            m_listener->start();

        m_extraDetailsChecked = true;
    }
#endif

    /*
    qDebug() << "Store info:"
        << "\nStore name                :" << m_storeInfo.m_storeName
        << "\nTotal entries             :" << m_storeInfo.m_totalEntries
        << "\nUsed entries              :" << m_storeInfo.m_usedEntries
        << "\nRead only access          :" << m_storeInfo.m_readOnlyAccess
        << "\nNumber supported          :" << m_storeInfo.m_numberSupported
        << "\nName supported            :" << m_storeInfo.m_nameSupported
        << "\nSecond name supported     :" << m_storeInfo.m_secondNameSupported
        << "\nAdditional name supported :" << m_storeInfo.m_additionalNumberSupported
        << "\nEmail supported           :" << m_storeInfo.m_emailSupported;
    */
}

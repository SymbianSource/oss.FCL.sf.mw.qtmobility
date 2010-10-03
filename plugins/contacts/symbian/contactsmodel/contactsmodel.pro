#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).


#
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description:
#
#


TEMPLATE = subdirs
BLD_INF_RULES.prj_mmpfiles = "./groupsql/cntmodel.mmp"\
                             "./group/cntview.mmp"\
                             "./group/template.mmp"\
                             "./groupsql/cntsrv.mmp"\
                             "./groupsql/cntplsql.mmp"\
                             "./cntmatchlog/group/cntmatchlog.mmp"\
                             "./cntvcard/cntvcard.mmp"\
                             "./cntphone/cntphone.mmp"

# Exports
deploy.path = /

# IBY files
iby.path = epoc32/rom/include
iby.sources = cntmodel.iby cntplsql.iby

for(iby, iby.sources):BLD_INF_RULES.prj_exports += "groupsql/$$iby $$deploy.path$$iby.path/$$iby"
BLD_INF_RULES.prj_exports += "cntmatchlog/group/cntmatchlog.iby $$deploy.path$$iby.path/cntmatchlog.iby"

# Headers
BLD_INF_RULES.prj_exports += "./inc/cntdef.h              /epoc32/include/cntdef.h"\
                             "./inc/cntdb.h               /epoc32/include/cntdb.h"\
                             "./inc/cntdbobs.h            /epoc32/include/cntdbobs.h"\
                             "./inc/cntfield.h            /epoc32/include/cntfield.h"\
                             "./inc/cntfldst.h            /epoc32/include/cntfldst.h"\
                             "./inc/cntfilt.h             /epoc32/include/cntfilt.h"\
                             "./inc/cntitem.h             /epoc32/include/cntitem.h"\
                             "./inc/cntview.h             /epoc32/include/cntview.h"\
                             "./inc/cntviewbase.h         /epoc32/include/cntviewbase.h"\
                             "./inc/cntsync.h             /epoc32/include/cntsync.h"\
                             "./inc/cntviewfindconfig.h   /epoc32/include/cntviewfindconfig.h"\
                             "./inc/cntviewfindconfig.inl /epoc32/include/cntviewfindconfig.inl"\
                             "./inc/cntviewsortplugin.h   /epoc32/include/cntviewsortplugin.h"\
                             "./inc/cntmodel.rh           /epoc32/include/cntmodel.rh"\
                             "./inc/cntdef.hrh            /epoc32/include/cntdef.hrh"\
                             "./cntvcard/cntvcard.h       /epoc32/include/cntvcard.h"\
                             "./inc/cntviewstore.h        /epoc32/include/cntviewstore.h"

BLD_INF_RULES.prj_exports += "./inc/cntphonenumparser.h APP_LAYER_PLATFORM_EXPORT_PATH(cntphonenumparser.h)"\
                             "./inc/cntviewsortpluginbase.h APP_LAYER_PLATFORM_EXPORT_PATH(cntviewsortpluginbase.h)"\
                             "./inc/cntsyncecom.h APP_LAYER_PLATFORM_EXPORT_PATH(cntsyncecom.h)"\
                             "./inc/cntconvertercallback.h APP_LAYER_PLATFORM_EXPORT_PATH(cntconvertercallback.h)"\
                             "./inc/cntdb_internal.h APP_LAYER_PLATFORM_EXPORT_PATH(cntdb_internal.h)"


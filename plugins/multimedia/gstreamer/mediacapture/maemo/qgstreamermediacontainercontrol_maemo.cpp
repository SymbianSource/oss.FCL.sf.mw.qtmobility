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

#include "qgstreamermediacontainercontrol_maemo.h"


#include <QtCore/qdebug.h>

QGstreamerMediaContainerControl::QGstreamerMediaContainerControl(QObject *parent)
    :QMediaContainerControl(parent)
{
    QList<QByteArray> formatCandidates;
    formatCandidates << "mp4" << "ogg" << "wav" << "amr";

    m_elementNames["mp4"] = "hantromp4mux";
    m_elementNames["ogg"] = "oggmux";
    m_elementNames["wav"] = "wavenc";
    m_elementNames["amr"] = "ffmux_amr";

    QSet<QString> allTypes;

    foreach( const QByteArray& formatName, formatCandidates ) {
        QByteArray elementName = m_elementNames[formatName];
        GstElementFactory *factory = gst_element_factory_find(elementName.constData());
        if (factory) {
            m_supportedContainers.append(formatName);
            const gchar *descr = gst_element_factory_get_description(factory);
            m_containerDescriptions.insert(formatName, QString::fromUtf8(descr));


            if (formatName == QByteArray("raw")) {
                m_streamTypes.insert(formatName, allTypes);
            } else {
                QSet<QString> types = supportedStreamTypes(factory, GST_PAD_SINK);
                m_streamTypes.insert(formatName, types);
                allTypes.unite(types);
            }

            gst_object_unref(GST_OBJECT(factory));
        }
    }

    if (!m_supportedContainers.isEmpty())
        setContainerMimeType(m_supportedContainers[0]);
}

QSet<QString> QGstreamerMediaContainerControl::supportedStreamTypes(GstElementFactory *factory, GstPadDirection direction)
{
    QSet<QString> types;
    const GList *pads = gst_element_factory_get_static_pad_templates(factory);
    for (const GList *pad = pads; pad; pad = g_list_next(pad)) {
        GstStaticPadTemplate *templ = (GstStaticPadTemplate*)pad->data;
        if (templ->direction == direction) {
            GstCaps *caps = gst_static_caps_get(&templ->static_caps);
            for (uint i=0; i<gst_caps_get_size(caps); i++) {
                GstStructure *structure = gst_caps_get_structure(caps, i);
                types.insert( QString::fromUtf8(gst_structure_get_name(structure)) );
            }
            gst_caps_unref(caps);
        }
    }

    return types;
}


QSet<QString> QGstreamerMediaContainerControl::supportedStreamTypes(const QString &container) const
{
    return m_streamTypes.value(container);
}

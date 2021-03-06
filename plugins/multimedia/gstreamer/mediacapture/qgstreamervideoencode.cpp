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

#include "qgstreamervideoencode.h"
#include "qgstreamercapturesession.h"
#include "qgstreamermediacontainercontrol.h"

#include <QtCore/qdebug.h>

#include <math.h>

QGstreamerVideoEncode::QGstreamerVideoEncode(QGstreamerCaptureSession *session)
    :QVideoEncoderControl(session), m_session(session)
{
    QList<QByteArray> codecCandidates;
    codecCandidates << "video/h264" << "video/xvid" << "video/mpeg4" << "video/mpeg1" << "video/mpeg2" << "video/theora";

    m_elementNames["video/h264"] = "x264enc";
    m_elementNames["video/xvid"] = "xvidenc";
    m_elementNames["video/mpeg4"] = "ffenc_mpeg4";
    m_elementNames["video/mpeg1"] = "ffenc_mpeg1video";
    m_elementNames["video/mpeg2"] = "ffenc_mpeg2video";
    m_elementNames["video/theora"] = "theoraenc";

    m_codecOptions["video/h264"] = QStringList() << "quantizer";
    m_codecOptions["video/xvid"] = QStringList() << "quantizer" << "profile";
    m_codecOptions["video/mpeg4"] = QStringList() << "quantizer";
    m_codecOptions["video/mpeg1"] = QStringList() << "quantizer";
    m_codecOptions["video/mpeg2"] = QStringList() << "quantizer";
    m_codecOptions["video/theora"] = QStringList();

    foreach( const QByteArray& codecName, codecCandidates ) {
        QByteArray elementName = m_elementNames[codecName];
        GstElementFactory *factory = gst_element_factory_find(elementName.constData());
        if (factory) {
            m_codecs.append(codecName);
            const gchar *descr = gst_element_factory_get_description(factory);
            m_codecDescriptions.insert(codecName, QString::fromUtf8(descr));

            m_streamTypes.insert(codecName,
                                 QGstreamerMediaContainerControl::supportedStreamTypes(factory, GST_PAD_SRC));

            gst_object_unref(GST_OBJECT(factory));
        }
    }

    //if (!m_codecs.isEmpty())
    //    m_videoSettings.setCodec(m_codecs[0]);
}

QGstreamerVideoEncode::~QGstreamerVideoEncode()
{
}

QList<QSize> QGstreamerVideoEncode::supportedResolutions(const QVideoEncoderSettings &, bool *continuous) const
{
    if (continuous)
        *continuous = m_session->videoInput() != 0;

    return m_session->videoInput() ? m_session->videoInput()->supportedResolutions() : QList<QSize>();
}

QList< qreal > QGstreamerVideoEncode::supportedFrameRates(const QVideoEncoderSettings &, bool *continuous) const
{
    if (continuous)
        *continuous = false;

    return m_session->videoInput() ? m_session->videoInput()->supportedFrameRates() : QList<qreal>();
}

QStringList QGstreamerVideoEncode::supportedVideoCodecs() const
{
    return m_codecs;
}

QString QGstreamerVideoEncode::videoCodecDescription(const QString &codecName) const
{
    return m_codecDescriptions.value(codecName);
}

QStringList QGstreamerVideoEncode::supportedEncodingOptions(const QString &codec) const
{
    return m_codecOptions.value(codec);
}

QVariant QGstreamerVideoEncode::encodingOption(const QString &codec, const QString &name) const
{
    return m_options[codec].value(name);
}

void QGstreamerVideoEncode::setEncodingOption(
        const QString &codec, const QString &name, const QVariant &value)
{
    m_options[codec][name] = value;
}

QVideoEncoderSettings QGstreamerVideoEncode::videoSettings() const
{
    return m_videoSettings;
}

void QGstreamerVideoEncode::setVideoSettings(const QVideoEncoderSettings &settings)
{
    m_videoSettings = settings;
}

GstElement *QGstreamerVideoEncode::createEncoder()
{
    GstBin *encoderBin = GST_BIN(gst_bin_new("video-encoder-bin"));
    Q_ASSERT(encoderBin);

    GstElement *capsFilter = gst_element_factory_make("capsfilter", "capsfilter-video");
    gst_bin_add(encoderBin, capsFilter);

    GstElement *colorspace = gst_element_factory_make("ffmpegcolorspace", NULL);
    gst_bin_add(encoderBin, colorspace);

    QString codec = m_videoSettings.codec();
    //qDebug() << "create encoder for video codec" << codec;

    GstElement *encoderElement = gst_element_factory_make( m_elementNames.value(codec).constData(), "video-encoder");
    gst_bin_add(encoderBin, encoderElement);

    gst_element_link_many(capsFilter, colorspace, encoderElement, NULL);

    // add ghostpads
    GstPad *pad = gst_element_get_static_pad(capsFilter, "sink");
    gst_element_add_pad(GST_ELEMENT(encoderBin), gst_ghost_pad_new("sink", pad));
    gst_object_unref(GST_OBJECT(pad));

    pad = gst_element_get_static_pad(encoderElement, "src");
    gst_element_add_pad(GST_ELEMENT(encoderBin), gst_ghost_pad_new("src", pad));
    gst_object_unref(GST_OBJECT(pad));

    if (encoderElement) {
        if (m_videoSettings.encodingMode() == QtMultimediaKit::ConstantQualityEncoding) {
            QtMultimediaKit::EncodingQuality qualityValue = m_videoSettings.quality();

            if (codec == QLatin1String("video/h264")) {
                //constant quantizer mode
                g_object_set(G_OBJECT(encoderElement), "pass", 4, NULL);
                int qualityTable[] = {
                    50, //VeryLow
                    35, //Low
                    21, //Normal
                    15, //High
                    8 //VeryHigh
                };
                g_object_set(G_OBJECT(encoderElement), "quantizer", qualityTable[qualityValue], NULL);
            } else if (codec == QLatin1String("video/xvid")) {
                //constant quantizer mode
                g_object_set(G_OBJECT(encoderElement), "pass", 3, NULL);
                int qualityTable[] = {
                    32, //VeryLow
                    12, //Low
                    5, //Normal
                    3, //High
                    2 //VeryHigh
                };
                int quant = qualityTable[qualityValue];
                g_object_set(G_OBJECT(encoderElement), "quantizer", quant, NULL);
            } else if (codec == QLatin1String("video/mpeg4") ||
                       codec == QLatin1String("video/mpeg1") ||
                       codec == QLatin1String("video/mpeg2") ) {
                //constant quantizer mode
                g_object_set(G_OBJECT(encoderElement), "pass", 2, NULL);
                //quant from 1 to 30, default ~3
                double qualityTable[] = {
                    20, //VeryLow
                    8.0, //Low
                    3.0, //Normal
                    2.5, //High
                    2.0 //VeryHigh
                };
                double quant = qualityTable[qualityValue];
                g_object_set(G_OBJECT(encoderElement), "quantizer", quant, NULL);
            } else if (codec == QLatin1String("video/theora")) {
                int qualityTable[] = {
                    8, //VeryLow
                    16, //Low
                    32, //Normal
                    45, //High
                    60 //VeryHigh
                };
                //quality from 0 to 63
                int quality = qualityTable[qualityValue];
                g_object_set(G_OBJECT(encoderElement), "quality", quality, NULL);
            }
        } else {
            int bitrate = m_videoSettings.bitRate();
            if (bitrate > 0) {
                g_object_set(G_OBJECT(encoderElement), "bitrate", bitrate, NULL);
            }
        }

        QMap<QString,QVariant> options = m_options.value(codec);
        QMapIterator<QString,QVariant> it(options);
        while (it.hasNext()) {
            it.next();
            QString option = it.key();
            QVariant value = it.value();

            switch (value.type()) {
            case QVariant::Int:
                g_object_set(G_OBJECT(encoderElement), option.toAscii(), value.toInt(), NULL);
                break;
            case QVariant::Bool:
                g_object_set(G_OBJECT(encoderElement), option.toAscii(), value.toBool(), NULL);
                break;
            case QVariant::Double:
                g_object_set(G_OBJECT(encoderElement), option.toAscii(), value.toDouble(), NULL);
                break;
            case QVariant::String:
                g_object_set(G_OBJECT(encoderElement), option.toAscii(), value.toString().toUtf8().constData(), NULL);
                break;
            default:
                qWarning() << "unsupported option type:" << option << value;
                break;
            }

        }
    }

    if (!m_videoSettings.resolution().isEmpty() || m_videoSettings.frameRate() > 0.001) {
        GstCaps *caps = gst_caps_new_empty();
        QStringList structureTypes;
        structureTypes << "video/x-raw-yuv" << "video/x-raw-rgb";

        foreach(const QString &structureType, structureTypes) {
            GstStructure *structure = gst_structure_new(structureType.toAscii().constData(), NULL);

            if (!m_videoSettings.resolution().isEmpty()) {
                gst_structure_set(structure, "width", G_TYPE_INT, m_videoSettings.resolution().width(), NULL);
                gst_structure_set(structure, "height", G_TYPE_INT, m_videoSettings.resolution().height(), NULL);
            }

            if (m_videoSettings.frameRate() > 0.001) {
                QPair<int,int> rate = rateAsRational();

                //qDebug() << "frame rate:" << num << denum;

                gst_structure_set(structure, "framerate", GST_TYPE_FRACTION, rate.first, rate.second, NULL);
            }

            gst_caps_append_structure(caps,structure);
        }

        //qDebug() << "set video caps filter:" << gst_caps_to_string(caps);

        g_object_set(G_OBJECT(capsFilter), "caps", caps, NULL);
    }

    return GST_ELEMENT(encoderBin);
}

QPair<int,int> QGstreamerVideoEncode::rateAsRational() const
{
    qreal frameRate = m_videoSettings.frameRate();

    if (frameRate > 0.001) {
        //convert to rational number
        QList<int> denumCandidates;
        denumCandidates << 1 << 2 << 3 << 5 << 10 << 1001 << 1000;

        qreal error = 1.0;
        int num = 1;
        int denum = 1;

        foreach (int curDenum, denumCandidates) {
            int curNum = qRound(frameRate*curDenum);
            qreal curError = qAbs(qreal(curNum)/curDenum - frameRate);

            if (curError < error) {
                error = curError;
                num = curNum;
                denum = curDenum;
            }

            if (curError < 1e-8)
                break;
        }

        return QPair<int,int>(num,denum);
    }

    return QPair<int,int>();
}


QSet<QString> QGstreamerVideoEncode::supportedStreamTypes(const QString &codecName) const
{
    return m_streamTypes.value(codecName);
}

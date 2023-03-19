#include "audio.h"
#include <QAudioInput>
#include <qendian.h>
#include <QDebug>

Audio::Audio(QObject *parent) : QObject(parent)
{

}


void Audio::initializeAudio(const QAudioDeviceInfo &deviceInfo)
{
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");
    if (!deviceInfo.isFormatSupported(format)) {
        qWarning() << "Default format not supported - trying to use nearest";
        format = deviceInfo.nearestFormat(format);
    }
    this->m_audioInfo.reset(new AudioInfo(format));
    connect(this->m_audioInfo.data(), &AudioInfo::update, this, &Audio::update_slot);
    this->m_audioInput.reset(new QAudioInput(deviceInfo, format));
    this->m_audioInfo->start();
    toggleMode();
}


void Audio::toggleMode()
{
    this->m_audioInput->stop();
    this->toggleSuspend();
    // Change bewteen pull and push modes
    if (this->m_pullMode) {
        this->m_audioInput->start(m_audioInfo.data());
    } else {
        auto io = this->m_audioInput->start();
        connect(io, &QIODevice::readyRead,
            [&, io]() {
                qint64 len = this->m_audioInput->bytesReady();
//                const int BufferSize = 4096;
//                if (len > BufferSize) len = BufferSize;
                QByteArray buffer(len, 0);
                qint64 l = io->read(buffer.data(), len);
                if (l > 0)
                    this->m_audioInfo->write(buffer.constData(), l);
            });
    }
    this->m_pullMode = !this->m_pullMode;
}


void Audio::toggleSuspend()
{
    // toggle suspend/resume
    if (this->m_audioInput->state() == QAudio::SuspendedState || this->m_audioInput->state() == QAudio::StoppedState) {
        this->m_audioInput->resume();
    } else if (this->m_audioInput->state() == QAudio::ActiveState) {
        this->m_audioInput->suspend();
    } else if (this->m_audioInput->state() == QAudio::IdleState) {
        // no-op
    }
}


void Audio::deviceChanged(QAudioDeviceInfo info)
{
    this->m_audioInfo->stop();
    this->m_audioInput->stop();
    this->m_audioInput->disconnect(this);
    initializeAudio(info);
}

void Audio::update_slot(qreal level)
{
    emit update(level);
}

/**********************************************************************************************************************************/

AudioInfo::AudioInfo(const QAudioFormat &format)
    : m_format(format)
{
    switch (this->m_format.sampleSize()) {
    case 8:
        switch (this->m_format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            this->m_maxAmplitude = 255;
            this->m_shiftAmplitude = 0;
            break;
        case QAudioFormat::SignedInt:
            this->m_maxAmplitude = 255;
            this->m_shiftAmplitude = 128;
            break;
        default:
            break;
        }
        break;
    case 16:
        switch (this->m_format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            this->m_maxAmplitude = 65535;
            this->m_shiftAmplitude = 0;
            break;
        case QAudioFormat::SignedInt:
            this->m_maxAmplitude = 65535;
            this->m_shiftAmplitude = 32768;
            break;
        default:
            break;
        }
        break;
    case 32:
        switch (this->m_format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            this->m_maxAmplitude = 0xffffffff;
            this->m_shiftAmplitude = 0;
            break;
        case QAudioFormat::SignedInt:
            this->m_maxAmplitude = 0xffffffff;
            this->m_shiftAmplitude = 0x80000000;
            break;
        case QAudioFormat::Float:
            this->m_maxAmplitude = 0x7fffffff; // Kind of
            this->m_shiftAmplitude = 0;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void AudioInfo::start()
{
    open(QIODevice::WriteOnly);
}

void AudioInfo::stop()
{
    if (this->isOpen()) this->close();
}

qint64 AudioInfo::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data)
    Q_UNUSED(maxlen)
    return 0;
}

qint64 AudioInfo::writeData(const char *data, qint64 len)
{
    if (this->m_maxAmplitude) {
        Q_ASSERT(this->m_format.sampleSize() % 8 == 0);
        const int channelBytes = this->m_format.sampleSize() / 8;
        const int sampleBytes = this->m_format.channelCount() * channelBytes;
        Q_ASSERT(len % sampleBytes == 0);
        const int numSamples = len / sampleBytes;
        quint32 value = 0;
        const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);
        for (int i = 0; i < numSamples; ++i) {
            for (int j = 0; j < this->m_format.channelCount(); ++j) {
                if (0 == j) {
                    if (this->m_format.sampleSize() == 8 && this->m_format.sampleType() == QAudioFormat::UnSignedInt) {
                        value = *reinterpret_cast<const quint8*>(ptr);
                    } else if (this->m_format.sampleSize() == 8 && this->m_format.sampleType() == QAudioFormat::SignedInt) {
                        value = qAbs(*reinterpret_cast<const qint8*>(ptr) + this->m_shiftAmplitude);
                    } else if (this->m_format.sampleSize() == 16 && this->m_format.sampleType() == QAudioFormat::UnSignedInt) {
                        if (this->m_format.byteOrder() == QAudioFormat::LittleEndian)
                            value = qFromLittleEndian<quint16>(ptr);
                        else
                            value = qFromBigEndian<quint16>(ptr);
                    } else if (this->m_format.sampleSize() == 16 && this->m_format.sampleType() == QAudioFormat::SignedInt) {
                        if (this->m_format.byteOrder() == QAudioFormat::LittleEndian)
                            value = qAbs(qFromLittleEndian<qint16>(ptr) + this->m_shiftAmplitude);
                        else
                            value = qAbs(qFromBigEndian<qint16>(ptr));
                    } else if (this->m_format.sampleSize() == 32 && this->m_format.sampleType() == QAudioFormat::UnSignedInt) {
                        if (this->m_format.byteOrder() == QAudioFormat::LittleEndian)
                            value = qFromLittleEndian<quint32>(ptr);
                        else
                            value = qFromBigEndian<quint32>(ptr);
                    } else if (this->m_format.sampleSize() == 32 && this->m_format.sampleType() == QAudioFormat::SignedInt) {
                        if (this->m_format.byteOrder() == QAudioFormat::LittleEndian)
                            value = qAbs(qFromLittleEndian<qint32>(ptr) + this->m_shiftAmplitude);
                        else
                            value = qAbs(qFromBigEndian<qint32>(ptr));
                    } else if (this->m_format.sampleSize() == 32 && this->m_format.sampleType() == QAudioFormat::Float) {
                        value = qAbs(*reinterpret_cast<const float*>(ptr) * 0x7fffffff); // assumes 0-1.0
                    }
                }
                ptr += channelBytes;
            }
            emit update(qreal(value) / this->m_maxAmplitude);
        }
    } else {
        emit update(this->m_level);
    }
    return len;
}


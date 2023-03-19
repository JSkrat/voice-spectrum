#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QAudioInput>

class AudioInfo;
class Audio : public QObject
{
    Q_OBJECT
private:
    QScopedPointer<AudioInfo> m_audioInfo;
    QScopedPointer<QAudioInput> m_audioInput;
    bool m_pullMode = true;

public:
    explicit Audio(QObject *parent = nullptr);
    void initializeAudio(const QAudioDeviceInfo &deviceInfo);
    void toggleMode();
    void toggleSuspend();
    void deviceChanged(QAudioDeviceInfo info);
signals:
    void update(qreal level);
public slots:
    void update_slot(qreal level);
};


class AudioInfo : public QIODevice
{
    Q_OBJECT

public:
    AudioInfo(const QAudioFormat &format);

    void start();
    void stop();

    qreal level() const { return m_level; }

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    const QAudioFormat m_format;
    quint32 m_maxAmplitude = 0;
    quint32 m_shiftAmplitude = 0;
    qreal m_level = 0.0; // 0.0 <= m_level <= 1.0

signals:
    void update(qreal level);
};


#endif // AUDIO_H

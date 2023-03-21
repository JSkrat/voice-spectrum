#ifndef SPECTRUMCALCULATOR_H
#define SPECTRUMCALCULATOR_H

#include <QObject>
//#include "audio.h"
#include "kiss_fft.h"
#include "spectraldata.h"

/// get waveform in slot setLevel
///  calculate spectrums and stuff
class SpectrumCalculator : public QObject
{
    Q_OBJECT
    const int bufLength;
    SpectralData &data;
    QVector <float> levelsForFFT;
    kiss_fft_cfg cfg;
public:
    explicit SpectrumCalculator(SpectralData &data, int bufferLength, QObject *parent = nullptr);

signals:
    void update();

public slots:
    void setLevel(qreal value);
};

#endif // SPECTRUMCALCULATOR_H

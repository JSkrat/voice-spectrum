#ifndef SPECTRALDATA_H
#define SPECTRALDATA_H

#include <QVector>

class SpectralData
{
public:
    int position = 0;
    int baseFrequency = 0;
    const int waveformLength = 512;
    const int spectrumLength;
    QVector <qreal> levels;
    QVector <qreal> amplitudes;
    QVector <qreal> phases;
    QVector <qreal> normalized;
    QVector <qreal> voice;
    SpectralData(int spectrumLength);
    void waveformAddSample(qreal value);
};

#endif // SPECTRALDATA_H

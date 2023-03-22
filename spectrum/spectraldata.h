#ifndef SPECTRALDATA_H
#define SPECTRALDATA_H

#include <QVector>

/// waveform and spectrums data source
///  calculator put all the data here
///  and redrerers draw from here
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
    QVector <qreal> smooth;
    QVector <qreal> derivative_1;
    QVector <qreal> derivative_2;
    QVector <qreal> voice;
    SpectralData(int spectrumLength);
    void waveformAddSample(qreal value);
};

#endif // SPECTRALDATA_H

#include "spectrumcalculator.h"

void SpectrumCalculator::calculateCompressedSpectrum(qreal maxAmplitude)
{
    // amplitude -> normalized
    // compress volume
    const qreal peakAmplitude = 200;
    const int minFrequency = 10;
    const qreal threshold = 100;
    this->data.baseFrequency = 0;
    for (int i = 1; i < this->bufLength; i++) {
        this->data.normalized[i] = peakAmplitude/maxAmplitude*this->data.amplitudes[i];
        if (minFrequency < i && threshold <= data.normalized[i] && 0 == this->data.baseFrequency) {
            this->data.baseFrequency = i;
        }
    }
}

void SpectrumCalculator::calculateSmoothSpectrum()
{
    // normalized -> smooth
    // gauss blur (sigma=1)
    // http://demofox.org/gauss.html
    const QVector<qreal> kernel = {0.0614, 0.2448, 0.3877, 0.2448, 0.0614};
    for (int i = 0; i < this->bufLength; i++) {
        qreal sum = 0;
        for (int j = 0; j < kernel.length(); j++) {
            int pixelIndex = i+j-kernel.length()/2;
            if (0 > pixelIndex) pixelIndex = 0;
            if (this->bufLength-1 < pixelIndex) pixelIndex = this->bufLength-1;
            sum += this->data.normalized[pixelIndex] * kernel[j];
        }
        this->data.smooth[i] = sum;
    }
}

void SpectrumCalculator::calculatePeaks()
{
    // smooth -> derivative_1, derivative_2
    // peaksat.m. ("Peaks Above Threshold") Syntax: P=peaksat(x,y,threshold). This function detects
    //  every y value that (a) has lower y values on both sides and (b) is above the specified threshold.
    //  Returns a 2 by n matrix P with the x and y values of each peak, where n is the number of detected peaks.
    // peaksatG.m. ("Peaks Above Threshold/Gaussian") Syntax: P=peaksatG(x,y,threshold,peakgroup).
    //  This function is similar to peakat.m but it additionally performs a Gaussian least-squares
    //  fit to the top of each detected peak to estimate its width and area; the number of data
    //  points at the top of the peak that are fit is determined by the input argument "peakgroup".
    //  Returns a 5 by n matrix P with the x and y values of each peak, where n is the number of detected peaks.

    // calculate derivatives first
    for (int i = 0; i < this->bufLength; i++) {
        int iPrev = i-1;
        if (0 > iPrev) iPrev = 0;
        this->data.derivative_1[i] = this->data.smooth[i] - this->data.smooth[iPrev];
        this->data.derivative_2[i] = this->data.derivative_1[i] - this->data.derivative_1[iPrev];
    }
}

SpectrumCalculator::SpectrumCalculator(SpectralData &data, int bufferLength, QObject *parent) :
    QObject(parent),
    bufLength(bufferLength),
    data(data)
{
    this->levelsForFFT.clear();
    // now init fft
    this->cfg = kiss_fft_alloc(this->bufLength, 0, 0, 0);
}

/// slot to accept audio data
/// recalculate spectrogramm every this->bufLength'th tick
void SpectrumCalculator::setLevel(qreal value)
{
    // waveform buffer
    this->data.waveformAddSample(value);
    // accumulate waveform sample for fft
    this->levelsForFFT.append(value); // amplitude, real part
    this->levelsForFFT.append(0); // imaginary part
    qreal maxAmplitude = 0;

    if (this->bufLength*2 <= this->levelsForFFT.length()) {
        kiss_fft_cpx *bufout = (kiss_fft_cpx*) malloc(sizeof(kiss_fft_cpx) * this->bufLength);
        kiss_fft(this->cfg, reinterpret_cast<const kiss_fft_cpx*>(this->levelsForFFT.constData()), bufout);
        for (int i = 1; i < this->bufLength; i++) {
            this->data.amplitudes[i] = sqrt(pow(bufout[i].r, 2) + pow(bufout[i].i, 2));
            if (maxAmplitude < data.amplitudes[i]) maxAmplitude = data.amplitudes[i];
            // apparently the phase is atan(r/i) or something like that, but not sure if we need that at all
//            this->data.phases[i] = bufout[i].i;
        }
        // populate compressed spectrum and find fundamental frequency of the voice
        this->calculateCompressedSpectrum(maxAmplitude);
        this->calculateSmoothSpectrum();
        this->calculatePeaks();
        free(bufout);
        this->levelsForFFT.clear();
    }
    // update the widget
    this->update();
}

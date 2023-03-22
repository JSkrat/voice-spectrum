#include "spectrumcalculator.h"

// magical code for kernel generation taken from http://demofox.org/gauss.html
float erf(float x) {
  // save the sign of x
  const int sign = (x >= 0) ? 1 : -1;
  x = std::abs(x);
  // constants
  const float a1 =  0.254829592;
  const float a2 = -0.284496736;
  const float a3 =  1.421413741;
  const float a4 = -1.453152027;
  const float a5 =  1.061405429;
  const float p  =  0.3275911;
  // A&S formula 7.1.26
  const float t = 1.0/(1.0 + p*x);
  const float y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * std::exp(-x * x);
  return sign * y; // erf(-x) = -erf(x);
}

float IntegrateGaussian(const float x, const float sigma)
{
  const float p1 = erf((x-0.5)/sigma*std::sqrt(0.5));
  const float p2 = erf((x+0.5)/sigma*std::sqrt(0.5));
  return (p2-p1)/2.0;
}

QVector<float> Generate(const float sigma)
{
  // calculate the kernel
  const float support = 0.995;
  const int radius = std::ceil(std::sqrt(-2.0 * sigma * sigma * std::log(1.0 - support)));
  QVector<float> minimalKernel = {};
  float sum = 0.0;
  for (int i = 0; i <= radius; ++i) {
    const float value = IntegrateGaussian(i, sigma);
    if (i == 0)
        sum += value;
    else
        sum += value * 2;
    minimalKernel.append(value);
  }
  for (int i = 0; i <= radius; ++i)
    minimalKernel[i] /= sum;
  return minimalKernel;
}

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

// ///////////////////////////////////////////////////////////////////////////// //

void SpectrumCalculator::calculateCompressedSpectrum(qreal maxAmplitude)
{
    // amplitude -> normalized
    // compress volume
    const qreal peakAmplitude = 200;
    const int minFrequency = 10;
    const qreal threshold = 100;
//    this->data.baseFrequency = 0;
    for (int i = 1; i < this->bufLength; ++i) {
        this->data.normalized[i] = peakAmplitude/maxAmplitude*this->data.amplitudes[i];
//        if (minFrequency < i && threshold <= data.normalized[i] && 0 == this->data.baseFrequency) {
//            this->data.baseFrequency = i;
//        }
    }
}

void SpectrumCalculator::calculateSmoothSpectrum()
{
    // normalized -> smooth
    // gauss blur (sigma=1)
    // http://demofox.org/gauss.html
//    const QVector<qreal> kernel = {0.0614, 0.2448, 0.3877, 0.2448, 0.0614};
    const QVector<float> kernel = Generate(this->smoothSigma);
    for (int i = 0; i < this->bufLength; i++) {
        qreal sum = 0;
        for (int j = -kernel.length()+1; j <= kernel.length()-1; ++j) {
            int pixelIndex = i + j;
            if (0 > pixelIndex) pixelIndex = 0;
            if (this->bufLength-1 < pixelIndex) pixelIndex = this->bufLength-1;
            sum += this->data.normalized[pixelIndex] * kernel[std::abs(j)];
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
    for (int i = 0; i < this->bufLength; ++i) {
        int iPrev = i-1;
        if (0 > iPrev) iPrev = 0;
        this->data.derivative_1[i] = this->data.smooth[i] - this->data.smooth[iPrev];
        this->data.derivative_2[i] = this->data.derivative_1[i] - this->data.derivative_1[iPrev];
    }
    // now process peaks
    this->data.baseFrequency = 0;
    for (int i = 1; i < this->bufLength-1; ++i) {
        bool d1SignChanged = sgn(this->data.derivative_1[i+1]) != sgn(this->data.derivative_1[i]);
        if ((0 == this->data.derivative_1[i] || d1SignChanged) && this->baseFrequencyThreshold < -this->data.derivative_2[i]) {
            if (10 < i && 0 == this->data.baseFrequency) this->data.baseFrequency = i;
        }
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

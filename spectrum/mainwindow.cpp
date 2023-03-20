#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAudioInput>
#include <QLayout>
#include <QPainter>
#include <QDebug>
#include "kiss_fft.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_canvas(this),
    grabber(this)
{
    ui->setupUi(this);
//    this->layout()->addWidget(this->m_canvas);
    // populate input audio devices combobox
    const QAudioDeviceInfo &defaultDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
    this->ui->m_deviceBox->addItem(defaultDeviceInfo.deviceName(), QVariant::fromValue(defaultDeviceInfo));
    for (auto &deviceInfo: QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        if (deviceInfo != defaultDeviceInfo) this->ui->m_deviceBox->addItem(deviceInfo.deviceName(), QVariant::fromValue(deviceInfo));
    }
    connect(this->ui->m_deviceBox, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::deviceChanged);
    // audio capturer
    this->grabber.initializeAudio(QAudioDeviceInfo::defaultInputDevice());
    connect(&(this->grabber), &Audio::update, &(this->m_canvas), &RenderArea::setLevel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ====================================================================================================================================

void MainWindow::deviceChanged(int index)
{
    QVariant itemData = this->ui->m_deviceBox->itemData(index);
    this->grabber.deviceChanged(itemData.value<QAudioDeviceInfo>());
}


RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
{
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    this->setGeometry(10, 45, this->width, this->height + 128);
    this->levels.fill(0, this->width);
    this->levelsForFFT.clear();
    this->amplitudes.fill(0, this->bufLength);
    this->phases.fill(0, this->bufLength);
    // now init fft
    this->cfg = kiss_fft_alloc(this->bufLength, 0, 0, 0);
}

/// redraw method
void RenderArea::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
//    painter.setPen(Qt::black);
//    painter.drawRect(QRect(painter.viewport().left()+10,
//                           painter.viewport().top()+10,
//                           painter.viewport().right()-20,
//                           painter.viewport().bottom()-20));
    // green waveform
    for (int i = 0; i < this->levels.count(); i++) {
        painter.fillRect(i, this->height * (1 - this->levels.at(i)), 1, this->height * this->levels.at(i), Qt::green);
    }
    painter.fillRect(this->position+1, 0, 1, this->height, Qt::black);
    // blue instant spectrogram
    for (int i = 0; i < this->bufLength; i++) {
        painter.fillRect(i, this->height, 1, (this->amplitudes.at(i))*4, Qt::blue);
    }
}

/// slot to accept audio data
/// recalculate spectrogramm every this->bufLength'th tick
void RenderArea::setLevel(qreal value)
{
    this->m_level = value;
    // waveform buffer
    this->levels[this->position++] = value;
    if (this->width <= this->position) this->position = 0;
    // accumulate waveform sample for fft
    this->levelsForFFT.append(value); // amplitude, real part
    this->levelsForFFT.append(0); // imaginary part
    if (this->bufLength*2 <= this->levelsForFFT.length()) {
        kiss_fft_cpx *bufout = (kiss_fft_cpx*) malloc(sizeof(kiss_fft_cpx) * this->bufLength);
        kiss_fft(this->cfg, reinterpret_cast<const kiss_fft_cpx*>(this->levelsForFFT.constData()), bufout);
        for (int i = 0; i < this->bufLength; i++) {
            this->amplitudes[i] = sqrt(pow(bufout[i].r, 2) + pow(bufout[i].i, 2));
            // apparently the phase is atan(r/i) or something like that, but not sure if we need that at all
//            this->phases[i] = bufout[i].i;
        }
        free(bufout);
        this->levelsForFFT.clear();
    }
    // update the widget
    this->update();
}



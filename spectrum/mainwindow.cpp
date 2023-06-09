#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAudioInput>
#include <QLayout>
#include <QDebug>
#include "renderwaveform.h"
#include "renderspectrum.h"
#include "renderdebug.h"

#define BUFFER_LENGTH 1024

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    grabber(this),
    data(BUFFER_LENGTH)
{
    // calculator converts waveform to a spectrum, normalized spectrum and other stuff and puts that into data
    this->calculator = new SpectrumCalculator(this->data, BUFFER_LENGTH, this);
    // renderers draw data on a form in specified windows of coordinates
    this->render.append({
                            new RenderWaveform(10, 45, BUFFER_LENGTH/2, 50, this->data, this),
                            new RenderSpectrum(10, 100, BUFFER_LENGTH/2, 200, this->data, this),
                            new RenderDebug(10, 305, BUFFER_LENGTH/2, 200, this->data, this),
                        });
    ui->setupUi(this);
    // populate input audio devices combobox
    const QAudioDeviceInfo &defaultDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
    this->ui->m_deviceBox->addItem(defaultDeviceInfo.deviceName(), QVariant::fromValue(defaultDeviceInfo));
    for (auto &deviceInfo: QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        if (deviceInfo != defaultDeviceInfo) this->ui->m_deviceBox->addItem(deviceInfo.deviceName(), QVariant::fromValue(deviceInfo));
    }
    connect(this->ui->m_deviceBox, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::deviceChanged);
    // audio capturer
    this->grabber.initializeAudio(QAudioDeviceInfo::defaultInputDevice());
    connect(&(this->grabber), &Audio::update, (this->calculator), &SpectrumCalculator::setLevel);
    // calculator to renderers connection
    for (auto *r: this->render) {
        connect(this->calculator, &SpectrumCalculator::update, r, [r]{r->update();});
    }
    // force sync ui
    this->on_smoothSigma_valueChanged(this->ui->smoothSigma->value());
}

MainWindow::~MainWindow()
{
    delete ui;
    delete this->calculator;
}

void MainWindow::deviceChanged(int index)
{
    QVariant itemData = this->ui->m_deviceBox->itemData(index);
    this->grabber.deviceChanged(itemData.value<QAudioDeviceInfo>());
}

void MainWindow::on_smoothSigma_valueChanged(int value)
{
    const float sigma = static_cast<float>(value)/100.0;
    this->calculator->smoothSigma = sigma;
    QString text = QString("σ %1").arg(sigma);
    this->ui->smoothSigma->setToolTip(text);
    this->ui->smoothSigmaValue->setText(text);
}

void MainWindow::on_baseFrequencyThreshold_valueChanged(int value)
{
    const float threshold = static_cast<float>(value)/10.0;
    this->calculator->baseFrequencyThreshold = threshold;
    QString text = QString("t %1").arg(threshold);
    this->ui->baseFrequencyThreshold->setToolTip(text);
    this->ui->baseFrequencyThresholdValue->setText(text);
}

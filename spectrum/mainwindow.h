#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioInput>
#include "audio.h"
#include "spectrumcalculator.h"
#include "spectraldata.h"
#include "renderer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    SpectrumCalculator *calculator;
    Audio grabber;
    SpectralData data;
    // renderers
    QList <Renderer*> render;

private slots:
    void deviceChanged(int index);

};

#endif // MAINWINDOW_H

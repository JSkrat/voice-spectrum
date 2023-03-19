#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioInput>
#include "audio.h"
#include "kiss_fft.h"


namespace Ui {
class MainWindow;
}


class RenderArea : public QWidget
{
    Q_OBJECT

public:
    explicit RenderArea(QWidget *parent = nullptr);

public slots:
    void setLevel(qreal value);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal m_level = 0;
    int position = 0;
    const int width = 600;
    const int height = 50;
    const int bufLength = 1024;
    QVector <qreal> levels;
    QVector <qreal> amplitudes;
    QVector <qreal> phases;
    QPixmap m_pixmap;
    QVector <float> levelsForFFT;
    kiss_fft_cfg cfg;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    RenderArea m_canvas;
    Audio grabber;

private slots:
    void deviceChanged(int index);

};

#endif // MAINWINDOW_H

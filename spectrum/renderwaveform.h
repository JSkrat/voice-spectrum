#ifndef RENDERWAVEFORM_H
#define RENDERWAVEFORM_H

#include <QWidget>
#include <QPainter>
#include "renderer.h"
#include "spectraldata.h"

class RenderWaveform : public Renderer
{
    Q_OBJECT
public:
    explicit RenderWaveform(int x, int y, int w, int h, SpectralData &data, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

public slots:
};

#endif // RENDERWAVEFORM_H

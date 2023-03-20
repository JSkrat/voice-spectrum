#ifndef RENDERER_H
#define RENDERER_H

#include <QWidget>
#include "spectraldata.h"

class Renderer : public QWidget
{
    Q_OBJECT
protected:
    SpectralData &data;
    int width;
    int height;
public:
    explicit Renderer(int x, int y, int w, int h, SpectralData &data, QWidget *parent = nullptr);

signals:

public slots:
};

#endif // RENDERER_H

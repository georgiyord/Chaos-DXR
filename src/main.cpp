#include "RechtRenderer.hpp"
#include "utils.hpp"
#include <QStatusBar>
#include <QtCore/qobjectdefs.h>
#include <basetsd.h>
#include <minwindef.h>
#include <qapplication.h>
#include <qdebug.h>
#include <qevent.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlogging.h>
#include <qmainwindow.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qsize.h>
#include <qstring.h>
#include <qtimer.h>
#include <qtypeinfo.h>
#include <qwidget.h>
#include <qwindow.h>
#include <qwindowdefs_win.h>
#include <span>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "RuntimeObject.lib") // winrt::com_ptr::as

class DX12Viewport : public QWindow
{
    RechtRenderer renderer;
    QImage m_renderedImage;
    QTimer *updateImageTimer;
    QTimer *updateFpsTimer;
    constexpr static UINT64 averageFpsBufferCapacity = 10;
    UINTRingBuffer<averageFpsBufferCapacity> averageFpsBuffer{};
    UINT averageFPS = 1;
    UINT currentFPS = 0;
    HSVGradientColor gradientColor;

    constexpr static UINT CYCLE_DURATION = 1000;

  public:
    DX12Viewport()
        : renderer(reinterpret_cast<HWND>(winId())), updateImageTimer(new QTimer(this)),
          updateFpsTimer(new QTimer(this))
    {
        QObject::connect(
            updateImageTimer, &QTimer::timeout, this,
            [this]() {
                // const FLOAT offset =
                //     std::abs(2.F * (frameIdx % CYCLE_DURATION) / static_cast<FLOAT>(CYCLE_DURATION) - 1.F);
                // const std::array<FLOAT, 4> color{
                //     offset,
                //     1.F - offset,
                //     0.F,
                //     1.F,
                // };
                // const std::array<FLOAT, 4> color{
                //     static_cast<float>(frameIdx % 3 == 0),
                //     static_cast<float>(frameIdx % 3 == 1),
                //     static_cast<float>(frameIdx % 3 == 2),
                //     1.F,
                // };
                renderer.renderMovingBar(gradientColor.getColorView(), width(), height());
                // renderer.renderRecht(gradientColor.getColorView(), width(), height());
            },
            Qt::DirectConnection);
        updateImageTimer->start();
        QObject::connect(
            updateFpsTimer, &QTimer::timeout, this,
            [this]() {
                static SIZE_T lastFrameIdx = 0;
                currentFPS = renderer.get_frameIdx() - lastFrameIdx;
                lastFrameIdx = renderer.get_frameIdx();
                averageFpsBuffer.push(currentFPS);
                averageFPS = averageFpsBuffer.average();
                gradientColor.updateDelta(1.F / averageFPS);
            },
            Qt::DirectConnection);
        updateFpsTimer->start(1000);
        updateImageTimer->start();
    }

    [[nodiscard]] SIZE_T getFPS() const
    {
        return currentFPS;
    }
    [[nodiscard]] UINT getAverageFPS() const
    {
        return averageFPS;
    }

    void resizeEvent(QResizeEvent *event) override
    {
        QWindow::resizeEvent(event);
        qDebug() << "QWindow size:" << event->size();
    }
};

class DX12MainWindow : public QMainWindow
{
    DX12Viewport viewPort;
    QWidget *viewPortContainer;
    QLabel *fpsMeter;
    QTimer *updateFpsTimer;

  public:
    DX12MainWindow(QWidget *parent = nullptr)
        : QMainWindow(parent), viewPortContainer(QWidget::createWindowContainer(&viewPort, this)),
          fpsMeter(new QLabel(this)), updateFpsTimer(new QTimer(this))
    {
        statusBar()->addPermanentWidget(fpsMeter);
        fpsMeter->setTextFormat(Qt::TextFormat::PlainText);
        QObject::connect(
            updateFpsTimer, &QTimer::timeout, this,
            [this]() {
                QString str{"FPS: "};
                str.append(QString::number(viewPort.getFPS()));
                str.append(" | Average FPS: ");
                str.append(QString::number(viewPort.getAverageFPS()));
                fpsMeter->setText(str);
            },
            Qt::DirectConnection);
        updateFpsTimer->start(1000);
        setCentralWidget(viewPortContainer);
        resize(320, 240);
        setWindowTitle("DX12 Demo");
        show();
    }

    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
        qDebug() << "QWindow container size:" << viewPortContainer->size();
    }
};

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    DX12MainWindow window;

    return app.exec();
}
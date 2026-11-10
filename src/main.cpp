#include "MovingBarRenderer.hpp"
#include "utils.hpp"
#include <QStatusBar>
#include <QtCore/qobjectdefs.h>
#include <basetsd.h>
#include <memory>
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

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "RuntimeObject.lib") // winrt::com_ptr::as

class DX12Viewport : public QWindow
{
    std::unique_ptr<MovingBarRenderer> renderer{nullptr};
    QImage m_renderedImage;
    QTimer *updateImageTimer;
    QTimer *updateFpsTimer;
    constexpr static UINT64 averageFpsBufferCapacity = 10;
    UINTRingBuffer<averageFpsBufferCapacity> averageFpsBuffer{};
    UINT averageFPS = 1;
    UINT currentFPS = 0;
    // HSVGradient gradientColor;
    RedGreenGradient gradientColor;

    constexpr static UINT CYCLE_DURATION = 1000;

  protected:
    void resizeEvent(QResizeEvent *ev) override
    {
        QWindow::resizeEvent(ev);
        qDebug() << "QWindow size:" << ev->size();
        if (renderer)
        {
            const auto size = ev->size();
            renderer->resize(size.width(), size.height());
        }
        else
        {
            qWarning() << "Could not resize renderer as it does not exist. Maybe QWindow is still initialising?";
        }
    }

  public:
    DX12Viewport() : updateImageTimer(new QTimer(this)), updateFpsTimer(new QTimer(this))
    {
        renderer = std::make_unique<MovingBarRenderer>(reinterpret_cast<HWND>(winId()));
        QObject::connect(
            updateImageTimer, &QTimer::timeout, this,
            [this]() {
                renderer->setBarColor(gradientColor.getColorView());
                renderer->render();
            },
            Qt::DirectConnection);
        updateImageTimer->start();
        QObject::connect(
            updateFpsTimer, &QTimer::timeout, this,
            [this]() {
                static SIZE_T lastFrameIdx = 0;
                currentFPS = renderer->get_frameIdx() - lastFrameIdx;
                lastFrameIdx = renderer->get_frameIdx();
                averageFpsBuffer.push(currentFPS);
                averageFPS = averageFpsBuffer.average();
                gradientColor.updateDelta(1.F / averageFPS);
            },
            Qt::DirectConnection);
        updateFpsTimer->start(1000);
    }

    [[nodiscard]] SIZE_T getFPS() const
    {
        return currentFPS;
    }
    [[nodiscard]] UINT getAverageFPS() const
    {
        return averageFPS;
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
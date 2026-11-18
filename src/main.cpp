#include "Renderers/TriangleRenderer/TriangleRenderer.hpp"
#include "utils.hpp"
#include <Windows.h>
#include <memory>
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
#include <qstatusbar.h>
#include <qstring.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qwindow.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "RuntimeObject.lib") // winrt::com_ptr::as

class DX12Viewport : public QWindow
{
    std::unique_ptr<TriangleRenderer> renderer{nullptr};
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
            renderer->changeResolution(width(), height());
        }
    }

    bool event(QEvent *ev) override
    {
        qDebug() << ev->type();
        if (ev->type() == QEvent::Type::PlatformSurface)
        {
            switch (dynamic_cast<QPlatformSurfaceEvent *>(ev)->surfaceEventType())
            {
            case QPlatformSurfaceEvent::SurfaceCreated:
                renderer = std::make_unique<TriangleRenderer>(reinterpret_cast<HWND>(winId()));
                renderer->setTriangleTranslation(true);
                renderer->setTriangleRotation(true);
                renderer->setTriangleCheckeredPattern(true);
                renderer->set_bgColor({0.0F, 0.5F, 0.0F, 1.F});
                break;
            case QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed:
                renderer->shutdown();
                renderer = nullptr;
                break;
            }
        }
        return QWindow::event(ev);
    }

  public:
    DX12Viewport() : updateImageTimer(new QTimer(this)), updateFpsTimer(new QTimer(this))
    {
        renderer = std::make_unique<TriangleRenderer>(reinterpret_cast<HWND>(winId()));
        QObject::connect(
            updateImageTimer, &QTimer::timeout, this,
            [this]() {
                if (renderer)
                {
                    renderer->update();
                    renderer->render();
                }
            },
            Qt::DirectConnection);
        updateImageTimer->start();
        QObject::connect(
            updateFpsTimer, &QTimer::timeout, this,
            [this]() {
                if (renderer)
                {
                    static SIZE_T lastFrameIdx = 0;
                    currentFPS = renderer->get_frameIdx() - lastFrameIdx;
                    lastFrameIdx = renderer->get_frameIdx();
                    averageFpsBuffer.push(currentFPS);
                    averageFPS = averageFpsBuffer.average();
                    gradientColor.updateDelta(1.F / averageFPS);
                }
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
        resize(1280, 720);
        show();
    }

    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
        qDebug() << "QWindow container size:" << viewPortContainer->size();
    }
};

class DX12App : public QApplication
{
    DX12MainWindow window;

  public:
    DX12App(int argc, char *argv[]) : QApplication(argc, argv)
    {
    }
};

int main(int argc, char *argv[])
{
    DX12App app(argc, argv);

    return DX12App::exec();
}
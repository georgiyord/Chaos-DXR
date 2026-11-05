#include <cstddef>
#include <qimage.h>
#include <qlabel.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qwidget.h>
#include <winsock.h>
#define D3DCOMPILE_DEBUG 1

#include "RechtRenderer.hpp"
#include "Renderer.hpp"
#include "utils.hpp"
#include <basetsd.h>
#include <cstdlib>
#include <cstring>
#include <fileapi.h>
#include <handleapi.h>
#include <minwindef.h>
#include <span>
#include <strsafe.h>
#include <wil/resource.h>
#include <wil/wistd_type_traits.h>
#include <windows.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")

#include <QStatusBar>
#include <QtWidgets>
#include <qgraphicsitem.h>
#include <qgraphicsview.h>
#include <qmainwindow.h>
#include <qpixmap.h>

class DX12Viewport : public QWidget
{
    QImage m_renderedImage;
    QTimer updateImageTimer;
    Renderer renderer;
    RechtData rechtData;
    UINT64 frameIdx = 0;

    constexpr static UINT CYCLE_DURATION = 1000;

  protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter{this};
        painter.drawImage(rect(), m_renderedImage);
    };

  public:
    DX12Viewport(QWidget *parent = nullptr)
        : QWidget(parent), rechtData(renderer.AsRechtRenderer()->RenderSquare(randomColor()))
    {
        // setAlignment(Qt::AlignCenter);
        setMinimumSize(1, 1);
        QObject::connect(
            &updateImageTimer, &QTimer::timeout, this,
            [this]() {
                static QPainter painter(this);
                const FLOAT offset =
                    std::abs(2.F * (frameIdx % CYCLE_DURATION) / static_cast<FLOAT>(CYCLE_DURATION) - 1.F);
                const std::array<FLOAT, 4> color{offset, 1.F - offset, 0.F, 1.F};
                rechtData = renderer.AsRechtRenderer()->RenderSquare(color, width(), height());
                m_renderedImage = QImage{reinterpret_cast<const UCHAR *>(rechtData.data()),
                                         static_cast<int>(rechtData.width()), static_cast<int>(rechtData.height()),
                                         static_cast<qsizetype>(rechtData.rowPitch()), QImage::Format::Format_RGBA8888};
                // setPixmap(QPixmap::fromImage(m_renderedImage));
                ++frameIdx;
                update();
            },
            Qt::DirectConnection);
        updateImageTimer.start();
    }

    [[nodiscard]] UINT64 getFrameIdx() const
    {
        return frameIdx;
    }
};

class DX12MainWindow : public QMainWindow
{
    DX12Viewport viewPort;
    QLabel fpsMeter;
    QTimer updateFpsTimer;
    constexpr static UINT64 averageFpsBufferCapacity = 10;
    UINTRingBuffer<averageFpsBufferCapacity> averageFpsBuffer{};

  public:
    DX12MainWindow(QWidget *parent = nullptr) : QMainWindow(parent), viewPort(this)
    {
        statusBar()->addPermanentWidget(&fpsMeter);
        fpsMeter.setTextFormat(Qt::TextFormat::PlainText);
        QObject::connect(
            &updateFpsTimer, &QTimer::timeout, this,
            [this]() {
                static UINT64 lastFrameIdx = 0;
                const UINT64 frames = viewPort.getFrameIdx() - lastFrameIdx;
                lastFrameIdx = viewPort.getFrameIdx();
                averageFpsBuffer.push(frames);
                QString str{"FPS: "};
                str.append(QString::number(frames));
                str.append(" | Average FPS: ");
                str.append(QString::number(averageFpsBuffer.average()));
                fpsMeter.setText(str);
            },
            Qt::DirectConnection);
        updateFpsTimer.start(1000);
        setCentralWidget(&viewPort);
        resize(320, 240);
        // setWindowTitle(QApplication::translate("toplevel", "Top-level widget"));
        show();
    }
};

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    DX12MainWindow window;

    return app.exec();
}
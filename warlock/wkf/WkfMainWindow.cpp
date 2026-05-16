#include "MercatorMapWidget.hpp"
#include "WkfMainWindow.hpp"

#include "qsurfaceformat.h"
#include "qtoolbar.h"
#include "QStatusBar"
#include "QFileDialog"
#include "QFileInfo"
#include <algorithm>

namespace wkf
{

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(4);  // 4倍多重采样抗锯齿
    QSurfaceFormat::setDefaultFormat(format);

    setupUI();
    setupConnections();
}


void MainWindow::setupUI() {
    // 创建地图控件
    m_mapWidget = new MercatorMapWidget(this);
    setCentralWidget(m_mapWidget);

    // 创建工具栏
    QToolBar* toolBar = addToolBar("工具");
    
    // 缩放控制
    toolBar->addAction("+", m_mapWidget, &MercatorMapWidget::zoomIn);
    toolBar->addAction("-", m_mapWidget, &MercatorMapWidget::zoomOut);
    toolBar->addAction("重置", m_mapWidget, &MercatorMapWidget::resetView);
    
    toolBar->addSeparator();

    // 矢量数据加载
    toolBar->addAction("加载矢量", [this]() {
        QString path = QFileDialog::getOpenFileName(this, "加载矢量数据", "projects", "JSON (*.json)");
        if (!path.isEmpty())
            m_mapWidget->LoadVectorData(path);
    });

    toolBar->addSeparator();

    // 快速定位按钮
    // toolBar->addAction("北京", [this]() { m_mapWidget->setViewCenter(116.4074, 39.9042); });
    // toolBar->addAction("伦敦", [this]() { m_mapWidget->setViewCenter(-0.1276, 51.5072); });
    // toolBar->addAction("纽约", [this]() { m_mapWidget->setViewCenter(-74.0060, 40.7128); });

    // toolBar->addSeparator();
    
    // 缩放级别显示
    m_zoomLabel = new QLabel("Zoom: 2.0 X", this);
    toolBar->addWidget(m_zoomLabel);
    
    setWindowTitle("MiniSim");
    resize(1920, 1080);

}

void MainWindow::onZoomChanged(float aZoomLevel) 
{
    m_zoomLabel->setText(QString("Zoom: %1 X").arg(aZoomLevel, 0, 'f', 1));
}

void MainWindow::onMapClicked(const QPoint& pos) 
{
    // QVector2D lonLat = m_mapWidget->screenToLonLat(pos);
    // QString status = QString("经度: %1°, 纬度: %2°")
    //                     .arg(lonLat.x(), 0, 'f', 6)
    //                     .arg(lonLat.y(), 0, 'f', 6);
    // statusBar()->showMessage(status, 3000);
}

void MainWindow::setupConnections() {
    // 点击地图显示坐标
    connect(m_mapWidget, &MercatorMapWidget::customContextMenuRequested, 
            this, &MainWindow::onMapClicked);
    connect(m_mapWidget, &MercatorMapWidget::ZoomChange, this, &MainWindow::onZoomChanged);
}

void MainWindow::UpdatePlatformMap(const PlatformMap &aPlatformMap)
{
    emit SignalUpdatePlatformMap(aPlatformMap);
}


}
#pragma once

#include "VectorFeature.hpp"
#include "VectorRenderer.hpp"
#include "WkfMainWindow.hpp"
#include "wkf_export.h"

#include <QOpenGLWidget>
#include "QOpenGLExtraFunctions"
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QTime>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include <qobjectdefs.h>
#include <qtimer.h>

#include <memory>

namespace wkf 
{

class WKF_EXPORT MercatorMapWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions {
    Q_OBJECT
    
public:
    explicit MercatorMapWidget(QWidget* parent = nullptr);
    ~MercatorMapWidget();
    
    void UpdatePlatformMap(const PlatformMap &aMap);

    void LoadVectorData(const QString& path);

    void setViewCenter(double lon, double lat);
    void setZoomLevel(double level);
    void zoomIn();
    void zoomOut();
    void resetView();
    
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    // 交互事件
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    
private:
    double latToMercatorY(double lat) const;
    double mercatorYToLat(double y) const;
    
    // 投影坐标 <-> 世界坐标
    QVector2D projectLonLat(double lon, double lat) const;
    QVector2D unprojectWorld(const QVector2D& worldPos) const;
    
    QMatrix4x4 getViewMatrix() const;
    QMatrix4x4 getProjectionMatrix() const;
    
    void renderBackground();
    void renderGrid();
    void renderText();
    void renderTestPoints();
    
    void updateGridData();
    
    QVector2D screenToWorld(const QPointF& screenPos) const;
    QPointF worldToScreen(const QVector2D& worldPos) const;

    void PeparePlatformSprites();
    int RenaderPlatforms();

    void PrepareRenderTile();
    int RenderTiles();

    void PrepareVectorRenderer();
    int RenderVectorFeatures();

private:

    QTimer mUpdateTimer;

    QOpenGLShaderProgram* m_program;
    QOpenGLBuffer m_gridVbo;
    QOpenGLVertexArrayObject m_gridVao;
    
    QVector2D m_viewCenter;
    double m_zoomLevel;
    QMatrix4x4 m_viewMatrix;
    QMatrix4x4 m_projectionMatrix;
    
    bool m_dragging;
    QPoint m_lastMousePos;
    QVector2D m_dragStartViewCenter;
    
    struct GridLine {
        QVector3D start;
        QVector3D end;
        QColor color;
    };
    QVector<GridLine> m_longitudeLines;
    QVector<GridLine> m_latitudeLines;
    
    int m_gridDensity;
    QColor m_bgColor;
    QColor m_gridColor;
    QColor m_equatorColor;
    QColor m_primeMeridianColor;
    
    // 测试点（用于验证）
    QVector<QVector2D> m_testPoints;

    // Vector data
    VectorFeatureSet m_vectorFeatureSet;
    std::unique_ptr<VectorRenderer> m_vectorRenderer;

    // OpenGL平台目标
    PlatformMap m_platformMap;
    GLuint m_platformVAO {0};
    GLuint m_platformVBO {0};
    QOpenGLShaderProgram* m_platformProgram{nullptr};
    // QOpenGLVertexArrayObject m_platformVAO;

    // Tile Target
    GLuint m_tileVAO {0};
    GLuint m_tileVBO {0};
    QOpenGLShaderProgram* m_tileProgram{nullptr};

    // Performance metrics
    QElapsedTimer m_perfTimer;
    int m_frameCount = 0;
    int m_drawCallCount = 0;
    float m_currentFPS = 0.0f;
    int m_currentDrawCalls = 0;
    int m_estimatedVRAM_MB = 0;
    
signals:
    void ZoomChange(float aZoomLevel);

};

}
#include "MercatorMapWidget.hpp"
#include "ResourceManager.hpp"
#include "SpriteAttachment.hpp"
#include "UtMath.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include <QCoreApplication>
#include <QFile>
#include <QOpenGLContext>
#include <QOpenGLShader>
#include <QPainter>
#include <QDebug>
#include <cmath>
#include <cstddef>
#include <gl/gl.h>
#include <qchar.h>
#include <qmainwindow.h>
#include <qmatrix4x4.h>
#include <qobjectdefs.h>
#include <qopenglext.h>
#include <qpoint.h>
#include <qtimer.h>
#include <qtransform.h>
#include <qvector2d.h>
#include <qvector3d.h>
#include <vector>
#include "TiffLoader.hpp"

namespace wkf 
{

// 常量定义
constexpr double PI = 3.14159265358979323846;
constexpr double EARTH_RADIUS = 6378137.0;  // WGS84椭球体长半轴（米）
constexpr double EARTH_CIRCUMFERENCE = 2.0 * PI * EARTH_RADIUS;
constexpr double MAX_LAT = 85.05112878;     // 墨卡托投影有效最大纬度（±85.05°）

// 顶点着色器（简单绘制）
const char* VERTEX_SHADER_SRC = R"(
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

uniform mat4 modelViewProjection;
out vec3 fragColor;

void main() {
    gl_Position = modelViewProjection * vec4(position, 1.0);
    fragColor = color;
}
)";

// 片段着色器
const char* FRAGMENT_SHADER_SRC = R"(
#version 330 core
in vec3 fragColor;
out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
)";

MercatorMapWidget::MercatorMapWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , m_program(nullptr)
    , m_viewCenter(0.0f, 0.0f)          // 中心在经度0，纬度0
    , m_zoomLevel(2.0)                  // 初始缩放级别
    , m_dragging(false)
    , m_gridDensity(10)
    , m_bgColor(255, 255, 255, 255)
    , m_gridColor(0, 0, 0, 255)
    , m_equatorColor(255, 100, 100)    // 红色赤道
    , m_primeMeridianColor(100, 100, 255)
{
    setFocusPolicy(Qt::StrongFocus);
    
    // 添加测试点
    m_testPoints.append(QVector2D(0.0f, 0.0f));      // 赤道与本初子午线交点
    m_testPoints.append(QVector2D(116.4074f, 39.9042f));  // 北京
    m_testPoints.append(QVector2D(-74.0060f, 40.7128f));  // 纽约
    m_testPoints.append(QVector2D(151.2093f, -33.8688f)); // 悉尼

    // CallBack
    if (MainWindow *window = dynamic_cast<MainWindow *>(parent))
    {
        connect(window, &MainWindow::SignalUpdatePlatformMap, this, &MercatorMapWidget::UpdatePlatformMap);
    }

    connect(&mUpdateTimer, &QTimer::timeout, this, [this]()
    {
        this->update();
    });
    mUpdateTimer.setInterval(10);
    mUpdateTimer.start();
}

MercatorMapWidget::~MercatorMapWidget() {
    makeCurrent();
    delete m_program;
    m_gridVbo.destroy();
    m_gridVao.destroy();
    doneCurrent();
}

// ==================== 墨卡托投影计算 ====================
double MercatorMapWidget::latToMercatorY(double lat) const {
    // 将纬度限制在有效范围内
    lat = qBound(-MAX_LAT, lat, MAX_LAT);
    
    // 墨卡托投影公式: y = R * ln(tan(π/4 + φ/2))
    // 其中 φ 是纬度（弧度）
    double phi = lat * PI / 180.0;
    double y = EARTH_RADIUS * std::log(std::tan(PI/4.0 + phi/2.0));
    
    // 归一化到 [-1, 1] 范围（方便OpenGL渲染）
    return y / (EARTH_CIRCUMFERENCE / 2.0);
}

double MercatorMapWidget::mercatorYToLat(double y) const {
    // 反墨卡托投影公式: φ = 2 * atan(exp(y/R)) - π/2
    // 先将归一化坐标转回米制
    double y_meters = y * (EARTH_CIRCUMFERENCE / 2.0);
    double lat_rad = 2.0 * std::atan(std::exp(y_meters / EARTH_RADIUS)) - PI/2.0;
    return lat_rad * 180.0 / PI;
}

QVector2D MercatorMapWidget::projectLonLat(double lon, double lat) const {
    double x = (lon / 180.0) * (EARTH_CIRCUMFERENCE / 2.0);
    // 归一化到 [-1, 1]
    x = x / (EARTH_CIRCUMFERENCE / 2.0);
    
    double y = latToMercatorY(lat);
    
    return QVector2D(x, y);
}

QVector2D MercatorMapWidget::unprojectWorld(const QVector2D& worldPos) const {
    double x_meters = worldPos.x() * (EARTH_CIRCUMFERENCE / 2.0);
    double lon = (x_meters / EARTH_RADIUS) * 180.0 / PI;
    
    double lat = mercatorYToLat(worldPos.y());
    
    return QVector2D(lon, lat);
}

void MercatorMapWidget::initializeGL() {
    initializeOpenGLFunctions();
    
    // 设置OpenGL状态
    glClearColor(m_bgColor.redF(), m_bgColor.greenF(), m_bgColor.blueF(), 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    // 创建着色器程序
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, VERTEX_SHADER_SRC);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, FRAGMENT_SHADER_SRC);
    m_program->link();
    
    if (!m_program->isLinked()) {
        qDebug() << "Shader linking failed:" << m_program->log();
    }
    
    // 初始化网格VAO/VBO
    m_gridVao.create();
    m_gridVbo.create();
    
    // 初始更新网格数据
    updateGridData();

    PeparePlatformSprites();

    PrepareRenderTile();

    PrepareVectorRenderer();
}

void MercatorMapWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    updateGridData();
}


// ==================== 网格数据更新 ====================
void MercatorMapWidget::updateGridData() {
    // 根据缩放级别调整网格密度
    if (m_zoomLevel < 3.0) {
        m_gridDensity = 10;  // 低缩放：每10度一条线
    } else if (m_zoomLevel < 6.0) {
        m_gridDensity = 5;   // 中缩放：每5度一条线
    } else {
        m_gridDensity = 1;   // 高缩放：每1度一条线
    }
    
    // 清空现有网格数据
    m_longitudeLines.clear();
    m_latitudeLines.clear();
    
    // 计算当前可见的世界坐标范围
    QMatrix4x4 invView = (getProjectionMatrix() * getViewMatrix()).inverted();
    QVector4D bottomLeftNDC(-1.0f, -1.0f, 0.0f, 1.0f);
    QVector4D topRightNDC(1.0f, 1.0f, 0.0f, 1.0f);
    QVector4D bottomLeftWorld = invView * bottomLeftNDC;
    QVector4D topRightWorld = invView * topRightNDC;
    
    float minX = bottomLeftWorld.x();
    float maxX = topRightWorld.x();
    float minY = bottomLeftWorld.y();
    float maxY = topRightWorld.y();
    
    for (int lon = -180; lon <= 180; lon += m_gridDensity) {
        // 添加经线（垂直线）
        double x = projectLonLat(lon, 0).x();
        
        // 只添加可见范围内的经线
        if (x >= minX && x <= maxX) {
            GridLine line;
            line.start = QVector3D(x, -1.0f, 0.0f);
            line.end = QVector3D(x, 1.0f, 0.0f);
            
            // 本初子午线用特殊颜色
            if (lon == 0) {
                line.color = m_primeMeridianColor;
            } else {
                line.color = m_gridColor;
            }
            
            m_longitudeLines.append(line);
        }
    }
    
    // 添加纬线（水平线）
    for (int lat = -80; lat <= 80; lat += m_gridDensity) {
        double y = latToMercatorY(lat);
        
        // 只添加可见范围内的纬线
        if (y >= minY && y <= maxY) {
            GridLine line;
            line.start = QVector3D(-1.0f, y, 0.0f);
            line.end = QVector3D(1.0f, y, 0.0f);
            
            // 赤道用特殊颜色
            if (lat == 0) {
                line.color = m_equatorColor;
            } else {
                line.color = m_gridColor;
            }
            
            m_latitudeLines.append(line);
        }
    }
    
    // 更新网格VBO数据
    if (m_gridVao.isCreated()) {
        m_gridVao.bind();
        
        // 计算总顶点数（每条线2个顶点）
        int totalLines = m_longitudeLines.size() + m_latitudeLines.size();
        int vertexCount = totalLines * 2;
        
        // 准备顶点数据：位置 + 颜色
        QVector<float> vertexData;
        vertexData.reserve(vertexCount * 6);  // 每个顶点: (x,y,z) + (r,g,b)
        
        // 添加经线数据
        for (const GridLine& line : m_longitudeLines) {
            vertexData << line.start.x() << line.start.y() << line.start.z()
                       << line.color.redF() << line.color.greenF() << line.color.blueF();
            vertexData << line.end.x() << line.end.y() << line.end.z()
                       << line.color.redF() << line.color.greenF() << line.color.blueF();
        }
        
        // 添加纬线数据
        for (const GridLine& line : m_latitudeLines) {
            vertexData << line.start.x() << line.start.y() << line.start.z()
                       << line.color.redF() << line.color.greenF() << line.color.blueF();
            vertexData << line.end.x() << line.end.y() << line.end.z()
                       << line.color.redF() << line.color.greenF() << line.color.blueF();
        }
        
        // 上传到VBO
        m_gridVbo.bind();
        m_gridVbo.allocate(vertexData.constData(), vertexData.size() * sizeof(float));
        
        // 设置顶点属性指针
        glEnableVertexAttribArray(0);  // 位置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        
        glEnableVertexAttribArray(1);  // 颜色
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        
        m_gridVbo.release();
        m_gridVao.release();
    }
}

void MercatorMapWidget::paintGL() {
    // --- FPS tracking ---
    if (!m_perfTimer.isValid())
        m_perfTimer.start();
    ++m_frameCount;
    qint64 elapsed = m_perfTimer.elapsed();
    if (elapsed >= 1000) {
        m_currentFPS = m_frameCount * 1000.0f / elapsed;
        m_frameCount = 0;
        m_perfTimer.restart();
    }

    m_drawCallCount = 0;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 更新矩阵
    m_viewMatrix = getViewMatrix();
    m_projectionMatrix = getProjectionMatrix();
    QMatrix4x4 mvp = m_projectionMatrix * m_viewMatrix;

    // 使用着色器程序
    m_program->bind();
    m_program->setUniformValue("modelViewProjection", mvp);

    // 渲染网格
    // if (m_gridVao.isCreated()) {
    //     m_gridVao.bind();
    //     glDrawArrays(GL_LINES, 0, (m_longitudeLines.size() + m_latitudeLines.size()) * 2);
    //     ++m_drawCallCount;
    //     m_gridVao.release();
    // }

    m_program->release();

    m_drawCallCount += RenderTiles();
    m_drawCallCount += RenderVectorFeatures();
    m_drawCallCount += RenaderPlatforms();

    // Store current draw calls for display
    m_currentDrawCalls = m_drawCallCount;

    // Estimate VRAM
    TiffLoader &tileLoader = wkfEnv.GetResourceManager().GetTileLoader();
    m_estimatedVRAM_MB = tileLoader.GetEstimatedVRAM_MB();

    // Draw overlay: labels + stats
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::TextAntialiasing);

        // --- City labels ---
        QFont labelFont("Microsoft YaHei", 10);
        painter.setFont(labelFont);

        auto visibleFeatures = m_vectorFeatureSet.GetVisibleFeatures(m_zoomLevel);
        for (const auto* f : visibleFeatures) {
            if (f->type != VectorFeatureType::Point || f->name.isEmpty())
                continue;
            if (f->coordinates.empty())
                continue;
            double lon = f->coordinates[0].x();
            double lat = f->coordinates[0].y();
            auto world_coord = projectLonLat(lon, lat);
            QPointF screenPos = worldToScreen(world_coord);

            if (screenPos.x() < 0 || screenPos.x() > width() ||
                screenPos.y() < 0 || screenPos.y() > height())
                continue;

            painter.setPen(f->strokeColor.darker(150));
            painter.drawText(screenPos + QPointF(f->width + 3, 4), f->name);
        }

        // --- Performance stats ---
        QFont statFont("Consolas", 11);
        painter.setFont(statFont);
        painter.setPen(QColor(220, 220, 220));
        painter.setBrush(QColor(0, 0, 0, 160));
        int boxX = 10, boxY = 10;
        int boxW = 280, boxH = 80;
        painter.drawRect(boxX, boxY, boxW, boxH);

        painter.setPen(Qt::white);
        int lineY = boxY + 18;
        painter.drawText(boxX + 8, lineY, QString("FPS: %1").arg(m_currentFPS, 0, 'f', 1));
        lineY += 18;
        painter.drawText(boxX + 8, lineY, QString("Draw Calls: %1").arg(m_currentDrawCalls));
        lineY += 18;
        painter.drawText(boxX + 8, lineY, QString("VRAM Est: %1 MB").arg(m_estimatedVRAM_MB));
        lineY += 18;
        painter.drawText(boxX + 8, lineY, QString("Vector Features: %1").arg(visibleFeatures.size()));
    }
}

// ==================== 交互事件 ====================
void MercatorMapWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastMousePos = event->pos();
        m_dragStartViewCenter = m_viewCenter;
    }
    QOpenGLWidget::mousePressEvent(event);
}

void MercatorMapWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!m_dragging)
        return;


    QVector2D worldNow = screenToWorld(m_lastMousePos);

    // // 拖拽起点 → 世界坐标
    QVector2D worldStart = screenToWorld(event->pos());

    // 世界位移（鼠标向右，世界向左）
    QVector2D deltaWorld = worldNow - worldStart;

    m_viewCenter = m_dragStartViewCenter + deltaWorld;

    updateGridData();
    update();
}


void MercatorMapWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
    }
    QOpenGLWidget::mouseReleaseEvent(event);
}

QMatrix4x4 MercatorMapWidget::getViewMatrix() const {
    QMatrix4x4 view;
    view.translate(-m_viewCenter.x(), -m_viewCenter.y());
    
    return view;
}

QMatrix4x4 MercatorMapWidget::getProjectionMatrix() const {
    QMatrix4x4 projection;
    
    float aspect = static_cast<float>(width()) / height();
    float worldHeight = 2.0f / std::pow(2.0f, m_zoomLevel);
    float worldWidth = worldHeight * aspect;
    
    projection.ortho(-worldWidth/2, worldWidth/2, -worldHeight/2, worldHeight/2, -1.0f, 1.0f);
    
    return projection;
}


QVector2D MercatorMapWidget::screenToWorld(const QPointF& screenPos) const
{
    float ndcX =  2.0f * screenPos.x() / width()  - 1.0f;
    float ndcY =  1.0f - 2.0f * screenPos.y() / height();

    
    QMatrix4x4 invVP = (getProjectionMatrix() * getViewMatrix()).inverted();

    QVector4D worldH = invVP * QVector4D(ndcX, ndcY, 0.0f, 1.0f);

    
    if (!qFuzzyIsNull(worldH.w())) {
        worldH /= worldH.w();
    }

    return QVector2D(worldH.x(), worldH.y());
}

QPointF MercatorMapWidget::worldToScreen(const QVector2D& worldPos) const
{
    
    QMatrix4x4 vp = getProjectionMatrix() * getViewMatrix();
    QVector4D clip = vp * QVector4D(worldPos.x(), worldPos.y(), 0.0f, 1.0f);

    
    if (!qFuzzyIsNull(clip.w())) {
        clip /= clip.w();
    }

    
    float x = (clip.x() * 0.5f + 0.5f) * width();
    float y = (1.0f - (clip.y() * 0.5f + 0.5f)) * height();

    return QPointF(x, y);
}



void MercatorMapWidget::wheelEvent(QWheelEvent* event)
{
    QPoint numDegrees = event->angleDelta() / 8;
    if (numDegrees.isNull())
        return;

    double zoomDelta = numDegrees.y() / 120.0 * 0.5;

    QVector2D worldBefore = screenToWorld(event->pos());

    double oldZoom = m_zoomLevel;
    m_zoomLevel = qBound(0.5, m_zoomLevel + zoomDelta, 10.0);

    if (qFuzzyCompare(oldZoom, m_zoomLevel))
        return;

    QVector2D worldAfter = screenToWorld(event->pos());
    
    m_viewCenter += (worldBefore - worldAfter);

    updateGridData();
    update();
    event->accept();

    emit ZoomChange(m_zoomLevel);
}



// ==================== 公共接口 ====================
void MercatorMapWidget::setViewCenter(double lon, double lat) {
    m_viewCenter = projectLonLat(lon, lat);
    updateGridData();
    update();
}

void MercatorMapWidget::setZoomLevel(double level) {
    m_zoomLevel = qBound(0.5, level, 10.0);
    updateGridData();
    update();
    emit ZoomChange(m_zoomLevel);
}

void MercatorMapWidget::zoomIn() {
    setZoomLevel(m_zoomLevel + 0.5);
}

void MercatorMapWidget::zoomOut() {
    setZoomLevel(m_zoomLevel - 0.5);
}

void MercatorMapWidget::resetView() {
    m_viewCenter = QVector2D(0.0f, 0.0f);
    m_zoomLevel = 2.0f;
    updateGridData();
    update();

    emit ZoomChange(m_zoomLevel);
}

void MercatorMapWidget::UpdatePlatformMap(const PlatformMap &aMap)
{
    m_platformMap = aMap;
}

void MercatorMapWidget::PeparePlatformSprites()
{
    ResourceManager &resources = wkfEnv.GetResourceManager();
    for (auto &sprite : resources.mSpriteMap)
    {
        QImage img = sprite.second->mImage.convertToFormat(QImage::Format_RGBA8888);

        sprite.second->mTexturePtr = std::make_unique<QOpenGLTexture>(img.mirrored());
        sprite.second->mTexturePtr->setMinificationFilter(QOpenGLTexture::Linear);
        sprite.second->mTexturePtr->setMagnificationFilter(QOpenGLTexture::Linear);
    }

    float size = 0.008;

    // 顶点: {x, y, u, v}
    float vertices[] = {
        -size, -size,  0.0f, 1.0f,
         size, -size,  1.0f, 1.0f,
         size,  size,  1.0f, 0.0f,

        -size, -size,  0.0f, 1.0f,
         size,  size,  1.0f, 0.0f,
        -size,  size,  0.0f, 0.0f
    };

    glGenVertexArrays(1, &m_platformVAO);
    glGenBuffers(1, &m_platformVBO);

    glBindVertexArray(m_platformVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_platformVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_platformProgram = new QOpenGLShaderProgram(this);

    const char* VERTEX_SHADER_SRC = R"(
    #version 330 core
    layout(location = 0) in vec2 position;
    layout(location = 1) in vec2 texc;

    uniform mat4 mvp;
    out vec2 TexCoord;

    void main() {
        gl_Position = mvp * vec4(position, 0.0, 1.0);
        TexCoord = texc;
    }
    )";

    m_platformProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, VERTEX_SHADER_SRC);
    m_platformProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
        "#version 330 core\n"
        "in vec2 TexCoord;"
        "out vec4 FragColor;"
        "uniform sampler2D ourTexture;"
        "void main() {"
        "   FragColor = 1.0 - vec4(texture(ourTexture, TexCoord).xyz, 0.0);"
        "}"
    );
    m_platformProgram->link();
}


int MercatorMapWidget::RenaderPlatforms()
{
    int drawCalls = 0;

    m_platformProgram->bind();
    m_platformProgram->setUniformValue("ourTexture", 0);

    ResourceManager &resources = wkfEnv.GetResourceManager();
    for (const auto &item : m_platformMap)
    {
        double lat = item.second.mLocationWCS.at(0), lon = item.second.mLocationWCS.at(1);
        float angle = item.second.mOrientationWCS[0] * UtMath::cDEG_PER_RAD;

        const QVector2D &xy = projectLonLat(lon, lat);

        QMatrix4x4 model;
        model.translate(xy.x(), xy.y());
        model.rotate(180.0 - angle, QVector3D{0.0, 0.0, 1.0});
        model.scale(4.0 / std::pow(2, m_zoomLevel));

        QMatrix4x4 view;
        view.translate(QVector3D{-m_viewCenter.x(), -m_viewCenter.y(), 0.0});

        QMatrix4x4 projection = getProjectionMatrix();

        QString icon = QString::fromStdString(item.second.mIcon);
        auto iter = resources.mSpriteMap.find(icon);
        if (iter != resources.mSpriteMap.end() && iter->second->mTexturePtr)
        {
            iter->second->mTexturePtr->bind();
            m_platformProgram->setUniformValue("mvp", projection * view * model);

            glBindVertexArray(m_platformVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            ++drawCalls;
            glBindVertexArray(0);
        }
    }

    m_platformProgram->release();
    return drawCalls;
}

void appendQuad(std::vector<float>& out,
                float x0, float y0,   // 左上 (p1)
                float x1, float y1,   // 右下 (p2)
                float u0 = 0.0f, float v0 = 0.0f,
                float u1 = 1.0f, float v1 = 1.0f)
{
    x0 = x0 + x0 - 1, y0 = y0 + y0 - 1, x1 = x1 + x1 - 1, y1 = y1 + y1 - 1;
    // 三角形 1: 左上 → 右上 → 右下
    out.insert(out.end(), {x0, y0, u0, v1}); // 左上
    out.insert(out.end(), {x1, y0, u1, v1}); // 右上
    out.insert(out.end(), {x1, y1, u1, v0}); // 右下

    // 三角形 2: 左上 → 右下 → 左下
    out.insert(out.end(), {x0, y0, u0, v1}); // 左上
    out.insert(out.end(), {x1, y1, u1, v0}); // 右下
    out.insert(out.end(), {x0, y1, u0, v0}); // 左下
}


void MercatorMapWidget::PrepareRenderTile()
{
    TiffLoader &tileLoader = wkfEnv.GetResourceManager().GetTileLoader();
    std::vector<Bound> &&grids = tileLoader.GetCurrentViewportGrids(m_viewCenter.x(), m_viewCenter.y(), m_zoomLevel);
    
    std::vector<float> vertcies;
    vertcies.reserve(grids.size() * 6 * 4);
    
    for (size_t i = 0; i < grids.size(); ++i)
    {
        const Bound &bound = grids[i];
        float x0 = bound.p1.first;   // left
        float y0 = bound.p1.second;  // top
        float x1 = bound.p2.first;   // right
        float y1 = bound.p2.second;  // bottom
        appendQuad(vertcies, x0, y0, x1, y1);
    }

    glGenVertexArrays(1, &m_tileVAO);
    glGenBuffers(1, &m_tileVBO);

    glBindVertexArray(m_tileVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_tileVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * grids.size() * 6 * 4, vertcies.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_tileProgram = new QOpenGLShaderProgram(this);

    const char* VERTEX_SHADER_SRC = R"(
    #version 330 core
    layout(location = 0) in vec2 position;
    layout(location = 1) in vec2 texc;

    uniform mat4 mvp;
    out vec2 TexCoord;

    void main() {
        gl_Position = mvp * vec4(position, 0.0, 1.0);
        TexCoord = texc;
    }
    )";

    m_tileProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, VERTEX_SHADER_SRC);
    m_tileProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
        "#version 330 core\n"
        "in vec2 TexCoord;"
        "out vec4 FragColor;"
        "uniform sampler2D ourTexture;"
        "void main() {"
        "   FragColor = texture(ourTexture, TexCoord);"
        "}"
    );
    m_tileProgram->link();


}

int MercatorMapWidget::RenderTiles()
{
    TiffLoader &tileLoader = wkfEnv.GetResourceManager().GetTileLoader();

    std::vector<Bound> &&grids = tileLoader.GetCurrentViewportGrids(m_viewCenter.x(), m_viewCenter.y(), m_zoomLevel);
    std::vector<float> vertcies;
    vertcies.reserve(grids.size() * 6 * 4);

    for (size_t i = 0; i < grids.size(); ++i)
    {
        const Bound &bound = grids[i];
        float x0 = bound.p1.first;   // left
        float y0 = bound.p1.second;  // top
        float x1 = bound.p2.first;   // right
        float y1 = bound.p2.second;  // bottom
        appendQuad(vertcies, x0, y0, x1, y1);
    }

    glBindVertexArray(m_tileVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_tileVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * grids.size() * 6 * 4, vertcies.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    QMatrix4x4 mvp = getProjectionMatrix() * getViewMatrix();

    m_tileProgram->bind();
    m_tileProgram->setUniformValue("mvp", mvp);

    // Get Tiles
    std::vector<Tiff> tiffs = tileLoader.GetCurrentViewportTiffs(m_viewCenter.x(), m_viewCenter.y(), m_zoomLevel);

    glBindVertexArray(m_tileVAO);

    int drawCalls = 0;
    for (size_t i = 0; i < tiffs.size(); ++i) {
        const auto& tiff = tiffs[i];
        auto tex = tileLoader.GetOrCreateTexture(tiff.z, tiff.x, tiff.y);
        if (!tex) continue;
        tex->bind();
        glDrawArrays(GL_TRIANGLES, static_cast<int>(i * 6), 6);
        ++drawCalls;
    }
    glBindVertexArray(0);


    m_tileProgram->release();

    return drawCalls;
}

void MercatorMapWidget::PrepareVectorRenderer()
{
    m_vectorRenderer = std::make_unique<VectorRenderer>();
    m_vectorRenderer->Initialize(this);
}

void MercatorMapWidget::LoadVectorData(const QString& path)
{
    m_vectorFeatureSet.LoadFromJson(path);
}

int MercatorMapWidget::RenderVectorFeatures()
{
    if (!m_vectorRenderer)
        return 0;

    auto visibleFeatures = m_vectorFeatureSet.GetVisibleFeatures(m_zoomLevel);
    if (visibleFeatures.empty())
        return 0;

    m_vectorRenderer->PrepareRender(visibleFeatures, m_zoomLevel,
        [this](double lon, double lat) {
            return projectLonLat(lon, lat);
        });

    QMatrix4x4 mvp = getProjectionMatrix() * getViewMatrix();
    return m_vectorRenderer->Render(mvp);
}

}
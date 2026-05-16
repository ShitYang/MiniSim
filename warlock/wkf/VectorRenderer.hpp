#pragma once

#include "VectorFeature.hpp"

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLExtraFunctions>
#include <functional>
#include <memory>
#include <vector>

namespace wkf {

class VectorRenderer {
public:
    ~VectorRenderer();

    void Initialize(QOpenGLExtraFunctions* gl);
    void PrepareRender(
        const std::vector<const VectorFeature*>& features,
        double zoomLevel,
        const std::function<QVector2D(double, double)>& projectFn);
    int Render(const QMatrix4x4& mvp);

private:
    struct Vertex {
        float x, y;
        float r, g, b, a;
    };

    struct Batch {
        std::vector<Vertex> vertices;
        GLenum primitive;
        float width;
    };

    void BuildPointBatch(const VectorFeature& f, const std::function<QVector2D(double, double)>& projectFn);
    void BuildLineBatch(const VectorFeature& f, const std::function<QVector2D(double, double)>& projectFn);
    void BuildPolygonBatches(const VectorFeature& f, const std::function<QVector2D(double, double)>& projectFn);
    std::vector<QVector2D> TriangulateRing(const std::vector<QVector2D>& ring);

    QOpenGLExtraFunctions* m_gl = nullptr;
    QOpenGLShaderProgram* m_program = nullptr;

    // Rebuilt each frame per zoom level
    std::vector<Batch> m_batches;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    QOpenGLVertexArrayObject* m_vaoObj = nullptr;
    QOpenGLBuffer* m_vboObj = nullptr;
    bool m_vaoCreated = false;
};

} // namespace wkf

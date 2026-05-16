#include "VectorRenderer.hpp"

#include <QDebug>
#include <QVector2D>
#include <cmath>
#include <gl/gl.h>

namespace wkf {

static const char* VECTOR_VERTEX_SHADER = R"(
#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;

uniform mat4 mvp;
out vec4 fragColor;

void main() {
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    fragColor = color;
}
)";

static const char* VECTOR_FRAGMENT_SHADER = R"(
#version 330 core
in vec4 fragColor;
out vec4 outColor;

void main() {
    outColor = fragColor;
}
)";

VectorRenderer::~VectorRenderer() {
    if (m_vaoCreated) {
        if (m_vaoObj) {
            m_vaoObj->destroy();
            delete m_vaoObj;
        }
        if (m_vboObj) {
            m_vboObj->destroy();
            delete m_vboObj;
        }
    }
    delete m_program;
}

void VectorRenderer::Initialize(QOpenGLExtraFunctions* gl) {
    m_gl = gl;

    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, VECTOR_VERTEX_SHADER);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, VECTOR_FRAGMENT_SHADER);
    m_program->link();

    if (!m_program->isLinked()) {
        qWarning() << "VectorRenderer shader link failed:" << m_program->log();
    }

    m_vaoObj = new QOpenGLVertexArrayObject();
    m_vboObj = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_vaoObj->create();
    m_vao = m_vaoObj->objectId();
    m_vboObj->create();
    m_vbo = m_vboObj->bufferId();
    m_vaoCreated = true;
}

void VectorRenderer::PrepareRender(
    const std::vector<const VectorFeature*>& features,
    double /*zoomLevel*/,
    const std::function<QVector2D(double, double)>& projectFn)
{
    m_batches.clear();

    for (const auto* f : features) {
        switch (f->type) {
        case VectorFeatureType::Point:
            BuildPointBatch(*f, projectFn);
            break;
        case VectorFeatureType::LineString:
            BuildLineBatch(*f, projectFn);
            break;
        case VectorFeatureType::Polygon:
            BuildPolygonBatches(*f, projectFn);
            break;
        }
    }
}

void VectorRenderer::BuildPointBatch(const VectorFeature& f,
                                      const std::function<QVector2D(double, double)>& projectFn)
{
    Batch batch;
    batch.primitive = GL_POINTS;
    batch.width = f.width;

    QColor c = f.strokeColor;
    float cr = static_cast<float>(c.redF());
    float cg = static_cast<float>(c.greenF());
    float cb = static_cast<float>(c.blueF());
    float ca = static_cast<float>(c.alphaF());
    for (const QVector2D& coord : f.coordinates) {
        QVector2D pos = projectFn(coord.x(), coord.y());
        batch.vertices.push_back({pos.x(), pos.y(), cr, cg, cb, ca});
    }

    if (!batch.vertices.empty())
        m_batches.push_back(std::move(batch));
}

void VectorRenderer::BuildLineBatch(const VectorFeature& f,
                                     const std::function<QVector2D(double, double)>& projectFn)
{
    Batch batch;
    batch.primitive = GL_LINE_STRIP;
    batch.width = f.width;

    QColor c = f.strokeColor;
    float cr = static_cast<float>(c.redF());
    float cg = static_cast<float>(c.greenF());
    float cb = static_cast<float>(c.blueF());
    float ca = static_cast<float>(c.alphaF());
    for (const QVector2D& coord : f.coordinates) {
        QVector2D pos = projectFn(coord.x(), coord.y());
        batch.vertices.push_back({pos.x(), pos.y(), cr, cg, cb, ca});
    }

    if (batch.vertices.size() >= 2)
        m_batches.push_back(std::move(batch));
}

void VectorRenderer::BuildPolygonBatches(const VectorFeature& f,
                                          const std::function<QVector2D(double, double)>& projectFn)
{
    for (const auto& ring : f.rings) {
        if (ring.size() < 3)
            continue;

        // Project ring vertices
        std::vector<QVector2D> projected;
        projected.reserve(ring.size());
        for (const QVector2D& coord : ring) {
            projected.push_back(projectFn(coord.x(), coord.y()));
        }

        // Fill batch: triangulate and draw triangles
        std::vector<QVector2D> triangles = TriangulateRing(projected);
        if (!triangles.empty()) {
            Batch fillBatch;
            fillBatch.primitive = GL_TRIANGLES;
            fillBatch.width = 0.0f;
            QColor fc = f.fillColor;
            float fr = static_cast<float>(fc.redF());
            float fg = static_cast<float>(fc.greenF());
            float fb = static_cast<float>(fc.blueF());
            float fa = static_cast<float>(fc.alphaF());
            for (const QVector2D& p : triangles) {
                fillBatch.vertices.push_back({p.x(), p.y(), fr, fg, fb, fa});
            }
            m_batches.push_back(std::move(fillBatch));
        }

        // Border batch: draw line loop around the ring
        Batch borderBatch;
        borderBatch.primitive = GL_LINE_LOOP;
        borderBatch.width = f.width;
        QColor sc = f.strokeColor;
        float sr = static_cast<float>(sc.redF());
        float sg = static_cast<float>(sc.greenF());
        float sb = static_cast<float>(sc.blueF());
        float sa = static_cast<float>(sc.alphaF());
        for (const QVector2D& p : projected) {
            borderBatch.vertices.push_back({p.x(), p.y(), sr, sg, sb, sa});
        }
        m_batches.push_back(std::move(borderBatch));
    }
}

// Ear-clipping triangulation for simple polygons (handles concave).
// Returns triangle vertices (3 * N).
std::vector<QVector2D> VectorRenderer::TriangulateRing(const std::vector<QVector2D>& ring) {
    std::vector<QVector2D> result;

    int n = static_cast<int>(ring.size());
    if (n < 3)
        return result;

    // Working copy of indices
    std::vector<int> indices(n);
    for (int i = 0; i < n; ++i)
        indices[i] = i;

    // Cross product z-component (2D cross = x1*y2 - y1*x2)
    auto cross2d = [](const QVector2D& a, const QVector2D& b) -> float {
        return a.x() * b.y() - a.y() * b.x();
    };

    auto isConvex = [&](int i0, int i1, int i2) -> bool {
        const QVector2D& a = ring[i0];
        const QVector2D& b = ring[i1];
        const QVector2D& c = ring[i2];
        return cross2d(b - a, c - b) < 0.0f;
    };

    auto pointInTriangle = [](const QVector2D& p, const QVector2D& a,
                               const QVector2D& b, const QVector2D& c) -> bool {
        auto sign = [](const QVector2D& p1, const QVector2D& p2, const QVector2D& p3) -> float {
            return (p1.x() - p3.x()) * (p2.y() - p3.y()) -
                   (p2.x() - p3.x()) * (p1.y() - p3.y());
        };
        float d1 = sign(p, a, b);
        float d2 = sign(p, b, c);
        float d3 = sign(p, c, a);
        bool hasNeg = (d1 < 0.0f) || (d2 < 0.0f) || (d3 < 0.0f);
        bool hasPos = (d1 > 0.0f) || (d2 > 0.0f) || (d3 > 0.0f);
        return !(hasNeg && hasPos);
    };

    int safety = n * 3;
    while (indices.size() > 2 && safety-- > 0) {
        int m = static_cast<int>(indices.size());
        bool earFound = false;

        for (int i = 0; i < m; ++i) {
            int prev = indices[(i + m - 1) % m];
            int curr = indices[i];
            int next = indices[(i + 1) % m];

            if (!isConvex(prev, curr, next))
                continue;

            // Check if any other vertex is inside this ear
            bool isEar = true;
            for (int j = 0; j < m; ++j) {
                int p = indices[j];
                if (p == prev || p == curr || p == next)
                    continue;
                if (pointInTriangle(ring[p], ring[prev], ring[curr], ring[next])) {
                    isEar = false;
                    break;
                }
            }

            if (isEar) {
                result.push_back(ring[prev]);
                result.push_back(ring[curr]);
                result.push_back(ring[next]);
                indices.erase(indices.begin() + i);
                earFound = true;
                break;
            }
        }

        if (!earFound)
            break; // degenerate polygon
    }

    return result;
}

int VectorRenderer::Render(const QMatrix4x4& mvp) {
    if (!m_program || !m_vaoCreated || m_batches.empty())
        return 0;

    int drawCalls = 0;
    m_program->bind();
    m_program->setUniformValue("mvp", mvp);

    for (const Batch& batch : m_batches) {
        if (batch.vertices.empty())
            continue;

        // Upload vertex data for this batch
        m_gl->glBindVertexArray(m_vao);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER,
                           static_cast<GLsizeiptr>(batch.vertices.size() * sizeof(Vertex)),
                           batch.vertices.data(), GL_DYNAMIC_DRAW);

        // Set vertex attributes
        m_gl->glEnableVertexAttribArray(0);
        m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                    reinterpret_cast<void*>(0));
        m_gl->glEnableVertexAttribArray(1);
        m_gl->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                    reinterpret_cast<void*>(2 * sizeof(float)));

        // Set line/point width
        if (batch.primitive == GL_POINTS && batch.width > 0.0f)
            glPointSize(batch.width);
        else if ((batch.primitive == GL_LINE_STRIP || batch.primitive == GL_LINE_LOOP) && batch.width > 0.0f)
            glLineWidth(batch.width);

        m_gl->glDrawArrays(batch.primitive, 0, static_cast<GLsizei>(batch.vertices.size()));
        ++drawCalls;
    }

    m_gl->glBindVertexArray(0);
    m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_program->release();
    return drawCalls;
}

} // namespace wkf

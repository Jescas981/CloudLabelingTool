#pragma once
#include "CollectionScriptable.hpp"
#include "Perceptral/core/KeyCodes.h"
#include "Perceptral/core/Log.h"
#include <Perceptral/Perceptral.h>
#include <Perceptral/core/assets/PointCloud.h>
#include <Perceptral/scene/Components.h>
#include <future>
#include <imgui.h>
#include <mutex>

// ─── Tool mode ───────────────────────────────────────────────────────────────
enum class SelectionMode { Rectangle, Lasso };

class SelectionToolScriptable : public Perceptral::Scriptable {
public:
  // ─── Lifecycle ─────────────────────────────────────────────────────────────
  void onCreate() override {
    auto &ns = getEntityByTag("Collection")
                   .getComponent<Perceptral::Component::NativeScript>();

    m_collection = Perceptral::GetScript<CollectionScriptable>(ns);

    m_shader = Perceptral::AssetManager::load<Perceptral::Shader>(
        "app:selection_tool.glsl");

    setMode(SelectionMode::Lasso);
    m_vertexArray = Perceptral::VertexArray::create();
    // Allocate large buffer — lasso can have many points
    m_vertexBuffer = Perceptral::VertexBuffer::create(
        nullptr, k_MaxLassoVerts * 2 * sizeof(float));
    m_vertexBuffer->setLayout(
        {{Perceptral::ShaderDataType::Float2, "a_Position"}});
    m_vertexArray->addVertexBuffer(m_vertexBuffer);
  }

  // ─── Render ────────────────────────────────────────────────────────────────
  void onRender() override {
    if (!m_isSelecting)
      return;

    m_shader->bind();
    Perceptral::Renderer::setDepthTest(false);
    Perceptral::Renderer::setBlending(true);

    if (m_mode == SelectionMode::Rectangle)
      renderRect();
    else
      renderLasso();

    Perceptral::Renderer::setDepthTest(true);
    Perceptral::Renderer::setBlending(false);
    m_shader->unbind();
  }

  // ─── Input ─────────────────────────────────────────────────────────────────
  void onEvent(Perceptral::Event &e) override {
    Perceptral::EventDispatcher d(e);

    d.dispatch<Perceptral::KeyPressedEvent>([this](auto &e) {
      if (e.getKeyCode() == Perceptral::KeyCode::LeftControl ||
          e.getKeyCode() == Perceptral::KeyCode::RightControl)
        m_ctrlHeld = true;
      // Tab to toggle mode
      if (e.getKeyCode() == Perceptral::KeyCode::Tab && !m_isSelecting)
        m_mode = (m_mode == SelectionMode::Rectangle)
                     ? SelectionMode::Lasso
                     : SelectionMode::Rectangle;
      return false;
    });

    d.dispatch<Perceptral::KeyReleasedEvent>([this](auto &e) {
      if (e.getKeyCode() == Perceptral::KeyCode::LeftControl ||
          e.getKeyCode() == Perceptral::KeyCode::RightControl)
        m_ctrlHeld = false;
      return false;
    });

    d.dispatch<Perceptral::MouseButtonPressedEvent>([this](auto &e) {
      if (e.getMouseButton() == Perceptral::MouseButton::Left) {
        m_isSelecting = true;
        m_deselect = m_ctrlHeld;
        m_rectStart = m_mousePos;
        m_rectEnd = m_mousePos;
        m_lassoPoints.clear();
        m_lassoPoints.push_back(m_mousePos);
      }
      return false;
    });

    d.dispatch<Perceptral::MouseMovedEvent>([this](auto &e) {
      m_mousePos = {e.getX(), e.getY()};
      if (m_isSelecting) {
        m_rectEnd = m_mousePos;
        if (m_mode == SelectionMode::Lasso) {
          // Only add if moved enough — avoids thousands of duplicate points
          if (m_lassoPoints.empty() ||
              (m_mousePos - m_lassoPoints.back()).norm() > 4.0f)
            m_lassoPoints.push_back(m_mousePos);
        }
      }
      return false;
    });

    d.dispatch<Perceptral::MouseButtonReleasedEvent>([this](auto &e) {
      if (m_isSelecting &&
          e.getMouseButton() == Perceptral::MouseButton::Left) {
        m_isSelecting = false;
        if (m_mode == SelectionMode::Rectangle)
          applyRect(m_rectStart, m_rectEnd);
        else
          applyLasso(m_lassoPoints);
      }
      return false;
    });
  }

  // ─── Public API ────────────────────────────────────────────────────────────
  void setMode(SelectionMode mode) { m_mode = mode; }
  SelectionMode getMode() const { return m_mode; }

private:
  // ─── Rectangle render ──────────────────────────────────────────────────────
  void renderRect() {
    auto &io = ImGui::GetIO();
    float W = io.DisplaySize.x, H = io.DisplaySize.y;

    float minX = std::min(m_rectStart.x(), m_rectEnd.x());
    float minY = std::min(m_rectStart.y(), m_rectEnd.y());
    float maxX = std::max(m_rectStart.x(), m_rectEnd.x());
    float maxY = std::max(m_rectStart.y(), m_rectEnd.y());

    auto ndcX = [&](float x) { return (x / W) * 2.0f - 1.0f; };
    auto ndcY = [&](float y) { return -((y / H) * 2.0f - 1.0f); };

    float x0 = ndcX(minX), y0 = ndcY(minY);
    float x1 = ndcX(maxX), y1 = ndcY(maxY);

    float fillVerts[8] = {x0, y0, x1, y0, x0, y1, x1, y1};
    float borderVerts[10] = {x0, y0, x1, y0, x1, y1, x0, y1, x0, y0};

    // Fill
    m_shader->setInt("u_IsFill", 1);
    m_shader->setFloat4("u_FillColor", {0.39f, 0.78f, 1.0f, 0.12f});
    m_vertexBuffer->setData(fillVerts, sizeof(fillVerts));
    Perceptral::Renderer::drawArrays(m_vertexArray, 0, 4,
                                     Perceptral::PrimitiveType::TriangleStrip);
    // Border
    m_shader->setInt("u_IsFill", 0);
    m_shader->setFloat4("u_BorderColor", {0.39f, 0.78f, 1.0f, 1.0f});
    Perceptral::Renderer::setLineWidth(1.5f);
    m_vertexBuffer->setData(borderVerts, sizeof(borderVerts));
    Perceptral::Renderer::drawArrays(m_vertexArray, 0, 5,
                                     Perceptral::PrimitiveType::LineStrip);
  }

  // ─── Lasso render ──────────────────────────────────────────────────────────
  void renderLasso() {
    if (m_lassoPoints.size() < 2)
      return;

    auto &io = ImGui::GetIO();
    float W = io.DisplaySize.x, H = io.DisplaySize.y;

    auto ndcX = [&](float x) { return (x / W) * 2.0f - 1.0f; };
    auto ndcY = [&](float y) { return -((y / H) * 2.0f - 1.0f); };

    // Compute centroid in NDC
    float cx = 0, cy = 0;
    for (auto &p : m_lassoPoints) {
      cx += p.x();
      cy += p.y();
    }
    cx /= m_lassoPoints.size();
    cy /= m_lassoPoints.size();
    float cxNDC = ndcX(cx), cyNDC = ndcY(cy);

    // ── Fill — TriangleFan from centroid ──────────────────────────────────
    // Layout: centroid, then all border points + first repeated to close
    std::vector<float> fillVerts;
    fillVerts.reserve((m_lassoPoints.size() + 2) * 2);

    fillVerts.push_back(cxNDC); // centroid
    fillVerts.push_back(cyNDC);
    for (auto &p : m_lassoPoints) {
      fillVerts.push_back(ndcX(p.x()));
      fillVerts.push_back(ndcY(p.y()));
    }
    // Close fan — repeat first border point
    fillVerts.push_back(ndcX(m_lassoPoints[0].x()));
    fillVerts.push_back(ndcY(m_lassoPoints[0].y()));

    uint32_t fanCount = static_cast<uint32_t>(m_lassoPoints.size() + 2);
    if (fanCount > k_MaxLassoVerts)
      return;

    m_shader->setInt("u_IsFill", 1);
    m_shader->setFloat4("u_FillColor", {0.39f, 0.78f, 1.0f, 0.12f});
    m_vertexBuffer->setData(fillVerts.data(), fillVerts.size() * sizeof(float));
    Perceptral::Renderer::drawArrays(m_vertexArray, 0, fanCount,
                                     Perceptral::PrimitiveType::TriangleFan);

    // ── Border — LineStrip, closed ────────────────────────────────────────
    std::vector<float> borderVerts;
    borderVerts.reserve((m_lassoPoints.size() + 1) * 2);
    for (auto &p : m_lassoPoints) {
      borderVerts.push_back(ndcX(p.x()));
      borderVerts.push_back(ndcY(p.y()));
    }
    borderVerts.push_back(ndcX(m_lassoPoints[0].x()));
    borderVerts.push_back(ndcY(m_lassoPoints[0].y()));

    uint32_t lineCount = static_cast<uint32_t>(m_lassoPoints.size() + 1);

    m_shader->setInt("u_IsFill", 0);
    m_shader->setFloat4("u_BorderColor", {0.39f, 0.78f, 1.0f, 1.0f});
    Perceptral::Renderer::setLineWidth(1.5f);
    m_vertexBuffer->setData(borderVerts.data(),
                            borderVerts.size() * sizeof(float));
    Perceptral::Renderer::drawArrays(m_vertexArray, 0, lineCount,
                                     Perceptral::PrimitiveType::LineStrip);
  }

  void applyRect(Eigen::Vector2f min, Eigen::Vector2f max) {
    auto *cloud = m_collection->getSelected();
    if (!cloud)
      return;

    Eigen::Vector2f rectMin = {std::min(min.x(), max.x()),
                               std::min(min.y(), max.y())};
    Eigen::Vector2f rectMax = {std::max(min.x(), max.x()),
                               std::max(min.y(), max.y())};
    if ((rectMax - rectMin).norm() < 3.0f)
      return;

    auto &cloudData =
        cloud->entity.getComponent<Perceptral::Component::PointCloudData>();
    auto &camera = getScene()
                       .getMainCamera()
                       .getComponent<Perceptral::Component::Camera>();

    auto selected = parallelSelect(
        cloudData.asset->points, camera.viewProjectionMatrix,
        ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y,
        [rectMin, rectMax](const Eigen::Vector2f &sp) {
          return sp.x() >= rectMin.x() && sp.x() <= rectMax.x() &&
                 sp.y() >= rectMin.y() && sp.y() <= rectMax.y();
        });

    applyToSelection(cloud, selected, m_deselect);
    m_collection->applySelection(m_collection->getSelectedIndex());
  }

  void applyLasso(std::vector<Eigen::Vector2f> polygon) {
    auto *cloud = m_collection->getSelected();
    if (!cloud || polygon.size() < 3)
      return;

    auto &cloudData =
        cloud->entity.getComponent<Perceptral::Component::PointCloudData>();
    auto &camera = getScene()
                       .getMainCamera()
                       .getComponent<Perceptral::Component::Camera>();

    auto selected = parallelSelect(
        cloudData.asset->points, camera.viewProjectionMatrix,
        ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y,
        [&polygon](const Eigen::Vector2f &sp) {
          return pointInPolygon(sp, polygon);
        });

    applyToSelection(cloud, selected, m_deselect);
    m_collection->applySelection(m_collection->getSelectedIndex());
  }

  void applyToSelection(CloudEntry *cloud, const std::vector<uint32_t> &indices,
                        bool deselect) {
    if (!deselect) {
      // Default — add
      cloud->selectedPoints.insert(cloud->selectedPoints.end(), indices.begin(),
                                   indices.end());
    } else {
      // Ctrl — remove
      std::unordered_set<uint32_t> toRemove(indices.begin(), indices.end());
      cloud->selectedPoints.erase(
          std::remove_if(cloud->selectedPoints.begin(),
                         cloud->selectedPoints.end(),
                         [&](uint32_t idx) { return toRemove.count(idx); }),
          cloud->selectedPoints.end());
    }
  }

  template <typename Predicate>
  static std::vector<uint32_t>
  parallelSelect(const std::vector<Eigen::Vector3f> &points,
                 const Eigen::Matrix4f &vpMatrix, float displayW,
                 float displayH, Predicate pred) {
    const uint32_t total = static_cast<uint32_t>(points.size());
    const uint32_t threadCount =
        std::max(1u, std::min(std::thread::hardware_concurrency(), total));
    const uint32_t chunkSize = (total + threadCount - 1) / threadCount;

    std::vector<std::vector<uint32_t>> buckets(threadCount);
    std::vector<std::thread> workers(threadCount);

    for (uint32_t t = 0; t < threadCount; t++) {
      const uint32_t begin = t * chunkSize;
      const uint32_t end = std::min(begin + chunkSize, total);

      workers[t] = std::thread([&, t, begin, end]() {
        for (uint32_t i = begin; i < end; i++) {
          Eigen::Vector4f clip =
              vpMatrix * Eigen::Vector4f(points[i].x(), points[i].y(),
                                         points[i].z(), 1.0f);
          if (clip.w() <= 0.0f)
            continue;

          Eigen::Vector3f ndc = clip.head<3>() / clip.w();
          Eigen::Vector2f sp = {(ndc.x() * 0.5f + 0.5f) * displayW,
                                (1.0f - (ndc.y() * 0.5f + 0.5f)) * displayH};

          if (pred(sp))
            buckets[t].push_back(i);
        }
      });
    }

    // Block until all threads done — intentionally synchronous
    for (auto &w : workers)
      w.join();

    std::vector<uint32_t> merged;
    for (auto &b : buckets)
      merged.insert(merged.end(), b.begin(), b.end());
    return merged;
  }

  // ─── Point in polygon — ray casting ────────────────────────────────────────
  static bool pointInPolygon(const Eigen::Vector2f &p,
                             const std::vector<Eigen::Vector2f> &poly) {
    bool inside = false;
    int n = static_cast<int>(poly.size());
    for (int i = 0, j = n - 1; i < n; j = i++) {
      float xi = poly[i].x(), yi = poly[i].y();
      float xj = poly[j].x(), yj = poly[j].y();
      if (((yi > p.y()) != (yj > p.y())) &&
          (p.x() < (xj - xi) * (p.y() - yi) / (yj - yi) + xi))
        inside = !inside;
    }
    return inside;
  }

  // ─── Project world → screen ────────────────────────────────────────────────
  Eigen::Vector2f projectToScreen(const Eigen::Vector3f &worldPos,
                                  const Perceptral::Component::Camera &camera) {
    auto &io = ImGui::GetIO();
    Eigen::Vector4f clip =
        camera.viewProjectionMatrix *
        Eigen::Vector4f(worldPos.x(), worldPos.y(), worldPos.z(), 1.0f);
    if (clip.w() <= 0.0f)
      return {-1.0f, -1.0f};
    Eigen::Vector3f ndc = clip.head<3>() / clip.w();
    return {(ndc.x() * 0.5f + 0.5f) * io.DisplaySize.x,
            (1.0f - (ndc.y() * 0.5f + 0.5f)) * io.DisplaySize.y};
  }

  // ─── Constants ─────────────────────────────────────────────────────────────
  static constexpr uint32_t k_MaxLassoVerts = 4096;

  // ─── State ─────────────────────────────────────────────────────────────────
  CollectionScriptable *m_collection{nullptr};
  std::shared_ptr<Perceptral::Shader> m_shader;
  std::shared_ptr<Perceptral::VertexArray> m_vertexArray;
  std::shared_ptr<Perceptral::VertexBuffer> m_vertexBuffer;

  SelectionMode m_mode{SelectionMode::Rectangle};
  bool m_isSelecting{false};
  bool m_ctrlHeld{false};
  bool m_deselect{false};

  Eigen::Vector2f m_mousePos{};
  Eigen::Vector2f m_rectStart{};
  Eigen::Vector2f m_rectEnd{};
  std::vector<Eigen::Vector2f> m_lassoPoints;
};
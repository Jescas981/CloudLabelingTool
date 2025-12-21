#pragma once

#include <string>
#include <memory>

namespace Perceptral {

class PointCloud;

class PLYLoader {
public:
    PLYLoader();
    ~PLYLoader();

    // Load PLY file
    static std::unique_ptr<PointCloud> load(const std::string& filepath);

    // Save PLY file
    static bool save(const std::string& filepath, const PointCloud& pointCloud, bool binary = true);

private:
    static std::unique_ptr<PointCloud> loadASCII(const std::string& filepath, size_t vertexCount);
    static std::unique_ptr<PointCloud> loadBinary(const std::string& filepath, size_t vertexCount, bool bigEndian);
};

} // namespace Perceptral

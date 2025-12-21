#include <Perceptral/io/PointCloudExporter.h>
#include <Perceptral/scene/PointCloud.h>
#include <Perceptral/core/Log.h>
#include <happly.h>
#include <iomanip>

namespace Perceptral {

bool PointCloudExporter::savePLYWithLabels(const std::string& filepath,
                                          const PointCloud& pointCloud,
                                          const std::vector<uint8_t>& labels,
                                          bool binary)
{
    if (binary) {
        return savePLYBinary(filepath, pointCloud, labels);
    } else {
        return savePLYASCII(filepath, pointCloud, labels);
    }
}

bool PointCloudExporter::savePLYASCII(const std::string& filepath,
                                     const PointCloud& pointCloud,
                                     const std::vector<uint8_t>& labels)
{
    auto cloud = pointCloud.getCloud();
    if (!cloud || cloud->empty()) {
        PC_CORE_ERROR("Cannot save empty point cloud to {}", filepath);
        return false;
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        PC_CORE_ERROR("Failed to open file for writing: {}", filepath);
        return false;
    }

    // Write PLY header
    file << "ply\n";
    file << "format ascii 1.0\n";
    file << "comment Created by CloudAnnotationTool\n";
    file << "element vertex " << cloud->size() << "\n";
    file << "property float x\n";
    file << "property float y\n";
    file << "property float z\n";
    file << "property uchar label\n";
    file << "end_header\n";

    // Write point data
    for (size_t i = 0; i < cloud->size(); ++i) {
        const auto& point = cloud->points[i];
        uint8_t label = 0;
        if (!labels.empty() && i < labels.size()) {
            label = labels[i];
        }

        file << std::fixed << std::setprecision(6)
             << point.x << " "
             << point.y << " "
             << point.z << " "
             << static_cast<int>(label) << "\n";
    }

    file.close();
    PC_CORE_INFO("Saved PLY file (ASCII) with labels: {} ({} points)", filepath, cloud->size());
    return true;
}

bool PointCloudExporter::savePLYBinary(const std::string& filepath,
                                      const PointCloud& pointCloud,
                                      const std::vector<uint8_t>& labels)
{
    auto cloud = pointCloud.getCloud();
    if (!cloud || cloud->empty()) {
        PC_CORE_ERROR("Cannot save empty point cloud to {}", filepath);
        return false;
    }

    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        PC_CORE_ERROR("Failed to open file for writing: {}", filepath);
        return false;
    }

    // Write PLY header (ASCII, even for binary files)
    std::string header;
    header += "ply\n";
    header += "format binary_little_endian 1.0\n";
    header += "comment Created by CloudAnnotationTool\n";
    header += "element vertex " + std::to_string(cloud->size()) + "\n";
    header += "property float x\n";
    header += "property float y\n";
    header += "property float z\n";
    header += "property uchar label\n";
    header += "end_header\n";

    file.write(header.c_str(), header.size());

    // Write point data in binary
    for (size_t i = 0; i < cloud->size(); ++i) {
        const auto& point = cloud->points[i];
        uint8_t label = 0;
        if (!labels.empty() && i < labels.size()) {
            label = labels[i];
        }

        // Write x, y, z as floats
        file.write(reinterpret_cast<const char*>(&point.x), sizeof(float));
        file.write(reinterpret_cast<const char*>(&point.y), sizeof(float));
        file.write(reinterpret_cast<const char*>(&point.z), sizeof(float));

        // Write label as uint8
        file.write(reinterpret_cast<const char*>(&label), sizeof(uint8_t));
    }

    file.close();
    PC_CORE_INFO("Saved PLY file (binary) with labels: {} ({} points)", filepath, cloud->size());
    return true;
}

} // namespace Perceptral

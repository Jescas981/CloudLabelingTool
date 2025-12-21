#include "io/PLYLoader.h"
#include "scene/PointCloud.h"
#include <happly.h>
#include <core/Log.h>
#include <iostream>

namespace CloudCore {

PLYLoader::PLYLoader()
{
}

PLYLoader::~PLYLoader()
{
}

std::unique_ptr<PointCloud> PLYLoader::load(const std::string& filepath)
{
    try {
        // Load PLY file
        happly::PLYData plyIn(filepath);
        
        // Get vertex element
        auto& vertexElement = plyIn.getElement("vertex");
        
        // Read coordinates
        std::vector<float> x = vertexElement.getProperty<float>("x");
        std::vector<float> y = vertexElement.getProperty<float>("y");
        std::vector<float> z = vertexElement.getProperty<float>("z");
        
        // Read class labels (handle the case where they might not exist)
        std::vector<uint32_t> labels;
        bool hasLabels = false;
        try {
            labels = vertexElement.getProperty<uint32_t>("class");
            hasLabels = true;
        } catch (...) {
            // No class labels in file, will default to 0
        }
        
        // Create PCL point cloud
        pcl::PointCloud<pcl::PointXYZL>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZL>());
        cloud->width = x.size();
        cloud->height = 1;
        cloud->is_dense = false;
        cloud->points.resize(cloud->width * cloud->height);
        
        // Fill point cloud - set labels to 0 if not present
        for (size_t i = 0; i < x.size(); ++i) {
            cloud->points[i].x = x[i];
            cloud->points[i].y = y[i];
            cloud->points[i].z = z[i];
            cloud->points[i].label = hasLabels ? labels[i] : 0;
        }
        
        auto pointCloud = std::make_unique<PointCloud>(cloud);
        pointCloud->setName(filepath);
        pointCloud->computeBounds();
        pointCloud->setHasLabels(hasLabels);
        
        return pointCloud;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to load PLY file: " << filepath << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        return nullptr;
    }
}

bool PLYLoader::save(const std::string& filepath, const PointCloud& pointCloud, bool binary)
{
    try {
        auto cloud = pointCloud.getCloud();
        
        // Prepare data vectors
        std::vector<float> x, y, z;
        std::vector<uint32_t> labels;
        
        x.reserve(cloud->size());
        y.reserve(cloud->size());
        z.reserve(cloud->size());
        labels.reserve(cloud->size());
        
        // Extract data from point cloud
        for (const auto& point : cloud->points) {
            x.push_back(point.x);
            y.push_back(point.y);
            z.push_back(point.z);
            labels.push_back(point.label);
        }
        
        // Create PLY data structure
        happly::PLYData plyOut;
        
        // Add vertex element with properties
        plyOut.addElement("vertex", x.size());
        plyOut.getElement("vertex").addProperty<float>("x", x);
        plyOut.getElement("vertex").addProperty<float>("y", y);
        plyOut.getElement("vertex").addProperty<float>("z", z);
        plyOut.getElement("vertex").addProperty<uint32_t>("class", labels);
        
        // Write to file
        if (binary) {
            plyOut.write(filepath, happly::DataFormat::Binary);
        } else {
            plyOut.write(filepath, happly::DataFormat::ASCII);
        }
        
        CC_INFO( "Saved PLY file: {} with {} points",filepath, cloud->size());
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to save PLY file: " << filepath << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

} // namespace CloudCore
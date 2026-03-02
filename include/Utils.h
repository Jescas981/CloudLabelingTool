#pragma once
#include <Perceptral/core/Log.h>
#include <nfd.h>
#include <string>

namespace PointCloudTool {

void initOpenFileDialog() {
  if (NFD_Init() != NFD_OKAY) {
    PC_ERROR("Failed to initialize NFD: {}", NFD_GetError());
  }
}

std::string openFileDialog(const std::string &title,
                           const std::string &filter) {
  nfdchar_t *outPath = nullptr;
  nfdfilteritem_t filterItem[1] = {{title.c_str(), filter.c_str()}};
  nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, nullptr);

  if (result == NFD_OKAY) {
    PC_INFO("Selected file: {}", outPath);
    return outPath;
    NFD_FreePath(outPath);
  } else if (result == NFD_CANCEL) {
    PC_INFO("User cancelled file dialog");
  } else {
    PC_ERROR("File dialog error: {}", NFD_GetError());
  }
  return "";
}

} // namespace PointCloudTool
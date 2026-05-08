#pragma once
#include <vector>
#include <string>

bool listRootFiles(std::vector<std::string>& out);
bool copyFileOut(const char* srcPath, const char* dstAndroidPath);
bool deleteFile(const char* path);
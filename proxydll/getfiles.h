#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <tchar.h>
#include <shlwapi.h>
#include <map>

bool getFileNames(std::wstring folderPath, std::map<std::wstring, std::vector<std::wstring>>& file_paths, std::wstring name);

bool getFileExts(std::wstring folderPath, std::map<std::wstring, std::vector<std::wstring>>& file_paths, std::wstring ext, int kaisoulimit, int kaisou = 0);


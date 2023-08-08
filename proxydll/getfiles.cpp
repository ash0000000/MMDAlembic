#include <windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <tchar.h>
#include <shlwapi.h>
#include <map>

/**
* @brief フォルダ以下のファイル一覧を取得する関数
* @param[in]    folderPath  フォルダパス
* @param[out]   file_names  ファイル名一覧
* return        true:成功, false:失敗
*/
bool getFileNames(std::wstring folderPath, std::map<std::wstring, std::vector<std::wstring>>& file_paths, std::wstring name)
{
	HANDLE hFind;
	WIN32_FIND_DATA win32fd;
	std::wstring search_name = folderPath;
	search_name.append(_T("\\*"));

	hFind = FindFirstFile(search_name.c_str(), &win32fd);

	if (hFind == INVALID_HANDLE_VALUE) {
		//throw std::runtime_error("file not found");
		return false;
	}

	/* 指定のディレクトリ以下のファイル名をファイルがなくなるまで取得する */
	do {
		std::wstring tmp = win32fd.cFileName;
		std::wstring folderPath2 = folderPath;
		std::wstring search_name2 = folderPath;
		if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			/* ディレクトリの場合は何もしない */
			//printf("directory\n");
			if (std::equal(tmp.begin(), tmp.end(), "."))
				continue;
			if (std::equal(tmp.begin(), tmp.end(), ".."))
				continue;
			search_name2.append(tmp);
			search_name2.append(_T("\\"));
			getFileNames(search_name2, file_paths, name);
		}
		else {
			search_name2.append(tmp);
			std::filesystem::path path_tmp(search_name2);
			if (path_tmp.filename() == name) {
				file_paths[folderPath2].push_back(search_name2);
				//printf("%s\n", file_paths.back().c_str());
			}
		}
	} while (FindNextFile(hFind, &win32fd));

	FindClose(hFind);
	return true;
}

bool getFileExts(std::wstring folderPath, std::map<std::wstring, std::vector<std::wstring>>& file_paths, std::wstring ext, int kaisoulimit, int kaisou = 0)
{
	HANDLE hFind;
	WIN32_FIND_DATA win32fd;
	std::wstring search_name = folderPath;
	search_name.append(_T("\\*"));

	hFind = FindFirstFile(search_name.c_str(), &win32fd);

	if (hFind == INVALID_HANDLE_VALUE) {
		//throw std::runtime_error("file not found");
		return false;
	}

	/* 指定のディレクトリ以下のファイル名をファイルがなくなるまで取得する */
	do {
		std::wstring tmp = win32fd.cFileName;
		std::wstring folderPath2 = folderPath;
		std::wstring search_name2 = folderPath;
		if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			//printf("directory\n");
			if (kaisou >= kaisoulimit)continue;
			if (std::equal(tmp.begin(), tmp.end(), "."))
				continue;
			if (std::equal(tmp.begin(), tmp.end(), ".."))
				continue;
			search_name2.append(tmp);
			search_name2.append(_T("\\"));
			getFileExts(search_name2, file_paths, ext, kaisoulimit, kaisou + 1);
		}
		else {
			search_name2.append(tmp);
			std::filesystem::path path_tmp(search_name2);
			if (path_tmp.extension() == ext) {
				file_paths[folderPath2].push_back(search_name2);
				//printf("%s\n", file_paths.back().c_str());
			}
		}
	} while (FindNextFile(hFind, &win32fd));

	FindClose(hFind);
	return true;
}

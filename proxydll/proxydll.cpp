#include <windows.h>
#include <string>
#include <tchar.h>
#include "getfiles.h"
#include <vector>
#include <fstream>
#include <ostream>
#include <map>
#include <dsound.h>
#include <d3d9.h>
#include <psapi.h>
HINSTANCE g_hinst;
HWND g_hWnd = NULL;	//ウィンドウハンドル
HRESULT(WINAPI* original_DirectSoundCreate)(LPGUID, LPDIRECTSOUND*, LPUNKNOWN)(NULL);
HRESULT WINAPI DirectSoundCreate(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter) {
	return original_DirectSoundCreate(lpGuid, ppDS, pUnkOuter);
}

// Direct3DCreate9
IDirect3D9* (WINAPI* original_direct3d_create)(UINT)(NULL);
HRESULT(WINAPI* original_direct3d9ex_create)(UINT, IDirect3D9Ex**)(NULL);
extern "C" {
	// 偽Direct3DCreate9
	IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion) {
		return (*original_direct3d_create)(SDKVersion);
	}
	HRESULT WINAPI Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** ppD3D) {
		return (*original_direct3d9ex_create)(SDKVersion, ppD3D);
	}
} // extern "C"

////for mmaccel
//void (WINAPI* mmaccel_run)(HMODULE)(NULL);
//void (WINAPI* mmaccel_end)()(NULL);

void OpenConsole()
{
	FILE* in, * out;
	AllocConsole();
	//HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	//HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	DWORD prev_mode;
	GetConsoleMode(hInput, &prev_mode);
	SetConsoleMode(hInput, ENABLE_EXTENDED_FLAGS | (prev_mode & ~ENABLE_QUICK_EDIT_MODE));
	freopen_s(&out, "CONOUT$", "w", stdout);//CONOUT$
	freopen_s(&in, "CONIN$", "r", stdin);
}

std::map<std::wstring, bool> success_load_dll;

BOOL loadDSOUND(std::wstring system_path_buffer) {
	std::wstring dll_path = system_path_buffer;
	dll_path.append(_T("\\dsound.DLL"));
	HMODULE dll_module(LoadLibrary(dll_path.c_str()));
	if (!dll_module) {
		return FALSE;
	}
	original_DirectSoundCreate = reinterpret_cast<HRESULT(WINAPI*)(LPGUID, LPDIRECTSOUND*, LPUNKNOWN)>(GetProcAddress(dll_module, "DirectSoundCreate"));
	if (!original_DirectSoundCreate) {
		return FALSE;
	}
}
BOOL loadD3D9(std::wstring system_path_buffer) {
	std::wstring d3d9_path(system_path_buffer);
	d3d9_path.append(_T("\\D3D9.DLL"));
	// オリジナルのD3D9.DLLのモジュール
	HMODULE d3d9_module(LoadLibrary(d3d9_path.c_str()));
	if (!d3d9_module) {
		return FALSE;
	}
	// オリジナルDirect3DCreate9の関数ポインタを取得
	original_direct3d_create = reinterpret_cast<IDirect3D9 * (WINAPI*)(UINT)>(GetProcAddress(d3d9_module, "Direct3DCreate9"));
	if (!original_direct3d_create) {
		return FALSE;
	}
	original_direct3d9ex_create = reinterpret_cast<HRESULT(WINAPI*)(UINT, IDirect3D9Ex**)>(GetProcAddress(d3d9_module, "Direct3DCreate9Ex"));
	if (!original_direct3d9ex_create) {
		return FALSE;
	}
}
void moveMMEPlugin(std::wstring base_path) {
	std::vector<std::wstring> mmefiles;
	//mmefiles.push_back(_T("d3d9.dll"));
	mmefiles.push_back(_T("MMHack.dll"));
	mmefiles.push_back(_T("MMEffect.dll"));
	mmefiles.push_back(_T("MMEffect.txt"));
	mmefiles.push_back(_T("REFERENCE.txt"));
	std::wstring pluginmmedir = base_path;
	pluginmmedir.append(_T("plugin\\mme\\"));
	std::filesystem::create_directories(pluginmmedir);
	for (const auto& mmefile : mmefiles) {
		std::wstring mmefilepath = base_path;
		mmefilepath.append(mmefile);
		if (!std::filesystem::exists(mmefilepath)) {
			continue;
		}
		std::wstring mmemovepath = pluginmmedir;
		mmemovepath.append(mmefile);
		std::filesystem::rename(mmefilepath, mmemovepath);
	}
}
void loadMMEPlugin(std::wstring base_path) {
	char defaultDllDir[MAX_PATH] = "";
	GetDllDirectoryA(sizeof(defaultDllDir), defaultDllDir);
	SetDllDirectoryA(defaultDllDir);
	std::vector<std::wstring> mmedllpaths;
	std::wstring mmedllpath1 = base_path;
	std::wstring mmedllpath2 = base_path;
	//mmedllpath1.append(_T("d3d9.dll"));
	mmedllpath1.append(_T("MMHack.dll"));
	mmedllpath2.append(_T("plugin\\mme\\MMHack.dll"));
	mmedllpaths.push_back(mmedllpath1);
	mmedllpaths.push_back(mmedllpath2);
	for (const auto& mmedllpath : mmedllpaths) {
		if (std::filesystem::exists(mmedllpath)) {
			printf("[proxy] LoadLibrary: %ls \n", mmedllpath.c_str());
			std::filesystem::path path_tmp(mmedllpath);
			std::wstring dlldirpath = path_tmp.parent_path();
			SetDllDirectory(dlldirpath.c_str());
			HMODULE plugin_module(LoadLibrary(mmedllpath.c_str()));
			if (plugin_module) {
				success_load_dll[mmedllpath] = true;
				break;
			}
			else {
				DWORD code = GetLastError();
				printf("[proxy] Error: LoadLibrary : %lu \n", code);
				//return FALSE;
			}
		}
	}
	SetDllDirectoryA(defaultDllDir);
}
//BOOL loadMMAPlugin(std::wstring base_path) {
//	char defaultDllDir[MAX_PATH] = "";
//	GetDllDirectoryA(sizeof(defaultDllDir), defaultDllDir);
//	SetDllDirectoryA(defaultDllDir);
//	std::wstring mmadllpath = base_path;
//	mmadllpath.append(_T("plugin\\MMAccel\\mmaccel.dll"));
//	if (std::filesystem::exists(mmadllpath)) {
//		printf("[proxy] LoadLibrary: %ls \n", mmadllpath.c_str());
//		std::filesystem::path path_tmp(mmadllpath);
//		std::wstring dlldirpath = path_tmp.parent_path();
//		SetDllDirectory(dlldirpath.c_str());
//		HMODULE plugin_module(LoadLibrary(mmadllpath.c_str()));
//		if (plugin_module) {
//			success_load_dll[mmadllpath] = true;
//			mmaccel_run = reinterpret_cast<void (WINAPI*)(HMODULE)>(GetProcAddress(plugin_module, "mmaccel_run"));
//			if (!mmaccel_run) {
//				return FALSE;
//			}
//			mmaccel_end = reinterpret_cast<void (WINAPI*)()>(GetProcAddress(plugin_module, "mmaccel_end"));
//			if (!mmaccel_end) {
//				return FALSE;
//			}
//		}
//		else {
//			DWORD code = GetLastError();
//			printf("[proxy] Error: LoadLibrary : %lu \n", code);
//			//return FALSE;
//		}
//	}
//	SetDllDirectoryA(defaultDllDir);
//	// プロセスID
//	DWORD processID = GetCurrentProcessId();
//	// プロセスハンドル
//	HANDLE hProcessaaa = OpenProcess(
//		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
//	if (!hProcessaaa)
//		return FALSE;
//	HMODULE hModule = NULL;	// HINSTANCE ≒ HMODULE
//	DWORD dummy = 0;
//	BOOL bResult = EnumProcessModules(hProcessaaa, &hModule, sizeof(HMODULE), &dummy);
//	mmaccel_run(hModule);
//}
void loadPluginDir(std::wstring base_path) {
	char defaultDllDir[MAX_PATH] = "";
	GetDllDirectoryA(sizeof(defaultDllDir), defaultDllDir);
	std::wstring searchpath = base_path;
	searchpath.append(_T("plugin\\"));
	std::map<std::wstring, std::vector<std::wstring>> dll_paths;
	//std::wstring name = L"d3d9.dll";
	//getFileNames(searchpath, dll_paths, name);
	std::wstring ext = L".dll";
	getFileExts(searchpath, dll_paths, ext, 1);
	//SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	std::wstring plugindir;
	std::vector<std::wstring> plugindlls;
	for (const auto& item : dll_paths) {
		std::wstring plugindir = item.first;
		std::vector<std::wstring> plugindlls = item.second;
		//AddDllDirectory(plugindir.c_str());
		SetDllDirectory(plugindir.c_str());
		for (std::wstring dllpath : plugindlls) {
			if (success_load_dll.count(dllpath) != 0) {
				continue;
			}
			printf("[proxy] LoadLibrary: %ls \n", dllpath.c_str());
			HMODULE plugin_module(LoadLibrary(dllpath.c_str()));
			if (plugin_module) {
				success_load_dll[dllpath] = true;
			}
			else {
				DWORD code = GetLastError();
				printf("[proxy] Error: LoadLibrary : %lu \n", code);
				//return FALSE;
			}
		}
	}
	SetDllDirectoryA(defaultDllDir);
}

bool initialize()
{
	OpenConsole();
	// システムパス保存用
	TCHAR system_path_buffer[1024];
	GetSystemDirectory(system_path_buffer, MAX_PATH);

	//BOOL ret = loadDSOUND(system_path_buffer);
	BOOL ret = loadD3D9(system_path_buffer);
	if (!ret)return FALSE;

	// MMDフルパスの取得.
	wchar_t app_full_path[1024];
	GetModuleFileName(NULL, app_full_path, sizeof(app_full_path) / sizeof(wchar_t));
	std::wstring path(app_full_path);
	std::wstring base_path = path.substr(0, path.rfind(_T("MikuMikuDance.exe")));

	moveMMEPlugin(base_path);
	loadMMEPlugin(base_path);
	//ret = loadMMAPlugin(base_path);
	//if (!ret)return FALSE;
	loadPluginDir(base_path);

	return TRUE;
}
////乗っ取り対象ウィンドウの検索
//static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam)
//{
//	HANDLE hProcess = (HANDLE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
//	if (GetModuleHandle(NULL) == hProcess)
//	{
//		//自分のプロセスが作ったウィンドウを見つけた
//		char szClassName[256];
//		GetClassNameA(hWnd, szClassName, sizeof(szClassName) / sizeof(szClassName[0]));
//		std::string name(szClassName);
//		if (name == "Polygon Movie Maker") {
//			g_hWnd = hWnd;
//			return FALSE;	//break
//		}
//	}
//	return TRUE;	//continue
//}
//
////ウィンドウの乗っ取り
//static void overrideGLWindow()
//{
//	if (!g_hWnd) {
//		EnumWindows(enumWindowsProc, 0);
//	}
//}
// DLLエントリポイント
BOOL APIENTRY DllMain(HINSTANCE hinst, DWORD reason, LPVOID)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		g_hinst = hinst;
		//overrideGLWindow();
		initialize();
		break;
	case DLL_THREAD_ATTACH:
		//overrideGLWindow();
		break;
	case DLL_PROCESS_DETACH:
		//mmaccel_end();
		break;
	}
	return TRUE;
}

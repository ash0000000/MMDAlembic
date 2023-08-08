// proxydll.h : proxydll DLL のメイン ヘッダー ファイル
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CproxydllApp
// このクラスの実装に関しては proxydll.cpp をご覧ください
//

class CproxydllApp : public CWinApp
{
public:
	CproxydllApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

#pragma once
#include"DAL\MSAudio.hxx"
#include"DAL\SoundPlayer.h"
#ifdef _DEBUG
//デバッグビルド時
#ifdef _M_X64
//64bitビルド時
#ifdef _DLL
//ランタイムがDLLの時
#pragma comment(lib,"DirectXAudioLibrary-mdd_x64.lib")
#else
//ランタイムがDLLでない時
#pragma comment(lib,"DirectXAudioLibrary-md_x64.lib")
#endif

#else
//32bitビルド時
#ifdef _DLL
//ランタイムがDLLの時
#pragma comment(lib,"DirectXAudioLibrary-mdd_x86.lib")
#else
//ランタイムがDLLでない時
#pragma comment(lib,"DirectXAudioLibrary-md_x86.lib")
#endif


#endif

#else
//リリースビルド時
#ifdef _M_X64
//64bitビルド時

#ifdef _DLL
//ランタイムがDLLの時
#pragma comment(lib,"DirectXAudioLibrary-mtd_x64.lib")
#else
//ランタイムがDLLでない時
#pragma comment(lib,"DirectXAudioLibrary-mt_x64.lib")
#endif

#else
//32bitビルド時
#ifdef _DLL
//ランタイムがDLLの時
#pragma comment(lib,"DirectXAudioLibrary-mtd_x86.lib")
#else
//ランタイムがDLLでない時
#pragma comment(lib,"DirectXAudioLibrary-mt_x86.lib")
#endif


#endif

#endif
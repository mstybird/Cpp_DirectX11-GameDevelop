#pragma once
#include"DX11TextureManager\DX11TextureManager.h"
#include"DX11TextureManager\DX11Texture.h"

#ifdef _DEBUG
//�f�o�b�O�r���h��
#ifdef _M_X64
//64bit�r���h��
#ifdef _DLL
//�����^�C����DLL�̎�
#pragma comment(lib,"DX11TextureManager-mdd_x64.lib")
#else
//�����^�C����DLL�łȂ���
#pragma comment(lib,"DX11TextureManager-md_x64.lib")
#endif

#else
//32bit�r���h��
#ifdef _DLL
//�����^�C����DLL�̎�
#pragma comment(lib,"DX11TextureManager-mdd_x86.lib")
#else
//�����^�C����DLL�łȂ���
#pragma comment(lib,"DX11TextureManager-md_x86.lib")
#endif


#endif

#else
//�����[�X�r���h��
#ifdef _M_X64
//64bit�r���h��

#ifdef _DLL
//�����^�C����DLL�̎�
#pragma comment(lib,"DX11TextureManager-mtd_x64.lib")
#else
//�����^�C����DLL�łȂ���
#pragma comment(lib,"DX11TextureManager-mt_x64.lib")
#endif

#else
//32bit�r���h��
#ifdef _DLL
//�����^�C����DLL�̎�
#pragma comment(lib,"DX11TextureManager-mtd_x86.lib")
#else
//�����^�C����DLL�łȂ���
#pragma comment(lib,"DX11TextureManager-mt_x86.lib")
#endif


#endif

#endif
#pragma once
#define INPUTLAYOUT_POSITION2D(offset) { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 }
#define INPUTLAYOUT_POSITION3D(offset) { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 }
#define INPUTLAYOUT_POSITION4D(offset) { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 }
#define INPUTLAYOUT_NORMAL(offset) { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 }
#define INPUTLAYOUT_TEXCOORD2D(offset) { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 }
#define INPUTLAYOUT_TEXCOORD3D(offset) { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 }
#define INPUTLAYOUT_TEXCOORD4D(offset) { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 }

#define SAFE_RELEASE(x) if(x){(x)->Release(); (x)=NULL;}
#define SAFE_DELETE(x) if ((x))delete (x);(x)=nullptr;
#define SAFE_DELETE_ARRAY(x) if ((x))delete[] (x);(x)=nullptr;
//定数定義
#define WINDOW_WIDTH 640 //ウィンドウ幅
#define WINDOW_HEIGHT 480 //ウィンドウ高さ
#define APP_NAME "DirectX11 Program"

#define _CRTDBG_MAP_ALLOC #include <stdlib.h> #include <crtdbg.h>
#define ALIGN16 _declspec(align(16))
enum class TYPE_ANGLE {
	RADIAN = 0,
	DEGREE
};

#pragma once
#include<d3dx9.h>
#include"DXTranslation.h"
#include"DXCamera.h"
#include"DXProjection.h"
#include"DXDisplay.h"
class DX11RenderResource {
public:
	DX11RenderResource();
	~DX11RenderResource();
	//描画に必要なビュー行列と射影行列が必要な場合必ず呼ぶ
	void InitRenderMatrix();
	D3DXMATRIX*GetMatrixWorld();
	D3DXMATRIX*GetMatrixView();
	D3DXMATRIX*GetMatrixProjection();
	D3DXMATRIX GetMatrixWVP(DXDisplay*pDisplay);

	DXWorld smWorld;
	DXCamera *smView;
	DXProjection *smProj;

};
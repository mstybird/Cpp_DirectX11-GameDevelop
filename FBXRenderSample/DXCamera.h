#pragma once
#include"DXVector3.h"
class DXCamera {
public:
	void SetEyeT(float pX, float pY, float pZ);
	void SetEyeT(D3DXVECTOR3 pPosition);
	void SetLookT(float pX, float pY, float pZ);
	void SetLookT(D3DXVECTOR3 pLookAt);
	void SetUpV(float pX, float pY, float pZ);
	void SetUpV(D3DXVECTOR3 pVector);

	D3DXMATRIX*GetMatrix();

	DXVector3 mEyePosition;	//カメラの位置
	DXVector3 mLookPosition;//注視点
	DXVector3 mUpVector;	//頭上方向
	D3DXMATRIX mMatrix;

	D3DXMATRIX* operator*() {
		return GetMatrix();
	}
private:
	
};
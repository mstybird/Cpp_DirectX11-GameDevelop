#pragma once

#include<d3dx9.h>
class DXVector3;
class DXWorld;
class DXCamera {
public:
	static const DXVector3 sUpVector;
	//カメラの移動タイプ
	enum TYPEMOVE {
		TYPE_NORMAL = 0,	//向きに関係なくXYZ値で直接移動
		TYPE_PARALLEL,		//現在向いている向きを基準に移動
	};
	//移動方向
	class DIRECTION_TYPE {
	public:
		static const DXVector3 LEFTRIGHT;	//左右
		static const DXVector3 UPDOWN;		//上下
		static const DXVector3 FRONTBACK;	//手前奥
	};

	//コンストラクタ
	DXCamera();
	~DXCamera();

	//視点とターゲット(注視点)の設定
	void SetCamera(const DXWorld&pEyePosition, const DXWorld&pLookAtPosition);

	void Translation(TYPEMOVE pType, float pSpeed, const D3DXVECTOR3&pDirection, bool pLockoned = false);


	//カメラの回転
	void Rotate(float pX, float pY, float pZ);

	D3DXMATRIX*GetMatrix();

	DXVector3 *mEyePosition;	//カメラの位置
	DXVector3 *mLookPosition;//注視点
	DXVector3 *mUpVector;	//頭上方向
	DXVector3 *mRotate;		//カメラの角度
	D3DXMATRIX *mMatrix;
	

	D3DXMATRIX* operator*() {
		return GetMatrix();
	}
private:
	
};
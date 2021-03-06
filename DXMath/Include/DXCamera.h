#pragma once
#include<DXMath.hpp>
#include<memory>
//#include<d3dx9.h>
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
	void SetCamera(const DXWorld&pEyePosition, const DXWorld& pLookAtPosition);
	//ワールド座標から指定数ずらす場合
	void SetCamera(DXWorld&pEyePosition, const DXVector3&pDistance);
	//視点と注視点が確定している場合
	void SetCamera(DXWorld&pEyePosition, const DXVector3&pDistance, const DXVector3&aDistanceOffset);
	

	void CreateRay(DXVector3&pOutRay, const DXVector3&pRayDirection);

	//指定した分移動する
	void Translation(TYPEMOVE pType, float pSpeed, const DXVector3&pDirection, bool pLockoned = false);

	void Clone(DXCamera&pOutClone);

	//カメラの回転
	void Rotate(float pX, float pY, float pZ);

	DXMatrix* GetMatrix();
	void GetEyeT(DXVector3 &pOutTranslate);
	void GetLookT(DXVector3 &pOutTranslate);

	DXVector3 mEyePosition;	//カメラの位置
	DXVector3 mLookPosition;//注視点
	DXVector3 mUpVector;	//頭上方向
	DXVector3 mRotate;		//カメラの角度
	DXMatrix mMatrix;		//計算用
	
private:
	
};
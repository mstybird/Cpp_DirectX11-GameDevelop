#include "MSUtility.h"
#include"DX11RenderResource.h"
#include"DXWorld.h"
#include"DXMatrix.h"
#include"DXVector3.h"
#include<memory>

//Y軸回転角計算
//回転角は必ず、pDegree>最終回転角となる
float MSHormingY(DX11RenderResource & pEyeResource, DX11RenderResource & pTargetResource, const float pDegree)
{
	//視点リソースの角度を求める
	DXVector3 lEyeDir;
	float lEyeRot;
	D3DXVec3TransformNormal(&lEyeDir, &DXVector3{ 0,0,1 }, pEyeResource.GetWorld()->GetMatrix());
	lEyeDir.Normalize();
	lEyeRot = D3DXVec3Dot(&DXVector3(0, 0, 1), &lEyeDir);
	lEyeRot = D3DXToDegree(acos(lEyeRot));
	if (lEyeDir.x < 0) { lEyeRot *= -1; }

	//視点からターゲットへの角度を求める
	DXVector3 lTargetPos, lEyePos;
	//ターゲと
	
	pTargetResource.GetWorld()->GetMatrix()->GetT(lTargetPos);
	//自身
	pEyeResource.GetWorld()->GetMatrix()->GetT(lEyePos);
	lEyePos = lEyePos - lTargetPos;
	lEyePos.Normalize();
	float lToTargetRot = D3DXVec3Dot(&DXVector3(0, 0, 1), &lEyePos);
	lToTargetRot = D3DXToDegree(acos(lToTargetRot));
	if (lEyePos.x < 0) { lToTargetRot *= -1; }

	float lDiffAngle = lToTargetRot - lEyeRot;

	if (lDiffAngle < -180) {
		lDiffAngle += 360;
	}
	if (lDiffAngle > 180) {
		lDiffAngle -= 360;
	}

	float lResultDegree;
	if (lDiffAngle > 0) {
		if (180 - lDiffAngle < pDegree && 180 - lDiffAngle>-pDegree) {
			lResultDegree = -fabsf(lDiffAngle - 180);
		}
		else {
			lResultDegree = -pDegree;
		}
	}
	else {
		if (180 + lDiffAngle > -pDegree && 180 + lDiffAngle < pDegree) {
			lResultDegree = fabsf(lDiffAngle + 180);
		}
		else {
			lResultDegree = pDegree;
		}

	}

	return lResultDegree;
}

//Y軸回転角計算
//回転角は必ず、pDegree>最終回転角となる
float MSHormingY(DX11RenderResource & pEyeResource, DXVector3 & pTargetPos, const float pDegree)
{
	//視点リソースの角度を求める
	DXVector3 lEyeDir;
	float lEyeRot;
	D3DXVec3TransformNormal(&lEyeDir, &DXVector3{ 0,0,1 }, pEyeResource.GetWorld()->GetMatrix());
	lEyeDir.Normalize();
	lEyeRot = D3DXVec3Dot(&DXVector3(0, 0, 1), &lEyeDir);
	lEyeRot = D3DXToDegree(acos(lEyeRot));
	if (lEyeDir.x < 0) { lEyeRot *= -1; }

	//視点からターゲットへの角度を求める
	DXVector3 lTargetPos, lEyePos;
	//ターゲと
	lTargetPos = pTargetPos;
	//自身
	pEyeResource.GetWorld()->GetMatrix()->GetT(lEyePos);
	lEyePos = lEyePos - lTargetPos;
	lEyePos.Normalize();
	float lToTargetRot = D3DXVec3Dot(&DXVector3(0, 0, 1), &lEyePos);
	
	if (lToTargetRot >1.0f) {
		lToTargetRot = 1.0f;
	}
	else if (lToTargetRot < -1.0f) {
		lToTargetRot = -1.0f;
	}

	lToTargetRot = D3DXToDegree(acos(lToTargetRot));
	if (lEyePos.x < 0) { lToTargetRot *= -1; }

	float lDiffAngle = lToTargetRot - lEyeRot;

	if (lDiffAngle < -180) {
		lDiffAngle += 360;
	}
	if (lDiffAngle > 180) {
		lDiffAngle -= 360;
	}

	float lResultDegree;
	if (lDiffAngle > 0) {
		if (180 - lDiffAngle < pDegree && 180 - lDiffAngle>-pDegree) {
			lResultDegree = -fabsf(lDiffAngle - 180);
		}
		else {
			lResultDegree = -pDegree;
		}
	}
	else {
		if (180 + lDiffAngle > -pDegree && 180 + lDiffAngle < pDegree) {
			lResultDegree = fabsf(lDiffAngle + 180);
		}
		else {
			lResultDegree = pDegree;
		}

	}

	return lResultDegree;
}

//Threshold未満の値を0として計算する
bool IsZero(float pValue, float Threshold)
{
	Threshold = fabsf(Threshold);
	pValue = fabsf(pValue);
	return pValue<Threshold;
}

//pRyrから見た相対的な角度差を求める
float MSGetToRotateY(DX11RenderResource & pEyeResource, DX11RenderResource & pTargetResource)
{
	//視点リソースの角度を求める
	DXVector3 lEyeDir;
	float lEyeRot;
	D3DXVec3TransformNormal(&lEyeDir, &DXVector3{ 0,0,1 }, pEyeResource.GetWorld()->GetMatrix());
	lEyeDir.Normalize();
	lEyeRot = D3DXVec3Dot(&DXVector3(0, 0, 1), &lEyeDir);
	lEyeRot = D3DXToDegree(acos(lEyeRot));
	if (lEyeDir.x < 0) { lEyeRot *= -1; }

	//視点からターゲットへの角度を求める
	DXVector3 lTargetPos, lEyePos;
	//ターゲと
	pTargetResource.GetWorld()->GetMatrix()->GetT(lTargetPos);
	//自身
	pEyeResource.GetWorld()->GetMatrix()->GetT(lEyePos);
	lEyePos = lEyePos - lTargetPos;
	lEyePos.Normalize();
	float lToTargetRot = D3DXVec3Dot(&DXVector3(0, 0, 1), &lEyePos);
	lToTargetRot = D3DXToDegree(acos(lToTargetRot));
	if (lEyePos.x < 0) { lToTargetRot *= -1; }

	float lDiffAngle = lToTargetRot - lEyeRot;

	if (lDiffAngle < -180) {
		lDiffAngle += 360;
	}
	if (lDiffAngle > 180) {
		lDiffAngle -= 360;
	}

	float lResultDegree;
	if (lDiffAngle > 0) {
			lResultDegree = -fabsf(lDiffAngle - 180);
	}
	else {
			lResultDegree = fabsf(lDiffAngle + 180);
	}

	return lResultDegree;
}


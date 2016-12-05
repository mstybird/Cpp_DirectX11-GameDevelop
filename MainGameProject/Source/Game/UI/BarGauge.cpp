#include "BarGauge.h"
#include<DX11TextureManager.hpp>
#include"MSSprite2DRender.h"
BarGauge::BarGauge()
{
	mInImage.SetPivot({ 0.5f,0.5f });
}
void BarGauge::SetTextures(std::pair<DX11TextureManager, int> aOutImage, std::pair<DX11TextureManager, int> aInImage)
{
	mOutImage.SetTexture(aOutImage.first, aOutImage.second);
	mInImage.SetTexture(aInImage.first, aInImage.second);
}

void BarGauge::SetSize(const float & aWidth, const float & aHeight)
{
	mGaugeSize = { aWidth,aHeight };
}

void BarGauge::SetGaugeScale(const float & aX, const float & aY)
{
	mGaugeScale.x = aX;
	mGaugeScale.y = aY;
}

void BarGauge::SetOffset(const float&aOffsetLeft, const float&aOffsetTop)
{
	mGaugeOffset.x = aOffsetLeft;	//左
	mGaugeOffset.y = aOffsetTop;	//上
}

void BarGauge::SetOffset(const DXVector2&aOffset)
{
	mGaugeOffset = aOffset;
}

void BarGauge::SetParam(MSProgress & aParam)
{
	mResource = aParam;
}

void BarGauge::Update()
{
	//外枠のサイズと位置を設定
	mOutImage.SetSize(mGaugeSize);
	mOutImage.SetPosition(mGlobalPosition);
	
	//中身のサイズと拡大率、オフセットを設定
	auto mInPos = mGlobalPosition;
	mInPos += (mGaugeSize / 2);
	mInImage.SetSize(mGaugeSize);
	mInImage.SetPosition(mInPos);

	mInImage.SetScale(mGaugeScale);

	//右から左に減ると仮定
	mInImage.SetSplitSizeX(
	{ 0, mResource.GetNowPer() }
	);
}

void BarGauge::Render(MSSprite2DRender& aRender, UIBase*aParent)
{
	//親UIがあれば現在の値を一時退避
	DXVector2 mGaugePos;
	if (aParent != nullptr) {
		StoreGlobalData();

		mGlobalPosition += *aParent->GetGlobalPosition();
		mGlobalScale *= *aParent->GetGlobalScale();
		mGaugePos = mGlobalPosition + (mGaugeSize * *aParent->GetGlobalScale() / 2);
	}
	//ゲージフレームの設定
	mOutImage.SetSize(mGaugeSize);
	mOutImage.SetPosition(mGlobalPosition);
	mOutImage.SetScale(mGlobalScale);
	//ゲージ中身のスケール
	auto lGaugeScale = mGaugeScale*mGlobalScale;
	mInImage.SetSize(mGaugeSize);
	mInImage.SetPosition(mGaugePos);
	mInImage.SetScale(lGaugeScale);
	//右から左に減ると仮定
	mInImage.SetSplitSizeX(
	{ 0, mResource.GetNowPer() }
	);
	if (aParent != nullptr) {
		LoadGlobalData();
	}


	aRender.Render(mOutImage);
	aRender.Render(mInImage);
}

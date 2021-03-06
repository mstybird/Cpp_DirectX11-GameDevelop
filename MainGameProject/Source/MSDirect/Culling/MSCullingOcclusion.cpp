#include "MSCullingOcclusion.h"
#include"DX11Resrouce.h"
#include"DXMath.hpp"
#include"MSDirect.h"
#include"GameObjectBase.h"
#include<iostream>
#include<vector>
ID3D11RenderTargetView* MSCullingOcculusion::sRTV;
ID3D11DepthStencilView* MSCullingOcculusion::sDSV;
ID3D11Texture2D* MSCullingOcculusion::sDS2D;

ID3D11Query* MSCullingOcculusion::sOcculusionQuery;
ID3D11Device*MSCullingOcculusion::sDevice;
ID3D11DeviceContext*MSCullingOcculusion::sDeviceContext;
IDXGISwapChain*MSCullingOcculusion::sSwapChain;

//Direct3D 10, Occulusion Predicate Query

bool MSCullingOcculusion::IsCullingWorld(
	std::vector<GameObjectBase*>*pHitList,
	MS3DRender*pRender,
	GameObjectBase*pEyeResource,
	std::vector<GameObjectBase*>*pTargetResource,
	float pPixelper,
	std::function<void(void)>pRenderFunc
	)
{
	//ワールド行列から視点を生成

	//元に戻すためのカメラのコピーを生成
	DXCamera lEyeCameraCopy;
	pEyeResource->GetTransform()->GetCamera()->Clone(lEyeCameraCopy);

	//カメラをワールド行列を使って正面方向に作成する
	pEyeResource->GetTransform()->GetCamera()->SetCamera(*pEyeResource->GetWorld(), { 0,0,-1 });
	//視野を設定
	//pEyeResource.SetProjection(pEyeProjection);

	//クエリの開始
	{
		//描画先をクエリ処理用に変更

		//ID3D11RenderTargetView* lRTVCopy;
		//ID3D11DepthStencilView* lDSVCopy;
		D3D11_VIEWPORT* lMainViewPort = MSDirect::GetViewPort();
		auto lActiveView = MSDirect::GetActiveView();
		//lRTVCopy = MSDirect::GetRTV();
		//lDSVCopy = MSDirect::GetDSV();


		D3D11_VIEWPORT vp;
		vp.Width = 480;
		vp.Height = 320;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		sDeviceContext->RSSetViewports(1, &vp);
		sDeviceContext->OMSetRenderTargets(1, &sRTV, sDSV);

		//MS3DRender::Clear({ 1,1,1,1 });

		float ClearColor[4] = { 0.4f,0.2f,0.2f,1.0f };
		sDeviceContext->ClearRenderTargetView(sRTV, ClearColor);//レンダーターゲットクリア
		sDeviceContext->ClearDepthStencilView(sDSV, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);//深度ステンシルバッファクリア

		//障害物を描画する処理
		DXDisplay lTmpDisplay;
		pRender->GetDisplay(lTmpDisplay);
		pRender->SetRenderTarget(*pEyeResource->GetTransform());

		pRender->SetShader(pEyeResource->GetCollisionShader());
		pRenderFunc();


		//レンダリング判定をする描画

		for (auto lTarget : *pTargetResource) {
			BeginOcclusionQuery();

			pRender->SetShader(lTarget->GetCollisionShader());
			pRender->Render(lTarget);
		
		
		
			EndOcclusionQuery();

			UINT64 lDrawPixels{};
			//指定数以上ループした場合、強制で抜ける
			UINT size = sizeof(UINT64);
			//ピクセル数を取得
			while (S_OK != sDeviceContext->GetData(sOcculusionQuery, &lDrawPixels, size,0));


			//見えたと判定する必要なピクセル数の計算
			UINT64 lCheckPixels = (UINT64)((480 * 320)*pPixelper);

			if (lDrawPixels > lCheckPixels) {
				pHitList->push_back(lTarget);
			}

		}

		pRender->SetRenderTarget(lTmpDisplay);
		//カメラと視野をもとに戻す
		pEyeResource->GetTransform()->SetCamera(lEyeCameraCopy);
		//描画先を元に戻す
		sDeviceContext->RSSetViewports(1, lMainViewPort);
		sDeviceContext->OMSetRenderTargets(1, lActiveView->GetRTV(), *lActiveView->GetDSV());
	}


	return true;
}

void MSCullingOcculusion::Initialize(ID3D11Device * pDevice, ID3D11DeviceContext * pDeviceContext, IDXGISwapChain*pSwapChain)
{
	sDevice = pDevice;
	sDeviceContext = pDeviceContext;
	sSwapChain = pSwapChain;

	//カリングで使うレンダーターゲットビューの作成
	{
		ID3D11Texture2D*lBackBuffer;
		sSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&lBackBuffer);
		sDevice->CreateRenderTargetView(lBackBuffer, nullptr, &sRTV);
		SAFE_RELEASE(lBackBuffer);
	}

	//カリングで使うステンシルビューの作成
	{
		D3D11_TEXTURE2D_DESC lDescDepth;
		lDescDepth.Width = 480;
		lDescDepth.Height = 320;
		lDescDepth.MipLevels = 1;
		lDescDepth.ArraySize = 1;
		lDescDepth.Format = DXGI_FORMAT_D32_FLOAT;
		lDescDepth.SampleDesc.Count = 1;
		lDescDepth.SampleDesc.Quality = 0;
		lDescDepth.Usage = D3D11_USAGE_DEFAULT;
		lDescDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		lDescDepth.CPUAccessFlags = 0;
		lDescDepth.MiscFlags = 0;

		sDevice->CreateTexture2D(&lDescDepth, nullptr, &sDS2D);
		sDevice->CreateDepthStencilView(sDS2D, nullptr, &sDSV);

	}

	//実際にカリングする処理から続き
	CreateOcclusionQuery();


}

void MSCullingOcculusion::CreateOcclusionQuery()
{
	D3D11_QUERY_DESC lQueryDesc{};
	ZeroMemory(&lQueryDesc, sizeof(D3D11_QUERY_DESC));

	lQueryDesc.Query = D3D11_QUERY_OCCLUSION;
	
	sDevice->CreateQuery(&lQueryDesc, &sOcculusionQuery);
	
}

void MSCullingOcculusion::BeginOcclusionQuery()
{
	sDeviceContext->Begin(sOcculusionQuery);
}

void MSCullingOcculusion::EndOcclusionQuery()
{
	sDeviceContext->End(sOcculusionQuery);
}

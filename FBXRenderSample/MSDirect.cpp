#include"MSDirect.h"
#include"DX11Resrouce.h"
#include"MSSceneBase.h"
//シェーダ関係
#include"MSBaseShaderResource.h"
#include"MSBaseShader.h"
//スプライトレンダラー
#include"MSSprite2DResource.h"
#include"MSSprite2DRender.h"
//3Dレンダラー
#include"MS3DRender.h"
//FBX
#include"MSFbxManager.h"
//Other
#include"DXProjection.h"
#include"DX11Texture.h"

#include<D3DX11.h>

std::shared_ptr<MSDirect> MSDirect::sMSDirect{ std::make_shared<MSDirect>() };

void MSDirect::Destroy()
{
	sMSDirect.reset();
}

MSDirect::MSDirect() :
	mHwnd{ nullptr },
	m_pDevice{ nullptr },
	m_pDeviceContext{ nullptr },
	m_pSwapChain{ nullptr },
	m_pBackBuffer_TexRTV{ nullptr },
	m_pBackBuffer_DSTexDSV{ nullptr },
	m_pBackBuffer_DSTex{ nullptr },
	m_pRasterizerState{ nullptr }
{

}

MSDirect::~MSDirect() {
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pBackBuffer_TexRTV);
	SAFE_RELEASE(m_pBackBuffer_DSTexDSV);
	SAFE_RELEASE(m_pBackBuffer_DSTex);
	SAFE_RELEASE(m_pRasterizerState);
}

LRESULT MSDirect::MessageProcedule(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	if (scene.get() != nullptr) {

		switch (iMsg)
		{
		case WM_MOVE:
			//ウィンドウ移動時
			break;
		case WM_SIZE:
			//ウィンドウサイズ変更時
			break;
		case WM_ACTIVATE:
			//ウィンドウのアクティブ状態の切り替え時
			break;
		case WM_ENABLE:
			//ウィンドウの有効無効切り替え時
			break;
		case WM_SHOWWINDOW:
			//ウィンドウ表示/非表示切り替え時
			break;
		case WM_SETFOCUS:
			//ウィンドウがフォーカスを取得したとき
			break;
		case WM_KILLFOCUS:
			//ウィンドウがフォーカスを失ったとき
			break;
		case WM_GETMINMAXINFO:
			//ウィンドウの最大最小サイズ、最大時の位置、最小トラッキングサイズなどを取得
			break;
		case WM_SETTEXT:
			//ウィンドウの文字列の設定時
			break;
		case WM_GETTEXT:
			//ウィンドウの文字列の取得時
			break;
		case WM_KEYDOWN:
			//キー入力時
			scene->KeyDown(wParam);
			break;
		case WM_DESTROY:
			//ウィンドウ破棄時
			PostQuitMessage(0);
			break;
		}
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

const std::weak_ptr<MSDirect> MSDirect::GetInstance()
{
	return sMSDirect;
}

HRESULT MSDirect::InitD3D(HWND pHwnd)
{
	mHwnd = pHwnd;
	// デバイスとスワップチェーンの作成
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = WINDOW_WIDTH;
	sd.BufferDesc.Height = WINDOW_HEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = mHwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL pFeatureLevels = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL* pFeatureLevel = NULL;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		0, &pFeatureLevels, 1, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice,
		pFeatureLevel, &m_pDeviceContext)))
	{
		return FALSE;
	}


	//各種テクスチャーと、それに付帯する各種ビューを作成
	//バックバッファーテクスチャーを取得（既にあるので作成ではない）
	ID3D11Texture2D *pBackBuffer_Tex;
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer_Tex);
	//そのテクスチャーに対しレンダーターゲットビュー(RTV)を作成
	m_pDevice->CreateRenderTargetView(pBackBuffer_Tex, NULL, &m_pBackBuffer_TexRTV);
	SAFE_RELEASE(pBackBuffer_Tex);

	//デプスステンシルビュー用のテクスチャーを作成
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = WINDOW_WIDTH;
	descDepth.Height = WINDOW_HEIGHT;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	m_pDevice->CreateTexture2D(&descDepth, NULL, &m_pBackBuffer_DSTex);
	//そのテクスチャーに対しデプスステンシルビュー(DSV)を作成
	m_pDevice->CreateDepthStencilView(m_pBackBuffer_DSTex, NULL, &m_pBackBuffer_DSTexDSV);

	//レンダーターゲットビューと深度ステンシルビューをパイプラインにバインド
	m_pDeviceContext->OMSetRenderTargets(1, &m_pBackBuffer_TexRTV, m_pBackBuffer_DSTexDSV);
	//ビューポートの設定
	//D3D11_VIEWPORT vp;

	mViewPort.Width = WINDOW_WIDTH;
	mViewPort.Height = WINDOW_HEIGHT;
	mViewPort.MinDepth = 0.0f;
	mViewPort.MaxDepth = 1.0f;
	mViewPort.TopLeftX = 0;
	mViewPort.TopLeftY = 0;
	m_pDeviceContext->RSSetViewports(1, &mViewPort);
	//ラスタライズ設定
	D3D11_RASTERIZER_DESC rdc;
	ZeroMemory(&rdc, sizeof(rdc));
	rdc.CullMode = D3D11_CULL_NONE;
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.FrontCounterClockwise = TRUE;

	m_pDevice->CreateRasterizerState(&rdc, &m_pRasterizerState);
	m_pDeviceContext->RSSetState(m_pRasterizerState);

	//各ラッパーにDirectX11デバイス等を登録
	MSBaseShaderResource::sInitialize(m_pDevice, m_pDeviceContext);
	//MSPixelShader::sInitialize(m_pDevice, m_pDeviceContext);
	MSBaseShader::Init(m_pDevice, m_pDeviceContext);
	MSSpriteBaseResource::sInitialize(m_pDevice);
	MSSpriteBaseRender::Initialize(m_pDevice, m_pDeviceContext, m_pBackBuffer_TexRTV, m_pBackBuffer_DSTexDSV);
	MS3DRender::Initialize(m_pDevice, m_pDeviceContext, m_pBackBuffer_TexRTV, m_pBackBuffer_DSTexDSV);

	MSFbxManager::InitDevice(m_pDevice, m_pDeviceContext);
	DXProjection::SetAspect((FLOAT)WINDOW_WIDTH, (FLOAT)WINDOW_HEIGHT);
	DXTexture::Initialize(m_pDevice);


	return S_OK;
}


void MSDirect::Loop()
{

	scene->Update();
	scene->Render();
	m_pSwapChain->Present(1, 0);
}

void MSDirect::SetScene(std::unique_ptr<MSSceneBase>&& pScene)
{
	pScene->Initialize();
	std::unique_ptr<MSSceneBase> lBufferPtr = std::move(sMSDirect->scene);
	sMSDirect->scene = std::move(pScene);
	lBufferPtr.reset();
}

D3D11_VIEWPORT * MSDirect::GetViewPort()
{
	return &sMSDirect->mViewPort;
}


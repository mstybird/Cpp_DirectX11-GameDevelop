#pragma once
#include<memory>
#include<D3DX11.h>
#include<unordered_map>
#include"MSLayout.h"
#include<DXMath.hpp>
class MSSceneBase;
enum class MSKEY;

class MSDirect {
public:
	static const std::weak_ptr<MSDirect> GetInstance();
	HRESULT InitD3D(HWND pHwnd);
	void Loop();
	//新しいシーンのセット
	static void SetScene(std::unique_ptr<MSSceneBase>&&pScene);
	//ビューポートを取得する
	static D3D11_VIEWPORT*GetViewPort();
	//static ID3D11RenderTargetView* GetRTV();
	//static ID3D11DepthStencilView* GetDSV();
	static MSView* GetView();
	static MSView* GetActiveView();
	static MSView* SetActiveView(MSView* pView);
	static void SetDefaultView();
	static HWND GetWinHandle();

	static ID3D11Device*GetDevice();
	static ID3D11DeviceContext*GetDeviceContext();
	static IDXGISwapChain*GetSwapChain();
	//キー入力状態を初期化する
	static void ResetKeyStateAll();
	static void ResetKeyState(MSKEY pResetKey);
	//インスタンスの解放(シーンからは呼ばない)
	static void Destroy();
	MSDirect();
	~MSDirect();
	//メッセージプロシージャ
	//シーンにキー入力などの処理をさせる用
	LRESULT MessageProcedule(HWND, UINT, WPARAM, LPARAM);

private:

	static std::shared_ptr<MSDirect>sMSDirect;
	HWND mHwnd;


	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	IDXGISwapChain* m_pSwapChain;


	MSView mView;	//MSDirectが持つビュー情報
	MSView* mActiveView;//現在アクティブなビュー

	//ID3D11RenderTargetView* m_pBackBuffer_TexRTV;
	//ID3D11DepthStencilView* m_pBackBuffer_DSTexDSV;
	//ID3D11Texture2D* m_pBackBuffer_DSTex;

	ID3D11RasterizerState* m_pRasterizerState;
	D3D11_VIEWPORT mViewPort;

	std::unordered_map<MSKEY, bool> KeyList;
	std::unique_ptr<MSSceneBase>scene;
	LONG sceneCounter;
	//	MSSceneBase scene;
	bool mIsSceneChanging = false;

	//マウス初期位置
	POINT mMouseDefaultPosition;
	//直前フレーム位置
	POINT mMouseBeforePosition;

	//マウス現在位置
	POINT mMouseNowPosition;

};
#pragma once
#include<d3dx9.h>
#include<D3DX11.h>
class MSFbxManager;
class DX11RenderResource;
class MSBase3DShader;
class DXDisplay;
#include<memory>
//#include"DXDisplay.h"
//DirectX11�ŕ`�悷��N���X
class MS3DRender {
public:
	//�g�|���W�[�̏�����
	MS3DRender();
	~MS3DRender();
	//DirectX11�f�o�C�X�̓o�^
	static void Initialize(
		ID3D11Device*pDevice,
		ID3D11DeviceContext*pDeviceContext,
		ID3D11RenderTargetView*sRenderTargetView,
		ID3D11DepthStencilView*sDepthStencilView
		);
	//��ʃN���A
	static void Clear(D3DXVECTOR4 pColor);

	//�����_�����O
	void Render(const std::weak_ptr<MSFbxManager>fbxManager,const std::weak_ptr<DX11RenderResource>resource);

	//�`����(�r���[�s��Ǝˉe�s��)��ݒ�
	void SetRenderTarget(const std::weak_ptr<DX11RenderResource>resource);
	//�V�F�[�_�[�̓o�^
	void SetShader(const std::shared_ptr<MSBase3DShader>pShader);
protected:

	//�����_�����O�Ɏg�p����V�F�[�_�[

	std::weak_ptr<MSBase3DShader>shader;
	std::shared_ptr<DXDisplay>display;
	const D3D_PRIMITIVE_TOPOLOGY mPrimitiveTopology;

	 static ID3D11Device*sDevice;				//DirectX11�f�o�C�X
	 static ID3D11DeviceContext*sDeviceContext;	//DirectX11�f�o�C�X�R���e�L�X�g
	 static ID3D11RenderTargetView*sRenderTargetView;
	 static ID3D11DepthStencilView*sDepthStencilView;

};


#pragma once

class DX11RenderResource;
class MSFbxManager;
class DXProjection;
//������N���X
class MSCullingFrustum {
public:
	//�^�[�Q�b�g���L�����N�^�[�̎��E�������ׂ�
	/*
		����ɂ͋��E����p����(���E�����쐬����Ă��Ȃ��ꍇ�͓����Ŏ����I�ɍ����
	*/
	bool IsCullingWorld(
		DX11RenderResource&pCameraResource,
		DX11RenderResource&pTargetResource
	);
};
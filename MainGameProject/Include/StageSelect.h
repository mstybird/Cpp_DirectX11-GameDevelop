#pragma once
#include"MSSceneBase.h"
#include<NcgLua.hpp>
#include<DX11TextureManager.hpp>
#include"MySprite2DShader.h"
#include"MSSprite2DRender.h"
#include"MSSprite2DResource.h"
#include"StageSelectList.h"
#include<DXAL.hpp>
#include<CppTweener.h>
#include<MSThread.hpp>
#include<memory>
#include<iostream>
#include<vector>
#include<string>

/*
	Memo
	SelectList の完成
	ゲームの進行を改善

*/

namespace ValueSS {

	namespace UI {
		static const int cLuaID = 0;
		static const char* cLuaFilePath = "Resource/Script/StageSelect.lua";

		static const char* cBackgroundPosition = "BackgroundPosition";
		static const char* cBackgroundSize = "BackgroundSize";
		static const char* cBackgroundScale = "BackgroundScale";
		static const char* cBackGroundTexturePath = "BackgroundTexturePath";

		static const char* cTitlePosition = "TitlePosition";
		static const char* cTitleSize = "TitleSize";
		static const char* cTitleScale = "TitleScale";
		static const char* cTitleTexturePath = "TitleTexturePath";

		static const char* cThumnailPosition = "ThumbnailPosition";
		static const char* cThumnailSize = "ThumbnailSize";
		static const char* cThumnailScale = "ThumbnailScale";
		static const char* cThumnailTexturePath = "ThumbnailTexturePath";

		static const char* cStageListFramePosition = "StageListFramePosition";
		static const char* cStageListFrameSize = "StageListFrameSize";
		static const char* cStageListFrameScale = "StageListFrameScale";
		static const char* cStageListFrameTexturePath = "StageListFrameTexturePath";

		static const char* cButtonPosition = "ButtonPosition";
		static const char* cButtonSize = "ButtonSize";
		static const char* cButtonScale = "ButtonScale";
		static const char* cButtonOffset = "ButtonOffset";

		static const char* cStagePathList = "StagePathList";

		static const char* cDescTextPosition = "DescTextPosition";
		static const char* cDescTextSize = "DescTextSize";
		static const char* cDescTextScale = "DescTextScale";
		static const char* cDescTextTexturePath = "DescTextTexturePath";

		static const int cBackGroundID = 0;
		static const int cTitleID = 10;
		static const int cThumnailID = 20;
		static const int cStageListFrameID = 30;
		static const int cDescTextID = 30;
		static const int cStagePathOffsetID = 500;
		static const int cCursorID = 40;

	}

	//ID:5000~
	namespace Sound {
		static const int cLuaID = 5000;
		static const char* cLuaPath = "Resource/Script/StageSelectSound.lua";


		static const char* cBGMFilePath = "BGMPath";
		static const char* cSESelectPath = "SESelectPath";
		static const char* cSEEnterPath = "SEEnterPath";

	}

}

class SceneStageSelect :public MSSceneBase {
public:
	void Initialize()override;
	void Update()override;
	void KeyDown(MSKEY pKey)override;
	void KeyHold(MSKEY pKey)override;
	void MouseMove(const POINT & aNowPosition, const POINT & aDiffPosition)override;
	virtual void MouseDown(const MouseType aType)override;
	virtual void MouseUp(const MouseType aType)override;

	
	void Render()override;
	void Destroy() override {}

private:
	//2Dシェーダの初期化
	void InitShader();
	//テクスチャの読み込み
	void InitUI();
	//サウンドの読み込み
	void InitSound();
	//選択したステージの一時セーブ
	void StageSelectSave();

	//シーン遷移の処理
	void UpdateSceneChange();
private:
	
	enum class SceneSequence {
		eChangeFirst,
		eChangeLoop,
		eChangeEnd
	};

	//Luaデータベース
	NcgLuaDatabase mLuaDb;
	//テクスチャマネージャ
	DX11TextureManager mTexManager;

	//2Dスプライト関係
	MySprite2DShader m2DShader;
	MSSprite2DRender m2DRender;

	//背景
	MSSprite2DResource mBackground;
	//セレクトタイトル
	MSSprite2DResource mSelectTitle;
	//操作説明テキスト
	MSSprite2DResource mDescText;
	//マウスカーソル
	MSSprite2DResource mCursor;

	//ステージリスト
	StageSelectList mSelectList;
	int mButtonLastPushed = -1;
	SoundDevice mSoundDevice;
	SoundPlayer mBGM;
	SoundPlayer mSESelect;
	SoundPlayer mSEEnter;

	bool mIsSceneChange = false;
	//シーン遷移に使う遅延評価スレッド
	MSThread mScneThread;
	//シーン遷移に使うシーケンスフラグ
	SceneSequence mSequence;
	uint32_t mSceneTime;
	//遷移シーン
	std::unique_ptr<MSSceneBase> mScene;
	//Tween
	tween::Tweener mTweener;
	bool mIsMoveThumbnail = false;
};
#include "Enemy.h"
#include"NcgLua.hpp"
#include"EnemyAI.h"
#include"DX11RenderResource.h"
#include"MSCollisionRayPicking.h"
#include"DXMatrix.h"
#include"DXWorld.h"
#include"MSUtility.h"
#include"MSCullingOcclusion.h"
#include"MSCullingFrustum.h"
#include"StatusField.h"
#include"DXMatrix.h"
#include"Ball.h"
#include"CharacterBase.h"
#include"StatusBulletBase.h"
#include"ChangeStates.hxx"
#include"StatusField.h"
#include"BulletManager.h"
Enemy::Enemy()
{
	mCameraLen = { 0.0f,0.0f,-0.1f };
	mCameraOffset = { 0.0f,0.0f,0.0f };
	//mBulletNormal = std::make_unique<BulletNormal>();
	mStatus = std::make_unique<EnemyStatus>();
}
Enemy::~Enemy()
{
}
void Enemy::Initialize(StatusField&pSetStatus)
{
	CharacterBase::Initialize(pSetStatus);

	mAI = std::make_unique<EnemyAI>();
	mAI->CreateNodes(*mField);

}
void Enemy::InitFinal()
{
	//ゴールの初期化
	SetGoalIndex(mField->GetTeamAlly(this)->GetGoalIndex());
	
	mRayPick->SetFramePosition(*mTransform);
	//一番近いノードを初期座標としておく
	DXVector3 lPosition;
	GetWorld()->GetMatrix()->GetT(lPosition);
	auto lNearNode = mAI->GetNearNodeList(lPosition)[0];
	mAI->SetStartNode(lNearNode->GetID());
	mTransform->GetMesh()->SetAnimation(0);
}
void Enemy::SetAI(NcgLuaManager * aAI)
{
	mAI->mLuaAI = aAI;
	mAI->mLuaAI->SetFunction("GetPlan", 1, 11);	//関数の設定
}

void Enemy::SetGoalIndex(int aIndex)
{
	
	GetStatus<EnemyStatus>()->mGoalIndex = aIndex;
}



void Enemy::StartAI()
{
	mIsStopAI = false;
}

void Enemy::StopAI()
{
	mIsStopAI = true;
}

void Enemy::Update()
{
	UpdateMotion();
	UpdateBullets();
	UpdateStatus();
	if (mIsStopAI)return;


	if (mStatus->mLive == CharaStateFlag::ALIVE) {

		switch (mAI->GetNowAI())
		{
		case EnemyAIType::eUnKnown:
			UpdateAI();
			break;
		case EnemyAIType::eMoveToBall:
			BetaMoveToBall();
			break;
		case EnemyAIType::eMoveToGoal:
			BetaMoveToGoal();
			break;
		case EnemyAIType::eMoveToHide:
			BetaMoveToHide();
			break;
		case EnemyAIType::eChargeEnergy:
			BetaChargeEnergy();
			break;
		case EnemyAIType::eInSightAttack:
			BetaInSightAttack();
			break;
		case EnemyAIType::eMoveToBallHoldAlly:
			BetaMoveToBallHoldAlly();
			break;
		case EnemyAIType::eSearchEnemyShort:
			BetaSearchEnemyShort();
			break;
		case EnemyAIType::eSerachEnemyAll:
			BetaSearchEnemyAll();
			break;
		case EnemyAIType::eMoveToLookingTarget:
			BetaMoveToLookingTarget();
			break;
		case EnemyAIType::eMoveToBallTarget:
			BetaMoveToBallTarget();
			break;
		case EnemyAIType::eSearchForEnemyArea:
			BetaSearchForEnemyArea();
			break;
		case EnemyAIType::eSearchForAllyArea:
			BetaSearchForAllyArea();
			break;
		default:
			break;
		}
		//行動処理が全て終わってから衝突判定
		UpdateCollision(true);
	}


	UpdateAlive();

	UpdateCamera();
}

void Enemy::LivingIsRespawnWaitProc()
{

	//スポーン
	//InitStatus();
	mAI->ClearAI();
	Respawn();
	GetWorld()->mPosition;
	auto lNowNode = mAI->GetNearNodeList(GetWorld()->mPosition)[0];
	mAI->SetNowNode(lNowNode);
	mRayPick->SetFramePosition(*mTransform);


}

void Enemy::InitStatus(const StatusBase* aInitStatus)
{
	CharacterBase::InitStatus(aInitStatus);
	auto lStatus = GetStatus<EnemyStatus>();
	lStatus->mIsTargetingBall = false;
	lStatus->mTargetting = false;
	lStatus->mLastLookPosiion = {};
	lStatus->mAllyComplianceCount = 0;
	lStatus->mTargetting = false;
	lStatus->mAIType = EnemyAIType::Type::eUnKnown;
}

void Enemy::UpdateAI()
{
	mAI->ClearRoot();
	//敵そのもののステータス
	auto lStatus = GetStatus<EnemyStatus>();



	//現在セットされている弾のステータスを取得
	auto lBulletStatus = mBltManager->GetActiveStatus(this);




	struct tAIStatus {
		//BallHolder
		bool mBallHoldField{ false };
		bool mBallHoldAlly{ false };
		bool mBallHoldEnemy{ false };
		bool mBallHoldMe{ false };

		//EnemyState
		bool mInSigntEnemy{ false };
		bool mLockonEnemy{ false };
		bool mBallHoldTarget{ false };

		//AllyState
		bool mInSightAlly{ false };
		bool mBallAllyNear{ false };
		bool mAllyNear{ false };

		//Other
		bool mChargedEnergy{ false };

		

	}lAIStatus;

	lAIStatus.mBallHoldField = mField->mBallHoldChara == nullptr;


	auto lTeamAlly= mField->GetTeamAlly(mField->mBallHoldChara);
	auto lTeamEnemy = mField->GetTeamEnemy(mField->mBallHoldChara);
	if (lTeamAlly) {
		lAIStatus.mBallHoldAlly = lTeamAlly->IsMember(this);
	}
	if (lTeamEnemy) {
		lAIStatus.mBallHoldEnemy = lTeamEnemy->IsMember(this);
	}
	
	
	
	lAIStatus.mBallHoldMe = lStatus->mBall != nullptr;

	lAIStatus.mInSigntEnemy = lStatus->mTargetting;
	lAIStatus.mLockonEnemy = lStatus->mTargetChara != nullptr;

	lAIStatus.mBallHoldTarget = mField->mBallHoldChara != nullptr&&lStatus->mTargetChara == mField->mBallHoldChara;

	lAIStatus.mInSightAlly = false;


	lAIStatus.mBallAllyNear = false;
	if (lAIStatus.mBallHoldAlly==true) {
		if (GetDistance(mField->mBallHoldChara) < 10.0f) {
			lAIStatus.mBallAllyNear = true;
			lAIStatus.mAllyNear = true;
		}
	}

	//味方全走査
	lAIStatus.mAllyNear = false;
	for (decltype(auto)lAlly : *mField->GetTeamAlly(this)->GetMembers()) {
		if (lAIStatus.mAllyNear == true)break;
		//自身はチェックしない
		if (lAlly == this) continue;
		//一人でも近くにいればtrue
		if (GetDistance(lAlly) < 10.0f) {
			lAIStatus.mAllyNear = true;
		}
	}

	//キャラクターが保持するアクティブID
	auto ID = mBltManager->GetActiveBulletID(this);
	
	lAIStatus.mChargedEnergy = lStatus->mEnergy.GetNow() > lBulletStatus->mCost;

	mAI->Update(
		lAIStatus.mBallHoldField,
		lAIStatus.mBallHoldAlly,
		lAIStatus.mBallHoldEnemy,
		lAIStatus.mBallHoldMe,

		//EnemyState
		lAIStatus.mInSigntEnemy,
		lAIStatus.mLockonEnemy,
		lAIStatus.mBallHoldTarget,

		//AllyState
		lAIStatus.mInSightAlly,
		lAIStatus.mBallAllyNear,
		lAIStatus.mAllyNear,

		//Other
		lAIStatus.mChargedEnergy
		);
}

void Enemy::Render()
{
	assert(mRender);
	mRender->SetShader(mShader);
	mRender->Render(this);
	//mRender->Render(this,true);
	RenderBullets();

}


Ball* Enemy::UtlCollisionBall()
{
	auto lHitTargets = UpdateCollision(false);
	for (auto&lHitTarget : lHitTargets) {
		if (lHitTarget) {
			//ボールに当たった場合、そのボールを回収する
			Ball* lBall = dynamic_cast<Ball*>(lHitTarget);
			if (lBall) {
				return lBall;
			}
		}
	}
	return nullptr;
}

bool Enemy::MoveNode()
{
	MyNode*lAINode;
	static int count = 0;
	mAI->GetFrontNode(lAINode);
	//移動先ノードがなかったときは処理しない
	if (!lAINode)return false;
	float lAng = MSHormingY(*mTransform, lAINode->Position, 6.0f);
	GetWorld()->AddRC(0, lAng, 0);
	if (IsZero(lAng, 120.0f)) {
		GetWorld()->AddT(DXWorld::TYPE_ROTATE, 0.1f, { 0,0,1 });
		DXVector3 lLength;
		GetWorld()->GetMatrix()->GetT(lLength);
		lLength = lLength - lAINode->Position;
		if (lLength.GetDistance() < 1.0f) {
			if (lAINode->GetID() == 1) {
				++count;
			}
			return mAI->SetNextNode();

		}
	}
	return true;
}


std::vector<GameObjectBase*> Enemy::IsCulling()
{
	//視界内のオブジェクトリスト
	std::vector<GameObjectBase*> lTargetList;

	GameObjectBase* lLookTargetPtr{};
	//視界処理
	//MSCullingFrustum cf;

	std::vector<GameObjectBase*> lTargets;
	for (auto&lTarget : mSearchTargets) {
		//キャラクターであり、死んでいれば対象外
		auto lChara = dynamic_cast<CharacterBase*>(lTarget);
		if (lChara) {
			if (lChara->GetStatus()->mLive != CharaStateFlag::ALIVE) {
				continue;
			}
		}

		lTargets.push_back(lTarget);

	}

	{
		auto lTmpThis = mTransform;
		DXVector3 lTmpSThis = GetWorld()->mPosition;
		mCollisionMesh->GetWorld()->SetT(lTmpSThis);
		mCollisionMesh->GetWorld()->SetRC(GetWorld()->mRotationCenter);
		if (mIsCollisionScaleDefault == true) {
			GetWorld()->GetMatrix()->GetS(lTmpSThis);
			mCollisionMesh->GetWorld()->SetS(lTmpSThis);
		}
		auto& lThisCam = *mTransform->GetCamera();
		auto& lThisProj = *mTransform->GetProjection();
		mCollisionMesh->SetCamera(lThisCam);
		mCollisionMesh->SetProjection(lThisProj);


		mTransform = mCollisionMesh;

		float lSearchPixelPer;
		if (this->GetStatus<EnemyStatus>()->mTargetting) {
			lSearchPixelPer = 0.00001f;
		}
		else {
			lSearchPixelPer = 0.003f;
		}
		MSCullingOcculusion::IsCullingWorld(
			&lTargetList, mRender, this, &lTargets, lSearchPixelPer,
			[&, this]() {


			for (auto&lCollision : mCollisionTargets) {

				auto lTmpMesh = lCollision->mTransform;
				DXVector3 lTmpS;
				//判定メッシュをコリジョン用に変更
				lCollision->GetWorld()->GetMatrix()->GetT(lTmpS);
				lCollision->mCollisionMesh->GetWorld()->SetT(lTmpS);
				lCollision->mCollisionMesh->GetWorld()->SetRC(lCollision->GetWorld()->mRotationCenter);


				//コリジョンスケールが別で設定されていない場合は
				//メッシュのコリジョンスケールを使う
				if (lCollision->mIsCollisionScaleDefault == true) {
					lCollision->GetWorld()->GetMatrix()->GetS(lTmpS);
					lCollision->mCollisionMesh->GetWorld()->SetS(lTmpS);
				}

				lCollision->mTransform = lCollision->mCollisionMesh;

				if (lCollision->IsActive() == false) {
					lCollision->mTransform = lTmpMesh;
					continue;
				}
				//登録済みコリジョンがカリングターゲットと同じだった場合は障害物として描画しない

				bool lSameFlag{ false };
				for (auto&lTarget : lTargets) {
					if (lTarget == lCollision) {
						lCollision->mTransform = lTmpMesh;
						lSameFlag = true;
						break;
					}
				}
				if (lSameFlag == true) {
					continue;
				}

				mRender->SetShader(mCollisionShader);
				mRender->Render(lCollision, false, true);

				lCollision->mTransform = lTmpMesh;


			}
		}
		);

		/*) {
			lTargetList.push_back(lTarget);
		}*/

		mTransform = lTmpThis;

	}
	/*
	for (auto&lTarget : mSearchTargets) {
		if (true) {

			//キャラクターであり、死んでいれば対象外
			auto lChara = dynamic_cast<CharacterBase*>(lTarget);
			if (lChara) {
				if (lChara->GetStatus()->mLive != CharaStateFlag::ALIVE) {
					continue;
				}
			}

			auto lTmpThis = mTransform;
			DXVector3 lTmpSThis = GetWorld()->mPosition;
			mCollisionMesh->GetWorld()->SetT(lTmpSThis);
			mCollisionMesh->GetWorld()->SetRC(GetWorld()->mRotationCenter);
			if (mIsCollisionScaleDefault == true) {
				GetWorld()->GetMatrix()->GetS(lTmpSThis);
				mCollisionMesh->GetWorld()->SetS(lTmpSThis);
			}
			auto& lThisCam = *mTransform->GetCamera();
			auto& lThisProj = *mTransform->GetProjection();
			mCollisionMesh->SetCamera(lThisCam);
			mCollisionMesh->SetProjection(lThisProj);


			mTransform = mCollisionMesh;


			//	ここを最適化


			if (MSCullingOcculusion::IsCullingWorld(
				mRender, this, lTarget, 0.003f,
				[&,this]() {


				for (auto&lCollision : mCollisionTargets) {

					auto lTmpMesh = lCollision->mTransform;
					DXVector3 lTmpS;
					//判定メッシュをコリジョン用に変更
					lCollision->GetWorld()->GetMatrix()->GetT(lTmpS);
					lCollision->mCollisionMesh->GetWorld()->SetT(lTmpS);
					lCollision->mCollisionMesh->GetWorld()->SetRC(lCollision->GetWorld()->mRotationCenter);


					//コリジョンスケールが別で設定されていない場合は
					//メッシュのコリジョンスケールを使う
					if (lCollision->mIsCollisionScaleDefault == true) {
						lCollision->GetWorld()->GetMatrix()->GetS(lTmpS);
						lCollision->mCollisionMesh->GetWorld()->SetS(lTmpS);
					}

					lCollision->mTransform = lCollision->mCollisionMesh;

					if (lCollision->IsActive() == false) {
						lCollision->mTransform = lTmpMesh;
						continue;
					}
					//登録済みコリジョンがカリングターゲットと同じだった場合は障害物として描画しない
					if (lTarget == lCollision) {
						lCollision->mTransform = lTmpMesh;
						continue;
					}
					mRender->SetShader(mCollisionShader);
					mRender->Render(lCollision,false,true);

					lCollision->mTransform = lTmpMesh;


				}
			}
			)) {
				lTargetList.push_back(lTarget);
			}

			mTransform = lTmpThis;

		}
	}

	*/

	return std::move(lTargetList);
}


bool Enemy::CharacterSearchToClear(const std::vector<GameObjectBase*>& aLookTargets)
{

	for (auto&lTarget : aLookTargets) {
		if (dynamic_cast<CharacterBase*>(lTarget)) {
			//敵発見処理
			GetStatus<EnemyStatus>()->mTargetting = true;
			GetStatus<EnemyStatus>()->mTargetChara = lTarget;
			mAI->ClearAI();
			return true;
		}

	}
	return false;

}

bool Enemy::CharacterSearchToNextAI(const std::vector<GameObjectBase*>& aLookTargets)
{
	for (auto&lTarget : aLookTargets) {
		if (dynamic_cast<CharacterBase*>(lTarget)) {
			//敵発見処理
			GetStatus<EnemyStatus>()->mTargetting = true;
			GetStatus<EnemyStatus>()->mTargetChara = lTarget;
			mAI->NextAI();
			return true;
		}

	}
	return false;
}


void Enemy::BetaMoveToBall()
{
	//想定できる状況
	/*
		ボールとの衝突
		敵との遭遇
		味方がボールを取得
		敵がボールを取得
	*/
	
	//初期化処理
	if (GetStatus<EnemyStatus>()->mAIType != EnemyAIType::eMoveToBall) {
		printf("Task:%s\n", "MoveToBall");
		GetStatus<EnemyStatus>()->mAIType = EnemyAIType::eMoveToBall;
		//ボールのある場所をゴールとする
		DXVector3 lBallPos;
		mField->mBall->GetWorld()->GetMatrix()->GetT(lBallPos);
		auto lGoalID = mAI->GetNearNodeList(lBallPos)[0]->GetID();
		mAI->SetStartNode(mAI->GetNowNode()->GetID());
		mAI->GenerateRoot();
		mAI->CreateRoot(lGoalID);
	}

	//視界処理
	auto lLookTargets = IsCulling();

	//視界に入ったのがキャラクターの場合、
	//攻撃対象としてロックオン
	for (auto&lTarget : lLookTargets) {
		if (dynamic_cast<CharacterBase*>(lTarget)) {
			//追従対象をキャラクターに移す
			GetStatus<EnemyStatus>()->mIsTargetingBall = false;
			GetStatus<EnemyStatus>()->mTargetting = true;
			GetStatus<EnemyStatus>()->mTargetChara = lTarget;
			mAI->ClearAI();
			return;
		}
		//視界に入ったのがボールの場合
		else if (dynamic_cast<Ball*>(lTarget)) {
			GetStatus<EnemyStatus>()->mIsTargetingBall = true;
		}

	}

	//ボールを補足済みかそうでないかで処理を変更
	if (GetStatus<EnemyStatus>()->mIsTargetingBall == true) {
		//捕捉済みの場合はホーミング処理
		float lRotateY;
		lRotateY = MSHormingY(*mTransform, *mField->mBall->GetTransform(), 6.0f);
		GetWorld()->AddRC(0, lRotateY, 0);
		//移動処理
		if (IsZero(lRotateY, 0.1f)) {
			GetWorld()->AddT(DXWorld::TYPE_ROTATE, 0.1f, { 0,0,1 });
		}
	}

	else {
		//ノード単位での移動
		MoveNode();
	}


	//誰かがボールを取った場合、
	//フィールドにボールがなくなるので、
	//AIの更新
	if (mField->mBallHoldChara != nullptr) {
		mAI->ClearAI();
		return;
	}

	//自身がボールと衝突した場合
	auto lHitBall = UtlCollisionBall();
	//当たればボールを回収
	if (lHitBall) {
		mField->GetBall(this);
		GetStatus()->mBall = lHitBall;
		mField->SetBallHolder(this);
		//AIを進める
		mAI->NextAI();
	}



}

void Enemy::BetaMoveToGoal()
{
	/*
		想定できる状況
		敵との遭遇
	*/

	if (GetStatus<EnemyStatus>()->mAIType != EnemyAIType::eMoveToGoal) {
		printf("Task:%s\n", "MoveToGoal");
		GetStatus<EnemyStatus>()->mAIType = EnemyAIType::eMoveToGoal;

		//移動先をゴールにする
		mAI->SetStartNode(mAI->GetNowNode()->GetID());
		mAI->GenerateRoot();
		mAI->CreateRoot(mField->GetTeamAlly(this)->GetGoalIndex());

	}

	//移動中に敵と遭遇した場合、現在のAIを破棄する
	auto lLookTargets = IsCulling();

	for (auto&lTarget : lLookTargets) {
		if (dynamic_cast<CharacterBase*>(lTarget)) {
			//追従対象をキャラクターに移す
			GetStatus<EnemyStatus>()->mTargetting = true;
			GetStatus<EnemyStatus>()->mTargetChara = lTarget;
			mAI->ClearAI();
			return;
		}

	}


	if (!MoveNode()) {

		mAI->ClearAI();
	}

	if (mField->GetTeamAlly(this)->IsCollisionBase(this) == true) {
		mField->GoalProccess(this);
		mAI->ClearAI();
	}


}

void Enemy::BetaMoveToHide()
{
	//隠れる
	//隠れる動作中にボールが当たれば回収する
	//毎フレームレイチェックを行う
	//対象が死んだ場合、隠れる必要はなくなるので隠れる処理完了



	if (GetStatus<EnemyStatus>()->mAIType != EnemyAIType::eMoveToHide) {

		printf("Task:%s\n", "MoveToHide");

		GetStatus<EnemyStatus>()->mAIType = EnemyAIType::eMoveToHide;
		//自身の座標を一時退避
		DXVector3 lTempPosition;
		GetWorld()->GetMatrix()->GetT(lTempPosition);
		//近い順にノードを取得
		decltype(auto) lNearNodeList = mAI->GetNearNodeList(lTempPosition);
		//自身から一番近いノードをレイ発射位置とする
		mAI->SetStartNode(lNearNodeList[0]->GetID());


		//レイ発射位置を設定(ターゲット座標)
		mRayPick->SetFramePosition(*GetStatus<EnemyStatus>()->mTargetChara->GetTransform());

		DXVector3 lTmpVec3;
		Dijkstra::Node* lGoalNodePtr{};
		for (auto& lNode : lNearNodeList) {
			//ノードに移動したと仮定して処理
			GetWorld()->SetT(lNode->Position);



			//どれか一つでもヒットしていると隠れた判定
			for (auto&lCollision : mCollisionTargets) {

				auto lTmpMesh = lCollision->mTransform;
				DXVector3 lTmpS;
				//判定メッシュをコリジョン用に変更
				lCollision->GetWorld()->GetMatrix()->GetT(lTmpS);
				lCollision->mCollisionMesh->GetWorld()->SetT(lTmpS);
				lCollision->mCollisionMesh->GetWorld()->SetRC(lCollision->GetWorld()->mRotationCenter);


				//コリジョンスケールが別で設定されていない場合は
				//メッシュのコリジョンスケールを使う
				if (lCollision->mIsCollisionScaleDefault == true) {
					lCollision->GetWorld()->GetMatrix()->GetS(lTmpS);
					lCollision->mCollisionMesh->GetWorld()->SetS(lTmpS);
				}

				lCollision->mTransform = lCollision->mCollisionMesh;

				if (lCollision->IsActive() == false) {
					lCollision->mTransform = lTmpMesh;
					continue;
				}
				if (mRayPick->Collision(lTmpVec3, *mTransform, *lCollision->GetTransform())) {
					//ゴールノードの確定
					lGoalNodePtr = lNode;
				}
				lCollision->mTransform = lTmpMesh;

			}
			if (lGoalNodePtr) {
				break;
			}
		}

		//隠れることができない場合、一番遠くのノードに移動する
		lGoalNodePtr = lGoalNodePtr ? lGoalNodePtr : lNearNodeList.back();
		//位置を元に戻す
		GetWorld()->SetT(lTempPosition);
		//移動ルートの確定
		mAI->CreateRoot(lGoalNodePtr->GetID());
		//最後にターゲットを見た位置を記憶する
		GetStatus<EnemyStatus>()->mTargetChara->GetTransform()->GetWorld()->GetMatrix()->GetT(GetStatus<EnemyStatus>()->mLastLookPosiion);
		//隠れるため、必然的に視界から見失う
		GetStatus<EnemyStatus>()->mTargetting = false;


	}

	//隠れる最中にターゲットが死んだ場合、隠れる必要がなくなるので
	//その場でチャージを開始する
	if (GetStatus()->mTargetChara == nullptr) {
		mAI->NextAI();
		return;
	}

	bool lMoveNow = MoveNode();
	//移動し終わればチャージ
	if (!lMoveNow) {
		mAI->NextAI();
	}


	//毎フレームレイ判定を行い、
	//敵の範囲内から消えたら即チャージ
	for (decltype(auto)lCollision : mCollisionTargets) {
		
		auto lTmpMesh = lCollision->mTransform;
		DXVector3 lTmpS;
		//判定メッシュをコリジョン用に変更
		lCollision->GetWorld()->GetMatrix()->GetT(lTmpS);
		lCollision->mCollisionMesh->GetWorld()->SetT(lTmpS);
		lCollision->mCollisionMesh->GetWorld()->SetRC(lCollision->GetWorld()->mRotationCenter);


		//コリジョンスケールが別で設定されていない場合は
		//メッシュのコリジョンスケールを使う
		if (lCollision->mIsCollisionScaleDefault == true) {
			lCollision->GetWorld()->GetMatrix()->GetS(lTmpS);
			lCollision->mCollisionMesh->GetWorld()->SetS(lTmpS);
		}

		lCollision->mTransform = lCollision->mCollisionMesh;

		if (lCollision->IsActive() == false) {
			lCollision->mTransform = lTmpMesh;
			continue;
		}

		//Rayにはエネミー座標が入っている
		DXVector3 lTmpVec3;
		if (mRayPick->Collision(lTmpVec3, *GetStatus<EnemyStatus>()->mTargetChara->mTransform, *lCollision->GetTransform())) {
			//当たった場合、隠れているので次のAIに移行
			mAI->NextAI();
		}

		lCollision->mTransform = lTmpMesh;

	}
	



	//途中ボールに当たった場合、回収する
	auto lHitBall = UtlCollisionBall();
	if (lHitBall) {
		GetStatus()->mBall = lHitBall;
		mField->SetBallHolder(this);
		//回収後AIを進める
		mAI->NextAI();
	}

}

void Enemy::BetaChargeEnergy()
{
	//想定できる事例
	//敵との遭遇
	//ボールと接触

	//初期化
	if (GetStatus<EnemyStatus>()->mAIType != EnemyAIType::eChargeEnergy) {
		printf("Task:%s\n", "ChargeEnergy");
		GetStatus<EnemyStatus>()->mAIType = EnemyAIType::eChargeEnergy;
	}

	//移動中に敵を発見した場合、一定以上エネルギーが溜まっていればAIを破棄する
	auto lLookTargets = IsCulling();

	for (auto&lTarget: lLookTargets) {
		if (dynamic_cast<CharacterBase*>(lTarget)) {

			if (GetStatus<EnemyStatus>()->mEnergy.GetNowPer() >= 0.6f) {
				GetStatus<EnemyStatus>()->mTargetting = true;
				GetStatus<EnemyStatus>()->mTargetChara = lTarget;
				mAI->ClearAI();
				return;
			}

		}

	}


	//エネルギーチャージ
	if (GetStatus<EnemyStatus>()->mEnergy.GetNowPer() < 1.0f) {
		GetStatus<EnemyStatus>()->mEnergy.AddPer(0.01f);
	}
	//チャージが完了したらAI移行
	else {
		mAI->NextAI();
	}

	//途中ボールに当たった場合、回収する
	auto lHitBall = UtlCollisionBall();
	if (lHitBall) {
		GetStatus()->mBall = lHitBall;
		mField->SetBallHolder(this);
		//回収後AIを進める
		mAI->ClearAI();
	}

}

void Enemy::BetaInSightAttack()
{
	/*
		想定できる状況
		敵を見失う
		敵が倒れる
		ボールと接触
	*/

	if (GetStatus<EnemyStatus>()->mAIType != EnemyAIType::eInSightAttack) {
		printf("Task:%s\n", "InSightAttack");
		GetStatus<EnemyStatus>()->mAIType = EnemyAIType::eInSightAttack;
	}

	//ターゲットが倒れた場合、AIクリア
	{
		CharacterBase* lTargetChara = static_cast<CharacterBase*>(GetStatus<EnemyStatus>()->mTargetChara);
		if (lTargetChara->GetStatus()->mLive != CharaStateFlag::ALIVE) {
			//ターゲティングも初期化する
			GetStatus<EnemyStatus>()->mTargetting = false;
			GetStatus<EnemyStatus>()->mTargetChara = nullptr;
			mAI->ClearAI();
			return;
		}
	}

	//ターゲットホーミング処理
	//ターゲットの方向を向く
	float lRotateY = MSHormingY(*mTransform, *GetStatus<EnemyStatus>()->mTargetChara->GetTransform(), 3.0f);
	if (IsZero(lRotateY, 0.25f)) {
		//振り向ききれば攻撃登録
		//弾の発射
		if (mIsBulletShotWaiting == false) {
			if (ChangeStates::IsAttackDo(this, mBltManager)) {
				if (!ChangeStates::IsBulletWaiting(this, mBltManager)) {
					ChangeStates::FirstBulletProc(this, mBltManager);
					;
				}
			}
			else {
				mAI->NextAI();
			}
		}


	}
	else {
		//振り向き処理
		GetWorld()->AddRC(0, lRotateY, 0);
	}

	//敵が視界にいるか毎フレーム調べる
	auto lLookTargets = IsCulling();
	bool lIsLooking = false;
	if (GetStatus<EnemyStatus>()->mTargetChara != nullptr) {
		for (auto&lTarget : lLookTargets) {
			//現在捉えている敵だった場合視界内にいる
			if (GetStatus<EnemyStatus>()->mTargetChara == lTarget) {
				lIsLooking = true;
				break;
			}
		}
	}

	if (lIsLooking == false) {
		GetStatus<EnemyStatus>()->mTargetting = false;
		GetStatus<EnemyStatus>()->mLastLookPosiion = GetStatus<EnemyStatus>()->mTargetChara->GetWorld()->mPosition;
		//GetStatus<EnemyStatus>()->mTargetChara = nullptr;
		mAI->ClearAI();
	}

	//途中ボールに当たった場合、回収する
	auto lHitBall = UtlCollisionBall();
	if (lHitBall) {
		GetStatus()->mBall = lHitBall;
		mField->SetBallHolder(this);
		//回収後AIを進める
		mAI->NextAI();
	}

}

void Enemy::BetaMoveToBallHoldAlly()
{
	/*
		想定できる状況

		ボールを持っている味方が倒される
		敵を発見
		ボールと接触
	*/

	//ボールを持った味方がやられた場合(落とした場合、
	//AIクリア
	if (mField->mBallHoldChara == nullptr) {
		mAI->ClearAI();
		return;
	}


	if (GetStatus<EnemyStatus>()->mAIType != EnemyAIType::eMoveToBallHoldAlly) {
		printf("Task:%s\n", "MoveToBallHoldAlly");
		GetStatus<EnemyStatus>()->mAIType = EnemyAIType::eMoveToBallHoldAlly;

		//移動先をボール所持者にする

		mAI->SetStartNode(mAI->GetNowNode()->GetID());
		mAI->GenerateRoot();

		DXVector3& lPosition = mField->mBallHoldChara->GetWorld()->mPosition;
		auto lNearNode = mAI->GetNearNodeList(lPosition)[0];

		mAI->CreateRoot(lNearNode->GetID());

		GetStatus<EnemyStatus>()->mAllyComplianceCount = 0;
	}




	//60フレーム毎にゴール更新
	if (!MoveNode()||GetStatus<EnemyStatus>()->mAllyComplianceCount >= 60) {
		mAI->SetStartNode(mAI->GetNowNode()->GetID());
		mAI->GenerateRoot();

		DXVector3& lPosition = mField->mBallHoldChara->GetWorld()->mPosition;
		auto lNearNode = mAI->GetNearNodeList(lPosition)[0];

		mAI->CreateRoot(lNearNode->GetID());

		GetStatus<EnemyStatus>()->mAllyComplianceCount = 0;
	}

	//移動中に敵を発見した場合、現在のAIを破棄する
	auto lLookTargets = IsCulling();
	CharacterSearchToClear(lLookTargets);


	//途中ボールに当たった場合、回収する
	auto lHitBall = UtlCollisionBall();
	if (lHitBall) {
		GetStatus()->mBall = lHitBall;
		mField->SetBallHolder(this);
		//回収後AIを進める
		mAI->ClearAI();
	}

	++GetStatus<EnemyStatus>()->mAllyComplianceCount;


}

void Enemy::BetaSearchEnemyShort()
{
	//想定できる状況
	/*
		味方と離れる
		ボールがフィールドに落ちる
		敵との遭遇
	*/

	if (GetStatus<EnemyStatus>()->mAIType != EnemyAIType::eSerachEnemyAll) {
		printf("Task:%s\n", "SearchEnemyShort");
		GetStatus<EnemyStatus>()->mAIType = EnemyAIType::eSerachEnemyAll;
		GetStatus<EnemyStatus>()->mAllyComplianceCount = 0;

		//移動先を適当な場所にする
		mAI->ClearRoot();
	}

	if (mAI->IsRootEmpty() == true) {
		//ルートを生成する
		mAI->CreateNextRoot(false);
	}
	//移動に失敗すればAI更新
	if (!MoveNode()) {
		mAI->ClearAI();
	}

	//敵との遭遇
	auto lLookTargets = IsCulling();

	CharacterSearchToNextAI(lLookTargets);


	//ボール所持者がいなくなった場合
	if (mField->mBallHoldChara == nullptr) {
		mAI->ClearAI();
	}


//	//60フレーム毎に(現在未実装)
	//毎フレーム
	//味方との距離計算
	//一定距離離れたらAIクリア(更新)
	{
		DXVector3 lBallHolderPos;
		DXVector3 lThisPos;
		float lDistance;
		lBallHolderPos = mField->mBallHoldChara->GetWorld()->mPosition;
		lThisPos = GetWorld()->mPosition;
		lDistance = (lBallHolderPos - lThisPos).GetDistance();
		if (lDistance < 10.0f) {
			mAI->ClearAI();
		}
	}

	if (GetDistance(mField->mBallHoldChara) > 10.0f) {
		mAI->ClearAI();
	}




}

void Enemy::BetaSearchEnemyAll()
{
	/*
		想定できる状況
		//誰かがボールを取得
		敵を捉える
	*/

	if (GetStatus<EnemyStatus>()->mAIType != EnemyAIType::eSerachEnemyAll) {
		printf("Task:%s\n", "SearchEnemyAll");
		GetStatus<EnemyStatus>()->mAIType = EnemyAIType::eSerachEnemyAll;
	}

	//ランダム移動処理
	{
		if (mAI->IsRootEmpty() == true) {
			//ルートを生成する
			mAI->CreateNextRoot(false);
		}
		//移動に失敗すればAI更新
		if (!MoveNode()) {
			mAI->ClearAI();
		}

	}

	//auto lLookTarget = IsCulling();
	//if (lLookTarget) {
	//	//ボール以外の場合
	//	if (!dynamic_cast<Ball*>(lLookTarget)) {
	//		//発見したらシーケンスをすすめる
	//		mAI->NextAI();
	//		//敵を視認中
	//		GetStatus<EnemyStatus>()->mTargetting = true;
	//		//捉えたターゲットを記憶
	//		GetStatus<EnemyStatus>()->mTargetChara = lLookTarget;
	//		return;
	//	}
	//}


}

void Enemy::BetaMoveToLookingTarget()
{
	/*
		想定できる状況
		味方がボールを取る
		敵がボールを捕る
		敵との遭遇
		ボール所持者が死亡
	*/

	if (GetStatus<EnemyStatus>()->mAIType != EnemyAIType::eSerachEnemyAll) {
		printf("Task:%s\n", "MoveToLookingTarget");
		GetStatus<EnemyStatus>()->mAIType = EnemyAIType::eSerachEnemyAll;
	}

	//最後に敵を見た位置をゴールとする
	if (mAI->IsRootEmpty() == true) {
		//最後に敵を見た位置から一番近いノードをゴールとする
		auto lGoalID=mAI->GetNearNodeList(GetStatus<EnemyStatus>()->mLastLookPosiion)[0]->GetID();
	
		mAI->SetStartNode(mAI->GetNowNode()->GetID());
		mAI->GenerateRoot();
		mAI->CreateRoot(lGoalID);
	
	}
	
	//戻る最中に視界に入れば発見。そのターゲットをロックオンする
	auto lLookTargets = IsCulling();

	if (CharacterSearchToNextAI(lLookTargets)) {
		return;
	}

	
	
	if (!MoveNode()) {
		//移動完了してしまった場合、発見できなかったので、ロックオンフラグ(ターゲットをnullクリア)を外して
		//次のAIへ
		mAI->ClearAI();
			
		GetStatus<EnemyStatus>()->mTargetChara = nullptr;
	}
}

void Enemy::BetaMoveToBallTarget()
{
	//想定可能な状況
	/*
		敵との遭遇
		ボールが落ちる
	*/
	if (GetStatus<EnemyStatus>()->mAIType != EnemyAIType::eMoveToBallTarget) {
		printf("Task:%s\n", "MoveToBallTarget");
		GetStatus<EnemyStatus>()->mAIType = EnemyAIType::eMoveToBallTarget;
		DXVector3 lBallPos;
		mField->mBallHoldChara->GetWorld()->GetMatrix()->GetT(lBallPos);
		auto lGoalID = mAI->GetNearNodeList(lBallPos)[0]->GetID();
		mAI->SetStartNode(mAI->GetNowNode()->GetID());
		mAI->GenerateRoot();
		mAI->CreateRoot(lGoalID);
	}

	auto lLookTargets = IsCulling();
	//キャラクターであればロックオン
	//キャラクターであればロックオン
	if (CharacterSearchToClear(lLookTargets)) {
		return;
	}

	
	bool lMoveEnd = MoveNode();
	//ボールを落とした(フィールドに出現した場合)AIクリア
	if (mField->mBallHoldChara == nullptr) {
		mAI->ClearAI();
		return;
	}

	//移動し終わった場合(敵を発見できなかった場合)
	if (!lMoveEnd) {
		//再度敵を探す
		DXVector3 lBallPos;
		mField->mBallHoldChara->GetWorld()->GetMatrix()->GetT(lBallPos);
		auto lGoalID = mAI->GetNearNodeList(lBallPos)[0]->GetID();
		mAI->SetStartNode(mAI->GetNowNode()->GetID());
		mAI->GenerateRoot();
		mAI->CreateRoot(lGoalID);
	}
}

void Enemy::BetaSearchForEnemyArea()
{
	if (GetStatus<EnemyStatus>()->mAIType != EnemyAIType::eSearchForEnemyArea) {
		printf("Task:%s\n", "SearchForEnemyArea");
		GetStatus<EnemyStatus>()->mAIType = EnemyAIType::eSearchForEnemyArea;
	}

	assert(0);
}

void Enemy::BetaSearchForAllyArea()
{
	/*
		想定できる状況

		敵との遭遇
	*/
	
	if (GetStatus<EnemyStatus>()->mAIType != EnemyAIType::eSearchForAllyArea) {
		printf("Task:%s\n", "SearchForAllyArea");
		GetStatus<EnemyStatus>()->mAIType = EnemyAIType::eSearchForAllyArea;

		GetStatus<EnemyStatus>()->mAllyComplianceCount = 0;

		//相手チームを取得
		auto lEnemyTeam = mField->GetTeamEnemy(this);
		

		//味方エリアのどこかをゴールとする
		MyNode*lGoalNode;
		//味方チームのチームタイプを取得する
		eTeamType lEnemyTeamType = lEnemyTeam->GetTeamType();;
		do {

			lGoalNode = mAI->GetNodeRandom();
			//異なっていれば移動可能なノード
		} while (lGoalNode->mTeamType != lEnemyTeamType);
		//auto lGoalID = mAI->GetNearNodeList(*mField->mBallHoldChara->mTransform->GetWorld()->mPosition)[0]->GetID();

		mAI->SetStartNode(mAI->GetNowNode()->GetID());
		mAI->GenerateRoot();
		mAI->CreateRoot(lGoalNode->GetID());

	}

	//ボールを持った味方がやられた場合(落とした場合、
	//AIクリア
	if (mField->mBallHoldChara == nullptr) {
		mAI->ClearAI();
	}

	//移動できなくなったらルート再構築する
	if (!MoveNode()) {
		GetStatus<EnemyStatus>()->mAIType = EnemyAIType::eUnKnown;
		return;
	}
	//移動中に敵を発見した場合、現在のAIを破棄する
	auto lLookTargets = IsCulling();

	CharacterSearchToClear(lLookTargets);
}

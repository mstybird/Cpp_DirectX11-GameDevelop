#include "StatusField.h"
#include"DXWorld.h"
#include"DXMatrix.h"
#include"EnemyAI.h"
#include"CharacterBase.h"
#include<SpawnMapImporter.hpp>
#include<AIMapImport.hpp>
#include<cassert>
#include<string>
StatusField::~StatusField()
{
	for (decltype(auto)lNode : mFieldNodes) {
		delete lNode;
	}
	for (decltype(auto)lNode : mSpawnBallNodes) {
		delete lNode;
	}
}
void StatusField::Initialize()
{
	mTeamBlack.mBase.Initialize();
	mTeamWhite.mBase.Initialize();
}
void StatusField::InitRenderAndShader(MS3DRender & aRender, MSBase3DShader & aShader)
{
	mTeamBlack.mBase.SetShader(&aShader);
	mTeamWhite.mBase.SetShader(&aShader);
	mTeamBlack.mBase.SetRenderer(&aRender);
	mTeamWhite.mBase.SetRenderer(&aRender);
}
void StatusField::CreateFieldNodes()
{
	Comfort::AIMapImporter im;
	im.Import("Resource/NodeMap/fieldNodes.anm");
	auto v = im.GetList();
	using NodeControl::AddNodeSafe;
	//�m�[�h�̒ǉ�
	DXVector3 lNodePosition{};
	for (decltype(auto)lData : *v) {
		lNodePosition.Set(-lData.mX, lData.mY, lData.mZ);
		AddNodeSafe(
			mFieldNodes,
			new MyNode{
				lData.mID, 
				std::string("obj") +std::to_string(lData.mID),lNodePosition,lData.mTeamIDType
			}
		);
	}


	//�m�[�h�ԌŒ�R�X�g�͋����Ƃ���						
	auto LinkNodeSafeCustom = [this](const int32_t aFromID, const int32_t aToID, const bool aIsBothFlag) {
		using NodeControl::LinkNodeSafe;
		//�m�[�h�Ԃ̋����𓾂�					
		auto NodeLength = [this](const int32_t aFromID, const int32_t aToID) {
			auto& lFromPos = static_cast<MyNode*>(mFieldNodes[aFromID])->Position;
			auto& lToPos = static_cast<MyNode*>(mFieldNodes[aToID])->Position;
			return (lFromPos - lToPos).GetDistance();
		};
		LinkNodeSafe(mFieldNodes, { aFromID,aToID }, NodeLength(aFromID, aToID), aIsBothFlag);
	};

	//�����N�m�[�h�̒ǉ�
	for (decltype(auto)lNode : *v) {
		printf("%d\n", lNode.mID);
		for (decltype(auto)lLink : lNode.mLinkList) {
			LinkNodeSafeCustom(lNode.mID, lLink, true);
		}
	}


}
void StatusField::CreateSpawnCharaNodes()
{
	//���`�[���̃X�|�[���f�[�^
	{
		Comfort::SpawnMapImporter lImport;
		lImport.Import("Resource/NodeMap/spawnTeamBlack.anm");


		using NodeControl::AddNodeSafe;

		for (const auto& lNode : *lImport.GetList()) {
			AddNodeSafe(
				mTeamBlack.mSpawnCharaNodes,
				new MyNode{
				lNode.mID,
				std::string("obj") + std::to_string(lNode.mID),
				{ -lNode.mX,lNode.mY,lNode.mZ },
				0
			}
			);
		}
	}


	//���`�[���̃X�|�[���f�[�^
	{
		Comfort::SpawnMapImporter lImport;
		lImport.Import("Resource/NodeMap/spawnTeamWhite.anm");


		using NodeControl::AddNodeSafe;

		for (const auto& lNode : *lImport.GetList()) {
			AddNodeSafe(
				mTeamWhite.mSpawnCharaNodes,
				new MyNode{
				lNode.mID,
				std::string("obj") + std::to_string(lNode.mID),
				{ -lNode.mX,lNode.mY,lNode.mZ },
				0
			}
			);
		}
	}

	//AddNodeSafe(mSpawnCharaNodes, new MyNode{ 19,"obj19",{ 11.00,0.00,7.50 } });
	//AddNodeSafe(mSpawnCharaNodes, new MyNode{ 20,"obj20",{ -16.25,0.00,5.25 } });
}
void StatusField::CreateSpawnBallNodes()
{
	Comfort::SpawnMapImporter lImport;
	lImport.Import("Resource/NodeMap/spawnBall.anm");


	using NodeControl::AddNodeSafe;

	for (const auto& lNode : *lImport.GetList()) {
		AddNodeSafe(
			mSpawnBallNodes,
			new MyNode{
				lNode.mID,
				std::string("obj") + std::to_string(lNode.mID),
				{-lNode.mX,lNode.mY,lNode.mZ},
				0
			}
		);
	}

	//AddNodeSafe(mSpawnBallNodes, new MyNode{ 0,"obj0",{ -16.40,0.00,- 47.00 }  ,0 });
	//AddNodeSafe(mSpawnBallNodes, new MyNode{ 5,"obj5",{ 6.25,0.00,2.50 } });
	//AddNodeSafe(mSpawnBallNodes, new MyNode{ 13,"obj13",{ -4.00,0.00,8.50 } });
}
void StatusField::InitGoalIndex(const int aWhiteGoalIndex, const int aBlackGoalIndex)
{
	mTeamWhite.mGoalIndex = aWhiteGoalIndex;
	mTeamBlack.mGoalIndex = aBlackGoalIndex;
}
std::vector<Dijkstra::Node*> StatusField::GetFieldNodesClone()
{
	return mFieldNodes;
}
void StatusField::Respawn(CharacterBase * aSpawnChara)
{
	printf("%s is Respawn\n", typeid(aSpawnChara).name());

	//�`�[���̃X�|�[�����X�g�擾
	const auto& lSpawnNodeList = GetTeamAlly(aSpawnChara)->mSpawnCharaNodes;

	auto lCount = rand() % lSpawnNodeList.size();

	//�����_���ȃm�[�h������W�����o��
	auto& lPosition = static_cast<MyNode*>(lSpawnNodeList[lCount])->Position;

	aSpawnChara->GetWorld()->SetT(lPosition);
	aSpawnChara->SetActive(true);
	aSpawnChara->InitStatus(aSpawnChara->GetDefaultStatus());
	
}
void StatusField::RegisterTeamMember(CharacterBase * aRegistMember, eTeamType aType)
{
	switch (aType)
	{
	case eTeamType::White:
		mTeamWhite.mMembers.push_back(aRegistMember);
		break;
	case eTeamType::Black:
		mTeamBlack.mMembers.push_back(aRegistMember);
		break;
	default:
		break;
	}
}
StaticObject * StatusField::GetTeamBase(eTeamType aTeamType)
{
	StaticObject* lTeamBase{ nullptr };
	switch (aTeamType)
	{
	case eTeamType::White:
		lTeamBase = &mTeamWhite.mBase;
		break;
	case eTeamType::Black:
		lTeamBase = &mTeamBlack.mBase;
		break;
	default:
		break;
	}
	return lTeamBase;
}
StatusTeam * StatusField::GetTeamAlly(CharacterBase * aMember)
{
	if (aMember == nullptr)return nullptr;


	for (decltype(auto)lMember : mTeamBlack.mMembers) {
		if (lMember == aMember) {
			return &mTeamBlack;
		}
	}
	for (decltype(auto)lMember : mTeamWhite.mMembers) {
		if (lMember == aMember) {
			return &mTeamWhite;
		}
	}
	return nullptr;
}
StatusTeam * StatusField::GetTeamEnemy(CharacterBase * aMember)
{
	if (aMember == nullptr)return nullptr;

	for (decltype(auto)lMember : mTeamBlack.mMembers) {
		if (lMember == aMember) {
			return &mTeamWhite;
		}
	}
	for (decltype(auto)lMember : mTeamWhite.mMembers) {
		if (lMember == aMember) {
			return &mTeamBlack;
		}
	}
	return nullptr;
}
eTeamType StatusField::GetTeamType(CharacterBase * aChara)
{
	for (decltype(auto)lMember : mTeamBlack.mMembers) {
		if (lMember == aChara) {
			return eTeamType::Black;
		}
	}
	for (decltype(auto)lMember : mTeamWhite.mMembers) {
		if (lMember == aChara) {
			return eTeamType::White;
		}
	}
}
DXVector3 StatusField::GetNodePosition(const int aIndex)
{
	DXVector3 lResult{};
	for (auto&lNode : mFieldNodes) {
		if (lNode->GetID() == aIndex) {
			lResult = static_cast<MyNode*>(lNode)->Position;
			break;
		}
	}
	return lResult;
}
StatusField::StatusField():
	mBallHoldChara{nullptr},
	mBallIsField{false},
	mBall{nullptr},
	mTeamBlack(eTeamType::Black),
	mTeamWhite(eTeamType::White)
{
}

void StatusField::SetBallHolder(CharacterBase * pBallHolder)
{
	mBallHoldChara = pBallHolder;
	mBallIsField = false;
	mBall->SetActive(false);
}

void StatusField::RespawnBall(DXVector3*pPosition)
{
	mBall->SetActive(true);
	mBallIsField = true;
	mBallHoldChara = nullptr;

	if (pPosition != nullptr) {
		mBall->GetWorld()->SetT(*pPosition);
	}
	else {
		//�ʒu���s��̏ꍇ�A
		//�{�[�����X�|�[���\�ȏꏊ�����_���ɃX�|�[��������
		auto lCount = rand()%mSpawnBallNodes.size();
		auto& lPosition =static_cast<MyNode*>(mSpawnBallNodes[lCount])->Position;
		mBall->GetWorld()->SetT(lPosition);
	}
}

void NodeControl::AddNodeSafe(std::vector<Dijkstra::Node*>& aNodeList, Dijkstra::Node * aAddNode)
{
	//NULL�`�F�b�N
	assert(aAddNode != nullptr);
	
	//ID�d���`�F�b�N
	//�{�������Ă͂Ȃ�Ȃ��̂ŁA�����I��������
	for (const auto&lNode : aNodeList) {
		assert(lNode->GetID() != aAddNode->GetID());
	}
	aNodeList.push_back(aAddNode);
}

void NodeControl::LinkNodeSafe(std::vector<Dijkstra::Node*>& aNodeList, std::pair<int, int> aFromTo, float aCost, bool aBothFlag)
{
	assert(IsBetweenIndex<Dijkstra::Node*>(aNodeList,aFromTo.first));
	assert(IsBetweenIndex<Dijkstra::Node*>(aNodeList,aFromTo.second));
	assert(aCost > 0);
	aNodeList[aFromTo.first]->addNode(aNodeList[aFromTo.second], aCost);

	//�o���������N�̏ꍇ�ATo����From�ɑ΂��Ă������N����
	if (aBothFlag == true) {
		aNodeList[aFromTo.second]->addNode(aNodeList[aFromTo.first], aCost);
	}
}
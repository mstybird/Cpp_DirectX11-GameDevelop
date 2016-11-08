#pragma once
#include<vector>
#include"GameObjectBase.h"
#include"AI/EnemyAI.h"
#include"StatusTeam.h"
class CharacterBase;
namespace NodeControl {
	void AddNodeSafe(std::vector<Dijkstra::Node*>&aNodeList, Dijkstra::Node*aAddNode);
	void LinkNodeSafe(
		std::vector<Dijkstra::Node*>&aNodeList,
		std::pair<int, int>aFromTo,
		int aCost,
		bool aBothFlag
	);
	template<typename T>
	bool IsBetweenIndex(std::vector<T>aVector,unsigned int aIndex) {
		auto lMin{ 0 };
		auto lMax{ aVector.size() };
		return (lMin <= aIndex) && (aIndex <= lMax);
	}
}

//フィールド情報
struct StatusField {

	virtual ~StatusField();

	//誰がボールを所持しているか
	CharacterBase* mBallHoldChara;
	//フィールドにボールがあるかどうか
	bool mBallIsField;
	//ボール情報
	GameObjectBase* mBall;

	//フィールドのAI用ノードリスト
	std::vector<Dijkstra::Node*>mFieldNodes;
	//フィールドのスポーン用ノードリスト
	std::vector<Dijkstra::Node*>mSpawnCharaNodes;
	//ボールオブジェクトスポーン用ノードリスト
	std::vector<Dijkstra::Node*>mSpawnBallNodes;

	void CreateFieldNodes();
	void CreateSpawnCharaNodes();
	void CreateSpawnBallNodes();
	void InitGoalIndex();
	std::vector<Dijkstra::Node*>GetFieldNodesClone();
	//キャラクターをリスポーンさせる
	void Respawn(CharacterBase* aSpawnChara);

	//チームにメンバーを登録する
	void RegisterTeamMember(CharacterBase*aRegistMember,eTeamType aType);

	//所属しているチームを取得
	StatusTeam* GetTeamAlly(CharacterBase*aMember);
	//相手のチームを取得
	StatusTeam* GetTeamEnemy(CharacterBase*aMember);



	StatusField();
	void SetBallHolder(CharacterBase*pBallHolder);
	void RespawnBall(DXVector3*pPosition = nullptr);

	StatusTeam mTeamWhite;	//白チーム
	StatusTeam mTeamBlack;	//黒チーム

};
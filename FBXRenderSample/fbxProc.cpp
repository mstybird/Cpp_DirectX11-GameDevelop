#include"DX11FbxLoader.h"
#include<Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#pragma warning(disable : 4800)

DX11FbxLoader::~DX11FbxLoader()
{
	FbxDestroy();
}

//初期化処理
void DX11FbxLoader::FbxInit(std::string vfileName, bool animationLoad) {
	fileName = vfileName;
	PoseIndex = -1;
	
	Cache_Start = FBXSDK_TIME_INFINITE;
	Cache_Stop = FBXSDK_TIME_MINUS_INFINITE;

	//SDKの初期化
	SdkManager = FbxManager::Create();

	FBXSDK_printf("Autodesk FBX SDK version %s\n", SdkManager->GetVersion());

	//IO設定
	auto ios = FbxIOSettings::Create(SdkManager, IOSROOT);
	SdkManager->SetIOSettings(ios);

	//プラグインの読み込み
	FbxString lPath = FbxGetApplicationDirectory();
	SdkManager->LoadPluginsDirectory(lPath.Buffer());
	FBXSDK_printf("Loaded Plugins Directory is \" %s \"\n", lPath.Buffer());

	//シーンの作成
	Scene = FbxScene::Create(SdkManager, "MyScene");

	if (!SdkManager) {
		FBXSDK_printf("FBX Initialize Error\n");
	}

	//ファイルのインポート
	auto FileFormat = -1;
	Importer = FbxImporter::Create(SdkManager, "");
	if (!SdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(fileName.c_str(), FileFormat)) {
		//バイナリファイルだった場合、バイナリデータを読み込むための設定
		FileFormat = SdkManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");;
	}


	//インポータの初期化＆ファイルの設定
	if (Importer->Initialize(fileName.c_str(), FileFormat) == true) {
		WindowMessage = "Importing file ";
		WindowMessage += fileName.c_str();
		WindowMessage += "\nPlease wait!";

	}
	else
	{
		WindowMessage = "Unable to open file ";
		WindowMessage += fileName.c_str();
		WindowMessage += "\nError reported: ";
		WindowMessage += Importer->GetStatus().GetErrorString();
		WindowMessage += "\nEsc to exit";
	}
	FBXSDK_printf("%s\n",WindowMessage.Buffer());

	//ファイルのインポート
	FbxLoadFromFile();
	//アニメーションの初期化
	SceneAnim = Scene;
	LoadAnimationData();
}

void DX11FbxLoader::FbxLoadAnimationFromFile(std::string vfileName)
{
	//シーンの作成(アニメーションオンリー)
	SceneAnim = FbxScene::Create(SdkManager, "Animation");

	if (!SdkManager) {
		FBXSDK_printf("FBX Initialize Error\n");
	}

	//ファイルのインポート
	auto FileFormat = -1;
	Importer = FbxImporter::Create(SdkManager, "");
	if (!SdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(fileName.c_str(), FileFormat)) {
		//バイナリファイルだった場合、バイナリデータを読み込むための設定
		FileFormat = SdkManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");;
	}


	//インポータの初期化＆ファイルの設定
	if (Importer->Initialize(fileName.c_str(), FileFormat) == true) {
		WindowMessage = "Importing file ";
		WindowMessage += fileName.c_str();
		WindowMessage += "\nPlease wait!";

	}
	else
	{
		WindowMessage = "Unable to open file ";
		WindowMessage += fileName.c_str();
		WindowMessage += "\nError reported: ";
		WindowMessage += Importer->GetStatus().GetErrorString();
		WindowMessage += "\nEsc to exit";
	}
	FBXSDK_printf("%s\n", WindowMessage.Buffer());
	SceneAnim->FillAnimStackNameArray(AnimStackNameArray);

	LoadAnimationData();
}



//解放処理
void DX11FbxLoader::FbxDestroy() {
	//ジオメトリの解放
	ReleaseGeometryData();
	//アニメーションリスト文字列の解放
	FbxArrayDelete(AnimStackNameArray);
	//シーンを読み込んでいた場合シーンを開放する
	if (Scene) {
		UnLoadCacheRecursive(Scene);
		Scene = nullptr;
	}
	if (SdkManager)SdkManager->Destroy();
	SdkManager = nullptr;
}

void DX11FbxLoader::FbxLoadFromFile()
{

	if (Importer->Import(Scene) == true) {
		//座標系の変換
		FbxAxisSystem SceneAxisSystem = Scene->GetGlobalSettings().GetAxisSystem();
		FbxAxisSystem OurAxisSystem(
			FbxAxisSystem::eYAxis,
			FbxAxisSystem::eParityOdd,
			FbxAxisSystem::eRightHanded
		);
		//読み込んだファイルと座標系が異なる場合はシーンの座標系を変換
		if (SceneAxisSystem != OurAxisSystem) {
			OurAxisSystem.ConvertScene(Scene);
		}

		Scene->FillAnimStackNameArray(AnimStackNameArray);
		
		//ポリゴンを全て三角形に変換
		FbxGeometryConverter geometryConverter(SdkManager);
		//trueにすると直接書き換える
		geometryConverter.Triangulate(Scene, true);

		//最初のフレームのシーンを取得する
		this->LoadCacheRecursive(Scene, CurrentAnimLayer, fileName);

		//デフォーマーから
		this->PreparePointCacheData(Scene, Cache_Start, Cache_Stop);
		
		//ポーズのリストを作成する
		this->FillPoseArray(Scene, PoseArray);

		Scene->GetGlobalSettings().SetTimeMode(FbxTime::EMode::eFrames60);
		FrameTime.SetTime(0, 0, 0, 1, 0, Scene->GetGlobalSettings().GetTimeMode());

		//インポータはこれ以上不要なので解放する
		Importer->Destroy();
		Importer = nullptr;
	}
}

//ノードの属性とマテリアル情報を読み取り、シーンに必要なテクスチャファイル名を取得する
void DX11FbxLoader::LoadCacheRecursive(FbxScene * pScene, FbxAnimLayer * pAnimLayer, std::string& pFbxFileName)
{
	//テクスチャ数を取得
	const auto lTextureCount = Scene->GetTextureCount();
	FbxString filePath;
	
	//this->TextureFileName.clear();
	std::vector<FbxTexture::EMappingType> m;

	for (auto lTextureIndex = 0; lTextureIndex < lTextureCount; lTextureIndex++) {
		//テクスチャ取得
		auto lTexture = Scene->GetTexture(lTextureIndex);
		
		auto lFileTexture = FbxCast<FbxFileTexture>(lTexture);

		m.push_back(lFileTexture->GetMappingType());

		if (lFileTexture && !lFileTexture->GetUserDataPtr()) {
			//FBXファイルパスを取得
			const FbxString lFileName = lFileTexture->GetFileName();
			//printf("%s\n", lFileName.Buffer());
			bool lStatus = (bool)PathFileExists(lFileName.Buffer());
			if (lStatus)filePath = lFileName;

			const FbxString lAbsFbxFileName = FbxPathUtils::Resolve(pFbxFileName.c_str());
			const FbxString lAbsFolderName = FbxPathUtils::GetFolderName(lAbsFbxFileName);

			//読み込めなかった場合
			if (!lStatus) {
				//相対パスで読み込み
				const FbxString lResolvedFileName = FbxPathUtils::Bind(lAbsFolderName, lFileTexture->GetRelativeFileName());
				//printf("%s\n", lResolvedFileName.Buffer());
				lStatus = (bool)PathFileExists(lResolvedFileName.Buffer());
				if (lStatus)filePath = lResolvedFileName;
			}


			//読み込めなかった場合
			if (!lStatus) {
				//カレントディレクトリにあれば読み込む
				// Load texture from file name only (relative to FBX file)
				const FbxString lTextureFileName = FbxPathUtils::GetFileName(lFileName);
				const FbxString lResolvedFileName = FbxPathUtils::Bind(lAbsFolderName, lTextureFileName);
				lStatus = (bool)PathFileExistsA(lResolvedFileName.Buffer());
				//printf("%s\n", lResolvedFileName.Buffer());
				if (lStatus)filePath = lResolvedFileName;

			}

			

			//どこのテクスチャなのか記憶する
			const FbxString uvName = lFileTexture->UVSet.Get();
			//一つのUVに複数のテクスチャが存在する可能性がある
			//this->TextureFileName[uvName.Buffer()].push_back(filePath.Buffer());

			//読み込めたらファイルテクスチャのアドレスを渡す
			if (lStatus) {
				FbxString*lTextureName = new FbxString(filePath);
				lFileTexture->SetUserDataPtr(lTextureName);
			}
			
		}
	}

	//子ノードを探索
	LoadCacheRecursive(Scene->GetRootNode(), pAnimLayer);

}
//ここからモデルのメッシュ情報とか。マテリアルを取得する
void DX11FbxLoader::LoadCacheRecursive(FbxNode * pNode, FbxAnimLayer * pAnimLayer)
{
	//一つのノードに存在するマテリアルの数を取得
	const auto lMaterialCount = pNode->GetMaterialCount();
	//マテリアル数分ループ
	for (auto lMaterialIndex = 0; lMaterialIndex < lMaterialCount; lMaterialIndex++) {
		//マテリアルを取得
		auto lMaterial = pNode->GetMaterial(lMaterialIndex);
		
		if (lMaterial && !lMaterial->GetUserDataPtr()) {
			//マテリアルキャッシュ作成
			FbxAutoPtr<FbxMaterialCache> lMaterialCache(new FbxMaterialCache);
			//マテリアルキャッシュの初期化
			if (lMaterialCache->Initialize(lMaterial)) {
				//キャッシュのポインタをマテリアルに渡す
				lMaterial->SetUserDataPtr(lMaterialCache.Release());
			}
		}
	}

	//ノードの属性を取得
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
	if (lNodeAttribute) {
		//ノードがメッシュを表しているかどうか
		if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh) {
			//ノードを登録
			nodemeshes.push_back(pNode);
			//メッシュを取得
			FbxMesh*lMesh = pNode->GetMesh();
			if (lMesh && !lMesh->GetUserDataPtr()) {
				FbxAutoPtr<FbxVBO>lMeshCache(new FbxVBO);
				if (lMeshCache->Initialize(lMesh)) {
					//VBOクラスを登録
					lMesh->SetUserDataPtr(lMeshCache.Release());
				}
			}
		}
	}


	//子ノードの数を数える
	const auto lChildCount = pNode->GetChildCount(); {
		for (auto lChildIndex = 0; lChildIndex < lChildCount; lChildIndex++) {
			LoadCacheRecursive(pNode->GetChild(lChildIndex), pAnimLayer);
		}
	}

}

void DX11FbxLoader::UnLoadCacheRecursive(FbxScene * pScene)
{
	const int lTextureCount = pScene->GetTextureCount();
	for (int lTextureIndex = 0; lTextureIndex < lTextureCount; lTextureIndex++) {
		FbxTexture * lTexture = pScene->GetTexture(lTextureIndex);
		FbxFileTexture * lFileTexture = FbxCast<FbxFileTexture>(lTexture);
		if (lFileTexture && lFileTexture->GetUserDataPtr())
		{
			FbxString * lTextureName = static_cast<FbxString *>(lFileTexture->GetUserDataPtr());
			lFileTexture->SetUserDataPtr(NULL);
			//テクスチャイメージの解放
			//glDeleteTextures(1, lTextureName);
			delete lTextureName;
		}
	}
	UnLoadCacheRecursive(pScene->GetRootNode());
}

/*
	ノードの全捜査をなくすために、
	eNode属性を持つノードのみの配列を作成する。
	以降はそのノードにのみアクセスする
*/


void DX11FbxLoader::UnLoadCacheRecursive(FbxNode * pNode)
{
	//マテリアルの解放
	const int lMaterialCount = pNode->GetMaterialCount();
	for (int lMaterialIndex = 0; lMaterialIndex < lMaterialCount; ++lMaterialIndex) {
		FbxSurfaceMaterial*lMaterial = pNode->GetMaterial(lMaterialIndex);
		if (lMaterial&&lMaterial->GetUserDataPtr()) {
			FbxMaterialCache*lMaterialCache = static_cast<FbxMaterialCache*>(lMaterial->GetUserDataPtr());
			lMaterial->SetUserDataPtr(nullptr);
			delete lMaterialCache;
		}
	}
	//メッシュを解放する
	FbxNodeAttribute*lNodeAttribute = pNode->GetNodeAttribute();
	if (lNodeAttribute) {
		if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh) {

			FbxMesh*lMesh = pNode->GetMesh();
			if (lMesh&&lMesh->GetUserDataPtr()) {
				FbxVBO*lMeshCache = static_cast<FbxVBO*>(lMesh->GetUserDataPtr());
				lMesh->SetUserDataPtr(nullptr);
				delete lMeshCache;
			}

		}
	}

	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex) {
		UnLoadCacheRecursive(pNode->GetChild(lChildIndex));
	}

}

void DX11FbxLoader::PreparePointCacheData(FbxScene * pScene, FbxTime & pCache_Start, FbxTime & pCache_Stop)
{
	//ノード数を取得
	const auto lNodeCount = pScene->GetSrcObjectCount<FbxNode>();
	FbxStatus lStatus;
	for(auto lIndex=0;lIndex<lNodeCount; lIndex++){
		FbxNode* lNode = pScene->GetSrcObject<FbxNode>(lIndex);
		if (lNode->GetGeometry()) {
			int i;
			int lVertexCacheDeformerCount = lNode->GetGeometry()->GetDeformerCount(FbxDeformer::eVertexCache);
			lVertexCacheDeformerCount = lVertexCacheDeformerCount > 0 ? 1 : 0;
			for (i = 0; i < lVertexCacheDeformerCount; i++) {
				//頂点キャッシュ
				FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(lNode->GetGeometry()->GetDeformer(i, FbxDeformer::eVertexCache));
				if (!lDeformer)continue;
				FbxCache*lCache = lDeformer->GetCache();

				if (!lCache)continue;

				if (lDeformer->Active.Get()) {
					if (lCache->GetCacheFileFormat() == FbxCache::eMaxPointCacheV2) {
						//処理なし
					}
					else if (lCache->GetCacheFileFormat() == FbxCache::eMayaCache) {
						//MCからPC2フォーマットに変換する
						if (!lCache->ConvertFromMCToPC2(FbxTime::GetFrameRate(pScene->GetGlobalSettings().GetTimeMode()), 0, &lStatus)) {
							//失敗した時
							FbxString lTheErrorIs = lStatus.GetErrorString();
						}
					}


					//キャッシュを読み取る
					if (!lCache->OpenFileForRead(&lStatus)) {
						FbxString lTheErrorIs = lStatus.GetErrorString();
						lDeformer->Active = false;
					}
					else {
						FbxTime lChannel_Start;
						FbxTime lChannel_Stop;
						int lChannelIndex = lCache->GetChannelIndex(lDeformer->Channel.Get());
						if (lCache->GetAnimationRange(lChannelIndex, lChannel_Start, lChannel_Stop)) {
							// あイメーション開始フレーム設定
							if (lChannel_Start < pCache_Start) pCache_Start = lChannel_Start;

							// アニメーション終了フレーム設定
							if (lChannel_Stop > pCache_Stop)  pCache_Stop = lChannel_Stop;


						}
					}

				}

			}
		}
	}
}

void DX11FbxLoader::FillPoseArray(FbxScene * pScene, FbxArray<FbxPose*> pPoseArray)
{
	const auto lPoseCount = pScene->GetPoseCount();
	for (auto i = 0; i < lPoseCount; i++) {
		pPoseArray.Add(pScene->GetPose(i));
	}
}

//アニメーションの読み込み
void DX11FbxLoader::LoadAnimationData()
{
	int lCurrentAnimStackIndex = -1;	//現在のアニメーション
	const FbxArray<FbxString*>&lAnimStackNameArray = GetAnimStackNameArray();
	auto lPoseCount = lAnimStackNameArray.GetCount();
	for (int lPoseIndex = 0; lPoseIndex < lPoseCount; lPoseIndex++) {
		AnimData[lAnimStackNameArray[lPoseIndex]->Buffer()] = lPoseIndex;
		//現在のアニメーションスタックが現在適用されているアニメーションかどうか
		if (lAnimStackNameArray[lPoseIndex]->Compare(SceneAnim->ActiveAnimStackName.Get()) == 0) {
			//適用されているならそのアニメーションを現在のアニメーションとする
			lCurrentAnimStackIndex = lPoseIndex;
		}
	}
	//設定されたアニメーションをセットする
	EnableAnimation = lCurrentAnimStackIndex != -1 ? true : false;
	SetCurrentAnimStack(lCurrentAnimStackIndex);
}

void DX11FbxLoader::SetAnimation(std::string pName)
{
	SetCurrentAnimStack(AnimData[pName]);
}

void DX11FbxLoader::SetAnimation(int pIndex)
{
	SetCurrentAnimStack(pIndex);
}


std::vector<std::vector<FBXModelData*>>* DX11FbxLoader::GetGeometryData2()
{
	//メッシュデータの解放
	ReleaseGeometryData();

	FbxPose*lPose{ nullptr };
	if (PoseIndex != -1)
	{
		lPose = Scene->GetPose(PoseIndex);
	}
	FbxAMatrix lDummyGlobalPosition;


	if (Stop > Start)
	{
		CurrentTime += FrameTime;

		if (CurrentTime > Stop)
		{
			CurrentTime = Start;
		}
	}


	for (unsigned int i = 0; i < nodemeshes.size();i++) {

		FBXMesh*lMeshData = new FBXMesh;

		FbxNode* node = nodemeshes[i];
		FbxAMatrix lGlobalPosition;

		FbxComputeDeformer::GetGlobalPosition(lGlobalPosition, node, CurrentTime, lPose, &lDummyGlobalPosition);
		
		FbxAMatrix lGeometryOffset;
		FbxAMatrix lGlobalOffPosition;


		//ジオメトリのオフセットを取得
		FbxComputeDeformer::GetGeometry(lGeometryOffset, node);
		lGlobalOffPosition = lGlobalPosition*lGeometryOffset;

		//モデルデータに格納する行列を作成
		D3DXMATRIX*lDXGlobalPostion = new D3DXMATRIX;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				lDXGlobalPostion->m[i][j] = lGlobalOffPosition.Buffer()->Buffer()[i * 4 + j];
			}
		}

		//ノードのメッシュを取得
		FbxMesh* lMesh = node->GetMesh();

		//頂点数を取得
		const int lVertexCount = lMesh->GetControlPointsCount();

		//頂点が一つもない場合は処理しない
		if (lVertexCount == 0) {
			continue;
		}


		//VBO取得
		FbxVBO* lMeshCache = static_cast<FbxVBO*>(lMesh->GetUserDataPtr());
		//ボーンがあるならボーンを取得
		const bool lHasVertexCache = lMesh->GetDeformerCount(FbxDeformer::eVertexCache) && (static_cast<FbxVertexCacheDeformer*>(lMesh->GetDeformer(0, FbxDeformer::eVertexCache)))->Active.Get();
		const bool lHasShape = lMesh->GetShapeCount() > 0;
		const bool lHasSkin = lMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
		const bool lHasDeformation = lHasVertexCache || lHasShape || lHasSkin;


		//頂点バッファ
		FbxVector4* lVertexArray = nullptr;
		//法線バッファ
		FbxVector4* lNormalArray = nullptr;
		//VBO未作成 or ボーンがない場合、そのままコピー
		if (!lMeshCache || lHasDeformation) {

			lVertexArray = new FbxVector4[lVertexCount];
			//頂点バッファのコピー(取得)
			memmove(lVertexArray, lMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));
		}

		//アニメーション関係の処理
		//デフォーマーがあった場合、アニメーション処理

		if (lHasDeformation) {

			if (lHasVertexCache) {
				FbxComputeDeformer::ReadVertexCacheData(lMesh, CurrentTime, lVertexArray);
			}
			else {
				if (lHasShape)
				{
					// Deform the vertex array with the shapes.
					FbxComputeDeformer::ComputeShapeDeformation(lMesh, CurrentTime, CurrentAnimLayer, lVertexArray);
				}
				//we need to get the number of clusters
				const int lSkinCount = lMesh->GetDeformerCount(FbxDeformer::eSkin);
				int lClusterCount = 0;
				for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
				{
					lClusterCount += ((FbxSkin *)(lMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin)))->GetClusterCount();
				}
				if (lClusterCount)
				{
					// Deform the vertex array with the skin deformer.
					FbxComputeDeformer::ComputeSkinDeformation(lGlobalOffPosition, lMesh, CurrentTime, lVertexArray, lPose);

				}

			}



			if (lMeshCache)
				//更新した頂点の反映
				lMeshCache->UpdateVertexPosition(lMesh, lVertexArray);

		}



		std::vector<FBXModelData*>mdv;
		//アニメーションなどすべての処理が終わったら戻り値用のジオメトリを作成する
		if (lMeshCache) {
			const int lSubMeshCount = lMeshCache->GetSubMeshCount();
			//サブメッシュの個数分行う
			for (int lIndex = 0; lIndex < lSubMeshCount; lIndex++) {

				int lOffset = lMeshCache->mSubMeshes[lIndex]->IndexOffset;

				//モデルインスタンス作成
				FBXModelData* md = new FBXModelData();
				md->GlobalPosition = lDXGlobalPostion;
				//マテリアル情報の取得
				const FbxSurfaceMaterial*lMaterial = node->GetMaterial(lIndex);

				if (lMaterial) {
					FbxMaterialCache * lMaterialCache = static_cast<FbxMaterialCache *>(lMaterial->GetUserDataPtr());
					if (lMaterialCache)
					{
						lMaterialCache->SetCurrentMaterial(md);
					}
				}
				else {
					FbxMaterialCache::SetDefaultMaterial(md);
				}


				//ジオメトリバッファ作成
				auto loopCount = lMeshCache->mSubMeshes[lIndex]->TriangleCount * 3;
				md->IndexLength = loopCount;

				auto vertexCount = lMeshCache->lVertices.Size();
				md->PosLength = vertexCount / 4;
				md->Data.resize(md->PosLength);
				for (int i = 0; i < vertexCount / 4; ++i) {

					//頂点のコピー
					memcpy(&md->Data[i].Pos, &lMeshCache->lVertices.GetArray()[i * 4], sizeof(D3DXVECTOR4));


					if (!lHasDeformation)
						memcpy(&md->Data[i].Normal, &lMeshCache->lNormals.GetArray()[i * 3], sizeof(D3DXVECTOR3));




					if (lMeshCache->lUVs.Size()>0) {
						memcpy(&md->Data[i].UV, &lMeshCache->lUVs.GetArray()[i * 2], sizeof(D3DXVECTOR2));

					}
				}

				//インデックス座標を取得
				md->Index = lMeshCache->lIndices.GetArray() + lOffset;
				auto count = lMeshCache->lIndices.GetCount();
				//頂点座標とインデックス座標をもとに法線の計算
				if (lHasDeformation) {
					for (int i = 0; i < lMeshCache->lIndices.Size() / 3 - 2; i++) {
						D3DXVECTOR3 v0;
						D3DXVECTOR3 v1;
						D3DXVECTOR3 v2;
						D3DXVECTOR3 vv, vv1, vv2;
						v0 = (D3DXVECTOR3)md->Data[md->Index[i * 3 + 0]].Pos;
						v1 = (D3DXVECTOR3)md->Data[md->Index[i * 3 + 1]].Pos;
						v2 = (D3DXVECTOR3)md->Data[md->Index[i * 3 + 2]].Pos;
						vv1 = v1 - v0;
						vv2 = v2 - v0;

						D3DXVec3Cross(&vv, &vv1, &vv2);
						D3DXVec3Normalize(&vv, &vv);
						memcpy(&md->Data[md->Index[i * 3 + 0]].Normal, &vv, sizeof(D3DXVECTOR3));
						memcpy(&md->Data[md->Index[i * 3 + 1]].Normal, &vv, sizeof(D3DXVECTOR3));
						memcpy(&md->Data[md->Index[i * 3 + 2]].Normal, &vv, sizeof(D3DXVECTOR3));


					}
				}




				//md.Index = idx;
				//md.Pos = vv;
				mdv.push_back(md);
			}
		}
		else {

			//const int lPolygonCount = lMesh->GetPolygonCount();
			//for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; lPolygonIndex++) {
			//	const int VerticeCount = lMesh->GetPolygonSize(lPolygonIndex);

			//	SimpleVertex v;
			//	std::vector<SimpleVertex>vv;
			//	ModelData* md=new ModelData();

			//	for (int lVerticeIndex = 0; lVerticeIndex < lVertexCount; lVerticeIndex++) {
			//		auto x = lVertexArray[lMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex)];
			//		md->Pos = x[0];
			//		md->Pos = x[1];
			//		md->Pos = x[];
			//		v.Pos.x = x[0];
			//		v.Pos.y = x[1];
			//		v.Pos.z = x[2];
			//		
			//		vv.push_back(v);
			//	}
			//	md.Pos = vv;
			//	mdv.push_back(md);

			//}
		}
		Geometry.push_back(mdv);

		delete[]lVertexArray;


	}




	return &Geometry;
}



//どのアニメーションを使うか設定する
bool DX11FbxLoader::SetCurrentAnimStack(int pIndex)
{
	const int lAnimStackCount = AnimStackNameArray.GetCount();

	//アニメーションがない or 存在しないアニメーションの指定をした場合はエラー
	if (lAnimStackCount == 0 || pIndex >= lAnimStackCount) {
		return false;
	}

	FbxAnimStack*lCurrentAnimationStack = SceneAnim->FindMember<FbxAnimStack>(AnimStackNameArray[pIndex]->Buffer());
	if (lCurrentAnimationStack == nullptr) {
		//シーンにアニメーションスタックがない場合は設定できない
		return false;
	}

	//アニメーションのレイヤーを取得
	CurrentAnimLayer = lCurrentAnimationStack->GetMember<FbxAnimLayer>();
	//シーンにアニメーションレイヤーを設定する
	SceneAnim->SetCurrentAnimationStack(lCurrentAnimationStack);

	FbxTakeInfo*lCurrentTakeInfo = SceneAnim->GetTakeInfo(*(AnimStackNameArray[pIndex]));
	//アニメーション開始時間と終了時間を取得する
	if (lCurrentTakeInfo) {
		Start = lCurrentTakeInfo->mLocalTimeSpan.GetStart();
		Stop = lCurrentTakeInfo->mLocalTimeSpan.GetStop();
	}
	else {
		FbxTimeSpan lTimeLineTimeSpan;
		SceneAnim->GetGlobalSettings().GetTimelineDefaultTimeSpan(lTimeLineTimeSpan);
		Start = lTimeLineTimeSpan.GetStart();
		Stop = lTimeLineTimeSpan.GetStop();
	}

	//取得した時間が有効な時間か調べる
	if (Cache_Start < Start)
		Start = Cache_Start;

	if (Cache_Stop  > Stop)
		Stop = Cache_Stop;

	//スタート時間を設定しておく
	CurrentTime = Start;
	return true;
}

bool DX11FbxLoader::SetCurrentPoseIndex(int pPoseIndex)
{
	SetCurrentAnimStack(pPoseIndex);
	return false;
}



void DX11FbxLoader::ReleaseGeometryData()
{
	for (unsigned int i = 0; i < Geometry.size(); i++) {
		for (unsigned int j = 0; j < Geometry[i].size(); j++) {
			if (Geometry[i][j] != nullptr) {
				delete Geometry[i][j];
				Geometry[i][j] = nullptr;
			}
		}
		Geometry[i].clear();
	}
	Geometry.clear();

	for (unsigned int i = 0; i < mMesh.size(); i++) {
		if (mMesh[i] != nullptr) {
			delete mMesh[i];
		}
	}

}


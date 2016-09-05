//グローバル
/*
	FBXレンダリング用
*/
Texture2D gTexture:register(t0);
SamplerState gSampler:register(s0);

//アプリケーションごとに適用
cbuffer global:register(b0)
{
	matrix g_mW;//ワールド行列
	matrix g_mWVP; //ワールドから射影までの変換行列
	float4 g_vLightDir;  //ライトの方向ベクトル
};

//サブメッシュごとに適用
cbuffer global:register(b1) {
	float4 g_Diffuse = float4(1, 0, 0, 0); //拡散反射(色）
}

//構造体
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 UV:TEXCOORD;
};

//
//バーテックスシェーダー
//
VS_OUTPUT VS(float4 Pos : POSITION, float4 Normal : NORMAL, float2 Tex : TEXCOORD)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(Pos, g_mWVP);
	output.UV = Tex;
	return output;
}

//
//ピクセルシェーダー
//
float4 PS(VS_OUTPUT input) : SV_Target
{
	
	return gTexture.Sample(gSampler,input.UV);
}
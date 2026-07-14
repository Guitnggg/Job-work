#include "SceneHelper.h"

using namespace KamataEngine;

namespace SceneHelper {

/// <summary>
/// メニュー画面で奥行き感を出すための小惑星フィールド設定を返す。
/// ゲーム中より控えめな速度・数にして、背景演出として使いやすい値にしている。
/// </summary>
AsteroidFieldConfig CreateMenuAsteroidFieldConfig() {
	AsteroidFieldConfig config{};
	config.count = 10;
	config.spawnZMin = 0.0f;
	config.spawnZMax = 140.0f;
	config.recycleZ = -50.0f;
	config.spawnInterval = 1.0f;
	config.rangeX = 25.0f;
	config.rangeY = 15.0f;
	config.speedMin = -0.3f;
	config.speedMax = -0.1f;
	config.rotationMin = 0.01f;
	config.rotationMax = 0.03f;
	return config;
}

// 既に読み込み済みのテクスチャを、そのまま標準色のスプライトにする。
std::unique_ptr<Sprite> CreateSprite(uint32_t textureHandle, const Vector2& position) { return std::unique_ptr<Sprite>(Sprite::Create(textureHandle, position)); }

// 既に読み込み済みのテクスチャを、色指定付きのスプライトにする。
std::unique_ptr<Sprite> CreateSprite(uint32_t textureHandle, const Vector2& position, const Vector4& color) { return std::unique_ptr<Sprite>(Sprite::Create(textureHandle, position, color)); }


// テクスチャ読み込みとスプライト生成をひとまとめに行う。/// 呼び出し元がハンドルを保持したい場合だけ textureHandle に書き戻す。
std::unique_ptr<Sprite> CreateSprite(const char* texturePath, const Vector2& position, uint32_t* textureHandle) {
	const uint32_t handle = TextureManager::Load(texturePath);
	if (textureHandle) {
		*textureHandle = handle;
	}
	return CreateSprite(handle, position);
}

// テクスチャ読み込みと色指定付きスプライト生成をひとまとめに行う。
std::unique_ptr<Sprite> CreateSprite(const char* texturePath, const Vector2& position, const Vector4& color, uint32_t* textureHandle) {
	const uint32_t handle = TextureManager::Load(texturePath);
	if (textureHandle) {
		*textureHandle = handle;
	}
	return CreateSprite(handle, position, color);
}


// 直前のスプライト描画状態を終了させたうえで深度バッファを初期化する。
// 3Dモデルを描く前に呼ぶことで、2D描画の状態と深度情報が残らないようにする。
void Begin3DDraw(DirectXCommon* dxCommon, ID3D12GraphicsCommandList* commandList) {
	DrawSpriteLayer(commandList, []() {});
	dxCommon->ClearDepthBuffer();
}

} // namespace SceneHelper

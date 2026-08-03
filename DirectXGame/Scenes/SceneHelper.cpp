#include "SceneHelper.h"

using namespace KamataEngine;

namespace SceneHelper {

namespace {
constexpr int kMenuAsteroidCount = 10;
constexpr float kMenuAsteroidSpawnZMax = 140.0f;
constexpr float kMenuAsteroidRecycleZ = -50.0f;
constexpr float kMenuAsteroidRangeX = 25.0f;
constexpr float kMenuAsteroidRangeY = 15.0f;
constexpr float kMenuAsteroidSpeedMin = -0.3f;
constexpr float kMenuAsteroidSpeedMax = -0.1f;
constexpr float kMenuAsteroidRotationMin = 0.01f;
constexpr float kMenuAsteroidRotationMax = 0.03f;
}

/// <summary>
/// メニュー画面で奥行き感を出すための小惑星フィールド設定を返す。
/// ゲーム中より控えめな速度・数にして、背景演出として使いやすい値にしている。
/// </summary>
AsteroidFieldConfig CreateMenuAsteroidFieldConfig() {
	AsteroidFieldConfig config{};
	config.count = kMenuAsteroidCount;
	config.spawnZMin = 0.0f;
	config.spawnZMax = kMenuAsteroidSpawnZMax;
	config.recycleZ = kMenuAsteroidRecycleZ;
	config.spawnInterval = 1.0f;
	config.rangeX = kMenuAsteroidRangeX;
	config.rangeY = kMenuAsteroidRangeY;
	config.speedMin = kMenuAsteroidSpeedMin;
	config.speedMax = kMenuAsteroidSpeedMax;
	config.rotationMin = kMenuAsteroidRotationMin;
	config.rotationMax = kMenuAsteroidRotationMax;
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

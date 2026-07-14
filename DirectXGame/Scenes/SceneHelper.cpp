#include "SceneHelper.h"

using namespace KamataEngine;

namespace SceneHelper {

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

std::unique_ptr<Sprite> CreateSprite(uint32_t textureHandle, const Vector2& position) { return std::unique_ptr<Sprite>(Sprite::Create(textureHandle, position)); }

std::unique_ptr<Sprite> CreateSprite(uint32_t textureHandle, const Vector2& position, const Vector4& color) { return std::unique_ptr<Sprite>(Sprite::Create(textureHandle, position, color)); }

std::unique_ptr<Sprite> CreateSprite(const char* texturePath, const Vector2& position, uint32_t* textureHandle) {
	const uint32_t handle = TextureManager::Load(texturePath);
	if (textureHandle) {
		*textureHandle = handle;
	}
	return CreateSprite(handle, position);
}

std::unique_ptr<Sprite> CreateSprite(const char* texturePath, const Vector2& position, const Vector4& color, uint32_t* textureHandle) {
	const uint32_t handle = TextureManager::Load(texturePath);
	if (textureHandle) {
		*textureHandle = handle;
	}
	return CreateSprite(handle, position, color);
}

void Begin3DDraw(DirectXCommon* dxCommon, ID3D12GraphicsCommandList* commandList) {
	DrawSpriteLayer(commandList, []() {});
	dxCommon->ClearDepthBuffer();
}

} // namespace SceneHelper

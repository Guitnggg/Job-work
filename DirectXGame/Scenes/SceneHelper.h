#pragma once

#include <KamataEngine.h>
#include <cstdint>
#include <memory>
#include <utility>

#include "Application/Objects/Asteroid/AsteroidField.h"

namespace SceneHelper {

AsteroidFieldConfig CreateMenuAsteroidFieldConfig();

std::unique_ptr<KamataEngine::Sprite> CreateSprite(uint32_t textureHandle, const KamataEngine::Vector2& position);
std::unique_ptr<KamataEngine::Sprite> CreateSprite(uint32_t textureHandle, const KamataEngine::Vector2& position, const KamataEngine::Vector4& color);
std::unique_ptr<KamataEngine::Sprite> CreateSprite(const char* texturePath, const KamataEngine::Vector2& position, uint32_t* textureHandle = nullptr);
std::unique_ptr<KamataEngine::Sprite> CreateSprite(const char* texturePath, const KamataEngine::Vector2& position, const KamataEngine::Vector4& color, uint32_t* textureHandle = nullptr);

void Begin3DDraw(KamataEngine::DirectXCommon* dxCommon, ID3D12GraphicsCommandList* commandList);

template <class DrawFunc>
void DrawModelLayer(DrawFunc&& drawFunc) {
	KamataEngine::Model::PreDraw();
	std::forward<DrawFunc>(drawFunc)();
	KamataEngine::Model::PostDraw();
}

template <class DrawFunc>
void DrawSpriteLayer(ID3D12GraphicsCommandList* commandList, DrawFunc&& drawFunc) {
	KamataEngine::Sprite::PreDraw(commandList);
	std::forward<DrawFunc>(drawFunc)();
	KamataEngine::Sprite::PostDraw();
}

} // namespace SceneHelper

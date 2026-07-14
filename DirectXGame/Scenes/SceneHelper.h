#pragma once

#include <KamataEngine.h>
#include <cstdint>
#include <memory>
#include <utility>

#include "Application/Objects/Asteroid/AsteroidField.h"

/// <summary>
/// シーン共通の初期化・描画補助関数群。
/// 各シーンで重複しやすいスプライト生成、3D描画開始処理、
/// モデル/スプライト描画レイヤーの PreDraw/PostDraw 呼び出しをまとめる。
/// </summary>
namespace SceneHelper {

/// <summary>
/// タイトルやリザルトなど、メニュー系シーンで使う小惑星背景の標準設定を作成する。
/// </summary>
/// <returns>メニュー用に調整された小惑星フィールド設定</returns>
AsteroidFieldConfig CreateMenuAsteroidFieldConfig();

/// <summary>
/// 読み込み済みテクスチャハンドルからスプライトを生成する。
/// </summary>
/// <param name="textureHandle">使用するテクスチャハンドル</param>
/// <param name="position">表示位置</param>
/// <returns>生成されたスプライト</returns>
std::unique_ptr<KamataEngine::Sprite> CreateSprite(uint32_t textureHandle, const KamataEngine::Vector2& position);

/// <summary>
/// 読み込み済みテクスチャハンドルと初期色からスプライトを生成する。
/// </summary>
/// <param name="textureHandle">使用するテクスチャハンドル</param>
/// <param name="position">表示位置</param>
/// <param name="color">初期色</param>
/// <returns>生成されたスプライト</returns>
std::unique_ptr<KamataEngine::Sprite> CreateSprite(uint32_t textureHandle, const KamataEngine::Vector2& position, const KamataEngine::Vector4& color);

/// <summary>
/// テクスチャを読み込んでからスプライトを生成する。
/// 読み込んだハンドルを呼び出し元で再利用したい場合は textureHandle に受け取れる。
/// </summary>
/// <param name="texturePath">読み込むテクスチャパス</param>
/// <param name="position">表示位置</param>
/// <param name="textureHandle">読み込んだテクスチャハンドルの受け取り先</param>
/// <returns>生成されたスプライト</returns>
std::unique_ptr<KamataEngine::Sprite> CreateSprite(const char* texturePath, const KamataEngine::Vector2& position, uint32_t* textureHandle = nullptr);

/// <summary>
/// テクスチャを読み込み、初期色を指定してスプライトを生成する。
/// </summary>
/// <param name="texturePath">読み込むテクスチャパス</param>
/// <param name="position">表示位置</param>
/// <param name="color">初期色</param>
/// <param name="textureHandle">読み込んだテクスチャハンドルの受け取り先</param>
/// <returns>生成されたスプライト</returns>
std::unique_ptr<KamataEngine::Sprite> CreateSprite(const char* texturePath, const KamataEngine::Vector2& position, const KamataEngine::Vector4& color, uint32_t* textureHandle = nullptr);

/// <summary>
/// 2D描画状態を閉じて深度バッファをクリアし、3D描画を始められる状態にする。
/// </summary>
/// <param name="dxCommon">DirectX共通管理クラス</param>
/// <param name="commandList">描画コマンドリスト</param>
void Begin3DDraw(KamataEngine::DirectXCommon* dxCommon, ID3D12GraphicsCommandList* commandList);

/// <summary>
/// モデル描画用の PreDraw/PostDraw をまとめて実行する。
/// </summary>
/// <param name="drawFunc">モデル描画処理</param>
template <class DrawFunc>
void DrawModelLayer(DrawFunc&& drawFunc) {
	KamataEngine::Model::PreDraw();
	std::forward<DrawFunc>(drawFunc)();
	KamataEngine::Model::PostDraw();
}

/// <summary>
/// スプライト描画用の PreDraw/PostDraw をまとめて実行する。
/// </summary>
/// <param name="commandList">描画コマンドリスト</param>
/// <param name="drawFunc">スプライト描画処理</param>
template <class DrawFunc>
void DrawSpriteLayer(ID3D12GraphicsCommandList* commandList, DrawFunc&& drawFunc) {
	KamataEngine::Sprite::PreDraw(commandList);
	std::forward<DrawFunc>(drawFunc)();
	KamataEngine::Sprite::PostDraw();
}

} // namespace SceneHelper

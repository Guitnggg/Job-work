#pragma once

#include <KamataEngine.h>
#include <memory>

#include "Application/Background/Skydome/Skydome.h"
#include "Application/Objects/Asteroid/AsteroidField.h"

#include "Scenes/IScene.h"
class TitleScene;

/// <summary>
/// ゲームオーバー時に遷移するシーン
/// 背景スクロール演出→GameOver文字表示→タイトルへ戻る
/// </summary>
class FinishScene : public IScene {
public:
	/// <summary>
	/// インストラクタ
	/// </summary>
	FinishScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~FinishScene();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw() override;

public:
	/// <summary>
	/// シーン終了フラグ
	/// </summary>
	bool IsEnd() const override { return isEnd_; }

	/// <summary>
	/// シーン終了後の遷移先
	/// </summary>
	std::unique_ptr<IScene> NextScene() const override;

	/// <summary>
	/// デバック・識別用シーン名取得
	/// </summary>
	SceneName GetSceneName() const override { return SceneName::Finish; }

private:
	// DirectX / カメラ / 入力
	KamataEngine::DirectXCommon* dxCommon_ = nullptr;
	KamataEngine::Input* input_ = nullptr;
	KamataEngine::Camera camera_;

	// UIスプライト
	uint32_t finishTextureHandle_ = 0;
	std::unique_ptr<KamataEngine::Sprite> finishSprite_;
	uint32_t returnTextureHandle_ = 0;
	std::unique_ptr<KamataEngine::Sprite> returnSprite_;

	// 効果音
	uint32_t changeSEHandle_ = 0;

	// 点滅演出
	float blinkTimer_ = 0.0f;
	float blinkInterval_ = 1.0f;

	// 背景演出（天球 / 小惑星）
	std::unique_ptr<Skydome> skydome_;
	AsteroidField asteroidField_;

	// 終了フラグ
	bool isEnd_ = false;
};

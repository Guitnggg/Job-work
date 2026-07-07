#pragma once

#include <KamataEngine.h>
#include <algorithm>
#include <cmath>
#include <memory>

#include "Application/Background/Skydome/Skydome.h"
#include "Application/Objects/Asteroid/AsteroidField.h"
#include "Application/UI/Score/Score.h"

#include "Scenes/IScene.h"
class TitleScene;

enum class ClearPhase { CameraMove, TitlePop, ResultCount, WaitInput };

/// <summary>
/// ゲームクリア時に遷移するシーン
/// 背景演出→GameClearテキスト表示→スコアカウントアップ→入力待ち
/// →タイトルへ遷移
/// </summary>
class ClearScene : public IScene {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="finalScore">ゲーム中で獲得した最終スコア</param>
	ClearScene(int finalScore);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~ClearScene();

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
	/// フェードアウト/入力完了でシーン変遷
	/// </summary>
	bool IsEnd() const override { return isEnd_; } // シーン終了フラグ

	/// <summary>
	/// シーン終了後に遷移するシーンを返す
	/// </summary>
	std::unique_ptr<IScene> NextScene() const override;

	/// <summary>
	/// デバック用：シーン名取得
	/// </summary>
	SceneName GetSceneName() const override { return SceneName::Clear; }

private:
	// DirectX / 入力 / カメラ / 行列
	KamataEngine::DirectXCommon* dxCommon_ = nullptr;
	KamataEngine::Input* input_ = nullptr;
	KamataEngine::Camera camera_;

	// 効果音
	uint32_t changeSEHandle_ = 0;
	uint32_t pointSEHandle_ = 0;

	// 背景（天球 / 小惑星）
	std::unique_ptr<Skydome> skydome_;
	AsteroidField asteroidField_;

	// 演出フェーズ
	ClearPhase phase_ = ClearPhase::CameraMove;
	float phaseTimer_ = 0.0f;

	// スコアUI
	int finalScore_ = 0;
	int displayedScore_ = 0;
	Score scoreUI_;

	// GAME CLEAR テキスト
	std::unique_ptr<KamataEngine::Sprite> clearTextSprite_;
	KamataEngine::Vector2 clearTextBaseSize_ = {512.0f * 2, 128.0f * 2};

	// RETURN テキスト（スペースでタイトルに戻る）
	std::unique_ptr<KamataEngine::Sprite> returnTextSprite_;
	KamataEngine::Vector2 returnTextBaseSize_ = {1024.0f, 128.8f};

	// 終了フラグ
	bool isEnd_ = false;
};
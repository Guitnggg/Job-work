#pragma once

#include <KamataEngine.h>
#include <array>
#include <memory>

#include "Application/Background/Skydome/Skydome.h"
#include "Application/Objects/Asteroid/AsteroidField.h"
#include "Scenes/SceneName.h"

#include "Scenes/IScene.h"
class GameScene;

/// <summary>
/// 操作説明・ゲーム導入を目的としたイントロシーン
/// 背景アニメーション（小惑星＋天球）＋説明画像の表示を行い
/// ESCでタイトルへ戻る・SPACEでゲーム本編へ進むシーン遷移を担当する
/// </summary>
class IntroductionScene : public IScene {
public:
	/// <summary>
	/// 難易度選択
	/// </summary>
	enum class Difficulty {
		Tutorial = 0,
		Easy,
		Normal,
		Hard,
		Count
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	IntroductionScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~IntroductionScene();

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
	/// シーン終了フラグの取得
	/// </summary>
	bool IsEnd() const override { return isEnd_; }

	/// <summary>
	/// 遷移先のシーン取得
	/// </summary>
	std::unique_ptr<IScene> NextScene() const override;

	/// <summary>
	/// デバック識別用のシーン名取得
	/// </summary>
	SceneName GetSceneName() const override { return SceneName::Introduction; }

	/// <summary>
	/// シーン遷移の演出ヒント
	/// </summary>
	TransitionHint GetTransitionHint(SceneName to) const override;

private:
	// ===== 基本 =====
	KamataEngine::DirectXCommon* dxCommon_ = nullptr; // DirectX関連の管理クラス
	KamataEngine::Input* input_ = nullptr;            // 入力管理クラス
	KamataEngine::Camera camera_;                     // カメラ管理クラス

	// 各種テクスチャ
	uint32_t returnTitleTextureHandle_ = 0;
	std::unique_ptr<KamataEngine::Sprite> returnTitleSprite_;

	uint32_t introTextureHandle_ = 0;
	std::unique_ptr<KamataEngine::Sprite> introSprite_;

	std::array<uint32_t, static_cast<size_t>(Difficulty::Count)> difficultyTextureHandles_{};
	std::array<std::unique_ptr<KamataEngine::Sprite>, static_cast<size_t>(Difficulty::Count)> difficultySprites_;
	uint32_t difficultyCursorTextureHandle_ = 0;
	int selectedIndex_ = 0;

	// 各種サウンド
	uint32_t changeSEHandle_ = 0; // シーン変遷SE

	// 天球
	std::unique_ptr<Skydome> skydome_;

	// 小惑星
	AsteroidField asteroidField_;

	// 次のシーン
	SceneName nextScene_ = SceneName::None;

	// シーン終了フラグ
	bool isEnd_ = false;
};
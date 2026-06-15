#pragma once

#include <2d/Sprite.h>
#include <algorithm>
#include <audio/Audio.h>
#include <base/DirectXCommon.h>

#include "IScene.h"
#include "SceneName.h"

/// <summary>
/// シーン遷移の進行状態
/// None      : 遷移中ではない（通常再生）
/// FadeOut   : 黒フェードアウト中
/// LoadNextScene : 現在のシーンを解放して次のシーンを生成する（現実装では未使用）
/// FadeIn    : 黒フェードイン中
/// FlashOut  : 白フラッシュ
/// </summary>
enum class SceneTransitionState { None, FadeOut, LoadNextScene, FadeIn, FlashOut };

/// <summary>
/// ゲーム内の全シーンを統括するマネージャー
/// ・現在再生中のシーンの更新／描画
/// ・シーン終了判定の検知
/// ・遷移演出（黒フェード／白フラッシュ）の制御
/// ・新しいシーンの生成・Initialize呼び出し
/// </summary>
class SceneManager {
public:
	/// <summary>
	/// コンストラクタ：初期化処理
	/// </summary>
	SceneManager();

	/// <summary>
	/// デストラクタ：現在のシーンを解放
	/// </summary>
	~SceneManager();

	/// <summary>
	/// シーンを新しいものに切り替える
	/// </summary>
	/// <param name="newScene">次に実行するシーン</param>
	void ChangeScene(std::unique_ptr<IScene> newScene);

	/// <summary>
	/// 現在のシーンの更新処理
	/// シーンの終了フラグが立っていたら次のシーンに遷移する
	/// </summary>
	void Update();

	/// <summary>
	/// 現在のシーンの描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// 現在のシーンが存在しない＝全て終了したかどうか
	/// </summary>
	bool IsEnd() const;

	/// <summary>
	/// 現在のシーンを取得（ポインタ）
	/// </summary>
	IScene* GetCurrentScene() const;

	/// <summary>
	///  最初のsceneを設定する
	/// </summary>
	/// <param name="scene"></param>
	void SetInitialScene(std::unique_ptr<IScene> scene);

private:
	/// <summary>
	///
	/// </summary>
	void UpdateSceneBgm_();

	/// <summary>
	///
	/// </summary>
	uint32_t GetBgmHandleForScene_(SceneName sceneName);

private:
	/// <summary>
	/// 自動演出判定用
	/// Title → Introduction : 黒フェード
	/// Introduction → InGame : 白フラッシュ
	/// </summary>
	bool ShouldUseWhiteFlash(SceneName from, SceneName to) const;

private:
	std::unique_ptr<IScene> currentScene_ = nullptr; // 現在実行中のシーン
	std::unique_ptr<IScene> nextScene_ = nullptr;    // 遷移予定のシーン

	SceneTransitionState transitionState_ = SceneTransitionState::None;

	// === 黒フェード ===
	float transitionAlpha_ = 0.0f;
	float transitionSpeed_ = 0.02f;

	// === 白フラッシュ ===
	float flashTimer_ = 0.0f;
	float flashTime_ = 0.16f;

	// ベース値
	float baseTransitionSpeed_ = 0.02f;
	float baseFlashTime_ = 0.16f;

	bool pendingWhiteFlash_ = false;

	std::unique_ptr<KamataEngine::Sprite> fadeSprite_;
	std::unique_ptr<KamataEngine::Sprite> flashSprite_;

	KamataEngine::Audio* audio_ = nullptr;
	uint32_t opBgmHandle_ = 0;
	uint32_t gameBgmHandle_ = 0;
	uint32_t bgmVoiceHandle_ = 0;
	SceneName currentBgmScene_ = SceneName::None;
};

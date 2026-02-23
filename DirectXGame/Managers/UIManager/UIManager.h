#pragma once

#include <memory>
#include <algorithm>
#include <KamataEngine.h>

#include "UI/HPBar/Graph.h"
#include "UI/Score/Score.h"
#include "Application/Characters/Player/Player.h"

/// <summary>
/// HPバー／スコアなど画面上のUIをまとめて管理するクラス
/// </summary>
class UIManager {
public:
    UIManager() = default;
    ~UIManager() = default;

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="player">HP参照用のプレイヤー（所有しない）</param>
    void Initialize(Player* player);

    /// <summary>
    /// 更新処理（HPバー、スコアなど）
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

    /// <summary>
    /// スコアオブジェクトへのアクセス（必要なら）
    /// </summary>
    Score* GetScore() { return score_.get(); }

    /// <summary>
    /// 
    /// </summary>
    void SetHomingCooldownRate(float rate) { homingCooldownRate_ = std::clamp(rate, 0.0f, 1.0f); }

	/// <summary>
	/// 
	/// </summary>
	void SetHomingLockInfo(int32_t currentLockCount, int32_t maxLockCount, bool isLocking);

private:
    // HP参照用（所有権なし）
    Player* player_ = nullptr;

    // HPバー
    std::unique_ptr<Graph> graph_;

    // スコア
    std::unique_ptr<Score> score_;

    std::unique_ptr<KamataEngine::Sprite> homingCooldownBack_;
	std::unique_ptr<KamataEngine::Sprite> homingCooldownFront_;
	std::unique_ptr<KamataEngine::Sprite> lockProgressBar_;
	uint32_t homingBarTexHandle_ = 0;

	float homingCooldownRate_ = 1.0f;
	int32_t homingLockCount_ = 0;
	int32_t homingMaxLockCount_ = 5;
	bool isHomingLocking_ = false;
};

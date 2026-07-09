#pragma once

#include <KamataEngine.h>
#include <algorithm>
#include <memory>

#include "Application/Characters/Player/Player.h"
#include "Application/UI/HPBar/Graph.h"
#include "Application/UI/Score/Score.h"

/// <summary>
/// HPバー、スコア、ホーミング関連ゲージなどの画面UIをまとめて管理する。
/// </summary>
class UIManager {
public:
    UIManager() = default;
    ~UIManager() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="player">HPバーの対象となるプレイヤー</param>
    void Initialize(Player* player);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理	
    /// </summary>
    void Draw();

    /// <summary>
    /// スコア情報を取得する
    /// </summary>
    Score* GetScore() { return score_.get(); }

    /// <summary>
    /// ホーミングミサイルのクールダウン率を設定する
    /// </summary>
    void SetHomingCooldownRate(float rate) { homingCooldownRate_ = std::clamp(rate, 0.0f, 1.0f); }

    /// <summary>
    /// ホーミングミサイルのロックオン数と情報を設定する。
    /// </summary>
    void SetHomingLockInfo(int32_t currentLockCount, int32_t maxLockCount, bool isLocking);

private:
    // HP参照用。所有権は持たない
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

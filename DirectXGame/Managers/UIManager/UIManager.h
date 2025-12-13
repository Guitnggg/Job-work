#pragma once

#include <memory>
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

private:
    // HP参照用（所有権なし）
    Player* player_ = nullptr;

    // HPバー
    std::unique_ptr<Graph> graph_;

    // スコア
    std::unique_ptr<Score> score_;
};

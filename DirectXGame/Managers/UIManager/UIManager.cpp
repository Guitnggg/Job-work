#include "UIManager.h"

using namespace KamataEngine;

void UIManager::Initialize(Player* player)
{
    player_ = player;

    // HPバー（Graph）
    graph_ = std::make_unique<Graph>();
    graph_->Initialize();

    // スコア
    score_ = std::make_unique<Score>();
    score_->Initialize();
}

void UIManager::Update()
{
    // HPバー更新
    if (player_ && graph_) {
        // HP(0〜100想定) を 0.0〜1.0 に変換してGraphに渡す
        float hpRate = static_cast<float>(player_->GetHP()) / 100.0f;
        graph_->SetValue(hpRate);
        graph_->Update();
    }

    // スコア更新
    if (score_) {
        score_->Update();
    }
}

void UIManager::Draw()
{
    if (graph_) {
        graph_->Draw();
    }
    if (score_) {
        score_->Draw();
    }
}

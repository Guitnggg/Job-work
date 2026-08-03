#include "Graph.h"

#include <algorithm>
#include <cassert>

Graph::Graph() {}

void Graph::Initialize() {

    texHandle_ = KamataEngine::TextureManager::Load("./Resources/white1x1.png");

    backBar_.reset(KamataEngine::Sprite::Create(texHandle_, { kBarPosX, kBarPosY }, { 0.04f, 0.06f, 0.08f, 0.85f }));
    frontBar_.reset(KamataEngine::Sprite::Create(texHandle_, { kBarPosX, kBarPosY }, { 0.15f, 0.95f, 0.35f, 0.95f }));
    backBar_->SetSize({ kBarWidth, kBarHeight });
    frontBar_->SetSize({ kBarWidth, kBarHeight });

    // 初期値を100%に設定
    value_ = 1.0f;
}

void Graph::SetValue(float rate) {
    // 0.0～1.0の範囲にクランプ
    value_ = std::clamp(rate, 0.0f, 1.0f);
}

void Graph::Update() {
    // 横幅を value_ に応じて変化させる
    frontBar_->SetSize({ kBarWidth * value_, kBarHeight });
    if (value_ <= 0.25f) {
        frontBar_->SetColor({ 1.0f, 0.15f, 0.10f, 0.98f });
    } else if (value_ <= 0.50f) {
        frontBar_->SetColor({ 1.0f, 0.75f, 0.10f, 0.98f });
    } else {
        frontBar_->SetColor({ 0.15f, 0.95f, 0.35f, 0.95f });
    }
}

void Graph::Draw() {
    backBar_->Draw();
    frontBar_->Draw();
}

void Graph::SetPosition(const KamataEngine::Vector2& position) {
    backBar_->SetPosition(position);
    frontBar_->SetPosition(position);
}

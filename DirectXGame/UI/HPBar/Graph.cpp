#include "Graph.h"

#include <algorithm>
#include <cassert>

using namespace::KamataEngine;

Graph::Graph() {}

Graph::~Graph() {
    delete backBar_;
    delete frontBar_;
}

void Graph::Initialize() {

    texHandle = TextureManager::Load("./Resources/white1x1.png");

    backBar_ = Sprite::Create(texHandle, { 10, 10 }, { 1, 0, 0, 1 });
    frontBar_ = Sprite::Create(texHandle, { 10, 10 }, { 0, 1, 0, 1 });

    // 初期値を100%に設定
    value_ = 1.0f;
}

void Graph::SetValue(float rate) {
    // 0.0～1.0の範囲にクランプ
    value_ = std::clamp(rate, 0.0f, 1.0f);
}

void Graph::Update() {
    // 横幅を value_ に応じて変化させる
    frontBar_->SetSize({ 200 * value_, 10 });
}

void Graph::Draw() {
    backBar_->Draw();   // 赤
    frontBar_->Draw();  // 緑
}
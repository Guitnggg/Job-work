#include "Graph.h"

#include <algorithm>
#include <cassert>

Graph::Graph() {}

void Graph::Initialize() {

	texHandle_ = KamataEngine::TextureManager::Load("./Resources/white1x1.png");

	backBar_.reset(KamataEngine::Sprite::Create(texHandle_, {kBarPosX, kBarPosY}, {1, 0, 0, 1}));
	frontBar_.reset(KamataEngine::Sprite::Create(texHandle_, {kBarPosX, kBarPosY}, {0, 1, 0, 1}));

	// 初期値を100%に設定
	value_ = 1.0f;
}

void Graph::SetValue(float rate) {
	// 0.0～1.0の範囲にクランプ
	value_ = std::clamp(rate, 0.0f, 1.0f);
}

void Graph::Update() {
	// 横幅を value_ に応じて変化させる
	frontBar_->SetSize({kBarWidth * value_, kBarHeight});
}

void Graph::Draw() {
	backBar_->Draw();  // 赤
	frontBar_->Draw(); // 緑
}
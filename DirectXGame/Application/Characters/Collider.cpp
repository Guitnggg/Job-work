#include "Collider.h"

using namespace KamataEngine;

// デバック描画できる球体コライダーの変換情報とモデルを準備する
void Collider::Initialize() {
    // --- ワールド変換初期化 ---
    worldTransform_.Initialize();

    // 半径に応じてスケールを設定
    worldTransform_.scale_ = { radius_, radius_, radius_ };

    // --- モデル生成 ---
    // 球体モデルを使用し、デバッグ用の可視コライダーとして描画する
    model_.reset(Model::CreateSphere());
}

// コライダーの現在位置・スケールからワールド行列を更新する
void Collider::Update() {
    // --- ワールド行列更新 ---
    worldTransform_.UpdateMatrix();
}

// 当たり判定の範囲を確認できるように球体モデルを描画する
void Collider::Draw(const Camera& camera) {
    // デバッグ用途としてコライダー形状を描画
    model_->Draw(worldTransform_, camera, textureHandle_);
}

// 当たり判定半径を更新し、可視化モデルのスケールにも反映する
void Collider::SetRadius(float radius) {
    radius_ = radius;

    // 半径変更時はスケールにも反映する
    worldTransform_.scale_ = { radius_, radius_, radius_ };
}

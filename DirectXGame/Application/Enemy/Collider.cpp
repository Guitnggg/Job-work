#include "Collider.h"

using namespace KamataEngine;

void Collider::Initialize() {

    // ワールド変換の初期化
    worldTransform_.Initialize();

    // モデルの大きさを半径に合わせる
    worldTransform_.scale_ = { radius_, radius_, radius_ };

    // モデルの生成
    model_.reset(Model::CreateSphere());
}

void Collider::Update() {

    // ワールド変換行列の更新
    worldTransform_.UpdateMatrix();
}

void Collider::Draw(KamataEngine::Camera& camera) {

    // ワールド変換をモデルに反映して描画
    model_->Draw(worldTransform_, camera, textureHandle_);
}
#include "CharactorBase.h"

using namespace KamataEngine;

void CharactorBase::Initialize() {
   //  // ワールド変換の初期化
    worldTransform_.Initialize();

    // 移動量の初期化
    velocity_ = { 0.0f, 0.0f, 0.0f };

    // 体力の初期化（必要に応じて派生クラスで SetHP 使用）
    HP_ = 0;

    // コライダーの生成,初期化
    collider_ = std::make_unique<Collider>();
    collider_->Initialize();

    // キャラの位置とコライダーの位置を同期
    collider_->SetTranslate(GetWorldTranslation());
}

void CharactorBase::Update() {
    // ワールド変換行列の更新
    worldTransform_.UpdateMatrix();

    // キャラの位置とコライダーの位置を同期
    collider_->SetTranslate(GetWorldTranslation());

    // コライダーの更新
    collider_->Update();
}

KamataEngine::Vector3 CharactorBase::GetWorldTranslation() {
    Vector3 result;

    // ワールド行列から座標成分を抽出
    result.x = worldTransform_.matWorld_.m[3][0];
    result.y = worldTransform_.matWorld_.m[3][1];
    result.z = worldTransform_.matWorld_.m[3][2];

    return result;
}
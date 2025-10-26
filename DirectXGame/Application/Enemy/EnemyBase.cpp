#include "EnemyBase.h"

using namespace KamataEngine;

void EnemyBase::Initialize() {
    // ワールド変換の初期化
    worldTransform_.Initialize();

    // 移動量の初期化
    velocity_ = { 0.0f, 0.0f, 0.0f };

    // 体力の初期化
    HP_ = 0;

    // コライダーの生成
    collider_ = std::make_unique<Collider>();

    // コライダーの初期化
    collider_->Initialize();

    // キャラの位置とコライダーの位置を同期
    collider_->SetTranslate(GetWorldTranslation());
}

void EnemyBase::Update() {
    // ワールド変換行列の更新
    worldTransform_.UpdateMatrix();

    // キャラの位置とコライダーの位置を同期
    collider_->SetTranslate(GetWorldTranslation());

    // コライダーの更新
    collider_->Update();
}

KamataEngine::Vector3 EnemyBase::GetWorldTranslation() {
    Vector3 result;

    result.x = worldTransform_.matWorld_.m[3][0];
    result.y = worldTransform_.matWorld_.m[3][1];
    result.z = worldTransform_.matWorld_.m[3][2];

    return result;
}
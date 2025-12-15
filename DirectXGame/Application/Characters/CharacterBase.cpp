#include "CharacterBase.h"

using namespace KamataEngine;

void CharacterBase::Initialize() {
   //  // ワールド変換の初期化
    worldTransform_.Initialize();

    // コライダーの生成,初期化
    collider_ = std::make_unique<Collider>();
    collider_->Initialize();

    // キャラの位置とコライダーの位置を同期
    collider_->SetTranslate(GetWorldTranslation());
}

void CharacterBase::Update() {
    // ワールド変換行列の更新
    worldTransform_.UpdateMatrix();

    // キャラの位置とコライダーの位置を同期
    collider_->SetTranslate(GetWorldTranslation());

    // コライダーの更新
    collider_->Update();
}

KamataEngine::Vector3 CharacterBase::GetWorldTranslation() {
    Vector3 result;

    // ワールド行列から座標成分を抽出
    result.x = worldTransform_.matWorld_.m[3][0];
    result.y = worldTransform_.matWorld_.m[3][1];
    result.z = worldTransform_.matWorld_.m[3][2];

    return result;
}
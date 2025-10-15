#include "Player.h"

#include <cassert>
#include <algorithm>
#include <base/TextureManager.h>

using namespace KamataEngine;

Player::Player() {}

Player::~Player() {
    delete worldTransform_;
}

void Player::Initialize(KamataEngine::Camera* camera) {

    // 各初期化処理
    worldTransform_ = new WorldTransform();
    worldTransform_->Initialize();  // ワールド変換データ
    camera_ = camera;               // カメラ（ビューポート）

    model_ = Model::CreateFromOBJ("Player", true); // モデルの生成

    // Playerの初期座標を設定
    worldTransform_->translation_ = { 0.0f,-2.0f,20.0f };

    // 入力を受け付けるように
    input_ = Input::GetInstance();
}

void Player::Update() {
    // ロールのアニメーション
    if (isRolling_) {
        rollFrame_ += 1;
        if (rollFrame_ > rollDurationFrames_) {
            rollFrame_ = rollDurationFrames_;
        }   

        // 位置はイージングで補間
        float t = rollFrame_ / rollDurationFrames_;
        float et = EaseOutCubic(t);
        worldTransform_->translation_.x = rollStartPos_.x + (rollEndPos_.x - rollStartPos_.x) * et;
        worldTransform_->translation_.y = rollStartPos_.y;  // 今回はY/Zは据え置き
        worldTransform_->translation_.z = rollStartPos_.z;

        // 回転はZ軸を1回転（±2π）
        const float twoPi = 6.28318530717958647692f;
        worldTransform_->rotation_.z = rollStartRotZ_ - rollDir_ * twoPi * t;

        if (rollFrame_ >= rollDurationFrames_) {
            // 終了時に回転Zは元の位相へ（見た目ブレ防止で 2πの剰余を落とす）
            worldTransform_->rotation_.z = rollStartRotZ_;
            isRolling_ = false;
        }

        // 行列更新
        worldTransform_->UpdateMatrix();
        return; // ロール中は入力処理をスキップ
    }

    // 移動処理
    if (input_->TriggerKey(DIK_D)) {
        StartRoll(1.0f);
    }

    if (input_->TriggerKey(DIK_A)) {
        StartRoll(-1.0f);
    }

    // 補間後の X に対して
    worldTransform_->translation_.x = std::clamp(worldTransform_->translation_.x, -8.0f, 8.0f);


    // 行列更新
    worldTransform_->UpdateMatrix();
}

void Player::Draw() {
    model_->Draw(*worldTransform_, *camera_);
}

void Player::SetParent(const KamataEngine::WorldTransform* parent) {
    worldTransform_->parent_ = parent;
}

void Player::StartRoll(float dir){
    if (isRolling_) { return; }

    isRolling_ = true;
    rollFrame_ = 0;
    rollDir_ = dir;

    rollStartRotZ_ = worldTransform_->rotation_.z;
    rollStartPos_ = worldTransform_->translation_;

    // 
    rollEndPos_ = rollStartPos_;
    rollEndPos_.x += rollMoveDistance_ * rollDir_;
}

float Player::EaseOutCubic(float t) const{
    float inv = 1.0f - t;
    return 1.0f - inv * inv * inv;
}

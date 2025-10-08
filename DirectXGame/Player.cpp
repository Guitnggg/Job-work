#include "Player.h"

#include <cassert>
#include <algorithm>
#include <base/TextureManager.h>

using namespace KamataEngine;

Player::Player() {}

Player::~Player() {
    delete worldTransform_;
}

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {
    assert(model);

    // 各初期化処理
    worldTransform_ = new WorldTransform();
    worldTransform_->Initialize();  // ワールド変換データ
    camera_ = camera;               // カメラ（ビューポート）

    model_ = Model::CreateFromOBJ("Player", true); // モデルの生成

    // Playerの初期座標を設定
    worldTransform_->translation_ = { 0.0f,0.0f,-30.0f };

    // 入力を受け付けるように
    input_ = Input::GetInstance();
}

void Player::Update() {    
    const float kMoveSpeed = 0.5f;  // 移動速度
    const float kRotSpeed = 0.05f;  // 回転速度

    // バレルロールしながら移動
    if (input_->PushKey(DIK_D)) {
        worldTransform_->rotation_.z -= kRotSpeed;
        worldTransform_->translation_.x += kMoveSpeed;
    }

    if (input_->PushKey(DIK_A)) {
        worldTransform_->rotation_.z += kRotSpeed;
        worldTransform_->translation_.x -= kMoveSpeed;

    }
    
    // 行列更新
    worldTransform_->UpdateMatrix();
}

void Player::Draw() {
    model_->Draw(*worldTransform_, *camera_);
}
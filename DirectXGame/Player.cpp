#include "Player.h"

#include <cassert>
#include <algorithm>
#include <base/TextureManager.h>

using namespace KamataEngine;

Player::Player() {}

Player::~Player() {
    delete worldTransform_;
    delete model_;
}

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {
    assert(model);

    // 各初期化処理
    worldTransform_ = new WorldTransform();
    worldTransform_->Initialize();  // ワールド変換データ
    camera_ = camera;               // カメラ（ビューポート）

    model_ = Model::CreateFromOBJ("Player", true); // モデルの生成

    // Playerの初期座標を設定
    worldTransform_->translation_ = { 0.0f,0.0f,30.0f };

    // 入力を受け付けるように
    input_ = Input::GetInstance();
}

void Player::Update() {
    // 移動
    
    // 回転
    
    // 行列更新
    worldTransform_->UpdateMatrix();
}

void Player::Draw() {
    model_->Draw(*worldTransform_, *camera_);
}
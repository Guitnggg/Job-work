#include "Player.h"

#include <cassert>
#include <algorithm>
#include <base/TextureManager.h>

using namespace KamataEngine;

Player::Player() {}

Player::~Player() {
    delete model_;
}

void Player::Initialize(KamataEngine::Camera* camera) {
    camera_ = camera;

    textureHandle_ = TextureManager::Load("./Resources/Player/player.png");
    model_ = Model::CreateFromOBJ("player");  // playerのモデル指定

    input_ = Input::GetInstance();  // 入力できるように

    worldTransform_.Initialize();
    worldTransform_.rotation_ = { 0, 0, 0 };
    worldTransform_.scale_ = { 2, 2, 2 };

    currentHP_ = maxHP_;
}

void Player::Update() {
    // 上下移動
    if (input_->PushKey(DIK_W)) {
        worldTransform_.translation_.y += moveSpeed_;
    }
    if (input_->PushKey(DIK_S)) {
        worldTransform_.translation_.y -= moveSpeed_;
    }

    // 左右移動
    if (input_->PushKey(DIK_D)) {
        worldTransform_.translation_.x += moveSpeed_;
    }
    if (input_->PushKey(DIK_A)) {
        worldTransform_.translation_.x -= moveSpeed_;
    }

    // 範囲制限
    worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, -35.0f, 35.0f);
    worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, -19.0f, 19.0f);

    worldTransform_.UpdateMatrix();
}

void Player::Draw() {
    model_->Draw(worldTransform_, *camera_, textureHandle_);
}

void Player::TakeDamage(int amount) {
    currentHP_ -= amount;
    if (currentHP_ < 0) {
        currentHP_ = 0;
    }
}

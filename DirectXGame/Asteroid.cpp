#include "Asteroid.h"
#include <cassert>

Asteroid::Asteroid() {}

Asteroid::~Asteroid() {}

void Asteroid::Initialize(KamataEngine::Model* model, const KamataEngine::Vector3 pos, const KamataEngine::Vector3 velocity, KamataEngine::Vector3 rotate) {
    // NULLポインタチェック
    assert(model);

    worldTransform_.Initialize();

    // 引数で受け取った値をメンバ変数にセット
    model_ = model;
    worldTransform_.translation_ = pos;
    velocity_ = velocity;
    rotate_ = rotate;

}

void Asteroid::Update() {
    // 移動
    worldTransform_.translation_.x += velocity_.x;
    worldTransform_.translation_.y += velocity_.y;
    worldTransform_.translation_.z += velocity_.z;

    // 回転
    worldTransform_.rotation_.x += rotate_.x;
    worldTransform_.rotation_.y += rotate_.y;
    worldTransform_.rotation_.z += rotate_.z;

    // 奥行表現
    float distance = std::fabs(worldTransform_.translation_.z);
    float scale = 1.0f / (1.0f + distance * 0.05f);
    worldTransform_.scale_ = { scale, scale, scale };

    // ちょっとだけ加速
    velocity_.z *= 1.01f;

    // 行列更新
    worldTransform_.UpdateMatrix();
}

void Asteroid::Draw(const KamataEngine::Camera& camera) {
    model_->Draw(worldTransform_, camera);
}

void Asteroid::Respawn(const KamataEngine::Vector3 pos, const KamataEngine::Vector3 velocity, KamataEngine::Vector3 rotate){
    worldTransform_.translation_ = pos;
    velocity_ = velocity;
    rotate_ = rotate;
}

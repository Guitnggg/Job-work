#pragma once

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"
#include "Application/Charactor/CharactorBase.h"

class Bullet :public CharactorBase {
public:

    void Initialize()override;

    void Update()override;

    void Draw(KamataEngine::Camera* camera)override;

    void OnCollision(CharactorBase* /*other*/)override;

    bool IsDead() const { return isDead_; }

    // 射出初期化（ワールド座標・向き）
    void FireFrom(const KamataEngine::Vector3& worldPos, const KamataEngine::Vector3& dir);

    // パラメータ
    void SetSpeed(float s) { speed_ = s; }
    void SetLifeTime(float sec) { lifeTimeSec_ = sec; }

private:
    std::unique_ptr<KamataEngine::Model> model_;
    uint32_t textureHandle_ = 0u;

    KamataEngine::Vector3 dir_ = { 0,0,1 }; // +Z方向
    float speed_ = 2.8f;                   // 1フレームあたりの前進量
    float lifeTimeSec_ = 3.0f;            // 弾の寿命
    float t_ = 0.0f;                      // 経過時間
    bool isDead_ = false;
};
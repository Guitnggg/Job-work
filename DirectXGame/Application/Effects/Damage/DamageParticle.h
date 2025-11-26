#pragma once

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"
#include "math/Vector3.h"
#include "math/MathUtility.h"

class DamageParticle {
public:

    void Initialize(KamataEngine::Model* model,
        const KamataEngine::Vector3& pos,
        const KamataEngine::Vector3& vel,
        float lifeTime,
        float startScale,
        float endScale
    );

    void Update(float dt);

    void Draw(KamataEngine::Camera* camera);

    bool IsFinished()const { return currentTime_ >= lifeTime_; }

private:
    KamataEngine::WorldTransform worldTransform_;
    KamataEngine::Model* model_ = nullptr;
    KamataEngine::Vector3 velocity_{};

    float lifeTime_ = 0.0f;
    float currentTime_ = 0.0f;
    float startScale_ = 1.0f;
    float endScale_ = 0.0f;
};


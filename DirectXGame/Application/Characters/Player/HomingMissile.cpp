#include "HomingMissile.h"

#include "Application/Utility/MyMath/MyMath.h"
#include "base/TextureManager.h"

#include <cmath>

using namespace KamataEngine;

void HomingMissile::Initialize() {
    CharacterBase::Initialize();
    model_.reset(Model::CreateFromOBJ("Missile AGM-65", true));
    textureHandle_ = TextureManager::Load("Resources/missile/Texture.png");
    worldTransform_.scale_ = kMissileScale;
    worldTransform_.UpdateMatrix();

    if (collider_) {
        collider_->SetRadius(kColliderRadius);
        collider_->SetTranslate(GetWorldTranslation());
        collider_->Update();
    }

    elapsedTimeSec_ = 0.0f;
    isDead_ = false;
    target_ = nullptr;
    velocity_ = { 0.0f, 0.0f, 1.0f };
}

void HomingMissile::FireFrom(const Vector3& worldPos, CharacterBase* target) {
    worldTransform_.translation_ = worldPos;
    target_ = target;
    if (target_) {
        velocity_ = MyMath::Normalize(MyMath::Subtract(target_->GetWorldTranslation(), worldPos));
    }
    else {
        velocity_ = { 0.0f, 0.0f, 1.0f };
    }
    worldTransform_.UpdateMatrix();
}

void HomingMissile::Update() {
    if (isDead_) {
        return;
    }

    elapsedTimeSec_ += kFixedDeltaTime;
    if (elapsedTimeSec_ >= kLifeTimeSec) {
        isDead_ = true;
        return;
    }

    if (target_ && !target_->IsDead()) {
        const Vector3 targetDir = MyMath::Normalize(MyMath::Subtract(target_->GetWorldTranslation(), GetWorldTranslation()));
        velocity_ = MyMath::Normalize(MyMath::Add(MyMath::Multiply(velocity_, 1.0f - kTurnRate), MyMath::Multiply(targetDir, kTurnRate)));
    }

    worldTransform_.translation_ = MyMath::Add(worldTransform_.translation_, MyMath::Multiply(velocity_, kSpeed));
    worldTransform_.rotation_.y = std::atan2(velocity_.x, velocity_.z);
    worldTransform_.UpdateMatrix();

    if (collider_) {
        collider_->SetTranslate(GetWorldTranslation());
        collider_->Update();
    }
}

void HomingMissile::Draw(const Camera* camera) {
    if (!camera || isDead_ || !model_) {
        return;
    }

    if (textureHandle_ != 0u) {
        model_->Draw(worldTransform_, *camera, textureHandle_);
    }
    else {
        model_->Draw(worldTransform_, *camera);
    }

}

void HomingMissile::OnCollision(CharacterBase*) { isDead_ = true; }
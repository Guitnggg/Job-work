#include "Player.h"

#include <cassert>
#include <algorithm>
#include <base/TextureManager.h>

using namespace KamataEngine;

Player::~Player() {
    delete worldTransform_;
}

void Player::Initialize(KamataEngine::Camera* camera) {

    // 各初期化処理
    worldTransform_ = new WorldTransform();
    worldTransform_->Initialize();  // ワールド変換データ
    camera_ = camera;               // カメラ（ビューポート）

    model_ = Model::CreateFromOBJ("Player", true); // モデルの生成

    audio_ = Audio::GetInstance();
    // ★ ここは必要に応じてパスを合わせてください
    seExplosion_ = audio_->LoadWave("./Resources/SE/Explosion.wav");

    // Playerの初期座標を設定
    worldTransform_->translation_ = { 0.0f,-2.0f,20.0f };

    // 入力を受け付けるように
    input_ = Input::GetInstance();

    hp_ = 100;
    isDead_ = false;
    isExploding_ = false;
    isExplosionFinished_ = false;
    explosionFrame_ = 0;
}

void Player::Update() {
    // 爆発中の演出を優先
    if (isExploding_) {
        UpdateExplosion_();
        worldTransform_->UpdateMatrix();
        return; // 爆発中は操作不可
    }
    if (isDead_) {
        // 消滅後は何もしない（描画もしない）
        return;
    }

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
    if (input_->TriggerKey(DIK_D)) { StartRoll(1.0f); }

    if (input_->TriggerKey(DIK_A)) { StartRoll(-1.0f); }

    // 補間後の X に対して
    worldTransform_->translation_.x = std::clamp(worldTransform_->translation_.x, -8.0f, 8.0f);

    // 行列更新
    worldTransform_->UpdateMatrix();
}

void Player::Draw() {
    if (isExplosionFinished_) { return; }
    model_->Draw(*worldTransform_, *camera_);
}

void Player::SetParent(const KamataEngine::WorldTransform* parent) {
    worldTransform_->parent_ = parent;
}

KamataEngine::Vector3 Player::GetWorldTranslation() const {
    KamataEngine::Vector3 p;

    p.x = worldTransform_->matWorld_.m[3][0];
    p.y = worldTransform_->matWorld_.m[3][1];
    p.z = worldTransform_->matWorld_.m[3][2];

    return p;
}

void Player::StartRoll(float dir) {
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

float Player::EaseOutCubic(float t) const {
    float inv = 1.0f - t;
    return 1.0f - inv * inv * inv;
}

void Player::Damage(int amount) {
    if (isExploding_ || isDead_) { return; }
    hp_ -= amount;
    if (hp_ <= 0) {
        Kill();
    }
}

void Player::Kill() {
    if (isExploding_ || isDead_) { return; }
    // 爆発開始
    isExploding_ = true;
    explosionFrame_ = 0;

    if(audio_&&seExplosion_>=0){
        audio_->PlayWave(seExplosion_);
    }
}

void Player::UpdateExplosion_() {
   // 0.0 → 1.0
    float t = static_cast<float>(explosionFrame_) / static_cast<float>(explosionDurationFrames_);
    if (t < 0.0f) t = 0.0f; 
    if (t > 1.0f) t = 1.0f;

    // 縮小：1.0 → 0.0（EaseOutで最後スッと消える）
    float et = EaseOutCubic(t);              // 0→1（後半速く）
    float scaleMul = 1.0f - et;        // 1→0 に縮小
    if (scaleMul < 0.0f) scaleMul = 0.0f;   // ← ここで下限クランプ

    worldTransform_->translation_.y -= 0.1f; // 

    // 回転はそのまま付与（お好みで速度調整OK）
    worldTransform_->rotation_.y += 0.2f;

    explosionFrame_++;
    if (explosionFrame_ >= explosionDurationFrames_) {
        isExploding_ = false;
        isDead_ = true;
        isExplosionFinished_ = true;

        // 最終的に描画しない（消滅）
        worldTransform_->scale_ = { 0.0f, 0.0f, 0.0f };
    }
}
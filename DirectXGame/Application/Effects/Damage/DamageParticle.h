#pragma once

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"
#include "math/Vector3.h"
#include "math/MathUtility.h"

/// <summary>
/// ダメージ発生時の一瞬の破片・火花・光などを表現する軽量パーティクル。
/// 速度・寿命・開始スケール→終了スケールの補間により、
/// 出現後に飛散しながら徐々に消える演出を行う。
/// </summary>
class DamageParticle {
public:

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="model">描画に使用するモデル</param>
    /// <param name="pos">パーティクル発生位置</param>
    /// <param name="vel">悲惨方向＆速度</param>
    /// <param name="lifeTime">寿命（秒）</param>
    /// <param name="startScale">出現直後のスケール</param>
    /// <param name="endScale">寿命末期のスケール</param>
    void Initialize(KamataEngine::Model* model,
        const KamataEngine::Vector3& pos,
        const KamataEngine::Vector3& vel,
        float lifeTime,
        float startScale,
        float endScale
    );

    /// <summary>
    /// 更新処理
    /// </summary>
    /// <param name="dt">経過時間（秒）</param>
    void Update(float dt);

    /// <summary>
    /// 描画処理
    /// </summary>
    /// <param name="camera">描画に使用するカメラ</param>
    void Draw(KamataEngine::Camera* camera);

    /// <summary>
    /// パーティクルの寿命が尽きているか判定
    /// </summary>
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


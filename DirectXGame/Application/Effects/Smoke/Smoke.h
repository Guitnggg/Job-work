#pragma once

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"
#include "math/Vector3.h"

/// <summary>
/// 被弾・爆発・推進エンジンなどの演出に使用する残留系パーティクル（煙）。
/// 時間経過とともに移動し、スケールが変化し、寿命で消滅する。
/// DamageParticle より長く残るふわっとした演出向き。
/// </summary>
class Smoke{
public:

    /// <summary>
    /// コンストラクタ / デストラクタ 
    /// </summary>
    Smoke() = default;
    ~Smoke() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="model">描画に使用するモデル</param>
    /// <param name="position">初期位置</param>
    /// <param name="velocity">初期速度</param>
    /// <param name="lifeTime">寿命（秒）</param>
    /// <param name="startScale">開始スケール</param>
    /// <param name="endScale">終了スケール</param>
    void Initialize(KamataEngine::Model* model,
        const KamataEngine::Vector3& position,
        const KamataEngine::Vector3& velocity,
        float lifeTime,
        float startScale,
        float endScale
    );

    /// <summary>
    /// 更新処理
    /// </summary>
    /// <param name="dt">デルタタイム（秒）</param>
    void Update(float dt);

    /// <summary>
    /// 描画処理
    /// </summary>
    /// <param name="camera">描画に使用するカメラ</param>
    void Draw(KamataEngine::Camera* camera);

    /// <summary>
    /// 終了しているか
    /// </summary>
    bool IsFinished()const { return isFinished_; }

private:
    KamataEngine::WorldTransform worldTransform_; // 変換情報
    KamataEngine::Model* model_ = nullptr;        // モデル（非所有）

    KamataEngine::Vector3 velocity_ = { 0.0f, 0.0f, 0.0f }; // 速度

    float life_ = 0.0f;  // 経過時間
    float maxLife_ = 1.0f;  // 寿命
    float startScale_ = 1.0f;
    float endScale_ = 0.0f;

    bool isFinished_ = false;
};


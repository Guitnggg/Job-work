#pragma once

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"
#include "Application/Characters/CharacterBase.h"

/// <summary>
/// 単純な直進弾を表すクラス。
/// 発射位置と方向を与えることで飛行し、
/// 一定距離または寿命経過または衝突で消滅する。
/// </summary>
class Bullet :public CharacterBase {
public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize()override;

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update()override;

    /// <summary>
    /// 描画処理
    /// </summary>
    /// <param name="camera">描画に使用するカメラ</param>
    void Draw(KamataEngine::Camera* camera)override;

    /// <summary>
    /// 当たり判定
    /// </summary>
    /// <param name="">衝突相手</param>
    void OnCollision(CharacterBase* /*other*/)override;

    /// <summary>
    /// 死亡したかどうか
    /// </summary>
    bool IsDead() const { return isDead_; }

    /// <summary>
    /// 発射処理
    /// </summary>
    /// <param name="worldPos">発射位置（ワールド座標）</param>
    /// <param name="dir">進行方向（正規化されたベクトル）</param>
    void FireFrom(const KamataEngine::Vector3& worldPos, const KamataEngine::Vector3& dir);

public:  // パラメータ
    /// <summary>
    /// 飛行速度の設定（１フレーム当たりの移動距離）
    /// </summary>
    void SetSpeed(float s) { speed_ = s; }

    /// <summary>
    /// 寿命の設定（秒）
    /// </summary>
    void SetLifeTime(float sec) { lifeTimeSec_ = sec; }

private:
    std::unique_ptr<KamataEngine::Model> model_;
    uint32_t textureHandle_ = 0u;

    KamataEngine::Vector3 dir_ = { 0,0,1 }; // +Z方向
    float speed_ = 2.8f;                   // 1フレームあたりの前進量
    float lifeTimeSec_ = 3.0f;            // 弾の寿命
    float t_ = 0.0f;                      // 経過時間
    bool isDead_ = false;

    KamataEngine::Vector3 startPos_ = { 0,0,0 };
    float maxDistance_ = 200.0f; // Playerからの最大距離
};
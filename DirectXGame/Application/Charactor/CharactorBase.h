#pragma once

#include "3d/WorldTransform.h"
#include "3d/Camera.h"
#include "math/Vector3.h"
#include <memory>

#include "Collider.h"

class CharactorBase {
public:

    /// <summary>
    /// 仮想デストラクタ
    /// </summary>
    virtual ~CharactorBase() = default;

    /// <summary>
    /// 初期化
    /// </summary>
    virtual void Initialize();

    /// <summary>
    /// 更新
    /// </summary>
    virtual void Update();

    /// <summary>
    /// 描画
    /// </summary>
    /// <param name="camera"></param>
    virtual void Draw(KamataEngine::Camera* camera) = 0;

    /// <summary>
    /// 当たり判定時の処理
    /// </summary>
    /// <param name="enemy"></param>
    virtual void OnCollision(CharactorBase* enemy) = 0;

public:  /// === Getters === ///

    /// <summary>
    /// ワールド座標の取得
    /// </summary>
    /// <returns>ワールド座標</returns>
    KamataEngine::Vector3 GetWorldTranslation();

    /// <summary>
    /// 座標の取得
    /// </summary>
    /// <returns>modelの座標</returns>
    KamataEngine::Vector3 GetTranslation() { return worldTransform_.translation_; }

    /// <summary>
    /// 角度の取得
    /// </summary>
    /// <returns>modelの向き</returns>
    KamataEngine::Vector3 GetRotation() { return worldTransform_.rotation_; }

    /// <summary>
    /// 回転の取得
    /// </summary>
    /// <returns>modelの回転</returns>
    KamataEngine::Vector3 GetRotate() { return worldTransform_.rotation_; }

    /// <summary>
    /// コライダーの取得
    /// </summary>
    /// <returns>コライダー</returns>
    std::unique_ptr<Collider>& GetCollider() { return collider_; }

    /// <summary>
    /// HPの取得
    /// </summary>
    /// <returns></returns>
    int GetHP() { return HP_; }

    /// <summary>
    /// 最大HPの取得
    /// </summary>
    /// <returns></returns>
    int GetMaxHP() { return maxHp_; }

public:  /// === Setters === ///

    /// <summary>
    /// 座標の設定
    /// </summary>
    /// <param name="translate">modelの座標</param>
    void SetTranslate(KamataEngine::Vector3 translate) { worldTransform_.translation_ = translate; }

    /// <summary>
    /// 回転の設定
    /// </summary>
    /// <param name="rotate">modelの向き</param>
    void SetRotate(KamataEngine::Vector3 rotate) { worldTransform_.rotation_ = rotate; }

    /// <summary>
    /// 大きさの設定
    /// </summary>
    /// <param name="scale">modelの大きさ</param>
    void SetScale(KamataEngine::Vector3 scale) { worldTransform_.scale_ = scale; }

    /// <summary>
    /// HPの設定
    /// </summary>
    /// <param name="hp"></param>
    void SetHP(int hp) { HP_ = hp; }

protected:

    // ワールド変換情報
    KamataEngine::WorldTransform worldTransform_;

    // 速度
    KamataEngine::Vector3 velocity_ = { 0.0f,0.0f,0.0f };

    // 当たり判定
    std::unique_ptr<Collider> collider_;

    // 体力
    int HP_ = 0;

    // 最大体力
    int maxHp_ = 0;
};
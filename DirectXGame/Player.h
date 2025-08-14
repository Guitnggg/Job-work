#pragma once

#include <3d/WorldTransform.h>
#include <3d/Model.h>
#include <3d/Camera.h>
#include <input/Input.h>

class Player {
public:

    /// <summary>
    /// インストラクタ
    /// </summary>
    Player();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~Player();

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize(KamataEngine::Camera* camera);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

public:

    /// <summary>
    /// HPを減らす
    /// </summary>
    void TakeDamage(int amount);

    /// <summary>
    /// 現在のHPを取得
    /// </summary>
    int GetHP() const { return currentHP_; }

    /// <summary>
    /// 死亡しているかどうか
    /// </summary>
    bool IsDead() const { return currentHP_ <= 0; }

    /// <summary>
    /// Getter
    /// </summary>
    const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

private:

    KamataEngine::WorldTransform worldTransform_;  // ワールド変換
    KamataEngine::Camera* camera_;                 // カメラ
    KamataEngine::Model* model_ = nullptr;         // モデル
    KamataEngine::Input* input_ = nullptr;         // 入力

    uint32_t textureHandle_ = 0u;  // テクスチャハンドル

    float moveSpeed_ = 0.5f;  // 移動速度

    int maxHP_ = 100;      // 最大HP
    int currentHP_ = 100;  // 現在のHP
};


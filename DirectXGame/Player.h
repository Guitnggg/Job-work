#pragma once

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"
#include "input/Input.h"

class Player {
public:

    /// <summary>
    /// コンストラクタ
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
    /// 親となるワールドトランスフォーム
    /// </summary>
    void SetParent(const KamataEngine::WorldTransform* parent);

private:

    KamataEngine::WorldTransform* worldTransform_;
    KamataEngine::Camera* camera_;
    KamataEngine::Model* model_ = nullptr;
    KamataEngine::Input* input_ = nullptr;

    const float kMoveSpeed = 0.5f;  // 移動速度
    const float kRotSpeed = 0.05f;  // 回転速度
};


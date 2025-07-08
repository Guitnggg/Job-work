#pragma once

#include "3D/WorldTransform.h"
#include "3D/Camera.h"

using namespace KamataEngine;

class GameScene
{
public:

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

private:

    KamataEngine::WorldTransform* worldTransform_;
    KamataEngine::Camera* camera_;
};

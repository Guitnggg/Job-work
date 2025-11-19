#pragma once

#include <3d/WorldTransform.h>
#include <3d/Model.h>
#include <3d/Camera.h>

class Skydome {
public:

    // 初期化
    void Initialize(KamataEngine::Camera* camera);

    // 更新
    void Update();

    // 描画
    void Draw();

private:

    KamataEngine::WorldTransform worldTransform_;
    KamataEngine::Model* model_ = nullptr;
    KamataEngine::Camera* camera_ = nullptr;

};
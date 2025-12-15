#include "RailCamera.h"

#include "Application/Utility/MyMath/MyMath.h"

using namespace KamataEngine;

RailCamera::RailCamera() {}

RailCamera::~RailCamera() {}

void RailCamera::Initialize() {
    // ワールド変換データ初期化
    worldTransform_.Initialize();
    worldTransform_.translation_ = { 0,0,kInitialZ };

    // カメラ生成
    camera_ = new Camera();
    camera_->Initialize();
}

void RailCamera::Update() {
    // ワールド変換データの座標の数値を加算したりする（移動）
    worldTransform_.translation_.z += kMoveSpeedZ;

    // ワールド変換データの行列を更新
    worldTransform_.UpdateMatrix();

    // カメラの位置をワールド変換データの位置に合わせる
    camera_->matView = MyMath::Inverse(worldTransform_.matWorld_);
}
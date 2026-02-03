#include "Skydome.h"

using namespace KamataEngine;

void Skydome::Initialize(KamataEngine::Camera* camera) {
    // 天球モデルを読み込み
    // ※ Model の生成・破棄はエンジン側で管理される想定
    model_ = Model::CreateFromOBJ("skydome");

    // ワールド変換を初期化
    worldTransform_.Initialize();

    // 描画に使用するカメラ参照を保持（所有権は持たない）
    camera_ = camera;
}

void Skydome::Update() {
    // 天球は常時固定表示のため、現状更新処理なし
}

void Skydome::Draw() {
    // ワールド変換とカメラを使用して天球を描画
    model_->Draw(worldTransform_, *camera_);
}

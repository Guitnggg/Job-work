#include "Skydome.h"

using namespace KamataEngine;

void Skydome::Initialize(KamataEngine::Camera* camera) {
    // Skydome.objからモデル読み込み
    model_ = Model::CreateFromOBJ("skydome");
    // ワールド変換を初期化
    worldTransform_.Initialize();   
    // カメラ参照を保存
    camera_ = camera;
}

void Skydome::Update() {
    // 現状特に処理なし
}

void Skydome::Draw() {
    // ワールド変換とカメラ行列を使用してモデルを描画
    model_->Draw(worldTransform_, *camera_);
}
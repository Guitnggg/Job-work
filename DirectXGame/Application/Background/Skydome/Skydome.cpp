#include "Skydome.h"

using namespace KamataEngine;

// 天球モデルを読み込み、描画に使うカメラ参照とワールド変換を初期化する
void Skydome::Initialize(KamataEngine::Camera* camera) {
	// 天球モデルを読み込み
	// ※ Model の生成・破棄はエンジン側で管理される想定
	model_ = Model::CreateFromOBJ("skydome");

	// ワールド変換を初期化
	worldTransform_.Initialize();

	// 描画に使用するカメラ参照を保持（所有権は持たない）
	camera_ = camera;
}

// 天球は背景として固定表示するため、必要になった場合の更新処理
void Skydome::Update() {
	// 天球は常時固定表示のため、現状更新処理なし
}

// 保持しているモデル・ワールド変換・カメラを使って背景天球を描画する
void Skydome::Draw() {
	// ワールド変換とカメラを使用して天球を描画
	model_->Draw(worldTransform_, *camera_);
}
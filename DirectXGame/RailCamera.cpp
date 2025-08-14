#include "RailCamera.h"
#include <2d/ImGuiManager.h>

#include "myMath.h"

void RailCamera::Initialize()
{
	// ワールドトランスフォームの初期設定
	worldTransform_.Initialize();

	worldTransform_.translation_ = { 0,0,-50 };

	// カメラの初期化	
	camera_ = new Camera;
	camera_->Initialize();
}

void RailCamera::Update()
{
	// ワールドトランスフォームの座標の数値を加算したりする【移動】
	worldTransform_.translation_.z += 0.1f;

	// ワールドトランスフォームの角度の数値を加算したりする【回転】
	/*worldTransform_.rotation_.y += 0.001f;*/

	// ワールドトランスフォームのワールド行列再計算
	worldTransform_.UpdateMatrix();

	// カメラオブジェクトのワールド行列からビュー行列を計算する
	camera_->matView = Inverse(worldTransform_.matWorld_);

#ifdef _DEBUG

	// カメラの座標を画面表示する
	ImGui::Begin("Camera");

	// カメラのtranslationを表示
	ImGui::SliderFloat3("translation", &worldTransform_.translation_.x, -100.0f, 100.0f);

	// カメラのrotationを表示
	ImGui::SliderFloat3("rotation", &worldTransform_.rotation_.x, -100.0f, 100.0f);

	ImGui::End();

#endif // DEBUG

}

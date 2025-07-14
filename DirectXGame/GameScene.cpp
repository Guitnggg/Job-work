#include "GameScene.h"

using namespace KamataEngine;

GameScene::GameScene(){}

GameScene::~GameScene()
{
}

void GameScene::Initialize()
{
	// 各初期化処理
	dxCommon_ = DirectXCommon::GetInstance();  // DirectX
	input_ = Input::GetInstance();             // 入力
	audio_ = Audio::GetInstance();             // サウンド

	worldTransform_.Initialize();              // ワールド変換データ
	camera_.Initialize();                      // カメラ（ビューポート）

	// モデルの生成
	model_ = Model::Create();

}

void GameScene::Update()
{
}

void GameScene::Draw()
{

}

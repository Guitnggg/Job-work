#include <Windows.h>
#include <KamataEngine.h>

#include "SceneManager.h"
#include "TitleScene.h"

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

    // KamataEngineの初期化
    KamataEngine::Initialize(L"LE3C_14_タカキ_ケンゴ_タイトル");

    // DirectXCommonのインスタンスの取得
    DirectXCommon* dxCommon = DirectXCommon::GetInstance();

    //// 各シーンの初期化処理 ===============
    SceneManager sceneManager;
    sceneManager.SetInitialScene(new TitleScene());

    //===============
    // Mainループ
    //===============
    while (true) {

        // エンジンの更新
        if (KamataEngine::Update()) {
            break;
        }

        //=============
        // 更新処理
        //=============

        sceneManager.Update();

        //=============
        /// 描画処理 
        //=============

        // 描画処理開始       
        dxCommon->PreDraw();

        sceneManager.Draw();

        //描画処理終了       
        dxCommon->PostDraw();
    }

    // KamataEngineの終了 
    KamataEngine::Finalize();

    return 0;
}

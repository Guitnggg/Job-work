#include <Windows.h>
#include <memory>
#include <KamataEngine.h>

#include "Scenes/SceneManager.h"
#include "Scenes/Title/TitleScene.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

    // KamataEngineの初期化
    KamataEngine::Initialize(L"LE3C_10_タカキ_ケンゴ_AstroWing");
    {
        // ゲームプレイ中はOSカーソルを非表示にする
        //while (ShowCursor(FALSE) >= 0) {}

        // DirectXCommonのインスタンスの取得
        KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();

        // 各シーンの初期化処理 ===============
        SceneManager sceneManager;
        sceneManager.SetInitialScene(std::make_unique<TitleScene>());

        //===============
        // Mainループ
        //===============
        while (true) {

            // エンジンの更新
            if (KamataEngine::Update()) {
                break;
            }

#ifdef USE_IMGUI
            KamataEngine::ImGuiManager::GetInstance()->Begin();
#endif

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

#ifdef USE_IMGUI
            KamataEngine::ImGuiManager::GetInstance()->End();
            KamataEngine::ImGuiManager::GetInstance()->Draw();
#endif

            //描画処理終了
            dxCommon->PostDraw();
        }
    }

    // KamataEngineの終了 
    KamataEngine::Finalize();

    // アプリ終了時にOSカーソルを元に戻す
    while (ShowCursor(TRUE) < 0) {}

    return 0;
}

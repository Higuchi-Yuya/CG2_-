#pragma region 読み込むヘッダー

#define DIRECTINPUT_VERSION 0x0800 //DirectInputのバージョン指定
#include<dinput.h>

#pragma comment(lib,"d3dcompiler.lib")
#include<Windows.h>
#include <tchar.h>
#include<iostream>

#include<dxgi1_6.h>
#include<cassert>

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include <vector>


#include <math.h>
#include "Mesh.h"




using namespace std;


#pragma endregion
const float PI = 3.141592f;

#pragma region おまじない
// @brief コンソール画面にフォーマット付き文字列の表示
// @param format フォーマット(%dとか%fとかの)
// @param 可変長引数
// @remarks この関数はデバック用です。デバッグ時にしか動作しません
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//ウィンドウが破壊されたら呼ばれる
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);//OSに対して「このアプリはもう終わる」と伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
#pragma endregion

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

#pragma region WindowsAPI初期化処理

	// ウィンドウ横幅
	const int window_width = 1280;
	// ウィンドウ縦幅
	const int window_height = 720;


	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;	//ウィンドウプロシージャを設定
	w.lpszClassName = _T("DX12Sample");			//ウィンドウクラス名
	w.hInstance = GetModuleHandle(nullptr);		//ウィンドウハンドル
	w.hCursor = LoadCursor(NULL, IDC_ARROW);	//カーソル指定

	//ウィンドウクラスをOSに登録する
	RegisterClassEx(&w);
	//ウィンドウサイズ{X座標　Y座標　横幅　縦幅}
	RECT wrc = { 0,0,window_width,window_height };
	//関数を使ってウィンドウのサイズを自動で補正する
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindow(w.lpszClassName,//クラス名指定
		_T("LE2B_18_ヒグチ_ユウヤ_AL3"),     //タイトルバーの文字
		WS_OVERLAPPEDWINDOW,			     //タイトルバーと境界線があるウィンドウ
		CW_USEDEFAULT,					     //表示x座標はOSにお任せ
		CW_USEDEFAULT,					     //表示y座標はOSにお任せ
		wrc.right - wrc.left,			     //ウィンドウ幅
		wrc.bottom - wrc.top,			     //ウィンドウ高
		nullptr,						     //親ウィンドウハンドル
		nullptr,						     //メニューハンドル
		w.hInstance,					     //呼び出しアプリケーションハンドル
		nullptr);						     //追加パラメーター(オプション)

	//ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};

#pragma endregion

#pragma region DirectX初期化処理

	////////////////////////////////////////////////////
	//-------------DirectX12初期化処理ここから-----------//
	//////////////////////////////////////////////////

#ifdef _DEBUG
//デバッグレイヤーをオンに
	ID3D12Debug* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
#endif

	HRESULT result;
	ID3D12Device* device = nullptr;
	IDXGIFactory6* dxgiFactory = nullptr;
	IDXGISwapChain4* swapChain = nullptr;
	ID3D12CommandAllocator* commandAllocator = nullptr;
	ID3D12GraphicsCommandList* commandList = nullptr;
	ID3D12CommandQueue* commandQueue = nullptr;
	ID3D12DescriptorHeap* rtvHeap = nullptr;

	//DXGIファクトリーの生成
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//アダプターの列挙用
	std::vector<IDXGIAdapter4*>adapters;
	//ここに特定の名前を持つアダプターオブジェクトが入る
	IDXGIAdapter4* tmpAdapter = nullptr;

	//パフォーマンスが高い物から順に、全てのアダプターを列挙する
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND; i++) {
		//動的配列に追加する
		adapters.push_back(tmpAdapter);
	}
	//妥当なアダプタを選別する
	for (size_t i = 0; i < adapters.size(); i++) {
		DXGI_ADAPTER_DESC3 adapterDesc;
		//アダプターの情報を取得する
		adapters[i]->GetDesc3(&adapterDesc);

		//ソフトウェアデバイスを回避
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//デバイスを採用してループを抜ける
			tmpAdapter = adapters[i];
			break;
		}
	}

	//対応レベルの配列
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (size_t i = 0; i < _countof(levels); i++) {
		//採用したアダプターでデバイスを生成
		result = D3D12CreateDevice(tmpAdapter, levels[i], IID_PPV_ARGS(&device));
		if (result == S_OK) {
			//デバイスを生成できた時点でループを抜ける
			featureLevel = levels[i];
			break;
		}
	}

	//コマンドアローケータを生成
	result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(result));

	//コマンドリストを生成
	result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(result));

	//コマンドキューの設定
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

	//コマンドキューを生成
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));

	//スワップチェーンの設定
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = 1280;
	swapChainDesc.Height = 720;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;			//色情報の書式
	swapChainDesc.SampleDesc.Count = 1;							//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;			//バックバッファ用
	swapChainDesc.BufferCount = 2;								//バッファ数を2つに設定
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//フリップ後は破壊
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//スワップチェーンの生成
	result = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, (IDXGISwapChain1**)&swapChain);
	assert(SUCCEEDED(result));

	//デスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;			//レンダーターゲットビュー
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;		//裏表の2つ

	//デスクリプタヒープの生成
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	//バックバッファ
	std::vector<ID3D12Resource*>backBuffers;
	backBuffers.resize(swapChainDesc.BufferCount);

	//スワップチェーンの全てのバッファについて処理する
	for (size_t i = 0; i < backBuffers.size(); i++) {
		//スワップチェーンからバッファを取得
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		//デスクリプタヒープのハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		//裏か表かでアドレスがずれる
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//レンダーターゲットビューの設定
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		//シェーダーの計算結果をSRGBに変換して書き込む
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//レンダーターゲットビューの生成
		device->CreateRenderTargetView(backBuffers[i], &rtvDesc, rtvHandle);
	}

	//フェンスの生成
	ID3D12Fence* fence = nullptr;
	UINT64 fenceVal = 0;

	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	//DirectInputの初期化
	IDirectInput8* directInput = nullptr;
	result = DirectInput8Create(
		w.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	//キーボードデバイスの生成
	IDirectInputDevice8* keyboard = nullptr;
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	//入力データ形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);//標準形式
	assert(SUCCEEDED(result));



	/////////////////////////////////////////////////////////
	//--------------DirectX12初期化処理　ここまで-------------//
	///////////////////////////////////////////////////////
#pragma endregion

	Mesh mesh[10];

	Mesh::Vertex vertices[] = {
		// x      y     z       u     v
		{{-0.12f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // 左下
		{{-0.12f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // 左上
		{{+0.12f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // 右下
		{{+0.12f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // 右上
	};

	Mesh::Vertex vertices2[] = {
		// x      y     z       u     v
		{{-0.24f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // 左下
		{{-0.24f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // 左上
		{{-0.12f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // 右下
		{{-0.12f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // 右上
	};

	Mesh::Vertex vertices3[] = {
		// x      y     z       u     v
		{{-0.42f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // 左下
		{{-0.42f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // 左上
		{{-0.32f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // 右下
		{{-0.32f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // 右上
	};

	Mesh::Vertex vertices4[] = {
		// x      y     z       u     v
		{{-0.52f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // 左下
		{{-0.52f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // 左上
		{{-0.42f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // 右下
		{{-0.42f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // 右上
	};

	Mesh::Vertex vertices5[] = {
		// x      y     z       u     v
		{{-0.62f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // 左下
		{{-0.62f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // 左上
		{{-0.52f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // 右下
		{{-0.52f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // 右上
	};

	Mesh::Vertex vertices6[] = {
		// x      y     z       u     v
		{{-0.72f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // 左下
		{{-0.72f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // 左上
		{{-0.62f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // 右下
		{{-0.62f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // 右上
	};

	Mesh::Vertex vertices7[] = {
		// x      y     z       u     v
		{{-0.82f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // 左下
		{{-0.82f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // 左上
		{{-0.72f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // 右下
		{{-0.72f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // 右上
	};

	Mesh::Vertex vertices8[] = {
		// x      y     z       u     v
		{{+0.12f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // 左下
		{{+0.12f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // 左上
		{{+0.32f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // 右下
		{{+0.32f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // 右上
	};

	Mesh::Vertex vertices9[] = {
		// x      y     z       u     v
		{{+0.32f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // 左下
		{{+0.32f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // 左上
		{{+0.52f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // 右下
		{{+0.52f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // 右上
	};

	Mesh::Vertex vertices10[] = {
		// x      y     z       u     v
		{{+0.52f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // 左下
		{{+0.52f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // 左上
		{{+0.62f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // 右下
		{{+0.62f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // 右上
	};
	unsigned short indices[] = {
		0, 1, 2, // 三角形1つ目
		1, 2, 3, // 三角形2つ目
	};

	int vertices_count;
	int indices_count;

	vertices_count = _countof(vertices);
	indices_count = _countof(indices);

	mesh[0].Mesh_Initialization(device, vertices, indices, _countof(vertices), indices_count);
	mesh[1].Mesh_Initialization(device, vertices2, indices, _countof(vertices2), indices_count);
	mesh[2].Mesh_Initialization(device, vertices3, indices, _countof(vertices3), indices_count);
	mesh[3].Mesh_Initialization(device, vertices4, indices, _countof(vertices4), indices_count);
	mesh[4].Mesh_Initialization(device, vertices5, indices, _countof(vertices5), indices_count);
	mesh[5].Mesh_Initialization(device, vertices6, indices, _countof(vertices6), indices_count);
	mesh[6].Mesh_Initialization(device, vertices7, indices, _countof(vertices7), indices_count);
	mesh[7].Mesh_Initialization(device, vertices8, indices, _countof(vertices8), indices_count);
	mesh[8].Mesh_Initialization(device, vertices9, indices, _countof(vertices9), indices_count);
	mesh[9].Mesh_Initialization(device, vertices10, indices, _countof(vertices10), indices_count);
	
	//ゲームループ
	while (true) {
#pragma region ウィンドウメッセージ処理

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//アプリケーションが終わる時にmessageがWM_QUITになる
		if (msg.message == WM_QUIT) {
			break;
		}


#pragma endregion

#pragma region DirectX毎フレーム処理
		/////////////////////////////////////////////////////
		//----------DireceX毎フレーム処理　ここから------------//
		///////////////////////////////////////////////////

		// キーボード情報の取得開始
		keyboard->Acquire();
		// 全キーの入力状態を取得する
		BYTE key[256] = {};
		keyboard->GetDeviceState(sizeof(key), key);
		// 数字の0キーが押されていたら
		if (key[DIK_0])
		{
			OutputDebugStringA("Hit 0\n");  // 出力ウィンドウに「Hit 0」と表示
		}

		//バックバッファの番号を取得(2つなので0番か1番)
		UINT bbIndex = swapChain->GetCurrentBackBufferIndex();

		//1.リソースバリアで書き込み可能に変更
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffers[bbIndex];
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &barrierDesc);

		//2.描画先の変更
		//レンダーターゲットビューのハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

		//3.画面クリア			R	  G	   B	A
		FLOAT clearColor[] = { 0.1f,0.25f,0.5f,0.0f };//青っぽい色
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		if (key[DIK_SPACE])     // スペースキーが押されていたら
		{
			//画面クリアカラーの数値を書き換える
			FLOAT clearColor[] = { 11.1f,0.25f, 0.5f,0.0f }; // ピンクっぽい色
			commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		}

		////トリガー処理
		//bool キーを押した状態か(uint8_t キー番号);
		//bool キーを離した状態か(uint8_t キー番号);
		//bool キーを押した瞬間か(uint8_t キー番号);
		//bool キーを離した瞬間か(uint8_t キー番号);

		// キーボード情報の取得開始
		keyboard->Acquire();

		// 全キーの入力状態を取得する

		keyboard->GetDeviceState(sizeof(key), key);

		/*transformX = 0.0f;
		transformY = 0.0f;
		rotation = 0.0f;
		scale = 1.0f;*/

		// キー入力

		////平行移動
		//if (key[DIK_W]) {
		//	transformY += 0.05f;
		//}

		//if (key[DIK_S]) {
		//	transformY -= 0.05f;
		//}

		//if (key[DIK_A]) {
		//	transformX -= 0.05f;
		//}

		//if (key[DIK_D]) {
		//	transformX += 0.05f;
		//}

		//// 拡大縮小
		//if (key[DIK_Z]) {
		//	scale -= 0.1f;
		//}

		//if (key[DIK_C]) {
		//	scale += 0.1f;
		//}


		//// 回転
		//if (key[DIK_Q]) {
		//	rotation -= PI / 32;
		//}

		//if (key[DIK_E]) {
		//	rotation += PI / 32;
		//}


		//// アフィン行列の生成
		//affin[0][0] = scale * cos(rotation);
		//affin[0][1] = scale * (-sin(rotation));
		//affin[0][2] = transformX;

		//affin[1][0] = scale * sin(rotation);
		//affin[1][1] = scale * cos(rotation);
		//affin[1][2] = transformY;

		//affin[2][0] = 0.0f;
		//affin[2][1] = 0.0f;
		//affin[2][2] = 1.0f;

		//// アフィン変換
		//for (int i = 0; i < _countof(vertices); i++) {
		//	vertices[i].x = vertices[i].x * affin[0][0] +
		//		vertices[i].y * affin[0][1] + 1.0f * affin[0][2];
		//	vertices[i].y = vertices[i].x * affin[1][0] +
		//		vertices[i].y * affin[1][1] + 1.0f * affin[1][2];
		//	vertices[i].z = vertices[i].x * affin[2][0] +
		//		vertices[i].y * affin[2][1] + 1.0f * affin[2][2];
		//}


		//////////////////////////////////////////////
		//-------DireceX毎フレーム処理　ここまで--------//
		////////////////////////////////////////////

#pragma endregion

#pragma region グラフィックスコマンド

		//4.描画コマンドここから

		// ビューポート設定コマンド
		D3D12_VIEWPORT viewport{};
		viewport.Width = window_width;   //よこ 最大1280
		viewport.Height = window_height;  //たて 最大720
		viewport.TopLeftX = 0;  //左上X
		viewport.TopLeftY = 0;  //左上Y
		viewport.MinDepth = 0.0f; //最小頻度
		viewport.MaxDepth = 1.0f; //最大頻度

		// ビューポート設定コマンドを、コマンドリストに積む
		commandList->RSSetViewports(1, &viewport);

		//シザー矩形
		D3D12_RECT scissorRect{};
		scissorRect.left = 0; // 切り抜き座標左
		scissorRect.right = scissorRect.left + window_width; // 切り抜き座標右
		scissorRect.top = 0; // 切り抜き座標上
		scissorRect.bottom = scissorRect.top + window_height; // 切り抜き座標下

		// シザー矩形設定コマンドを、コマンドリストに積む
		commandList->RSSetScissorRects(1, &scissorRect);

		//Meshの描画--------------------------------------------------------------//
		for (int i = 0; i < 10; i++)
		{
			mesh[i].Mesh_Draw(device, indices_count, commandList);
		}
		
		
		//4.描画コマンドここまで

#pragma endregion

#pragma region 画面入れ替え


		//5.リソースバリアを戻す
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;//描画状態から
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;//表示状態へ
		commandList->ResourceBarrier(1, &barrierDesc);

		//命令のクローズ
		result = commandList->Close();
		assert(SUCCEEDED(result));
		//コマンドリストの実行
		ID3D12CommandList* commandLists[] = { commandList };
		commandQueue->ExecuteCommandLists(1, commandLists);

		//画面に表示するバッファをフリップ(裏表の入れ替え)
		result = swapChain->Present(1, 0);
		assert(SUCCEEDED(result));

		//コマンドの実行完了を待つ
		commandQueue->Signal(fence, ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal) {
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		//キューをクリア
		result = commandAllocator->Reset();
		assert(SUCCEEDED(result));
		//再びコマンドリストを貯める準備
		result = commandList->Reset(commandAllocator, nullptr);
		assert(SUCCEEDED(result));

#pragma endregion
	}
#pragma region  WindowsAPI後始末

	//もうクラスは使わないので登録を解除する
	UnregisterClass(w.lpszClassName, w.hInstance);

#pragma endregion
	return 0;
}

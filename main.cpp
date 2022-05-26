#pragma region �ǂݍ��ރw�b�_�[

#define DIRECTINPUT_VERSION 0x0800 //DirectInput�̃o�[�W�����w��
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

#pragma region ���܂��Ȃ�
// @brief �R���\�[����ʂɃt�H�[�}�b�g�t��������̕\��
// @param format �t�H�[�}�b�g(%d�Ƃ�%f�Ƃ���)
// @param �ϒ�����
// @remarks ���̊֐��̓f�o�b�N�p�ł��B�f�o�b�O���ɂ������삵�܂���
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//�E�B���h�E���j�󂳂ꂽ��Ă΂��
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);//OS�ɑ΂��āu���̃A�v���͂����I���v�Ɠ`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
#pragma endregion

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

#pragma region WindowsAPI����������

	// �E�B���h�E����
	const int window_width = 1280;
	// �E�B���h�E�c��
	const int window_height = 720;


	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;	//�E�B���h�E�v���V�[�W����ݒ�
	w.lpszClassName = _T("DX12Sample");			//�E�B���h�E�N���X��
	w.hInstance = GetModuleHandle(nullptr);		//�E�B���h�E�n���h��
	w.hCursor = LoadCursor(NULL, IDC_ARROW);	//�J�[�\���w��

	//�E�B���h�E�N���X��OS�ɓo�^����
	RegisterClassEx(&w);
	//�E�B���h�E�T�C�Y{X���W�@Y���W�@�����@�c��}
	RECT wrc = { 0,0,window_width,window_height };
	//�֐����g���ăE�B���h�E�̃T�C�Y�������ŕ␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//�E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName,//�N���X���w��
		_T("LE2B_18_�q�O�`_���E��_AL3"),     //�^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,			     //�^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT,					     //�\��x���W��OS�ɂ��C��
		CW_USEDEFAULT,					     //�\��y���W��OS�ɂ��C��
		wrc.right - wrc.left,			     //�E�B���h�E��
		wrc.bottom - wrc.top,			     //�E�B���h�E��
		nullptr,						     //�e�E�B���h�E�n���h��
		nullptr,						     //���j���[�n���h��
		w.hInstance,					     //�Ăяo���A�v���P�[�V�����n���h��
		nullptr);						     //�ǉ��p�����[�^�[(�I�v�V����)

	//�E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};

#pragma endregion

#pragma region DirectX����������

	////////////////////////////////////////////////////
	//-------------DirectX12������������������-----------//
	//////////////////////////////////////////////////

#ifdef _DEBUG
//�f�o�b�O���C���[���I����
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

	//DXGI�t�@�N�g���[�̐���
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//�A�_�v�^�[�̗񋓗p
	std::vector<IDXGIAdapter4*>adapters;
	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter4* tmpAdapter = nullptr;

	//�p�t�H�[�}���X�����������珇�ɁA�S�ẴA�_�v�^�[��񋓂���
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND; i++) {
		//���I�z��ɒǉ�����
		adapters.push_back(tmpAdapter);
	}
	//�Ó��ȃA�_�v�^��I�ʂ���
	for (size_t i = 0; i < adapters.size(); i++) {
		DXGI_ADAPTER_DESC3 adapterDesc;
		//�A�_�v�^�[�̏����擾����
		adapters[i]->GetDesc3(&adapterDesc);

		//�\�t�g�E�F�A�f�o�C�X�����
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//�f�o�C�X���̗p���ă��[�v�𔲂���
			tmpAdapter = adapters[i];
			break;
		}
	}

	//�Ή����x���̔z��
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (size_t i = 0; i < _countof(levels); i++) {
		//�̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
		result = D3D12CreateDevice(tmpAdapter, levels[i], IID_PPV_ARGS(&device));
		if (result == S_OK) {
			//�f�o�C�X�𐶐��ł������_�Ń��[�v�𔲂���
			featureLevel = levels[i];
			break;
		}
	}

	//�R�}���h�A���[�P�[�^�𐶐�
	result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(result));

	//�R�}���h���X�g�𐶐�
	result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(result));

	//�R�}���h�L���[�̐ݒ�
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

	//�R�}���h�L���[�𐶐�
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));

	//�X���b�v�`�F�[���̐ݒ�
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = 1280;
	swapChainDesc.Height = 720;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;			//�F���̏���
	swapChainDesc.SampleDesc.Count = 1;							//�}���`�T���v�����Ȃ�
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;			//�o�b�N�o�b�t�@�p
	swapChainDesc.BufferCount = 2;								//�o�b�t�@����2�ɐݒ�
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//�t���b�v��͔j��
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//�X���b�v�`�F�[���̐���
	result = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, (IDXGISwapChain1**)&swapChain);
	assert(SUCCEEDED(result));

	//�f�X�N���v�^�q�[�v�̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;			//�����_�[�^�[�Q�b�g�r���[
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;		//���\��2��

	//�f�X�N���v�^�q�[�v�̐���
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	//�o�b�N�o�b�t�@
	std::vector<ID3D12Resource*>backBuffers;
	backBuffers.resize(swapChainDesc.BufferCount);

	//�X���b�v�`�F�[���̑S�Ẵo�b�t�@�ɂ��ď�������
	for (size_t i = 0; i < backBuffers.size(); i++) {
		//�X���b�v�`�F�[������o�b�t�@���擾
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		//�f�X�N���v�^�q�[�v�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		//�����\���ŃA�h���X�������
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//�����_�[�^�[�Q�b�g�r���[�̐ݒ�
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		//�V�F�[�_�[�̌v�Z���ʂ�SRGB�ɕϊ����ď�������
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//�����_�[�^�[�Q�b�g�r���[�̐���
		device->CreateRenderTargetView(backBuffers[i], &rtvDesc, rtvHandle);
	}

	//�t�F���X�̐���
	ID3D12Fence* fence = nullptr;
	UINT64 fenceVal = 0;

	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	//DirectInput�̏�����
	IDirectInput8* directInput = nullptr;
	result = DirectInput8Create(
		w.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	//�L�[�{�[�h�f�o�C�X�̐���
	IDirectInputDevice8* keyboard = nullptr;
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	//���̓f�[�^�`���̃Z�b�g
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);//�W���`��
	assert(SUCCEEDED(result));



	/////////////////////////////////////////////////////////
	//--------------DirectX12�����������@�����܂�-------------//
	///////////////////////////////////////////////////////
#pragma endregion

	Mesh mesh[10];

	Mesh::Vertex vertices[] = {
		// x      y     z       u     v
		{{-0.12f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // ����
		{{-0.12f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // ����
		{{+0.12f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // �E��
		{{+0.12f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // �E��
	};

	Mesh::Vertex vertices2[] = {
		// x      y     z       u     v
		{{-0.24f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // ����
		{{-0.24f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // ����
		{{-0.12f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // �E��
		{{-0.12f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // �E��
	};

	Mesh::Vertex vertices3[] = {
		// x      y     z       u     v
		{{-0.42f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // ����
		{{-0.42f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // ����
		{{-0.32f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // �E��
		{{-0.32f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // �E��
	};

	Mesh::Vertex vertices4[] = {
		// x      y     z       u     v
		{{-0.52f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // ����
		{{-0.52f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // ����
		{{-0.42f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // �E��
		{{-0.42f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // �E��
	};

	Mesh::Vertex vertices5[] = {
		// x      y     z       u     v
		{{-0.62f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // ����
		{{-0.62f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // ����
		{{-0.52f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // �E��
		{{-0.52f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // �E��
	};

	Mesh::Vertex vertices6[] = {
		// x      y     z       u     v
		{{-0.72f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // ����
		{{-0.72f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // ����
		{{-0.62f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // �E��
		{{-0.62f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // �E��
	};

	Mesh::Vertex vertices7[] = {
		// x      y     z       u     v
		{{-0.82f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // ����
		{{-0.82f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // ����
		{{-0.72f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // �E��
		{{-0.72f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // �E��
	};

	Mesh::Vertex vertices8[] = {
		// x      y     z       u     v
		{{+0.12f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // ����
		{{+0.12f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // ����
		{{+0.32f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // �E��
		{{+0.32f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // �E��
	};

	Mesh::Vertex vertices9[] = {
		// x      y     z       u     v
		{{+0.32f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // ����
		{{+0.32f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // ����
		{{+0.52f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // �E��
		{{+0.52f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // �E��
	};

	Mesh::Vertex vertices10[] = {
		// x      y     z       u     v
		{{+0.52f, -0.2f, 0.0f}, {0.0f, 1.0f}}, // ����
		{{+0.52f, +0.2f, 0.0f}, {0.0f, 0.0f}}, // ����
		{{+0.62f, -0.2f, 0.0f}, {1.0f, 1.0f}}, // �E��
		{{+0.62f, +0.2f, 0.0f}, {1.0f, 0.0f}}, // �E��
	};
	unsigned short indices[] = {
		0, 1, 2, // �O�p�`1��
		1, 2, 3, // �O�p�`2��
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
	
	//�Q�[�����[�v
	while (true) {
#pragma region �E�B���h�E���b�Z�[�W����

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//�A�v���P�[�V�������I��鎞��message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT) {
			break;
		}


#pragma endregion

#pragma region DirectX���t���[������
		/////////////////////////////////////////////////////
		//----------DireceX���t���[�������@��������------------//
		///////////////////////////////////////////////////

		// �L�[�{�[�h���̎擾�J�n
		keyboard->Acquire();
		// �S�L�[�̓��͏�Ԃ��擾����
		BYTE key[256] = {};
		keyboard->GetDeviceState(sizeof(key), key);
		// ������0�L�[��������Ă�����
		if (key[DIK_0])
		{
			OutputDebugStringA("Hit 0\n");  // �o�̓E�B���h�E�ɁuHit 0�v�ƕ\��
		}

		//�o�b�N�o�b�t�@�̔ԍ����擾(2�Ȃ̂�0�Ԃ�1��)
		UINT bbIndex = swapChain->GetCurrentBackBufferIndex();

		//1.���\�[�X�o���A�ŏ������݉\�ɕύX
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffers[bbIndex];
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &barrierDesc);

		//2.�`���̕ύX
		//�����_�[�^�[�Q�b�g�r���[�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

		//3.��ʃN���A			R	  G	   B	A
		FLOAT clearColor[] = { 0.1f,0.25f,0.5f,0.0f };//���ۂ��F
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		if (key[DIK_SPACE])     // �X�y�[�X�L�[��������Ă�����
		{
			//��ʃN���A�J���[�̐��l������������
			FLOAT clearColor[] = { 11.1f,0.25f, 0.5f,0.0f }; // �s���N���ۂ��F
			commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		}

		////�g���K�[����
		//bool �L�[����������Ԃ�(uint8_t �L�[�ԍ�);
		//bool �L�[�𗣂�����Ԃ�(uint8_t �L�[�ԍ�);
		//bool �L�[���������u�Ԃ�(uint8_t �L�[�ԍ�);
		//bool �L�[�𗣂����u�Ԃ�(uint8_t �L�[�ԍ�);

		// �L�[�{�[�h���̎擾�J�n
		keyboard->Acquire();

		// �S�L�[�̓��͏�Ԃ��擾����

		keyboard->GetDeviceState(sizeof(key), key);

		/*transformX = 0.0f;
		transformY = 0.0f;
		rotation = 0.0f;
		scale = 1.0f;*/

		// �L�[����

		////���s�ړ�
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

		//// �g��k��
		//if (key[DIK_Z]) {
		//	scale -= 0.1f;
		//}

		//if (key[DIK_C]) {
		//	scale += 0.1f;
		//}


		//// ��]
		//if (key[DIK_Q]) {
		//	rotation -= PI / 32;
		//}

		//if (key[DIK_E]) {
		//	rotation += PI / 32;
		//}


		//// �A�t�B���s��̐���
		//affin[0][0] = scale * cos(rotation);
		//affin[0][1] = scale * (-sin(rotation));
		//affin[0][2] = transformX;

		//affin[1][0] = scale * sin(rotation);
		//affin[1][1] = scale * cos(rotation);
		//affin[1][2] = transformY;

		//affin[2][0] = 0.0f;
		//affin[2][1] = 0.0f;
		//affin[2][2] = 1.0f;

		//// �A�t�B���ϊ�
		//for (int i = 0; i < _countof(vertices); i++) {
		//	vertices[i].x = vertices[i].x * affin[0][0] +
		//		vertices[i].y * affin[0][1] + 1.0f * affin[0][2];
		//	vertices[i].y = vertices[i].x * affin[1][0] +
		//		vertices[i].y * affin[1][1] + 1.0f * affin[1][2];
		//	vertices[i].z = vertices[i].x * affin[2][0] +
		//		vertices[i].y * affin[2][1] + 1.0f * affin[2][2];
		//}


		//////////////////////////////////////////////
		//-------DireceX���t���[�������@�����܂�--------//
		////////////////////////////////////////////

#pragma endregion

#pragma region �O���t�B�b�N�X�R�}���h

		//4.�`��R�}���h��������

		// �r���[�|�[�g�ݒ�R�}���h
		D3D12_VIEWPORT viewport{};
		viewport.Width = window_width;   //�悱 �ő�1280
		viewport.Height = window_height;  //���� �ő�720
		viewport.TopLeftX = 0;  //����X
		viewport.TopLeftY = 0;  //����Y
		viewport.MinDepth = 0.0f; //�ŏ��p�x
		viewport.MaxDepth = 1.0f; //�ő�p�x

		// �r���[�|�[�g�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
		commandList->RSSetViewports(1, &viewport);

		//�V�U�[��`
		D3D12_RECT scissorRect{};
		scissorRect.left = 0; // �؂蔲�����W��
		scissorRect.right = scissorRect.left + window_width; // �؂蔲�����W�E
		scissorRect.top = 0; // �؂蔲�����W��
		scissorRect.bottom = scissorRect.top + window_height; // �؂蔲�����W��

		// �V�U�[��`�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
		commandList->RSSetScissorRects(1, &scissorRect);

		//Mesh�̕`��--------------------------------------------------------------//
		for (int i = 0; i < 10; i++)
		{
			mesh[i].Mesh_Draw(device, indices_count, commandList);
		}
		
		
		//4.�`��R�}���h�����܂�

#pragma endregion

#pragma region ��ʓ���ւ�


		//5.���\�[�X�o���A��߂�
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;//�`���Ԃ���
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;//�\����Ԃ�
		commandList->ResourceBarrier(1, &barrierDesc);

		//���߂̃N���[�Y
		result = commandList->Close();
		assert(SUCCEEDED(result));
		//�R�}���h���X�g�̎��s
		ID3D12CommandList* commandLists[] = { commandList };
		commandQueue->ExecuteCommandLists(1, commandLists);

		//��ʂɕ\������o�b�t�@���t���b�v(���\�̓���ւ�)
		result = swapChain->Present(1, 0);
		assert(SUCCEEDED(result));

		//�R�}���h�̎��s������҂�
		commandQueue->Signal(fence, ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal) {
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		//�L���[���N���A
		result = commandAllocator->Reset();
		assert(SUCCEEDED(result));
		//�ĂуR�}���h���X�g�𒙂߂鏀��
		result = commandList->Reset(commandAllocator, nullptr);
		assert(SUCCEEDED(result));

#pragma endregion
	}
#pragma region  WindowsAPI��n��

	//�����N���X�͎g��Ȃ��̂œo�^����������
	UnregisterClass(w.lpszClassName, w.hInstance);

#pragma endregion
	return 0;
}
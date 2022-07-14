#pragma once
#include<d3dcompiler.h>
#include<d3d12.h>
#include<DirectXMath.h>
#include <DirectXTex.h>
#include <string>
#pragma comment(lib,"d3d12.lib")
#include "Vector3.h"
using namespace DirectX;

class Mesh
{
public:
	// ���_�f�[�^�\����
	struct Vertex
	{
		XMFLOAT3 pos;    // xyz���W
		XMFLOAT3 normal; // �@���x�N�g��
		XMFLOAT2 uv;     // uv���W
	};

	// �萔�o�b�t�@�p�f�[�^�\����  (3D�ϊ��s��)
	struct ConstBufferDataTransform {
		XMMATRIX mat; // 3D�ϊ��s��
	};

	struct Object3d
	{
		//�萔�o�b�t�@ (�s��p)
		ID3D12Resource* constBuffTransform = nullptr;
		//�萔�o�b�t�@�}�b�v(�s��p)
		ConstBufferDataTransform* constMapTransform = nullptr;

		//�A�t�B���ϊ����
		XMFLOAT3 scale = { 1.0f,1.0f,1.0f };
		XMFLOAT3 rotation = { 0.0f,0.0f,0.0f };
		XMFLOAT3 position = { 0.0f,0.0f,0.0f };

		//���[���h�ϊ��s��
		XMMATRIX matWorld;

		//�e�I�u�W�F�N�g�ւ̃|�C���^
		Object3d* parent = nullptr;
	};

	

public:
	Mesh();

	~Mesh();

	void Mesh_Initialization(ID3D12Device* device,Vertex *vertices_, unsigned short *indices_, int vertices_count, int device_count);

	void Mesh_Update(BYTE key[]);

	void Mesh_Draw(ID3D12Device* device, int indices_count, ID3D12GraphicsCommandList* commandList);

	void InitializeObject3d(Object3d* object, ID3D12Device* device);

	void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection);

	void DrawObject3d(Object3d* object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW& vdView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices);
private:
	/*void ConstantBuffer_creation(struct ConstBufferData);*/
	
	
private:
	HRESULT result;
	ID3D12Device* device = nullptr;

	// ���_�o�b�t�@�r���[�̍쐬
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//�ݒ�����Ƀf�X�N���v�^�q�[�v�𐶐�
	ID3D12DescriptorHeap* srvHeap = nullptr;
	//�o�b�t�@�}�e���A��
	ID3D12Resource* constBuffMaterial = nullptr;
	//�f�X�N���v�^�q�[�v
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	//�C���f�b�N�X�o�b�t�@
	D3D12_INDEX_BUFFER_VIEW ibView{};
	
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;

	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle;

	//D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle;

	ID3D12Resource* constBuffTransform0 = nullptr;

	ConstBufferDataTransform* constMapTransform0 = nullptr;

	ID3D12Resource* constBuffTransform1 = nullptr;

	ConstBufferDataTransform* constMapTransform1 = nullptr;

	//���[���h�ϊ��s�� 0��
	XMMATRIX matWorld;
	XMMATRIX matScale;
	XMMATRIX matRot;
	XMMATRIX matTrans;

	//���[���h�ϊ��s�� 1��
	XMMATRIX matWorld1;
	XMMATRIX matScale1;
	XMMATRIX matRot1;
	XMMATRIX matTrans1;

	//���e�s��
	XMMATRIX matProjection;

	//�r���[�ϊ��s��
	XMMATRIX matView;
	XMFLOAT3 eye = { 0, 0, -100 };  //���_���W
	XMFLOAT3 target = { 0, 0, 0 };  //�����_���W
	XMFLOAT3 up = { 0, 1, 0 };      //������x�N�g��

	//�p�x
	float angle = 0.0f;

	UINT incrementSize;
	////���W
	//XMFLOAT3 scale = { 1.0f,1.0f,1.0f };
	//XMFLOAT3 rotation = { 0.0f,0.0f,0.0f };
	//XMFLOAT3 position = { 0.0f,0.0f,0.0f };

	static const size_t kObjectCount = 50;

	Object3d object3ds[kObjectCount];

};




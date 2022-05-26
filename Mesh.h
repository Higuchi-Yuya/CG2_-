#pragma once
#include<d3dcompiler.h>
#include<d3d12.h>
#include<DirectXMath.h>
#include <DirectXTex.h>
#include <string>
#pragma comment(lib,"d3d12.lib")

using namespace DirectX;

class Mesh
{
public:
	// ���_�f�[�^�\����
	struct Vertex
	{
		XMFLOAT3 pos; // xyz���W
		XMFLOAT2 uv;  // uv���W
	};
public:
	Mesh();

	~Mesh();

	void Mesh_Initialization(ID3D12Device* device,Vertex *vertices_, unsigned short *indices_, int vertices_count, int device_count);

	void Mesh_Draw(ID3D12Device* device, int indices_count, ID3D12GraphicsCommandList* commandList);

private:
	HRESULT result;
	ID3D12Device* device = nullptr;
	//�΃v�����X�e�[�g�̐���
	ID3D12PipelineState* pipelineStage = nullptr;
	// ���[�g�V�O�l�`��
	ID3D12RootSignature* rootSignature;
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
};




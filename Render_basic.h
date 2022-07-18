#pragma once
#include<d3dcompiler.h>
#include<d3d12.h>
#include <string>
#include <wrl.h>

class Render_basic
{
public:
	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	static Render_basic* GetInstance();
	static void DestroyInstance();
	void Initialization(ID3D12Device* device);
	ID3D12PipelineState*  GetPipelineState()const;
	ID3D12RootSignature* GetRootSignature()const;


private:
	Render_basic() = default;
	~Render_basic() = default;

	Render_basic(const Render_basic&) = delete;
	Render_basic& operator=(const Render_basic&) = delete;
	Render_basic(const Render_basic&&) = delete;
	Render_basic& operator=(const Render_basic&&) = delete;

	HRESULT result;
	// �p�C�v�����X�e�[�g�̐���
	ComPtr<ID3D12PipelineState> pipelineStage = nullptr;
	// ���[�g�V�O�l�`��
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
};


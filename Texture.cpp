#include "Texture.h"


//void Texture::texture_Initialize(DirectX::TexMetadata& metadata, DirectX::ScratchImage& scratchImg,D3D12_HEAP_PROPERTIES& textureHeapProp, ID3D12Device* device)
//{
//
//
//	
//
//	
//	//textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//	//textureResourceDesc.Format = metadata.format;
//	//textureResourceDesc.Width = metadata.width;
//	//textureResourceDesc.Height = (UINT)metadata.height;
//	//textureResourceDesc.DepthOrArraySize = (UINT16)metadata.arraySize;
//	//textureResourceDesc.MipLevels = (UINT16)metadata.mipLevels;
//	//textureResourceDesc.SampleDesc.Count = 1;
//
//	//// �e�N�X�`���o�b�t�@�̐���
//	//result = device->CreateCommittedResource(
//	//	&textureHeapProp,
//	//	D3D12_HEAP_FLAG_NONE,
//	//	&textureResourceDesc,
//	//	D3D12_RESOURCE_STATE_GENERIC_READ,
//	//	nullptr,
//	//	IID_PPV_ARGS(&texBuff));
//	//assert(SUCCEEDED(result));
//
//	//// �S�~�b�v�}�b�v�ɂ���
//	//for (size_t i = 0; i < metadata.mipLevels; i++) {
//	//	// �~�b�v�}�b�v���x�����w�肵�ăC���[�W���擾
//	//	const Image* img = scratchImg.GetImage(i, 0, 0);
//	//	// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
//	//	result = texBuff->WriteToSubresource(
//	//		(UINT)i,
//	//		nullptr,              // �S�̈�փR�s�[
//	//		img->pixels,          // ���f�[�^�A�h���X
//	//		(UINT)img->rowPitch,  // 1���C���T�C�Y
//	//		(UINT)img->slicePitch // 1���T�C�Y
//	//	);
//	//	assert(SUCCEEDED(result));
//	//}
//
//
//}
//
//void Texture::texture_srvInit(D3D12_CPU_DESCRIPTOR_HANDLE& srvHandle, ID3D12Device* device)
//{
//
//
//}

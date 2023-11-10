#pragma once
#include "../Common/CommonCore.h"


template<class T>
void MeshAssetCreate(T& MeshAsset,std::string filePath)
{
	std::vector<char>buffer;

	std::array<UINT, 4>DataDescribe = MeshAsset.GetDataDescribe();
	UINT HeadBufferByteSize = 4 * sizeof(UINT);
	UINT VertexBufferByteSize = DataDescribe[1];
	UINT IndexBufferByteSize = DataDescribe[3];
	//HeadBufferByteSize++ HeadBufferByteSize
	buffer.resize(HeadBufferByteSize + VertexBufferByteSize+IndexBufferByteSize);
	memcpy(buffer.data(), DataDescribe.data(), HeadBufferByteSize);
	memcpy(buffer.data() + HeadBufferByteSize, MeshAsset.vertices.data(), VertexBufferByteSize);
	memcpy(buffer.data() + HeadBufferByteSize + VertexBufferByteSize, MeshAsset.indices.data(), IndexBufferByteSize);


	std::ofstream outfile(filePath, std::ios::binary);
	outfile.write(buffer.data(), HeadBufferByteSize + VertexBufferByteSize + IndexBufferByteSize);
	outfile.close();
}


template<class T>
void MeshAssetLoad(T& MeshAsset, std::string filePath)
{
	std::vector<UINT> DataDescribe;
	UINT MeshDataDescribeSize = 4;
	UINT MeshDataDescribeByteSize = MeshDataDescribeSize * sizeof(UINT);

	DataDescribe.resize(MeshDataDescribeSize);

	std::ifstream infile(filePath, std::ios::binary);
	infile.read((char*)DataDescribe.data(),MeshDataDescribeByteSize);

	UINT VertexBufferSize = DataDescribe[0];
	UINT VertexBufferByteSize = DataDescribe[1];
	UINT IndexBufferSize = DataDescribe[2];
	UINT IndexBufferByteSize = DataDescribe[3];

	std::vector<char>MeshDataBuffer;
	MeshDataBuffer.resize(MeshDataDescribeByteSize+ VertexBufferByteSize + IndexBufferByteSize);

	infile.clear();
	infile.seekg(std::ios::beg);
	infile.read(MeshDataBuffer.data(), MeshDataDescribeByteSize + VertexBufferByteSize + IndexBufferByteSize);

	MeshAsset.vertices.resize(VertexBufferSize);
	MeshAsset.indices.resize(IndexBufferSize);

	memcpy(MeshAsset.vertices.data(), MeshDataBuffer.data()+ MeshDataDescribeByteSize, VertexBufferByteSize);
	memcpy(MeshAsset.indices.data(), MeshDataBuffer.data() + MeshDataDescribeByteSize+ VertexBufferByteSize, IndexBufferByteSize);

	infile.close();
}
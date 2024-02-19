#include "Shader.h"


UShader::UShader()
	:ShaderFileName(L""),OutName(""),ShaderType(EShaderType::Pixel)
{
}

UShader::UShader(std::wstring shaderfile, LPCSTR outname,EShaderType ShaderType)
	:ShaderFileName(shaderfile),OutName(outname),ShaderType(ShaderType)
{
}

ID3DBlob* UShader::GetShader() const
{
	return Shader.Get();
}

LPCSTR UShader::GetOutName() const
{
	return OutName;
}

std::wstring UShader::GetFilePath() const
{
	return ShaderFileName;
}

void UShader::CompileShader()
{
	UINT compileFlags = 0;

#if defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	switch (ShaderType)
	{
	case EShaderType::Vertex:
		ThrowIfFailed(D3DCompileFromFile(ShaderFileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, OutName, "vs_5_0", compileFlags, 0, &Shader, nullptr));
		break;
	case EShaderType::Pixel:
		ThrowIfFailed(D3DCompileFromFile(ShaderFileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, OutName, "ps_5_0", compileFlags, 0, &Shader, nullptr));
		break;
	case EShaderType::Compute:
		ThrowIfFailed(D3DCompileFromFile(ShaderFileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, OutName, "cs_5_0", compileFlags, 0, &Shader, nullptr));
		break;
	default:
		break;
	}
}





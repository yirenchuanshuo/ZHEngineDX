#include "Shader.h"
#include "../GameHelper/GameHelper.h"

UShader::UShader()
	:ShaderFileName(L""), VSOutName(""), PSOutName(""), blendMode(EBlendMode::Opaque)
{
}

UShader::UShader(std::wstring shaderfile, LPCSTR vsout, LPCSTR psout, EBlendMode blend)
	:ShaderFileName(shaderfile), VSOutName(vsout),PSOutName(psout),blendMode(blend)
{
}

ID3DBlob* UShader::GetVertexShader()const
{
	return vertexShader.Get();
}

ID3DBlob* UShader::GetPixelShader()const
{
	return pixelShader.Get();
}

EBlendMode UShader::GetBlendMode()const
{
	return blendMode;
}

void UShader::CreateShader()
{
	UINT compileFlags = 0;

#if defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ThrowIfFailed(D3DCompileFromFile(ShaderFileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, VSOutName, "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(ShaderFileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, PSOutName, "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
}

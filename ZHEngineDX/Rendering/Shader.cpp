#include "Shader.h"


UShader::UShader()
	:ShaderFileName(L"")
{
}

UShader::UShader(std::wstring shaderfile)
	:ShaderFileName(shaderfile)
{
}



void UShader::CompileShader()
{
	
}



//NormalShader
//----------------------------------------------
UNormalShader::UNormalShader()
	:UShader(L""), VSOutName(""), PSOutName(""), blendMode(EBlendMode::Opaque)
{
}

UNormalShader::UNormalShader(std::wstring shaderfile, LPCSTR vsout, LPCSTR psout, EBlendMode blend)
	:UShader(shaderfile), VSOutName(vsout), PSOutName(psout), blendMode(blend)
{

}

ID3DBlob* UNormalShader::GetVertexShader()const
{
	return vertexShader.Get();
}

ID3DBlob* UNormalShader::GetPixelShader()const
{
	return pixelShader.Get();
}

EBlendMode UNormalShader::GetBlendMode()const
{
	return blendMode;
}

void UNormalShader::CompileShader()
{
	UINT compileFlags = 0;

#if defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ThrowIfFailed(D3DCompileFromFile(ShaderFileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, VSOutName, "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(ShaderFileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, PSOutName, "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
}

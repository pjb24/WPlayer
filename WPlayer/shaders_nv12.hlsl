// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//----------------------------------------------------------------------

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PSInput VSMain(float4 position : POSITION, float2 uv : TEXCOORD)
{
    PSInput result;

    result.position = position;
    result.uv = uv;

    return result;
}


//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

Texture2D texture_y : register(t1);
Texture2D texture_chrominance : register(t2);

SamplerState g_sampler : register(s0);

// Derived from https://msdn.microsoft.com/en-us/library/windows/desktop/dd206750(v=vs.85).aspx
// Section: Converting 8-bit YUV to RGB888
static const float3x3 YUVtoRGBCoeffMatrix =
{
    1.164383f, 1.164383f, 1.164383f,
	0.000000f, -0.391762f, 2.017232f,
	1.596027f, -0.812968f, 0.000000f
};

float3 ConvertYUVtoRGB(float3 yuv)
{
	// Derived from https://msdn.microsoft.com/en-us/library/windows/desktop/dd206750(v=vs.85).aspx
	// Section: Converting 8-bit YUV to RGB888

	// These values are calculated from (16 / 255) and (128 / 255)
    yuv -= float3(0.062745f, 0.501960f, 0.501960f);
    yuv = mul(yuv, YUVtoRGBCoeffMatrix);

    return saturate(yuv);
}

min16float4 PSMain(PSInput input) : SV_TARGET
{
    float y = texture_y.Sample(g_sampler, input.uv).r;
    float2 uv = texture_chrominance.Sample(g_sampler, input.uv).rg;

    return min16float4(ConvertYUVtoRGB(float3(y, uv)), 1.f);
}

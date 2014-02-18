#ifndef _SHADERPARAMS_H
#define _SHADERPARAMS_H

#include <atlbase.h>
#if defined (_DEBUG)
#pragma comment(lib,"atlsd.lib")
#endif

#include "../../utilities/D3dIncludes.h"

// Struct to encapsulate the common resources required for some shaders
struct ShaderParams {
	CComPtr<ID3D11ShaderResourceView>	mSRV;
	CComPtr<ID3D11UnorderedAccessView>  mUAV;
};

#endif
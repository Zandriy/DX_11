/*
* Device.cpp
*
* Created on: May 10, 2017
* Author: Andrew Zhabura
*/

#include "Device.h"
#include <d3dcompiler.h>
#include "DDSTextureLoader.h"

namespace ZDX
{
	using namespace DirectX;

	struct SimpleVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Tex;
	};

	struct CBNeverChanges
	{
		XMMATRIX mView;
	};

	struct CBChangeOnResize
	{
		XMMATRIX mProjection;
	};

	struct CBChangesEveryFrame
	{
		XMMATRIX mWorld;
		XMFLOAT4 vMeshColor;
	};

	Device::Device(HWND hWnd)
	{
		HRESULT res = InitDevice(hWnd);
		if (FAILED(res))
		{
			throw res;
		}
		res = CreateShaders(L"Tutorial06.fx", L"Tutorial06.fx");
		if (FAILED(res))
		{
			throw res;
		}
		res = LoadData();
		if (FAILED(res))
		{
			throw res;
		}
	}


	Device::~Device()
	{
		if (m_pImmediateContext) m_pImmediateContext->ClearState();

		if (m_pSamplerLinear) m_pSamplerLinear->Release();
		if (m_pTextureRV) m_pTextureRV->Release();
		if (m_pCBNeverChanges) m_pCBNeverChanges->Release();
		if (m_pCBChangeOnResize) m_pCBChangeOnResize->Release();
		if (m_pCBChangesEveryFrame) m_pCBChangesEveryFrame->Release();
		if (m_pVertexBuffer) m_pVertexBuffer->Release();
		if (m_pIndexBuffer) m_pIndexBuffer->Release();
		if (m_pVertexLayout) m_pVertexLayout->Release();
		if (m_pVertexShader) m_pVertexShader->Release();
		if (m_pPixelShader) m_pPixelShader->Release();
		if (m_pDepthStencil) m_pDepthStencil->Release();
		if (m_pDepthStencilView) m_pDepthStencilView->Release();
		if (m_pRenderTargetView) m_pRenderTargetView->Release();
		if (m_pSwapChain1) m_pSwapChain1->Release();
		if (m_pSwapChain) m_pSwapChain->Release();
		if (m_pImmediateContext1) m_pImmediateContext1->Release();
		if (m_pImmediateContext) m_pImmediateContext->Release();
		if (m_pd3dDevice1) m_pd3dDevice1->Release();
		if (m_pd3dDevice) m_pd3dDevice->Release();
	}

	HRESULT Device::InitDevice(HWND hWnd)
	{
		HRESULT hr = S_OK;

		RECT rc;
		GetClientRect(hWnd, &rc);
		UINT width = rc.right - rc.left;
		UINT height = rc.bottom - rc.top;

		UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);

		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			m_driverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);

			if (hr == E_INVALIDARG)
			{
				// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
				hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
					D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
			}

			if (SUCCEEDED(hr))
				break;
		}
		if (FAILED(hr))
			return hr;

		// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
		IDXGIFactory1* dxgiFactory = nullptr;
		{
			IDXGIDevice* dxgiDevice = nullptr;
			hr = m_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
			if (SUCCEEDED(hr))
			{
				IDXGIAdapter* adapter = nullptr;
				hr = dxgiDevice->GetAdapter(&adapter);
				if (SUCCEEDED(hr))
				{
					hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
					adapter->Release();
				}
				dxgiDevice->Release();
			}
		}
		if (FAILED(hr))
			return hr;

		// Create swap chain
		IDXGIFactory2* dxgiFactory2 = nullptr;
		hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
		if (dxgiFactory2)
		{
			// DirectX 11.1 or later
			hr = m_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&m_pd3dDevice1));
			if (SUCCEEDED(hr))
			{
				(void)m_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&m_pImmediateContext1));
			}

			DXGI_SWAP_CHAIN_DESC1 sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.Width = width;
			sd.Height = height;
			sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = 1;

			hr = dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice, hWnd, &sd, nullptr, nullptr, &m_pSwapChain1);
			if (SUCCEEDED(hr))
			{
				hr = m_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&m_pSwapChain));
			}

			dxgiFactory2->Release();
		}
		else
		{
			// DirectX 11.0 systems
			DXGI_SWAP_CHAIN_DESC sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.BufferCount = 1;
			sd.BufferDesc.Width = width;
			sd.BufferDesc.Height = height;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = hWnd;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.Windowed = TRUE;

			hr = dxgiFactory->CreateSwapChain(m_pd3dDevice, &sd, &m_pSwapChain);
		}

		// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
		dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

		dxgiFactory->Release();

		if (FAILED(hr))
			return hr;

		// Create a render target view
		ID3D11Texture2D* pBackBuffer = nullptr;
		hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
		if (FAILED(hr))
			return hr;

		hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
		pBackBuffer->Release();
		if (FAILED(hr))
			return hr;

		// Create depth stencil texture
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		hr = m_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &m_pDepthStencil);
		if (FAILED(hr))
			return hr;

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		hr = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
		if (FAILED(hr))
			return hr;

		m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		// Setup the viewport
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_pImmediateContext->RSSetViewports(1, &vp);

		m_aspect = width / (FLOAT)height;

		return hr;
	}

	void Device::Render()
	{
		// Update our time
		static float t = 0.0f;
		if (m_driverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			t += (float)XM_PI * 0.0125f;
		}
		else
		{
			static ULONGLONG timeStart = 0;
			ULONGLONG timeCur = GetTickCount64();
			if (timeStart == 0)
				timeStart = timeCur;
			t = (timeCur - timeStart) / 1000.0f;
		}

		// Rotate cube around the origin
		m_World = XMMatrixRotationY(t);

		// Modify the color
		m_vMeshColor.x = (sinf(t * 1.0f) + 1.0f) * 0.5f;
		m_vMeshColor.y = (cosf(t * 3.0f) + 1.0f) * 0.5f;
		m_vMeshColor.z = (sinf(t * 5.0f) + 1.0f) * 0.5f;

		//
		// Clear the back buffer
		//
		m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, Colors::MidnightBlue);

		//
		// Clear the depth buffer to 1.0 (max depth)
		//
		m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		//
		// Update variables that change once per frame
		//
		CBChangesEveryFrame cb;
		cb.mWorld = XMMatrixTranspose(m_World);
		cb.vMeshColor = m_vMeshColor;
		m_pImmediateContext->UpdateSubresource(m_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);

		//
		// Render the cube
		//
		m_pImmediateContext->VSSetShader(m_pVertexShader, nullptr, 0);
		m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pCBNeverChanges);
		m_pImmediateContext->VSSetConstantBuffers(1, 1, &m_pCBChangeOnResize);
		m_pImmediateContext->VSSetConstantBuffers(2, 1, &m_pCBChangesEveryFrame);
		m_pImmediateContext->PSSetShader(m_pPixelShader, nullptr, 0);
		m_pImmediateContext->PSSetConstantBuffers(2, 1, &m_pCBChangesEveryFrame);
		m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTextureRV);
		m_pImmediateContext->PSSetSamplers(0, 1, &m_pSamplerLinear);
		m_pImmediateContext->DrawIndexed(36, 0, 0);

		//
		// Present our back buffer to our front buffer
		//
		m_pSwapChain->Present(0, 0);
	}

	HRESULT Device::CreateShaders(const WCHAR* vs, const WCHAR* ps)
	{
		HRESULT hr = S_OK;
		
		// Compile the vertex shader
		ID3DBlob* pVSBlob = nullptr;
		hr = CompileShaderFromFile(vs, "VS", "vs_4_0", &pVSBlob);
		if (FAILED(hr))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			return hr;
		}

		// Create the vertex shader
		hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader);
		if (FAILED(hr))
		{
			pVSBlob->Release();
			return hr;
		}

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);

		// Create the input layout
		hr = m_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
			pVSBlob->GetBufferSize(), &m_pVertexLayout);
		pVSBlob->Release();
		if (FAILED(hr))
			return hr;

		// Set the input layout
		m_pImmediateContext->IASetInputLayout(m_pVertexLayout);

		// Compile the pixel shader
		ID3DBlob* pPSBlob = nullptr;
		hr = CompileShaderFromFile(ps, "PS", "ps_4_0", &pPSBlob);
		if (FAILED(hr))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			return hr;
		}

		// Create the pixel shader
		hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPixelShader);
		pPSBlob->Release();
		if (FAILED(hr))
			return hr;

		return hr;
	}

	HRESULT Device::LoadData()
	{
		HRESULT hr = S_OK;
		// Create vertex buffer
		SimpleVertex vertices[] =
		{
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },

			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

			{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },

			{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },

			{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		};
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex) * 24;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = vertices;
		hr = m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
		if (FAILED(hr))
			return hr;

		// Set vertex buffer
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

		// Create index buffer
		WORD indices[] =
		{
			3,1,0,
			2,1,3,

			6,4,5,
			7,4,6,

			11,9,8,
			10,9,11,

			14,12,13,
			15,12,14,

			19,17,16,
			18,17,19,

			22,20,21,
			23,20,22
		};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(WORD) * 36;        // 36 vertices needed for 12 triangles in a triangle list
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = indices;
		hr = m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
		if (FAILED(hr))
			return hr;

		// Set index buffer
		m_pImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		// Set primitive topology
		m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Create the constant buffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(CBNeverChanges);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, &m_pCBNeverChanges);
		if (FAILED(hr))
			return hr;

		bd.ByteWidth = sizeof(CBChangeOnResize);
		hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, &m_pCBChangeOnResize);
		if (FAILED(hr))
			return hr;

		bd.ByteWidth = sizeof(CBChangesEveryFrame);
		hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, &m_pCBChangesEveryFrame);
		if (FAILED(hr))
			return hr;

		// Load the Texture
		hr = CreateDDSTextureFromFile(m_pd3dDevice, L"seafloor.dds", nullptr, &m_pTextureRV);
		if (FAILED(hr))
			return hr;

		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = m_pd3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear);
		if (FAILED(hr))
			return hr;

		// Initialize the world matrix
		m_World = XMMatrixIdentity();

		// Initialize the view matrix
		XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
		XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		m_View = XMMatrixLookAtLH(Eye, At, Up);

		CBNeverChanges cbNeverChanges;
		cbNeverChanges.mView = XMMatrixTranspose(m_View);
		m_pImmediateContext->UpdateSubresource(m_pCBNeverChanges, 0, nullptr, &cbNeverChanges, 0, 0);

		// Initialize the projection matrix
		m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_aspect, 0.01f, 100.0f);

		CBChangeOnResize cbChangesOnResize;
		cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
		m_pImmediateContext->UpdateSubresource(m_pCBChangeOnResize, 0, nullptr, &cbChangesOnResize, 0, 0);

		return hr;
	}

	HRESULT Device::CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
	{
		HRESULT hr = S_OK;

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows 
		// the shaders to be optimized and to run exactly the way they will run in 
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;

		// Disable optimizations to further improve shader debugging
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		ID3DBlob* pErrorBlob = nullptr;
		hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
			dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
		if (FAILED(hr))
		{
			if (pErrorBlob)
			{
				OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
				pErrorBlob->Release();
			}
			return hr;
		}
		if (pErrorBlob) pErrorBlob->Release();

		return S_OK;
	}

} // namespace ZDX
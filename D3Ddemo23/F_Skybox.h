#pragma once

#include"Runtime.h"


struct SKYBOXVERTEX
{
	float	x, y, z;
	float	u, v;//材质UV坐标
};
#define D3DFVF_SKYBOX D3DFVF_XYZ|D3DFVF_TEX1


class F_Skybox
{
private:
	LPDIRECT3DDEVICE9				m_pd3dDevice;			//D3D设备对象
	LPDIRECT3DVERTEXBUFFER9	m_pVertexBuffer;		//顶点缓存对象
	LPDIRECT3DTEXTURE9			m_pTexture[5];			//5个纹理接口对象
	float										m_Length;					//天空盒边长

public:
	F_Skybox(LPDIRECT3DDEVICE9  pDevice);		//构造函数
	virtual ~F_Skybox(void);								//析构函数

public:
	BOOL	InitSkyBox(float Length);   //初始化天空盒函数
	BOOL	LoadSkyTextureFromFile(wchar_t *pFrontTextureFile, wchar_t *pBackTextureFile, wchar_t *pLeftTextureFile, wchar_t *pRightTextureFile, wchar_t *pTopTextureFile);  //从文件加载天空盒五个方向上的纹理
	VOID		RenderSkyBox(D3DXMATRIX *pMatWorld, BOOL bRenderFrame); //渲染天空盒,根据第一个参数设定天空盒世界矩阵，第二个参数选择是否渲染出线框

};


F_Skybox::F_Skybox(LPDIRECT3DDEVICE9  pDevice)
{
	//给各个参数赋初值
	m_pVertexBuffer = NULL;
	m_pd3dDevice = pDevice;
	for (int i = 0; i<5; i++)
		m_pTexture[i] = NULL;
	m_Length = 0.0f;
}

BOOL F_Skybox::InitSkyBox(float Length)
{
	m_Length = Length;
	m_pd3dDevice->CreateVertexBuffer(20 * sizeof(SKYBOXVERTEX), 0,
		D3DFVF_SKYBOX, D3DPOOL_MANAGED, &m_pVertexBuffer, 0);

	
	SKYBOXVERTEX vertices[] =
	{
		//front
		{ -m_Length / 2, 0.0f,    m_Length / 2, 0.0f, 1.0f, },
	{ -m_Length / 2, m_Length / 2,   m_Length / 2, 0.0f, 0.0f, },
	{ m_Length / 2, 0.0f,    m_Length / 2, 1.0f, 1.0f, },
	{ m_Length / 2, m_Length / 2,   m_Length / 2, 1.0f, 0.0f, },

	//back
	{ m_Length / 2, 0.0f,   -m_Length / 2, 0.0f, 1.0f, },
	{ m_Length / 2, m_Length / 2,  -m_Length / 2, 0.0f, 0.0f, },
	{ -m_Length / 2, 0.0f,   -m_Length / 2, 1.0f, 1.0f, },
	{ -m_Length / 2, m_Length / 2,  -m_Length / 2, 1.0f, 0.0f, },

	//left
	{ -m_Length / 2, 0.0f,   -m_Length / 2, 0.0f, 1.0f, },
	{ -m_Length / 2, m_Length / 2,  -m_Length / 2, 0.0f, 0.0f, },
	{ -m_Length / 2, 0.0f,    m_Length / 2, 1.0f, 1.0f, },
	{ -m_Length / 2, m_Length / 2,   m_Length / 2, 1.0f, 0.0f, },

	//right
	{ m_Length / 2, 0.0f,   m_Length / 2, 0.0f, 1.0f, },
	{ m_Length / 2, m_Length / 2,  m_Length / 2, 0.0f, 0.0f, },
	{ m_Length / 2, 0.0f,  -m_Length / 2, 1.0f, 1.0f, },
	{ m_Length / 2, m_Length / 2, -m_Length / 2, 1.0f, 0.0f, },

	//top
	{ m_Length / 2, m_Length / 2, -m_Length / 2, 1.0f, 0.0f, },
	{ m_Length / 2, m_Length / 2,  m_Length / 2, 1.0f, 1.0f, },
	{ -m_Length / 2, m_Length / 2, -m_Length / 2, 0.0f, 0.0f, },
	{ -m_Length / 2, m_Length / 2,  m_Length / 2, 0.0f, 1.0f, },

	};

	//ready to fullfill vertex data
	void* pVertices;
	
	m_pVertexBuffer->Lock(0, 0, (void**)&pVertices, 0);
	
	memcpy(pVertices, vertices, sizeof(vertices));

	m_pVertexBuffer->Unlock();

	return TRUE;
}

BOOL F_Skybox::LoadSkyTextureFromFile(wchar_t *pFrontTextureFile, wchar_t *pBackTextureFile, wchar_t *pLeftTextureFile, wchar_t *pRightTextureFile, wchar_t *pTopTextureFile)
{
	
	D3DXCreateTextureFromFile(m_pd3dDevice, pFrontTextureFile, &m_pTexture[0]);  
	D3DXCreateTextureFromFile(m_pd3dDevice, pBackTextureFile, &m_pTexture[1]);  
	D3DXCreateTextureFromFile(m_pd3dDevice, pLeftTextureFile, &m_pTexture[2]);  
	D3DXCreateTextureFromFile(m_pd3dDevice, pRightTextureFile, &m_pTexture[3]);  
	D3DXCreateTextureFromFile(m_pd3dDevice, pTopTextureFile, &m_pTexture[4]); 
	return TRUE;
}




void F_Skybox::RenderSkyBox(D3DXMATRIX *pMatWorld, BOOL bRenderFrame)
{
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);  
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);  
	m_pd3dDevice->SetTransform(D3DTS_WORLD, pMatWorld);  
	m_pd3dDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(SKYBOXVERTEX));    
	m_pd3dDevice->SetFVF(D3DFVF_SKYBOX);  

										
	for (int i = 0; i<5; i++)
	{
		m_pd3dDevice->SetTexture(0, m_pTexture[i]);
		m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * 4, 2);
	}

	
	if (bRenderFrame) 
	{
		m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME); 
																		
		for (int i = 0; i<5; i++)
		{
			m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * 4, 2);	
		}

		m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);	
	}
}



F_Skybox::~F_Skybox(void)
{
	SAFE_RELEASE(m_pVertexBuffer);
	for (int i = 0; i<5; i++)
	{
		SAFE_RELEASE(m_pTexture[i]);
	}
}

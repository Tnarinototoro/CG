#pragma once
#include"base.h"
class F_Light
{
	D3DLIGHTTYPE m_LightType;
	D3DVECTOR m_Position;
	D3DVECTOR m_Direction;
	D3DLIGHT9 m_light;
	LPDIRECT3DDEVICE9 m_device;
	DWORD m_index;
	LPD3DXMESH m_Sphere;
	D3DMATERIAL9 m_mtrl;
	
public:
	F_Light(LPDIRECT3DDEVICE9 outdevice);
	~F_Light();
	void ChangePosition(D3DVECTOR v)
	{
		m_Position = v;
		m_light.Position = this->m_Position;
	}
	void ChangePosition(FLOAT x, FLOAT y, FLOAT z)
	{
		
		m_Position.x = x;
		m_Position.y = y;
		m_Position.z = z;
		m_light.Position = m_Position;
	}
	void EnableLight(BOOL On_Off)
	{
		m_device->LightEnable(m_index, On_Off);
		m_device->SetLight(m_index, &m_light);
		m_device->SetMaterial(&m_mtrl);
		m_device->SetTexture(0, NULL);
		D3DXMATRIX trans;
		D3DXMatrixTranslation(&trans, m_Position.x, m_Position.y, m_Position.z);
		m_device->SetTransform(D3DTS_WORLD, &trans);
		m_Sphere->DrawSubset(0);
	}
	// only can be called by point type light
	D3DVECTOR GetPosition()
	{
		if (m_LightType == D3DLIGHT_POINT)
			return  m_Position;
	}
};
F_Light::F_Light(LPDIRECT3DDEVICE9 outdevice)
{
	m_device = outdevice;
	m_index = 0;
	::ZeroMemory(&m_mtrl, sizeof(m_mtrl));  m_mtrl.Ambient = D3DXCOLOR(0.5f, 0.5f, 0.7f, 1.0f);
	m_mtrl.Diffuse = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);
	m_mtrl.Specular = D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.3f);
	m_mtrl.Emissive = D3DXCOLOR(0.3f, 0.0f, 0.1f, 1.0f);
	::ZeroMemory(&this->m_light, sizeof(this->m_light));
	m_Position.x = 0.0f;
	m_Position.y = 1000.0f;
	m_Position.z = 0.0f;
	m_LightType = D3DLIGHT_POINT;
	switch (m_LightType)
	{
	case D3DLIGHT_POINT:
	
		m_light.Type = D3DLIGHT_POINT;
		m_light.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.0f);
		m_light.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.0f);
		m_light.Specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.0f);
		m_light.Position = m_Position;
		m_light.Attenuation0 = 1.0f;
		m_light.Attenuation1 = 0;
		m_light.Attenuation2 = 0;
		m_light.Range = 1000;
		break;
	case D3DLIGHT_DIRECTIONAL:
		m_light.Type = D3DLIGHT_DIRECTIONAL;
		m_light.Ambient = D3DXCOLOR(0.5f, 0.3f, 0.7f, 1.0f);
		m_light.Diffuse = D3DXCOLOR(0.3f, 0.7f, 0.1f, 1.0f);
		m_light.Specular = D3DXCOLOR(0.2f, 0.5f, 0.4f, 1.f);
		m_light.Direction = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		break;
	case D3DLIGHT_SPOT:
		m_light.Type = D3DLIGHT_SPOT;
		m_light.Direction = D3DXVECTOR3(-1, -1, -1);
		m_light.Ambient = D3DXCOLOR(0.2f, 0.3f, 0.5f, 1.0f);
		m_light.Diffuse = D3DXCOLOR(0.2f, 0.3f, 0.5f, 1.0f);
		m_light.Specular = D3DXCOLOR(0.2f, 0.3f, 0.5f, 1.0f);
		m_light.Position = m_Position;
		m_light.Attenuation0 = 1.0f;
		m_light.Attenuation1 = 0;
		m_light.Attenuation2 = 0;
		m_light.Range = 300;
		m_light.Falloff = 0.1f;
		m_light.Phi = D3DX_PI / 3.0f;
		m_light.Theta = D3DX_PI / 6.0f;
		break;
	default:
		break;
		
	}
	D3DXCreateSphere(m_device, 15, 25, 25,
		&m_Sphere, NULL);
}
F_Light::~F_Light()
{
	m_Sphere->Release();
	SAFE_RELEASE(m_device);
}
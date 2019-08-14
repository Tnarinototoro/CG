#pragma once
#include"Runtime.h"
struct HelperVertex
{
	FLOAT x, y, z;
	DWORD color;

};
#define D3DFVF_HelperVertex (D3DFVF_XYZ|D3DFVF_DIFFUSE)
class Helper
{
	LPDIRECT3DDEVICE9 device;
	HelperVertex vertices[100];
	LPDIRECT3DVERTEXBUFFER9 helperbuffers;
	ID3DXFont *HelperText;
	BOOL Enable;
	RECT HelperTextRect;
public:
	Helper();
	~Helper();
	void Inithelper(IDirect3DDevice9 *out_device)
	{
		this->device = out_device;
		HelperTextRect.top = 20;
		HelperTextRect.bottom = 100;
		HelperTextRect.right = 100;
		HelperTextRect.left = 0;
		Enable = TRUE;
		for (int i = 0, j = -12; i < 50; i += 2, j++)
		{
			vertices[i].x = j;
			vertices[i].z = -12;
			vertices[i].y = 0;
			vertices[i].color = D3DCOLOR_XRGB(255, 0, 0);

			vertices[i + 1].x = j;
			vertices[i + 1].z = 12;
			vertices[i + 1].y = 0;
			vertices[i + 1].color = D3DCOLOR_XRGB(255, 0, 0);

			vertices[50 + i].x = -12;
			vertices[50 + i].z = j;
			vertices[50 + i].y = 0;
			vertices[50 + i].color = D3DCOLOR_XRGB(255, 0, 0);

			vertices[50 + i + 1].x = 12;
			vertices[50 + i + 1].z = j;
			vertices[50 + i + 1].y = 0;
			vertices[51 + i].color = D3DCOLOR_XRGB(255, 0, 0);

		}
		if (FAILED(device->CreateVertexBuffer(100 * sizeof(HelperVertex), 0, D3DFVF_HelperVertex, D3DPOOL_DEFAULT,
			&helperbuffers,
			NULL)))
		{
			MessageBox(NULL, L"Error in the hlperVertcise creating", L"Error in the buffer creating", 0);
		}
		if (FAILED(D3DXCreateFont(device, 20, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			L"Arial", &HelperText)))
		{
			MessageBox(NULL, L"Tutorial Text Init failed!", L"Error!", 0);
		}
	}
	void DrawHelper()
	{
		//draw XOZ panel
		if (Enable == TRUE)
		{


			device->SetStreamSource(0, helperbuffers, 0, sizeof(HelperVertex));
			device->SetFVF(D3DFVF_HelperVertex);
			device->DrawPrimitive(D3DPT_LINELIST, 0, 50);

			//Draw tutorial

			HelperText->DrawTextW(NULL, L"Click Lbutton to shoot!", -1, &HelperTextRect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(0, 123, 230, 255));
		}

	}
	void SetStaticTransForm()
	{
		D3DXMATRIX world;
		D3DXMatrixIdentity(&world);
		D3DXMatrixScaling(&world, 100, 100, 100);
		device->SetTransform(D3DTS_WORLD, &world);
	}
	void SetEnable(BOOL IsOn)
	{
		Enable = IsOn;
	}
	HRESULT LoadPointsBuffer()
	{
		VOID *verticbufferpoint;
		if (FAILED(helperbuffers->Lock(0, sizeof(vertices), (void**)&verticbufferpoint, 0)))
		{
			MessageBox(NULL, L"Error in the hlperVertcise Locking", L"Error in the buffer Locking", 0);
			return E_FAIL;
		}
		memcpy(verticbufferpoint, vertices, sizeof(vertices));
		helperbuffers->Unlock();
		return S_OK;
	}
};

Helper::Helper()
{
	;
}

Helper::~Helper()
{
	SAFE_RELEASE(helperbuffers);
	HelperText->Release();
}
class PopoutMessage
{
	LPDIRECT3DDEVICE9 m_device;
	LPD3DXFONT m_font;
	DWORD64 m_begin;
	DWORD64 m_current;
	DWORD64 m_agelimit;
	bool m_Activated;
	float m_v;
	RECT m_contentRec;
public:
	PopoutMessage(LPDIRECT3DDEVICE9 indevice);
	~PopoutMessage();
	void Init(float v, DWORD64 limit);
	void ResetPop();
	void PopOut(LPCWSTR popcontent);
};
PopoutMessage::PopoutMessage(LPDIRECT3DDEVICE9 device)
{
	m_device = device;
}
PopoutMessage::~PopoutMessage()
{
	SAFE_RELEASE(m_font);
}
void PopoutMessage::ResetPop()
{
	m_Activated = true;
}
void PopoutMessage::Init(float v, DWORD64 limit)
{
	this->m_v = v;
	this->m_agelimit = limit;
	this->m_contentRec.left = 0;
	this->m_contentRec.right = 400;
	this->m_contentRec.top = 900;
	this->m_contentRec.bottom = 1080;
	D3DXCreateFont(m_device, 20, 20, 20, 1000, false, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, _T("Calibri"), &m_font);
	m_Activated = false;
}
void PopoutMessage::PopOut(LPCWSTR content)
{
	if (!m_Activated)
	{
		m_begin = GetTickCount64();
		m_current = m_begin;
	}
	else
	{

		DWORD64 tmp = GetTickCount64();
		WCHAR tmp1[50];
		int charcount = swprintf_s(tmp1, L"%s", content);
		m_font->DrawTextW(NULL, content, charcount, &m_contentRec, DT_TOP | DT_RIGHT, D3DCOLOR_RGBA(0, 239, 136, 250));
		m_current = tmp;
		DWORD64 tmpage = m_current - m_begin;
		if (tmpage < m_agelimit / 2)
		{
			;
		}
		else if (tmpage >= m_agelimit / 2 && tmpage < m_agelimit)
		{
			m_v = (-m_v);
		}
		else
		{
			m_Activated = false;
			m_begin = GetTickCount64();
			m_current = m_begin;
		}

	}
}
class F_Clock
{
	DWORD64 StartTime; //how many milliseconds have passed;
	DWORD64 TotalTime;
	bool Forward; //let the time lapses or not
public:
	F_Clock();
	~F_Clock();
	bool CheckRunning()
	{
		return Forward;
	}
	void Start()
	{
		StartTime = GetTickCount64();
		Forward = true;
	}
	void Pause()
	{
		Forward = false;
	}
	void Reset()
	{
		Forward = false;
		StartTime = 0;
	}
	void Update()
	{
		if (Forward)
		{
			DWORD64 timenow = GetTickCount64();
			TotalTime += (timenow - StartTime);
			StartTime = timenow;
		}
		else
			;

	}
private:

};

F_Clock::F_Clock()
{
	TotalTime = 0;
	StartTime = 0;
	Forward = false;

}

F_Clock::~F_Clock()
{
	;
}
#include "Runtime.h"
//Function To transform float to dword Type
DWORD FtoDw(float f)
{
	return *((DWORD*)&f);
}
//Particle Vertex Definition
struct Particle
{
	D3DXVECTOR3 _Pos;
	D3DCOLOR _Color;
	static const DWORD FVF;
	Particle(D3DXVECTOR3 Pos,D3DCOLOR Color):_Pos(Pos),_Color(Color){}
	Particle(){}
};
const DWORD Particle::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
//Partticle attribute struct
struct Par_Attribute
{
	D3DXVECTOR3 _Position;
	D3DXVECTOR3 _Velocity;
	D3DXVECTOR3 _Acceleration;
	float _LifeTime;  // born time
	float _age;  //age
	D3DXCOLOR _color;
	D3DXCOLOR _colorFade;
	bool _IsAlive;
	bool _Submitted;
	Par_Attribute(D3DXVECTOR3 Pos, D3DXVECTOR3 Velocity,D3DXVECTOR3 Acc,float LifeTime,float age,D3DXCOLOR color,D3DXCOLOR colorFade,bool Isalive,bool submitted)
		:_Position(Pos),_Velocity(Velocity),_Acceleration(Acc),_LifeTime(LifeTime),_age(age),_color(color),_colorFade(colorFade),_IsAlive(Isalive),_Submitted(submitted){}
};
//particle source Gun
class Gun
{
public:
	Gun(LPDIRECT3DDEVICE9 device, int ClipVolume, int backbullets);
	~Gun();
	HRESULT Init(LPCWSTR TextureFilename, D3DXVECTOR3 _origin);
	void Update(float deltTime, float V);
	void PreRender();  //to set up pre render stages
	void Render(); //redering 
	void PostRender(); //to set up post render stage
	bool ReloadClip(int *ReturnState); //reload ammo
	bool PullTrigger(D3DXVECTOR3 Origin, D3DXVECTOR3 velocity);
	BOOL IsAllBulletsDead(int* countAlive)
	{
		*countAlive = 0;
		bool haveAtleastone=false;
		for (auto p : m_Bullets)
		{
			if (p->_IsAlive)
			{
				haveAtleastone=true;
				(*countAlive)++;
			}
		}
		if (haveAtleastone)
		{
			return false;
		}
		else
			return true;
	}
	void SetBackRemains(int Supply)
	{
		if (m_BackRemains + Supply >= m_BackVolume)
		{
			m_BackRemains = m_BackVolume;
		}
		else
		{
			m_BackRemains += Supply;
		}
	}
	int GetBackVolume()
	{
		return this->m_BackVolume;
	}
	int SetBackVolume(int NewVolume)
	{
		this->m_BackVolume = NewVolume;
	}
	int GetBackRemains()
	{
		return m_BackRemains;
	}
	int GetClipRemains()
	{
		return this->m_ClipRemains;
	}
	std::vector<Par_Attribute*>* GetBulletsData()
	{
		return &m_Bullets;
	}
protected: 
	D3DXVECTOR3 m_Origin;
	D3DXVECTOR3 Position;
	D3DXVECTOR3 v;
	LPDIRECT3DDEVICE9 m_device;
	IDirect3DTexture9* m_Texture;
	LPDIRECT3DVERTEXBUFFER9 m_Buffer;
	std::vector<Par_Attribute*> m_Bullets;
	
	int m_clipVolume;        
	int m_ClipRemains;
	int m_BackRemains;
	int m_BackVolume;
};
Gun::Gun(LPDIRECT3DDEVICE9 device, int ClipVolume,int backbullets)
{
	m_device = device;
	m_clipVolume = ClipVolume;
	m_ClipRemains = m_clipVolume;
	m_BackVolume = backbullets;
	m_BackRemains = m_BackVolume;
	m_Bullets.resize(m_clipVolume);
}
Gun::~Gun()
{
	SAFE_RELEASE(m_device);
	SAFE_RELEASE(m_Buffer);
	for (int i = 0; i < m_Bullets.size(); i++)
	{
		SAFE_DELETE(m_Bullets[i]);
	}
}
//init the gun bullets at the
HRESULT Gun::Init(LPCWSTR TextureFilename,D3DXVECTOR3 _origin)
{
	HRESULT hr;
	m_Origin=_origin;
	for (int i = 0; i < m_clipVolume; i++)
	{
		m_Bullets[i] = new Par_Attribute(_origin, _origin, _origin, 0, 1000, D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f), 0xff0000, false,false); // when your dead what ever 
	}
	if (FAILED(hr = m_device->CreateVertexBuffer(m_clipVolume * sizeof(Particle),
		D3DUSAGE_DYNAMIC  | D3DUSAGE_WRITEONLY,
		Particle::FVF, D3DPOOL_DEFAULT, &this->m_Buffer, 0)))
	{
		return E_FAIL;
	}
	if (FAILED(hr = D3DXCreateTextureFromFile(m_device, TextureFilename, &m_Texture)))
	{
		return E_FAIL;
	}
}
bool Gun::PullTrigger(D3DXVECTOR3 Origin,D3DXVECTOR3 velocity)
{
	//first we should turn to query clip whether it has bullets
	if (m_ClipRemains == 0)  // Sorry dude i dont have any more bullets you should reload now
	{
		return false;
	}
	else
	{
		for (auto p : m_Bullets)
		{
			if (!p->_IsAlive)
			{
				p->_IsAlive = true;
				p->_Submitted = false;
				p->_LifeTime = GetTickCount64();
				p->_age = 0;
				p->_Position = Origin;
				p->_Velocity = velocity;
				p->_Acceleration.x = 0;
				p->_Acceleration.y = 0;
				p->_Acceleration.z = 0;
				m_ClipRemains--;
				return true;
			}
			else
			{
				;
			}
		}
		return false; //flying too many
	}
}
void Gun::PreRender()
{
	m_device->SetRenderState(D3DRS_LIGHTING, false);
	m_device->SetRenderState(D3DRS_POINTSPRITEENABLE, true);
	m_device->SetRenderState(D3DRS_POINTSCALEENABLE, true);
	m_device->SetRenderState(D3DRS_POINTSIZE,FtoDw(6.5f));
	m_device->SetRenderState(D3DRS_POINTSIZE_MIN, FtoDw(0.0f));

	// control the size of the particle relative to distance
	m_device->SetRenderState(D3DRS_POINTSCALE_A, FtoDw(0.0f));
	m_device->SetRenderState(D3DRS_POINTSCALE_B, FtoDw(0.0f));
	m_device->SetRenderState(D3DRS_POINTSCALE_C, FtoDw(1.0f));

	// use alpha from texture
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}
void Gun::Render()
{
	 PreRender();
	int numberAlive;
	if (!IsAllBulletsDead(&numberAlive))  //we still have bullet flying!
	{
		Particle* pVertices;
		m_Buffer->Lock(0, numberAlive * sizeof(Particle), (void**)&pVertices, 0);
		
		//fill out all vertices
		for (auto p : m_Bullets)
		{
			if (p->_IsAlive)
			{
				pVertices->_Pos = p->_Position;
				pVertices->_Color=(D3DCOLOR)p->_color;
				pVertices++;
			}
		}
		m_Buffer->Unlock();
		m_device->SetStreamSource(0, m_Buffer, 0, sizeof(Particle));
		m_device->SetTexture(0, m_Texture);
		m_device->SetFVF(Particle::FVF);
		m_device->DrawPrimitive(D3DPT_POINTLIST, 0, numberAlive);
		
	}
	this->PostRender();
}
void Gun::PostRender()
{
	m_device->SetRenderState(D3DRS_LIGHTING, true);
	m_device->SetRenderState(D3DRS_POINTSPRITEENABLE, false);
	m_device->SetRenderState(D3DRS_POINTSCALEENABLE, false);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
}
void Gun::Update(float deltTime,float V)
{
	for (auto p : m_Bullets)
	{
		//we only update the alive bullets 
		if (p->_IsAlive)
		{
			if (p->_age >200 )
				p->_IsAlive = false;
			p->_Position += deltTime*V *(p->_Velocity);
			p->_age = GetTickCount64() - (p->_LifeTime);

		}
	}
	Position += V * v*deltTime;
}
bool Gun::ReloadClip(int *ReturnState)
{
	int HowManyWeneed = m_clipVolume - m_ClipRemains;
	if (HowManyWeneed > 0)
	{
		if (m_BackRemains < HowManyWeneed)
		{
			if (m_BackRemains > 0)
			{
				m_ClipRemains += m_BackRemains;
				m_BackRemains = 0;
				return true;
			}
			else
			{
				*ReturnState = LOAD_CLIP_FAIL_BACK_AMMO_SCANTY;
				return false;
			}
		}
		else
		{
			m_BackRemains -= HowManyWeneed;
			m_ClipRemains = m_clipVolume;
		}
	}
	else
	{
		*ReturnState = LOAD_CLIP_FAIL_CLIP_FULL;
		return false;
	}
}
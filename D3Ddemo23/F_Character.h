#pragma once
#include"Runtime.h"
#include "F_Camera.h"
#include "F_Animator.h"
#include"F_Audio.h"
#include "F_Input.h"
#include "StaticObject.h"
#include"F_ParticleGenerator.h"

//a struct that describes your shooting behavior,which can be seen as swift action ,so we will not concern about the time lag
struct shot
{
	bool m_active;
	D3DXVECTOR3 m_shotPos;
	D3DXVECTOR3 m_shotVec;
	shot(bool active, D3DXVECTOR3 m_pos, D3DXVECTOR3 m_angle) :m_active(active), m_shotPos(m_pos), m_shotVec(m_angle) {};
};
class F_Character
{

public:

	//common attribute
	std::string m_Name; //character names  which can also be recognized as the UUID of a character
	int m_TeamIndex;   //the position your are in the team list.
	bool  m_Activated; //if not activete the player will be frozen
	bool m_IsRed;
	int m_Hp;  //Hp you are having now
	int m_Isadvane;  // -1 reverse 0 still 1 advance
	int m_IsLeft;  //-1 left  1 right 0 nochange
	bool m_IsSpeedup; //1 speed up 0 no speed up
	bool m_IsMoving; //indicates whther you are inputing any action commands.
	int m_shotTimes; //how many bullets you have used

	D3DXVECTOR3 m_VecPos;
	D3DXVECTOR3 m_VecFacing; //the moing pawn facing angle
	D3DXVECTOR3 m_VecRight; //perpendicular to the facing vector
	D3DXVECTOR3 m_VecMoveAngle; //move direction

	std::string m_IPAddress;
	//certain attribute
	shot* m_NetShot;
	F_Character();
	~F_Character();
	bool m_IsHost;
	bool m_Isonline;

	F_Animator *m_Anim_instance; //animator instance
	void Update(F_Terrain *terrain);
	void RenderYourCharacter();
	void F_Character_init(LPDIRECT3DDEVICE9 device, LPCWSTR filename, D3DXVECTOR3* InitPosition, D3DXVECTOR3 *InitForwardAngle, D3DXVECTOR3 *initFacingAngle);
	bool HitDetection(D3DXVECTOR3 p_Orig, D3DXVECTOR3 p_Direc);
private:
	LPDIRECT3DDEVICE9 m_copyofdevice;
protected://only used inside the class
	int m_AnmsetIndex; //identify your moving state
	FLOAT m_speed;                //moving speed.
	FLOAT m_advanceTime;    //adjust  speed of animation and order of it  if <1 the order is normal if not will be reversed
};
F_Character::F_Character()
{

	m_AnmsetIndex = ANM_IDLE;
	m_IsMoving = false;
	m_speed = 0.0f;
	m_Hp = 100;
	m_advanceTime = 0.015;//default is normal order
	m_Activated = false;
	m_NetShot= new shot(false, D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(1, 0, 0));
	m_IPAddress = "";

}
F_Character::~F_Character()
{
	this->m_Anim_instance->~F_Animator();
	SAFE_DELETE(this->m_Anim_instance);
}
void F_Character::Update(F_Terrain *terrain)
{

	if (this->m_Activated)
	{
		D3DXVECTOR3 tm_forward;
		D3DXVECTOR3 tm_facing;
		tm_facing = this->m_VecFacing;
		tm_forward = tm_facing;
		m_Anim_instance->SetFacingAngle(tm_facing);
		m_Anim_instance->SetAnimatorPostion(m_VecPos);
		// WE have command then we can move
		if (m_IsMoving)
		{
			if (m_Isadvane == 1) //advancing!
			{
				m_advanceTime = 0.015;
				m_AnmsetIndex = ANM_WALK;
				m_speed = 2;
				m_VecRight.y = 0;
				D3DXVec3Normalize(&m_VecRight, &m_VecRight);
				if (m_IsLeft == 1)
				{
					D3DXVec3Normalize(&tm_forward, &tm_forward);
					m_VecRight += tm_forward;
					D3DXVec3Normalize(&m_VecRight, &m_VecRight);
					m_Anim_instance->SetForwardAngle(m_VecRight);

				}
				else if (m_IsLeft == -1)
				{
					D3DXVec3Normalize(&tm_forward, &tm_forward);
					m_VecRight = (-m_VecRight + tm_forward);
					D3DXVec3Normalize(&m_VecRight, &m_VecRight);
					m_Anim_instance->SetForwardAngle(m_VecRight);
				}
				else
				{
					m_Anim_instance->SetForwardAngle(tm_forward);
				}
			}
			else if (m_Isadvane == -1)//reversing!!!
			{
				m_advanceTime = 1.015;
				m_AnmsetIndex = ANM_WALK;
				m_speed = 2;

				m_VecRight.y = 0;
				D3DXVec3Normalize(&m_VecRight, &m_VecRight);
				if (m_IsLeft == 1)
				{
					D3DXVec3Normalize(&tm_forward, &tm_forward);
					m_VecRight += (-tm_forward);
					D3DXVec3Normalize(&m_VecRight, &m_VecRight);
					m_Anim_instance->SetForwardAngle(m_VecRight);
				}
				else if (m_IsLeft == -1)
				{
					D3DXVec3Normalize(&tm_forward, &tm_forward);
					m_VecRight = (-tm_forward + (-m_VecRight));
					D3DXVec3Normalize(&m_VecRight, &m_VecRight);
					m_Anim_instance->SetForwardAngle(m_VecRight);
				}
				else
				{
					m_Anim_instance->SetForwardAngle(-tm_forward);
				}
			}
			else
			{
				if (m_IsLeft == 1) //Turn right
				{
					m_advanceTime = 0.015;
					m_AnmsetIndex = ANM_WALK;
					m_speed = 2;
					m_VecRight.y = 0;
					D3DXVec3Normalize(&m_VecRight, &m_VecRight);
					m_Anim_instance->SetForwardAngle(m_VecRight);
				}
				else if (m_IsLeft == -1) //Turn Left
				{
					m_advanceTime = 0.015;
					m_AnmsetIndex = ANM_WALK;
					m_speed = 2;
					m_VecRight.y = 0;
					D3DXVec3Normalize(&m_VecRight, &m_VecRight);
					m_Anim_instance->SetForwardAngle(-m_VecRight);
				}
				else
				{
					m_IsMoving = false;
				}
			}
			if (m_IsSpeedup)
			{
				if (m_AnmsetIndex == ANM_WALK)
				{
					m_AnmsetIndex = ANM_JOG;
					m_speed = 4;
				}

			}
		}
		//Or we dont have so we keep still
		else
		{
			m_Anim_instance->SetForwardAngle(tm_forward);
			m_speed = 0;
			m_AnmsetIndex = ANM_IDLE;
			m_advanceTime = 0.015;
		}


		switch (m_AnmsetIndex)
		{
		case ANM_IDLE:
			m_Anim_instance->SetAnimSetByindex(m_Anim_instance->GetLoiterindex());
			break;
		case ANM_WALK:
			m_Anim_instance->SetAnimSetByindex(m_Anim_instance->GetWalkindex());
			break;
		case ANM_JOG:
			m_Anim_instance->SetAnimSetByindex(m_Anim_instance->GetJogindex());
		default:
			break;
		}
		m_Anim_instance->Update(m_advanceTime, m_copyofdevice, m_speed, terrain);
	}
}
void F_Character::F_Character_init(LPDIRECT3DDEVICE9 device, LPCWSTR filename, D3DXVECTOR3* InitPosition, D3DXVECTOR3 *InitForwardAngle, D3DXVECTOR3 *initFacingAngle)
{
	m_copyofdevice = device;
	m_Anim_instance = new F_Animator();
	m_Anim_instance->F_Animator_Init(device, filename);
	this->m_VecPos = *InitPosition;
	this->m_VecFacing = *initFacingAngle;
	this->m_VecMoveAngle = *InitForwardAngle;
	m_Anim_instance->SetAnimatorPostion(m_VecPos);
	m_Anim_instance->SetForwardAngle(m_VecMoveAngle);
	m_Anim_instance->SetFacingAngle(m_VecFacing);
	delete InitPosition; //avoiding memory leaking
	delete InitForwardAngle;
	delete initFacingAngle;
}
void F_Character::RenderYourCharacter()
{
	m_Anim_instance->Render(m_copyofdevice);
}
bool F_Character::HitDetection(D3DXVECTOR3 p_Orig, D3DXVECTOR3 p_Direc)
{

}
//use thie method after the animate update operation
//void F_Character::BindToMesh()
//{
//	//get where you are!
//	D3DXVECTOR3 tm_Pos = Anim_instance->GetPosition();
//	//set camera postion
//	D3DXVECTOR3 m_VecRight_facing = Anim_instance->GetFacingAngle();
//	tm_Pos += (-m_VecRight_facing)*100;
//	m_tpCamera->SetCameraPosition(&tm_Pos);
//}
// main player means the first perspective player! who also has a communicator .
class MainPlayer
{
public:
	MainPlayer();
	~MainPlayer();
	F_Audio *m_shooting, *m_ReloadClip;
	BOOL Init(LPDIRECT3DDEVICE9 device);
	VOID Render();
	VOID Update(FLOAT deltTime, FLOAT hLimit, F_Input *input, F_Character *cha);
	//Easy funcitons
	VOID SetCameraActive(bool p)
	{
		this->m_CamActive = p;
	}
	bool GetCamraActive()
	{
		return m_CamActive;
	}
	StaticObject * GetRifle()
	{
		return m_Rifle;
	}

	F_Camera * GetCamera()
	{
		return this->m_Camera;
	}
	Gun* GetBulletSource()
	{
		return this->m_BulletSource;
	}
	std::string GetName() { return m_name; }
	void SetName(std::string name) { this->m_name = name; }

private:
	std::string m_name; // as uuid
	F_Camera * m_Camera;
	BOOL m_CamActive;//identify whether this player is activated;
	LPD3DXMESH m_cameraSphere;
	LPDIRECT3DDEVICE9 m_device;
	StaticObject *m_Rifle;
	Gun* m_BulletSource;
	int m_LastLeft;
	int m_LastRight;
};
MainPlayer::MainPlayer()
{
	m_Camera = NULL;
	m_cameraSphere = NULL;
	m_Rifle = NULL;
	m_device = NULL;
	m_BulletSource = NULL;
	m_LastLeft = LBUTTON_RELEASED;
	m_LastRight = RBUTTON_RELEASED;
	m_ReloadClip = NULL;
	m_shooting = NULL;
}
BOOL MainPlayer::Init(LPDIRECT3DDEVICE9 device)
{
	m_Camera = new F_Camera(device, CAM_FPS);
	m_CamActive = true;
	m_device = device;
	m_Camera->SetProjMatrix();
	m_Camera->SetViewMatrix();
	D3DXCreateSphere(m_device, 5, 20, 20, &m_cameraSphere, NULL);
	m_Rifle = new StaticObject();
	m_Rifle->StaticObjectInit(m_device);
	m_Rifle->LoadModel(L"GameMedia\\rifle.X");
	m_Rifle->SetTransForm();

	m_BulletSource = new Gun(m_device, 20, 300);
	m_BulletSource->Init(L"Textures\\Lazer.dds", D3DXVECTOR3(0, 0, 0));

	m_shooting = new F_Audio();
	m_shooting->InitSound(m_shooting->Getm_Xaudio(), L"bgm\\SuperLazer.wav");

	m_ReloadClip = new F_Audio();
	m_ReloadClip->InitSound(m_ReloadClip->Getm_Xaudio(), L"bgm\\ReloadClip.wav");
	return true;
}
MainPlayer::~MainPlayer()
{
	SAFE_DELETE(m_Camera);
}
VOID MainPlayer::Render()
{
	D3DXMATRIX world;
	D3DXVECTOR3 pos;
	m_Camera->GetCameraPosition(&pos);
	D3DXMatrixTranslation(&world, pos.x, pos.y, pos.z);
	m_device->SetTransform(D3DTS_WORLD, &world);
	m_cameraSphere->DrawSubset(0);
	m_Rifle->JustSetWolrdMatrixComplex();
	m_Rifle->DrawObject();

	D3DXMatrixIdentity(&world);
	m_device->SetTransform(D3DTS_WORLD, &world);

	m_BulletSource->Render();
}

VOID MainPlayer::Update(FLOAT deltTime, FLOAT hLimit, F_Input *input, F_Character* cha)
{
	if (m_CamActive)
	{
		bool ls = input->IskeyPressed(KeyBoardType, DIK_LSHIFT);
		if (ls)
		{
			m_Camera->ReactToInput(input, hLimit + 100,2 );
		}
		else
		{
			m_Camera->ReactToInput(input, hLimit + 100, 1);
		}
		
		D3DXVECTOR3 pos;
		D3DXVECTOR3 look;
		D3DXVECTOR3 right;
		D3DXVECTOR3 up;
		D3DXMATRIX world;
		m_Camera->GetCameraPosition(&pos);
		m_Camera->GetLookVector(&look);
		m_Camera->GetUpVector(&up);
		m_Camera->GetRightVector(&right);

		D3DXVec3Normalize(&up, &up);
		D3DXVec3Normalize(&look, &look);
		D3DXVec3Normalize(&right, &right);
		D3DXMatrixIdentity(&world);
		//fullfill commands
		if (cha != NULL)
		{
			bool w = input->IskeyPressed(KeyBoardType, DIK_W);
			bool s = input->IskeyPressed(KeyBoardType, DIK_S);
			bool a = input->IskeyPressed(KeyBoardType, DIK_A);
			bool d = input->IskeyPressed(KeyBoardType, DIK_D);
			
			//judge going back or forward
			if (w)
				cha->m_Isadvane = 1;
			else if (s)
				cha->m_Isadvane = -1;
			else
				cha->m_Isadvane = 0;
			//judge going left or right
			if (a)
				cha->m_IsLeft = -1;
			else if (d)
				cha->m_IsLeft = 1;
			else
				cha->m_IsLeft = 0;
			//judge going speedup or not
			if (ls)
				cha->m_IsSpeedup = true;
			else
				cha->m_IsSpeedup = false;
			//judge still or not
			if (w || s || a || d)
				cha->m_IsMoving = true;
			else
				cha->m_IsMoving = false;
			//cram moving data
			cha->m_VecFacing = look;
			cha->m_VecPos = pos;
			cha->m_VecRight = right;
			//cram bullets data
			cha->m_shotTimes = m_BulletSource->GetBackVolume() - m_BulletSource->GetBackRemains();
			auto *tmppoint = m_BulletSource->GetBulletsData();
			shot *tmpbullet = cha->m_NetShot;
			for (auto p : *tmppoint)
			{
				if (p->_IsAlive && (!p->_Submitted))
				{
					if (tmpbullet->m_active == false)
					{
						tmpbullet->m_shotPos = p->_Position;
						tmpbullet->m_shotVec = p->_Velocity;
						tmpbullet->m_active = true;
					}
					
					p->_Submitted = true;
				}
			}

		}


		world._11 = right.x;
		world._12 = right.y;
		world._13 = right.z;

		world._21 = up.x;
		world._22 = up.y;
		world._23 = up.z;

		world._31 = look.x;
		world._32 = look.y;
		world._33 = look.z;
		pos += right * 5;
		pos += look * 10;
		pos -= up * 10;
		world._41 = pos.x;// += 5);
		world._42 = pos.y;// -= 12);
		world._43 = pos.z;


		if (input->RightButton() == RBUTTON_PRESSED && m_LastRight == RBUTTON_RELEASED)
		{
			int state;
			if (!m_BulletSource->ReloadClip(&state))
			{
				;
			}
		}
		if (input->RightButton() == RBUTTON_PRESSED && m_LastRight == RBUTTON_PRESSED)
		{
			m_Camera->GetCameraPosition(&pos);
			pos -= up * 10;
			world._41 = pos.x;// += 5);
			world._42 = pos.y;// -= 12);
			world._43 = pos.z;
		}
		if (input->LeftButton() == LBUTTON_PRESSED && m_LastLeft == LBUTTON_RELEASED)
		{
			pos.y += 7.2;
			m_BulletSource->PullTrigger(pos, look);
		}
		m_LastLeft = input->LeftButton();
		m_LastRight = input->RightButton();
		m_Rifle->SetWorldMatrix(world);

		m_BulletSource->Update(0.01, 4000);

	}

}
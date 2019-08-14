#pragma once
#include"Runtime.h"
#include"F_Input.h"
#include "Helper.h"
#include "StaticObject.h"
#include"F_Skybox.h"
#include "F_Camera.h"
#include "F_Light.h"
#include"F_Terrain.h"
#include "F_Ui.h"
#include "F_Audio.h"
#include"F_Character.h"
#include "Tools.h"

class SourcesManager
{
public:
	HWND m_hwnd;
	LPDIRECT3DDEVICE9 m_device;
	bool m_blueAllKilled;
	bool m_redAllKilled;
	bool m_SignalInvite;//to send an invite signal
	bool m_SignalConn; //to send an connect signal ,valida for guest mode only,if we are online,the signal will be invalid,else we will connect to the Server;
	bool m_SignalStartGame;
	bool m_SignalGameOver;
	//this signal will fistly be handled by GUI proc if the choice is different from 
	//which team i am now,so we will update this into npcs and names else nothing will be done 
	bool m_SignalPartyGoal; //signal that i want to join one team
	HINSTANCE m_inst;
	std::vector<StaticObject*> m_StaticObjects;
	Helper  *m_CoorHelper;
	F_Skybox* m_skybox;
	F_Input *m_Myinput;
	F_Light * m_sunlight;
	LPD3DXMESH m_torus, m_cylinder;
	F_Terrain* m_terrain;
	//timer to chontrole the speed of reloading ammo
	F_Timer* m_RC_controller;
	F_Timer* m_GameData_Interval;
	std::thread* m_ExNetService;
	StaticObject *m_model;
	MainPlayer* m_MPlayer;  //one game only one main player!
	std::map<int, F_Ui*> m_UiResources;
	std::map<std::string, F_Character*> m_NPCs;
	std::string m_MName;
	std::string m_Mip;
	std::vector<std::string> m_TeamRedNames;
	std::vector<std::string> m_TeamBlueNames;
	LPD3DXFONT					 m_TextFPS, m_AmmoText;
	POINT m_MousePos;
	int m_laststate;  //to constrain the speed of shooting!
	int m_rightstate;
	wchar_t		m_strFPS[50];
	int m_gamestate;
public:
	SourcesManager(LPDIRECT3DDEVICE9 device, HWND window, HINSTANCE windIns)
	{
		m_hwnd = window;
		m_device = device;
		m_inst = windIns;
		m_TextFPS = NULL;
		m_AmmoText = NULL;
		m_Myinput = new F_Input();
		m_Myinput->Init(window, windIns);
		m_laststate = LBUTTON_RELEASED;
		m_rightstate = RBUTTON_RELEASED;
		m_gamestate = RenderStage_MainUI;
		m_SignalInvite = false;
		m_SignalConn = false;
		m_SignalStartGame = false;
		m_SignalGameOver = false;
		m_blueAllKilled = false;
		m_redAllKilled = false;

	}
	VOID RenderAllSources();
	VOID UpdateAllSources();
	HRESULT Sources_Init(HWND hwnd);


};
HRESULT SourcesManager::Sources_Init(HWND hwnd)
{

	//helper Init
	m_CoorHelper = new Helper();
	m_CoorHelper->Inithelper(m_device);
	m_CoorHelper->LoadPointsBuffer();
	//model Init
	//Skybox init
	m_skybox = new F_Skybox(m_device);
	m_skybox->LoadSkyTextureFromFile(L"GameMedia\\frontsnow1.jpg",
		L"GameMedia\\backsnow1.jpg",
		L"GameMedia\\leftsnow1.jpg", L"GameMedia\\rightsnow1.jpg", L"GameMedia\\topsnow1.jpg");
	m_skybox->InitSkyBox(10000);
	//UiResources Init
	m_UiResources.insert(std::pair<int, F_Ui*>(SRC_MAINUI, new F_Ui(L"GameMedia\\MainBack.jpg", m_device)));
	m_UiResources.insert(std::pair<int, F_Ui*>(SRC_PAUSEUI, new F_Ui(NULL, m_device)));
	m_UiResources.insert(std::pair<int, F_Ui*>(SRC_ROOMUI, new F_Ui(L"GameMedia\\RoomBack.jpg", m_device)));
	m_UiResources.insert(std::pair<int, F_Ui*>(SRC_GAMEOVERUI, new F_Ui(L"GameMedia\\GameOverBack.jpg", m_device)));
	

	//SRC_MAINUI init
	m_UiResources[SRC_MAINUI]->Additem(new F_Ui_Item(1000, 500 + F_Ui_Item::m_perH, 1260, 500 + 2 * F_Ui_Item::m_perH, Ui_Button, IDC_JOINMATCH, "JOINTMATCHBUTTON", L"JoinMatch", m_device));
	m_UiResources[SRC_MAINUI]->Additem(new F_Ui_Item(1000, 500 + 2 * F_Ui_Item::m_perH, 1260, 500 + 3 * F_Ui_Item::m_perH, Ui_Button, IDC_NEWMATCH, "NEWMATCHBUTTON", L"NewMatch", m_device));
	F_Ui_Item* tmpui = new F_Ui_Item(1000, 500 + 3 * F_Ui_Item::m_perH, 1260, 500 + 4 * F_Ui_Item::m_perH, Ui_Button, IDC_QUITGAME,
		"QUITGAMEBUTTON", L"QuitGame", m_device);
	m_UiResources[SRC_MAINUI]->Additem(tmpui);
	//SRC_PAUSEUI init
	F_Ui_Item* quitUI = new F_Ui_Item(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2 + 260, WINDOW_HEIGHT / 2 + F_Ui_Item::m_perH,
		Ui_Button, IDC_QUITGAME, "QUITGAMEBUTTON", L"QuitGame", m_device);
	m_UiResources[SRC_PAUSEUI]->Additem(quitUI);
	m_UiResources[SRC_GAMEOVERUI]->Additem(tmpui);
	//SRC_ROOMUI init
	m_UiResources[SRC_ROOMUI]->Additem(new F_Ui_Item(0, 0, 500, 100, Ui_Label, IDC_LABEL_HOST, "GAMEHOSTLABEL", L"HostMode", m_device));
	m_UiResources[SRC_ROOMUI]->Additem(new F_Ui_Item(0, 0, 500, 100, Ui_Label, IDC_LABEL_GUEST, "GAMEGUESTLABEL", L"GuestMode", m_device));
	m_UiResources[SRC_ROOMUI]->Additem(new F_Ui_Item(WINDOW_WIDTH / 2 - 100, 800, WINDOW_WIDTH / 2 + 200, 850, Ui_Button, IDC_GAMESTART, "GAMESTARTBUTTON", L"GameStart", m_device));
	m_UiResources[SRC_ROOMUI]->Additem(new F_Ui_Item(WINDOW_WIDTH / 2 - 800, 800, WINDOW_WIDTH / 2 - 500, 850, Ui_Button, IDC_TEAMRED, "JOINRED", L"JoinRed", m_device));

	m_UiResources[SRC_ROOMUI]->Additem(new F_Ui_Item(WINDOW_WIDTH / 2 + 600, 800, WINDOW_WIDTH / 2 + 900, 850, Ui_Button, IDC_TEAMBLUE, "JOINBLUE", L"JoinBlue", m_device));
	(m_UiResources[SRC_ROOMUI]->GetItems())[IDC_TEAMRED]->SetActive(true);
	(m_UiResources[SRC_ROOMUI]->GetItems())[IDC_TEAMBLUE]->SetActive(true);
	m_UiResources[SRC_ROOMUI]->Additem(new F_Ui_Item(900, 0, 1300, 50, Ui_Button, IDC_INVITE, "INVITEBUTTON", L"Invite", m_device));
	m_UiResources[SRC_ROOMUI]->Additem(new F_Ui_Item(500, 0, 900, 50, Ui_Button, IDC_CONN, "CONNECTEBUTTON", L"Connect", m_device));
	//圆环体的创建
	D3DXCreateTorus(m_device, 200.5f, 1000.2f, 25, 25,
		&m_torus, NULL);
	D3DXCreateCylinder(m_device, 100, 100, 10, 20, 20, &m_cylinder, NULL);

	//D3DXCreateText(_device, GetDC(hwnd), L"This is calling D3DXCreateText",
	//0.001f, 0.4f, &font, NULL, NULL);

	//light Init
	m_sunlight = new F_Light(m_device);
	m_sunlight->EnableLight(true);
	//Font Init
	D3DXCreateFont(m_device, 10, 0, 0, 1000, false, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, _T("Calibri"), &m_TextFPS);
	D3DXCreateFont(m_device, 20, 20, 20, 1000, false, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, _T("Calibri"), &m_AmmoText);
	// terrain init 
	m_terrain = new F_Terrain(m_device);
	m_terrain->LoadTerrainFromFile(L"GameMedia\\heighmap.raw", L"GameMedia\\terrainstone.jpg");		//从文件加载高度图和纹理
	m_terrain->InitTerrain(200, 200, 50.0f, 1.0f);  //四个值分别是顶点行数，顶点列数，顶点间间距，缩放系数

	//MainPlyer Init
	m_MPlayer = new MainPlayer();
	m_MPlayer->Init(m_device);
	m_MPlayer->SetCameraActive(false);
	std::fstream fname;
	fname.open("Name.txt", std::ios::in);
	if (!fname)
	{
		MessageBox(NULL, L"Player name INIT FAILED!", L"ERROR", 0);
	}
	char tmp[20];
	fname.getline(tmp, sizeof(tmp));
	m_MPlayer->SetName(tmp);
	fname.close();
	m_MName = tmp;

	//Audio init
	BackgroundMusic = new F_Audio();
	BackgroundMusic->InitSound(BackgroundMusic->Getm_Xaudio(), L"bgm\\MainBgm.wav");
	//BackgroundMusic->GetSourceVoice()->SetVolume(0.5);
	BackgroundMusic->PlaysoundTotheend();

	LoseAudio = new F_Audio();
	LoseAudio->InitSound(LoseAudio->Getm_Xaudio(), L"bgm\\LOSELOOP.wav");

	WinAudio = new F_Audio();
	WinAudio->InitSound(WinAudio->Getm_Xaudio(), L"bgm\\WINLOOP.wav");

	HitAudio = new F_Audio();
	HitAudio->InitSound(HitAudio->Getm_Xaudio(), L"bgm\\Hit.wav");
	//timer init
	m_RC_controller = new F_Timer(200);
	m_GameData_Interval = new F_Timer(2);
	return S_OK;
}
VOID SourcesManager::RenderAllSources()
{
	
	RECT formatRect;
	GetClientRect(m_hwnd, &formatRect);
	formatRect.left = WINDOW_WIDTH / 2 - 5;
	formatRect.right = WINDOW_WIDTH / 2 + 5;
	formatRect.top = WINDOW_HEIGHT / 2 - 5;
	formatRect.bottom = WINDOW_HEIGHT / 2 + 5;
	//light set
	
	if (m_sunlight)
		m_sunlight->EnableLight(true);
	
	//sky box set
	D3DXMATRIX matSky, matTransSky, matRotSky;
	D3DXMatrixTranslation(&matTransSky, 0.0f, -2000.0f, 0.0f);
	D3DXMatrixRotationY(&matRotSky, -0.000005f*timeGetTime());
	matSky = matTransSky * matRotSky;
	m_skybox->RenderSkyBox(&matSky, false);

	//model set

	//render terrain
	D3DXMATRIX trans;
	D3DXMatrixTranslation(&trans, 0, 0, 0);
	m_terrain->RenderTerrain(&trans, true);
	//draw every character except myself
	if (m_NPCs.size() > 1)
	{
		for (std::map<std::string, F_Character*>::iterator it = m_NPCs.begin(); it != m_NPCs.end(); it++)
		{
			F_Character *tmpcha = it->second;
			//render only the other npcs
			if (it->first != m_MName)
			{
				if (!tmpcha->m_Activated)
				{
					tmpcha->m_Activated = true;
					tmpcha->F_Character_init(m_device,
						tmpcha->m_IsRed ? L"GameMedia\\tiny_4anim_Red.x" : L"GameMedia\\tiny_4anim_Blue.x", new D3DXVECTOR3(0, 500, 0),
						new D3DXVECTOR3(1, 0, 0), new D3DXVECTOR3(1, 0, 0));
				}
				else //joined game
				{
					if (tmpcha->m_Hp <= 0)
					{
						//we do not render those players whose hp is lower than 0
					}
					else
					{
						tmpcha->RenderYourCharacter();
					}
				}
			}
		}
	}		
	//detect every bullet effect
	//helper set
	m_CoorHelper->SetStaticTransForm();
	m_CoorHelper->DrawHelper();
	D3DXMATRIX x;
	D3DXMatrixTranslation(&x, 0, 0, 0);
	this->m_device->SetTransform(D3DTS_WORLD, &x);
	m_cylinder->DrawSubset(0);




	m_MPlayer->Render();


	int charCount1 = swprintf_s(m_strFPS, 50, L"O");
	m_TextFPS->DrawText(NULL, m_strFPS, charCount1, &formatRect, DT_TOP | DT_RIGHT, D3DCOLOR_RGBA(0, 239, 136, 250));
	GetClientRect(m_hwnd, &formatRect);
	int myhp = m_NPCs[m_MName]->m_Hp;
	int charCount2 = swprintf_s(m_strFPS, 50, L"CLip %d,AMMO %d,HP %d", m_MPlayer->GetBulletSource()->GetClipRemains(), m_MPlayer->GetBulletSource()->GetBackRemains(), myhp < 0 ? 0 : myhp);
	m_AmmoText->DrawText(NULL, m_strFPS, charCount2, &formatRect, DT_TOP | DT_RIGHT, D3DCOLOR_RGBA(0, 239, 136, 250));
	
}
VOID SourcesManager::UpdateAllSources()
{
	//Read keyboard states
	m_Myinput->StateRead(KeyBoardType);
	//Read MouseState
	m_Myinput->StateRead(MouseType);
	//Get Mousepositon
	GetCursorPos(&m_MousePos);
	ScreenToClient(m_hwnd, &m_MousePos);

	//switch on the gamestate
	switch (m_gamestate)
	{
	case RenderStage_MainUI:
	{

		break;
	}
	case RenderStage_InGame:
	{
		m_MPlayer->SetCameraActive(true);
		if (m_Myinput->IskeyPressed(KeyBoardType, DIK_R))
		{
			if (m_RC_controller->DoWeHaveSignalNow())
			{
				if (m_MPlayer->GetCamraActive())
				{
					int returnState;
					if (!m_MPlayer->GetBulletSource()->ReloadClip(&returnState))
					{
						;
					}
					m_MPlayer->m_ReloadClip->PauseAudio();
					m_MPlayer->m_ReloadClip->ClearPendingbuffer();
					m_MPlayer->m_ReloadClip->PlaysoundTotheend();
				}
			}
		}
		if (m_Myinput->LeftButton() == LBUTTON_PRESSED && m_laststate == LBUTTON_RELEASED)
		{
			if (m_MPlayer->GetCamraActive())
			{
				if (m_MPlayer->GetBulletSource()->GetClipRemains() > 0)
				{
					m_MPlayer->m_shooting->PauseAudio();
					m_MPlayer->m_shooting->ClearPendingbuffer();
					m_MPlayer->m_shooting->PlaysoundTotheend();
				}
			}
		}
		else
		{
			m_MPlayer->m_shooting->ClearPendingbuffer();
		}
		if (m_Myinput->RightButton() == RBUTTON_PRESSED && m_rightstate == RBUTTON_RELEASED)
		{

		}
		D3DXVECTOR3 xz;
		m_MPlayer->GetCamera()->GetCameraPosition(&xz);
		//load my self to the npcs
		F_Character *myowncha = m_NPCs[m_MName];
		if (m_GameData_Interval->DoWeHaveSignalNow())
		{
			m_MPlayer->Update(0.01, m_terrain->GetHeightY(xz.x, xz.z), m_Myinput, myowncha);
		}
		else
			m_MPlayer->Update(0.01, m_terrain->GetHeightY(xz.x, xz.z), m_Myinput, NULL);


		//fullfill the npcs of your self
		if (m_NPCs.size() > 1)
		{
			for (std::map<std::string, F_Character*>::iterator it = m_NPCs.begin(); it != m_NPCs.end(); it++)
			{
				F_Character *tmpcha = it->second;
				if (!tmpcha->m_Activated)
				{

					tmpcha->m_Activated = true;
					tmpcha->F_Character_init(m_device,
						tmpcha->m_IsRed ? L"GameMedia\\tiny_4anim_Red.x" : L"GameMedia\\tiny_4anim_Blue.x", new D3DXVECTOR3(0, 500, 0),
						new D3DXVECTOR3(1, 0, 0), new D3DXVECTOR3(1, 0, 0));

				}
				if (tmpcha->m_Hp > 0)
				{
					tmpcha->Update(m_terrain);
				}
			}

		}

		m_laststate = m_Myinput->LeftButton();
		m_rightstate = m_Myinput->RightButton();
		LockMouse2Client(m_hwnd);
		break;
	}
	default:
		break;
	}
}
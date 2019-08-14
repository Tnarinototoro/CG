#pragma once	
#include"SourceManager.h"

LPDIRECT3DDEVICE9       g_pd3dDevice = NULL;//device interface we have to access 
std::thread* g_NetThread;
SourcesManager *AllSources;  //all Models,Components
LRESULT CALLBACK	WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);  //window message proccedure function
HRESULT				Direct3D_Init(HWND hwnd, HINSTANCE hInstance); //Devices initialization
void						Direct3D_Render(HWND hwnd);
void						Direct3D_Update(HWND hwnd);
void  ProcessGui(int id, int Message, int &gms);
void RenderGui(F_Ui *Gui, bool L_buttondown, int Mouse_X, int Mouse_Y, int& gamestate, void(*func)(int id, int Message, int & gms));
HRESULT	Objects_Init(HWND hwnd, HINSTANCE ins);
void						Direct3D_CleanUp();
float                       Get_FPS();

//transform string to vector3 ,function constrained in specific form of data ending with token ,
D3DXVECTOR3 StringToVector3(std::string input);

//return the string form vector 3 ending with token of ;
std::string Vector3ToString(D3DXVECTOR3 input)
{
	char tmpchar[100];
	sprintf_s(tmpchar, "%.7f,%.7f,%.7f,;", input.x, input.y, input.z);
	return tmpchar;
}

//Extract Seginfo to implement the Character
void ReadSegInfo(std::string *outname, std::string *Ipaddress, bool *outIsred, bool *ishost, int* teamindex, char Playerinfo[]);

//Test whether the Socket Is readable or writeable rOrw is True Test Readable,False Writeable
int TestSocketRWeadable(SOCKET s, bool rOrw, int TivSec, int TivMicroSec);


//Transform Registration information to string type
std::string S_ExtractSeginfo();

//Allot a Label for character
void CreateLabelForCharacter(F_Character *cha);

//Accessible to All users,functioning to Register a player's information into Local assemble of players
void LetaPlayerJoin(std::string tmpname, std::string tmpip, bool goalparty, bool ishost, int teamindex);

//Limited to Player Use,Send request and Wait for anwser
void P_F_RequeseAndWaitForAnswer(std::string name, std::string Ipaddress, SOCKET *PipToS, bool goalteam);

//Limited to Server use,Receive quest and Respond with answer.
void S_F_WaitforReqandSendSolution(SOCKET *PipToP, std::string* Playername);

//Extract Gamedata from a character
std::string E_F_GameDataToString(F_Character* cha);

//Trnsform GameData Pack To string if Fillcha is null decipher the name of the player and return it
std::string E_F_GameDataToCharacter(F_Character *Fillcha, std::string GameData);

void SingleThread(std::string PlayerName);
//D3D device Initiation
void UseNetService();
HRESULT Direct3D_Init(HWND hwnd, HINSTANCE hInstance)
{
	LPDIRECT3D9  pD3D = NULL;
	if (NULL == (pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;


	D3DCAPS9 caps; int vp = 0;
	if (FAILED(pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps)))
	{
		return E_FAIL;
	}
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;


	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = WINDOW_WIDTH;
	d3dpp.BackBufferHeight = WINDOW_HEIGHT;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 3;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.Windowed = true;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = 0;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;


	if (FAILED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
		hwnd, vp, &d3dpp, &g_pd3dDevice)))
		return E_FAIL;




	if (!(S_OK == Objects_Init(hwnd, hInstance))) return E_FAIL;

	SAFE_RELEASE(pD3D)

		return S_OK;
}
//D3d Resources and renderstate Initiation
HRESULT	Objects_Init(HWND hwnd, HINSTANCE ins)
{
	AllSources = new SourcesManager(g_pd3dDevice, hwnd, ins);
	AllSources->Sources_Init(hwnd);
	g_NetThread = new std::thread(UseNetService);
	//Render Stage Default settings
	AllSources->m_ExNetService = g_NetThread;
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, true);
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);   //back face culling
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f)); //set ambient
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);;
	g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	g_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	g_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, true);
	return S_OK;
}
void Direct3D_Update(HWND hwnd)
{
	AllSources->UpdateAllSources();

}
void Direct3D_Render(HWND hwnd)
{

	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	g_pd3dDevice->BeginScene();

	switch (AllSources->m_gamestate)
	{
	case RenderStage_InGame:
		AllSources->RenderAllSources();
		break;
	case RenderStage_MainUI:
		RenderGui(AllSources->m_UiResources[SRC_MAINUI],
			AllSources->m_Myinput->LeftButton() == LBUTTON_PRESSED, AllSources->m_MousePos.x,
			AllSources->m_MousePos.y, AllSources->m_gamestate,
			ProcessGui);
		break;
	case RenderStage_GameOverUI:
		RenderGui(AllSources->m_UiResources[SRC_GAMEOVERUI],
			AllSources->m_Myinput->LeftButton() == LBUTTON_PRESSED,
			AllSources->m_MousePos.x, AllSources->m_MousePos.y, AllSources->m_gamestate,
			ProcessGui);
		break;
	case RenderStage_QuitGame:
		PostMessage(hwnd, WM_QUIT, 0, 0);
		break;

	case RenderStage_GamePause:
		RenderGui(AllSources->m_UiResources[SRC_MAINUI],
			AllSources->m_Myinput->LeftButton() == LBUTTON_PRESSED,
			AllSources->m_MousePos.x, AllSources->m_MousePos.y,
			AllSources->m_gamestate, ProcessGui);
		break;
	case RenderStage_JoinGameUI:
		RenderGui(AllSources->m_UiResources[SRC_ROOMUI], AllSources->m_Myinput->LeftButton() == LBUTTON_PRESSED,
			AllSources->m_MousePos.x, AllSources->m_MousePos.y,
			AllSources->m_gamestate, ProcessGui);
		break;
	case RenderStage_HostGameUI:

		RenderGui(AllSources->m_UiResources[SRC_ROOMUI], AllSources->m_Myinput->LeftButton() == LBUTTON_PRESSED,
			AllSources->m_MousePos.x, AllSources->m_MousePos.y,
			AllSources->m_gamestate, ProcessGui);
		break;
	default:
		break;
	}
	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);


}
void Direct3D_CleanUp()
{
	SAFE_RELEASE(g_pd3dDevice)
		if (g_NetThread->joinable())
		{
			g_NetThread->join();
		}
}
float Get_FPS()
{

	static float  fps = 0;
	static int    frameCount = 0;
	static float  currentTime = 0.0f;
	static float  lastTime = 0.0f;

	frameCount++;
	currentTime = timeGetTime()*0.001f;


	if (currentTime - lastTime > 1.0f)
	{
		fps = (float)frameCount / (currentTime - lastTime);
		lastTime = currentTime;
		frameCount = 0;
	}

	return fps;
}
void RenderGui(F_Ui *Gui, bool L_buttondown, int Mouse_X, int Mouse_Y, int& gamestate, void(*func)(int id, int Message, int & gms))
{
	Gui->RenderUi();
	int eventid = 0;
	std::map<int, F_Ui_Item*> items = Gui->GetItems();
	for (std::map<int, F_Ui_Item*>::iterator it = items.begin(); it != items.end(); it++)
	{

		it->second->F_UI_Item_Draw(Gui->GetDevice(), Mouse_X, Mouse_Y, L_buttondown);
		if (L_buttondown)
		{
			if (it->second->GetTouchable())
			{
				if (it->second->AboveItem(Mouse_X, Mouse_Y))
				{
					eventid = it->second->GetId();
				}

			}
		}
		else
		{
			;
		}

	}
	if (L_buttondown)
		if (func)
			func(eventid, GUI_MESSAGE_CLICKED, gamestate);


}
void  ProcessGui(int id, int Message, int &gms)
{
	if (Message == GUI_MESSAGE_CLICKED)
	{
		switch (id)
		{

		case IDC_GAMESTART:
		{

			BackgroundMusic->PauseAudio();
			AllSources->m_SignalStartGame = true;
			ShowCursor(false);
			ShowCursor(false);
			break;
		}
		case IDC_TEAMRED:
		{
			AllSources->m_SignalPartyGoal = true; //give a signal to change to red party 
			break;
		}
		case IDC_TEAMBLUE:
		{

			AllSources->m_SignalPartyGoal = false; //give a signal to change to blue party
			break;
		}
		case IDC_NEWMATCH:
		{
			if (AllSources->m_Mip.length() > 3)
			{
				BackgroundMusic->PauseAudio();
				LetaPlayerJoin(AllSources->m_MName, AllSources->m_Mip, true, true, 0);
				(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[IDC_LABEL_GUEST]->SetActive(false);
				(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[IDC_LABEL_HOST]->SetActive(true);
				(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[IDC_INVITE]->SetActive(true);
				(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[IDC_CONN]->SetActive(false);
				(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[IDC_TEAMRED]->SetActive(false);
				(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[IDC_TEAMBLUE]->SetActive(false);

				gms = RenderStage_HostGameUI;
				ShowCursor(true);
				break;
			}
			else
			{
				break;
			}
			break;

		}
		case IDC_QUITGAME:
		{
			BackgroundMusic->PauseAudio();
			gms = RenderStage_QuitGame;
			PostMessage(NULL, WM_DESTROY, 0, 0);
			break;
		}
		case IDC_JOINMATCH:
		{
			if (AllSources->m_Mip.length() > 3)
			{
				BackgroundMusic->PauseAudio();
				(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[IDC_LABEL_HOST]->SetActive(false);
				(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[IDC_LABEL_GUEST]->SetActive(true);
				(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[IDC_GAMESTART]->SetActive(false);
				(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[IDC_INVITE]->SetActive(false);
				gms = RenderStage_JoinGameUI;
				ShowCursor(true);
				break;
			}
			else
				break;
			break;

		}
		case IDC_INVITE:
		{
			AllSources->m_SignalInvite = true;
			break;
		}
		case IDC_CONN:
		{
			if (AllSources->m_NPCs.find(AllSources->m_MName) == AllSources->m_NPCs.end())
			{
				AllSources->m_SignalConn = true;
			}
			else
			{
				AllSources->m_SignalConn = false;
			}
			break;
		}
		default:
		{
			break;
		}
		}
	}
}
void LetaPlayerJoin(std::string tmpname, std::string tmpip, bool goalparty, bool ishost, int teamindex)
{
	//if this person is not in this game yet appearls
	if (AllSources->m_NPCs.find(tmpname) == AllSources->m_NPCs.end())
	{

		F_Character *tmpcha = new F_Character();
		tmpcha->m_Isonline = true;
		tmpcha->m_Activated = false;
		tmpcha->m_IsRed = goalparty;
		tmpcha->m_IsHost = ishost;
		tmpcha->m_IPAddress = tmpip;
		tmpcha->m_Name = tmpname;
		tmpcha->m_TeamIndex = teamindex;
		//red team still spare;
		if (goalparty ? AllSources->m_TeamRedNames.size() < 4 : AllSources->m_TeamBlueNames.size() < 4)
		{

			(goalparty ? AllSources->m_TeamRedNames : AllSources->m_TeamBlueNames).push_back(tmpname);
			tmpcha->m_TeamIndex = (goalparty ? AllSources->m_TeamRedNames : AllSources->m_TeamBlueNames).size();
			AllSources->m_SignalPartyGoal = tmpcha->m_IsRed;
		}
		else //wish space is full you have to change to opposite party
		{
			(goalparty ? AllSources->m_TeamBlueNames : AllSources->m_TeamRedNames).push_back(tmpname);
			tmpcha->m_IsRed = !(tmpcha->m_IsRed);
			tmpcha->m_TeamIndex = (goalparty ? AllSources->m_TeamBlueNames : AllSources->m_TeamRedNames).size();
			AllSources->m_SignalPartyGoal = tmpcha->m_IsRed;
		}


		AllSources->m_NPCs.insert(std::pair<std::string, F_Character*>(tmpname, tmpcha));

		//then we will create  label or  change the existing one
		CreateLabelForCharacter(tmpcha);

	}
	//if this player exists in the room so we need only to update it according to the request
	else
	{
		F_Character *tmpcha = AllSources->m_NPCs[tmpname];
		int oldindex = tmpcha->m_TeamIndex;
		bool oldisred = tmpcha->m_IsRed;
		int oldidc_id = oldindex + (oldisred ? IDC_PLAYER_RED_ID_BASE : IDC_PLAYER_BLUE_ID_BASE);
		if (oldisred == goalparty);//no change so we do nothing
		else
		{
			//we have vacant spcae for change
			if (goalparty ? (AllSources->m_TeamRedNames.size() < 4) : (AllSources->m_TeamBlueNames.size() < 4))
			{
				bool meettheplayer;
				//first erase it from the origin party names vector and push it to the goalparty name
				std::vector<std::string>* tmpPtr = &(oldisred ? (AllSources->m_TeamRedNames) : (AllSources->m_TeamBlueNames));
				std::vector<std::string>* tmpOppositePtr = &(oldisred ? (AllSources->m_TeamBlueNames) : (AllSources->m_TeamRedNames));
				std::vector<std::string>::iterator it = tmpPtr->begin();
				for (; it != tmpPtr->end(); it++)
				{
					if (*it == tmpname)
						break; //found it
				}
				tmpOppositePtr->push_back(tmpname);
				tmpPtr->erase(it);
				AllSources->m_NPCs[tmpname]->m_IsRed = !oldisred;
				AllSources->m_NPCs[tmpname]->m_TeamIndex = tmpOppositePtr->size();
				AllSources->m_SignalPartyGoal = AllSources->m_NPCs[tmpname]->m_IsRed;
				F_Ui_Item * tmpitem = (AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[oldidc_id];
				tmpitem->SetActive(false);

				CreateLabelForCharacter(AllSources->m_NPCs[tmpname]);
				if (oldindex != tmpPtr->size())
				{
					for (int i = oldindex - 1; i < tmpPtr->size(); i++)
					{
						std::string nnnnnm = ((*tmpPtr)[i]);
						int otheridc_id = (oldisred ? IDC_PLAYER_RED_ID_BASE : IDC_PLAYER_BLUE_ID_BASE) + AllSources->m_NPCs[nnnnnm]->m_TeamIndex;
						(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[otheridc_id]->SetActive(false);
						AllSources->m_NPCs[nnnnnm]->m_TeamIndex--;
						CreateLabelForCharacter(AllSources->m_NPCs[nnnnnm]);
					}
				}
			}
			else
			{
				//opposite positon is full insert failed,so keep it
				AllSources->m_SignalPartyGoal = tmpcha->m_IsRed;//need do nothing!
			}



		}
	}
}
//Extract all of the registered players information 
std::string S_ExtractSeginfo()
{
	std::string tmpstr = "";
	if (AllSources->m_TeamRedNames.size() > 0)
	{
		for (std::string m : AllSources->m_TeamRedNames)
		{
			char tmp[50];
			sprintf_s(tmp, "%s;%s;%d;%d;%d;\n", m.c_str(), AllSources->m_NPCs[m]->m_IPAddress.c_str(), (int)(AllSources->m_NPCs[m]->m_IsRed), (int)(AllSources->m_NPCs[m]->m_IsHost), AllSources->m_NPCs[m]->m_TeamIndex);
			tmpstr += tmp;
		}
	}
	if (AllSources->m_TeamBlueNames.size() > 0)
	{
		for (std::string m : AllSources->m_TeamBlueNames)
		{
			char tmp[50];
			sprintf_s(tmp, "%s;%s;%d;%d;%d;\n", m.c_str(), AllSources->m_NPCs[m]->m_IPAddress.c_str(), (int)(AllSources->m_NPCs[m]->m_IsRed), (int)(AllSources->m_NPCs[m]->m_IsHost), AllSources->m_NPCs[m]->m_TeamIndex);
			tmpstr += tmp;
		}
	}
	return tmpstr;
}
int TestSocketRWeadable(SOCKET s, bool rOrw, int TivSec, int TivMicroSec)
{
	int maxfdp = NULL;
	//set a socket list
	struct fd_set fds;
	//set a timeval
	struct timeval timeOut = { TivSec,TivMicroSec };
	//clear the socket list;
	FD_ZERO(&fds);
	//set the h_listener into the socket list;
	FD_SET(s, &fds);
	//test whether h_listener is still in the fds,if not meaning that there is no connection coming
	//test success we have a connection
	if (rOrw)//true test readable
	{
		return select(maxfdp, &fds, NULL, NULL, &timeOut);
	}
	else
	{
		return select(maxfdp, NULL, &fds, NULL, &timeOut);
	}
}
void ReadSegInfo(std::string *outname, std::string *Ipaddress, bool *outIsred, bool *ishost, int* teamindex, char Playerinfo[])
{
	std::string infoSeg = "";
	//fotmat of the info is name;isred
	int i = 0;
	for (char* start = Playerinfo; *start != '\n'; start++)
	{
		if (*start == ';')
		{
			i++;
			if (i == 1)
			{
				//name coming
				*outname = infoSeg;
				infoSeg = "";


			}
			else if (i == 2)
			{
				*Ipaddress = infoSeg;
				infoSeg = "";
			}
			else if (i == 3)
			{
				if (atoi(infoSeg.c_str()) == 1)
					*outIsred = true;
				else
					*outIsred = 0;
				infoSeg = "";
			}
			else if (i == 4)
			{
				if (atoi(infoSeg.c_str()) == 1)
					*ishost = true;
				else
					*ishost = false;
				infoSeg = "";
			}
			else //i==5
			{
				*teamindex = atoi(infoSeg.c_str());
				break;
			}

		}
		else
		{
			infoSeg += (*start);
		}
	}
}
void CreateLabelForCharacter(F_Character *cha)
{
	wchar_t tmpstr[20];
	int len = MultiByteToWideChar(CP_ACP, 0, cha->m_Name.c_str(), -1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, cha->m_Name.c_str(), -1, tmpstr, len);
	int x = cha->m_IsRed ? F_Ui_Item::m_redX : F_Ui_Item::m_blueX;
	int y = F_Ui_Item::m_baseY + cha->m_TeamIndex*F_Ui_Item::m_perH;
	int idc_id = (cha->m_IsRed ? IDC_PLAYER_RED_ID_BASE : IDC_PLAYER_BLUE_ID_BASE) + cha->m_TeamIndex;

	//No that label means that we jjust need to create a new label
	if ((AllSources->m_UiResources[SRC_ROOMUI]->GetItems()).find(idc_id) == (AllSources->m_UiResources[SRC_ROOMUI]->GetItems()).end())
	{
		AllSources->m_UiResources[SRC_ROOMUI]->Additem(new F_Ui_Item(x, y, x + 500, y + F_Ui_Item::m_perH, Ui_Label,
			idc_id, "CommonPlayer", tmpstr, AllSources->m_device));
		if (cha->m_IsHost)
			(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[idc_id]->SetLabelColor(0xffff00ff);
		else
		{
			if (cha->m_IsRed)
				(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[idc_id]->SetLabelColor(0xffff0000);
			else
				(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[idc_id]->SetLabelColor(0xff0000ff);
		}
	}
	else
	{
		F_Ui_Item *tmpitem = (AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[idc_id];
		if (cha->m_IsHost)
			tmpitem->SetLabelColor(0xffff00ff);
		else
		{
			if (cha->m_IsRed)
				tmpitem->SetLabelColor(0xffff0000);
			else
				tmpitem->SetLabelColor(0xff0000ff);
		}
		RECT tmprec;
		tmprec.left = x;
		tmprec.top = y;
		tmprec.right = x + 500;
		tmprec.bottom = y + F_Ui_Item::m_perH;
		tmpitem->SetItemRect(tmprec);
		tmpitem->SetContent(tmpstr);
		tmpitem->SetActive(true);

	}



}
void P_F_RequeseAndWaitForAnswer(std::string name, std::string Ipaddress, SOCKET *PipToS, bool goalteam)
{
	char bufferstr[50];
	sprintf_s(bufferstr, "%s;%s;%d;%d;%d\n", name.c_str(), Ipaddress.c_str(), (int)goalteam, (int)false, 0);
	//give server my info
	send(*PipToS, bufferstr, 50, 0);

	char bufferinfoServer[200];
	recv(*PipToS, bufferinfoServer, 200, 0);

	std::string ServerMsg = bufferinfoServer;
	std::string tmpchainfo = "";
	for (char p : ServerMsg)
	{
		if (p == '\n')
		{
			//get a seg;
			tmpchainfo += '\n';
			std::string tmpsegname;
			std::string tmpip;
			bool tmpsegIsred;
			bool tmpSegIshost;
			int tmpteamindex;
			char tmpsegchar[200];
			strcpy_s(tmpsegchar, tmpchainfo.c_str());
			ReadSegInfo(&tmpsegname, &tmpip, &tmpsegIsred, &tmpSegIshost, &tmpteamindex, tmpsegchar);
			LetaPlayerJoin(tmpsegname, tmpip, tmpsegIsred, tmpSegIshost, tmpteamindex);
			//complete extracting
			tmpchainfo = "";
		}
		else
		{
			tmpchainfo += p;
		}
	}
}
void S_F_WaitforReqandSendSolution(SOCKET *PipToP, std::string *playername)
{
	char Playerinfo[50];
	//wait until socket get transmitted the info of himself
	recv(*PipToP, Playerinfo, 50, 0);
	/*std::fstream fs;
	fs.open("serverlog.txt", std::ios::out);
	fs << Playerinfo;
	fs.flush();*/
	std::string tmpname;
	std::string tmpip;
	bool tmpIstred;
	bool tmpishost;
	int tmpteamindex;
	ReadSegInfo(&tmpname, &tmpip, &tmpIstred, &tmpishost, &tmpteamindex, Playerinfo);
	*playername = tmpname;
	LetaPlayerJoin(tmpname, tmpip, tmpIstred, tmpishost, tmpteamindex);


	// You are in the room right now;
	//Then send All information to every player
	std::string tmpstr2 = S_ExtractSeginfo();
	/*fs << tmpstr2;
	fs.flush();
	fs.close();*/
	send(*PipToP, tmpstr2.c_str(), 200, 0);
}
//End with token '\n'
std::string E_F_GameDataToString(F_Character* cha)
{
	std::string tmpstr = "";
	//plus this vector
	//std::vector<shot*> m_shotLists;
	//fill the fundamental information
	char tmp[1024];
	sprintf_s(tmp,
		"%s;%d;%d;%d;%d;%d;%d;%d;%d;%d;%.7f,%.7f,%.7f,;%.7f,%.7f,%.7f,;%.7f,%.7f,%.7f,;%.7f,%.7f,%.7f,;%d;%.7f,%.7f,%.7f,;%.7f,%.7f,%.7f,;",
		cha->m_Name.c_str(),
		cha->m_TeamIndex,
		cha->m_Activated,
		cha->m_IsRed,
		cha->m_Hp,
		cha->m_Isadvane,
		cha->m_IsLeft,
		cha->m_IsSpeedup,
		cha->m_IsMoving,
		cha->m_shotTimes,
		cha->m_VecPos.x,
		cha->m_VecPos.y,
		cha->m_VecPos.z,
		cha->m_VecFacing.x,
		cha->m_VecFacing.y,
		cha->m_VecFacing.z,
		cha->m_VecRight.x,
		cha->m_VecRight.y,
		cha->m_VecRight.z,
		cha->m_VecMoveAngle.x,
		cha->m_VecMoveAngle.y,
		cha->m_VecMoveAngle.z,
		cha->m_NetShot->m_active,
		cha->m_NetShot->m_shotPos.x,
		cha->m_NetShot->m_shotPos.y,
		cha->m_NetShot->m_shotPos.z,
		cha->m_NetShot->m_shotVec.x,
		cha->m_NetShot->m_shotVec.y,
		cha->m_NetShot->m_shotVec.z
	);
	tmpstr = tmp;
	tmpstr += "\n";
	return tmpstr;
}
D3DXVECTOR3 StringToVector3(std::string input)
{
	D3DXVECTOR3 result;
	int i = 0;
	std::string tmp = "";
	for (char p : input)
	{

		if (p == ',')
		{
			i++;
			if (i == 1)
				result.x = (float)atof(tmp.c_str());
			else if (i == 2)
				result.y = (float)atof(tmp.c_str());
			else if (i == 3)
				result.z = (float)atof(tmp.c_str());
			else
				;
			tmp = "";
		}
		else
		{
			tmp += p;
		}
	}
	return result;
}
std::string E_F_GameDataToCharacter(F_Character *Fillcha, std::string GameData)
{
	std::string tmpstr = "";
	int index = 0;
	bool tmpact = false;
	for (char per : GameData)
	{

		if (per == ';')
		{
			index++;
			if (index < 18)
			{
				switch (index)
				{
					//std::string m_Name;
				case 1:
				{
					if (Fillcha == NULL)
					{
						return tmpstr;
					}
					else
					{
						Fillcha->m_Name = tmpstr;
					}
					break;
				}
				//int m_TeamIndex; 
				case 2:
				{
					Fillcha->m_TeamIndex = atoi(tmpstr.c_str());
					break;
				}
				//bool  m_Activated; 
				case 3:
				{
					Fillcha->m_Activated = (bool)atoi(tmpstr.c_str());
					break;
				}
				//bool m_IsRed;
				case 4:
				{
					Fillcha->m_IsRed = (bool)atoi(tmpstr.c_str());
					break;
				}
				//int m_Hp;
				case 5:
				{
					int tmhp = atoi(tmpstr.c_str());
					if (Fillcha->m_Hp < tmhp)
					{
						;
					}
					else
					{
						Fillcha->m_Hp = atoi(tmpstr.c_str());
					}
					break;
				}
				//int m_Isadvane;
				case 6:
				{
					Fillcha->m_Isadvane = (bool)atoi(tmpstr.c_str());
					break;
				}
				//int m_IsLeft;
				case 7:
				{
					Fillcha->m_IsLeft = (bool)atoi(tmpstr.c_str());
					break;
				}
				//bool m_IsSpeedup; 
				case 8:
				{
					Fillcha->m_IsSpeedup = (bool)atoi(tmpstr.c_str());
					break;
				}
				//bool m_IsMoving;
				case 9:
				{
					Fillcha->m_IsMoving = (bool)atoi(tmpstr.c_str());
					break;
				}
				//int m_shotTimes; 
				case 10:
				{
					Fillcha->m_shotTimes = atoi(tmpstr.c_str());
					break;
				}
				//D3DXVECTOR3 m_VecPos;
				case 11:
				{
					Fillcha->m_VecPos = StringToVector3(tmpstr);
					break;
				}
				//D3DXVECTOR3 m_VecFacing; //the moing pawn facing angle
				case 12:
				{
					Fillcha->m_VecFacing = StringToVector3(tmpstr);
					break;
				}
				//D3DXVECTOR3 m_VecRight; //perpendicular to the facing vector
				case 13:
				{
					Fillcha->m_VecRight = StringToVector3(tmpstr);
					break;
				}
				//D3DXVECTOR3 m_VecMoveAngle; //move direction
				case 14:
				{
					Fillcha->m_VecMoveAngle = StringToVector3(tmpstr);
					break;
				}
				case 15:
				{
					Fillcha->m_NetShot->m_active = (bool)atoi(tmpstr.c_str());
				}
				case 16:
				{
					Fillcha->m_NetShot->m_shotPos = StringToVector3(tmpstr);
				}
				case 17:
				{
					Fillcha->m_NetShot->m_shotVec = StringToVector3(tmpstr);
				}
				default:
					break;
				}

			}
			tmpstr = "";


		}
		else
		{
			tmpstr += per;
		}

	}

	return "";
}
//Assigned thread for information exchange from every player
void UseNetService()
{
	//service init
	WSADATA data;
	char Myhostname[50];
	char *MyhostIp;

	std::string p_ServerIp;
	std::map<std::string, SOCKET*> PLayersSockets; //only used by server
	//std::map<std::string,
	in_addr hostipaddr;
	bool Ishost;
	std::mutex netmutex;
	HWND hwnd = AllSources->m_hwnd;
	std::fstream serverlog;
	serverlog.open("serverlog.txt", std::ios::out);
	F_Timer *GameData_Interval;
	GameData_Interval = new F_Timer(2);

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		MessageBox(hwnd, L"NET INIT FAILED!", L"ERROR", 0);
	if (gethostname(Myhostname, sizeof(Myhostname)) != 0)
		MessageBox(hwnd, L"HOSTNAME INIT FAILED!", L"ERROR", 0);
	hostent *tmp = gethostbyname(Myhostname);
	if (tmp == NULL)
		MessageBox(hwnd, L"GET HOST IP FIALED!", L"ERROR", 0);
	else
		memcpy(&hostipaddr, tmp->h_addr_list[0], sizeof(in_addr));
	MyhostIp = inet_ntoa(hostipaddr);
	netmutex.lock();
	AllSources->m_Mip = MyhostIp;
	netmutex.unlock();
	//Start service
	int state;
	netmutex.lock();
	state = AllSources->m_gamestate;
	netmutex.unlock();

	bool h_isbind = false; //evade binding to one port twice;
	SOCKET castsock;
	SOCKET h_listenSocket;
	SOCKET p_PiptoServer; //as const value for player
	SOCKET *h_PipToplayer; //tmp variable
	h_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	p_PiptoServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (h_listenSocket == INVALID_SOCKET)
		MessageBox(NULL, L"listen socket failed!", L"error", 0);
	if (p_PiptoServer == INVALID_SOCKET)
		MessageBox(NULL, L"pip to server socket failed!", L"error", 0);
	castsock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF | WSA_FLAG_OVERLAPPED);
	//Cast socket which you will use to send Radio message;
	sockaddr_in udpAddress;
	udpAddress.sin_family = AF_INET;
	udpAddress.sin_port = htons(PODCAST_PORT);
	udpAddress.sin_addr.S_un.S_addr = inet_addr(MyhostIp);
	//Cast Group that you will send info to or receive from ,you can perceive it as an intermedia
	sockaddr_in castgroup;
	castgroup.sin_family = AF_INET;
	castgroup.sin_port = htons(PODCAST_PORT);
	castgroup.sin_addr.S_un.S_addr = inet_addr(RADIO_IP);
	//Get the host addr for listener
	sockaddr_in host;
	host.sin_family = AF_INET;
	host.sin_port = htons(GAMEDATA_EXCHANGE_PORT);
	host.sin_addr.S_un.S_addr = inet_addr(MyhostIp);


	if (bind(castsock, (SOCKADDR*)&udpAddress, sizeof(udpAddress)) != 0)
		MessageBox(NULL, L"Bind CastSocke Failed", L"Error", 0);
	SOCKET sockM = WSAJoinLeaf(castsock, (SOCKADDR*)&castgroup,
		sizeof(castgroup), NULL, NULL, NULL, NULL, JL_BOTH);
	if (sockM == INVALID_SOCKET)
		MessageBox(NULL, L"Join Room Failed!", L"Error", 0);


	while (state != RenderStage_QuitGame)
	{
		//internet activity occurs once in XXXms
		if (state == RenderStage_HostGameUI)
		{
			//entered host ui so you will be the server;
			Ishost = true;

			//Host icon color changed
			netmutex.lock();
			((AllSources->m_UiResources[SRC_ROOMUI])->GetItems())[IDC_LABEL_HOST]->SetLabelColor(D3DCOLOR_RGBA(0, 230, 0, 125));
			((AllSources->m_UiResources[SRC_ROOMUI])->GetItems())[IDC_LABEL_HOST]->SetContent(L"HostOnline");
			((AllSources->m_UiResources[SRC_ROOMUI])->GetItems())[IDC_CONN]->SetActive(false);
			netmutex.unlock();

			//entered the server funciton loop;
			//postulate that all the Guest players all in the room ui in guest mode,waiting for the information of a new match 
			//what you are gonna do is 
			//two phased first recruiting step start :at the beginning of the hostmode ,end  at the sum  of the players from teamred and team blue is up to the limit;
			while (state == RenderStage_HostGameUI)
			{

				netmutex.lock();
				if (AllSources->m_SignalInvite)//when a siganl is given we send an invitation to ervery one;
				{
					sendto(castsock, MyhostIp, 50, 0, (SOCKADDR*)&castgroup, sizeof(castgroup)); //Give Server ip to everyone
					MessageBox(NULL, L"Send Invite Success!", L"Note!", 0);
					AllSources->m_SignalInvite = false;
				}
				netmutex.unlock();


				// if not bind,  so bind it 
				if (!h_isbind)
				{
					if (bind(h_listenSocket, (SOCKADDR*)&host, sizeof(host)) == SOCKET_ERROR)
						MessageBox(NULL, L"bind listener Failed!", L"Note!", 0);
					else
						h_isbind = true;
				}
				//listen 
				if (listen(h_listenSocket, 8) != 0)
					MessageBox(NULL, L"Listen Failed!", L"Error", 0);

				//if we can still accept other players then we accept 
				netmutex.lock();
				if (AllSources->m_NPCs.size() < 8) //if all npcs including myself is less than 8 we can still accept more players
				{

					if (TestSocketRWeadable(h_listenSocket, true, 0, 10))//readable
					{
						h_PipToplayer = new SOCKET();
						*h_PipToplayer = accept(h_listenSocket, NULL, NULL);
						//get a complete socket
						if (*h_PipToplayer != INVALID_SOCKET)
						{
							std::string player;
							S_F_WaitforReqandSendSolution(h_PipToplayer, &player);
							PLayersSockets.insert(std::pair<std::string, SOCKET *>(player, h_PipToplayer));
							h_PipToplayer = NULL;
						}
					}
					// no response
					else
					{
						;
					}



				}
				//if we have socket established we tend to do some responce
				if (PLayersSockets.size() >= 1)
				{
					for (std::map<std::string, SOCKET*>::iterator it = PLayersSockets.begin(); it != PLayersSockets.end(); it++)
					{
						if (*(it->second) != INVALID_SOCKET)
						{
							SOCKET tmp = *(it->second);
							if (AllSources->m_SignalStartGame)
							{
								//Clear all the info blocked in the queue
								while (TestSocketRWeadable(tmp, true, 0, 1))
								{
									char ttmp[50];
									recv(tmp, ttmp, 50, 0);
								}
								send(tmp, "START", 200, 0);
							}
							else //not start signal we can do party changing!
							{
								if (TestSocketRWeadable(tmp, true, 0, 10))
								{
									std::string playername;
									S_F_WaitforReqandSendSolution(&tmp, &playername);


								}
							}
						}
					}
					if (AllSources->m_SignalStartGame)
					{
						AllSources->m_SignalStartGame = false;
						AllSources->m_gamestate = RenderStage_InGame;

					}
					else;
				}
				else
				{
					;
				}
				std::string tmpstr2 = S_ExtractSeginfo();
				for (std::map<std::string, SOCKET*>::iterator its = PLayersSockets.begin(); its != PLayersSockets.end(); its++)
				{
					send(*(its->second), tmpstr2.c_str(), 200, 0);
				}
				netmutex.unlock();






				netmutex.lock();
				state = AllSources->m_gamestate;
				netmutex.unlock();
				Sleep(100);
			}

			closesocket(castsock);
			closesocket(sockM);
		}

		else if (state == RenderStage_JoinGameUI)
		{

			Ishost = false;
			netmutex.lock();
			((AllSources->m_UiResources[SRC_ROOMUI])->GetItems())[IDC_LABEL_GUEST]->SetLabelColor(D3DCOLOR_RGBA(0, 230, 0, 125));
			netmutex.unlock();

			while (state == RenderStage_JoinGameUI)
			{

				//if we get the server room
				if (atoi(p_ServerIp.c_str()) == 192) //we got the serverip;
				{
					netmutex.lock();
					//if not connectted to room then connect
					//We have signal that we should now connect to the server now

					//we find myself which indicate that we have successfully registered the game from the server
					if (AllSources->m_NPCs.find(AllSources->m_MName) != AllSources->m_NPCs.end())
					{
						F_Character *tmpcha = AllSources->m_NPCs[AllSources->m_MName];
						char tmpMsg[200];
						if (AllSources->m_SignalPartyGoal == tmpcha->m_IsRed) //there is goal is no opposite so we just need to wait for signal to start game or receive the update info of o
						{
							while (TestSocketRWeadable(p_PiptoServer, true, 0, 10))//update all of the information from the server
							{
								recv(p_PiptoServer, tmpMsg, 200, 0);

								std::string result = tmpMsg;
								if (result == "START")
								{
									AllSources->m_gamestate = RenderStage_InGame;
									while (TestSocketRWeadable(p_PiptoServer, true, 0, 1))
									{
										recv(p_PiptoServer, tmpMsg, 200, 0);
									}

								}
								else //it is not start message
								{

									std::string tmpchainfo = "";
									for (char p : result)
									{
										if (p == '\n')
										{
											//get a seg;
											tmpchainfo += '\n';
											std::string tmpsegname;
											std::string tmpip;
											bool tmpsegIsred;
											bool tmpSegIshost;
											int tmpteamindex;
											char tmpsegchar[200];
											strcpy_s(tmpsegchar, tmpchainfo.c_str());
											ReadSegInfo(&tmpsegname, &tmpip, &tmpsegIsred, &tmpSegIshost, &tmpteamindex, tmpsegchar);
											LetaPlayerJoin(tmpsegname, tmpip, tmpsegIsred, tmpSegIshost, tmpteamindex);
											//complete extracting
											tmpchainfo = "";
										}
										else
										{
											tmpchainfo += p;
										}
									}
								}

							}
						}
						else //goal different we need to change player positon
						{
							if (AllSources->m_SignalPartyGoal != AllSources->m_NPCs[AllSources->m_MName]->m_IsRed)
								P_F_RequeseAndWaitForAnswer(tmpcha->m_Name, AllSources->m_NPCs[AllSources->m_MName]->m_IPAddress, &p_PiptoServer, AllSources->m_SignalPartyGoal);

						}

						//AllSources->m_gamestate = RenderStage_InGame;
						//break;


					}
					else
					{
						if (AllSources->m_SignalConn)
						{
							sockaddr_in tmp;
							tmp.sin_addr.S_un.S_addr = inet_addr(p_ServerIp.c_str());
							tmp.sin_family = AF_INET;
							tmp.sin_port = htons(GAMEDATA_EXCHANGE_PORT);

							int ret = connect(p_PiptoServer, (sockaddr*)&tmp, sizeof(tmp));

							if (ret == SOCKET_ERROR)
							{
								MessageBox(NULL, L"Connect To Server Failed!", L"Error", 0);

							}
							else
							{
								MessageBox(NULL, L"Connect to server Success!", L"Note", 0);
								(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[IDC_TEAMRED]->SetActive(true);
								(AllSources->m_UiResources[SRC_ROOMUI]->GetItems())[IDC_TEAMBLUE]->SetActive(true);
								P_F_RequeseAndWaitForAnswer(AllSources->m_MName, AllSources->m_Mip, &p_PiptoServer, false);
								AllSources->m_SignalConn = false;
								AllSources->m_SignalPartyGoal = AllSources->m_SignalPartyGoal != AllSources->m_NPCs[AllSources->m_MName]->m_IsRed;

							}

						}
						//we Dont have signal so do nothing
						else;
					}

					netmutex.unlock();



				}
				//or we dont so we should loop to get it;
				else
				{

					if (TestSocketRWeadable(castsock, true, 0, 10))
					{
						char buffeer[50];
						recvfrom(castsock, buffeer, 50, 0, NULL, NULL);

						p_ServerIp = buffeer;
					}
				}




				//updte stage 
				netmutex.lock();
				state = AllSources->m_gamestate;
				netmutex.unlock();
				Sleep(100);
			}

		}
		else if (state == RenderStage_InGame || state == RenderStage_GamePause)
		{
			//clear all information
			if (Ishost)
			{
				F_Timer * host = new F_Timer(50);
				for (std::map<std::string, SOCKET*>::iterator it = PLayersSockets.begin(); it != PLayersSockets.end(); it++)
				{
					while (TestSocketRWeadable(*(it->second), true, 0, 2))
					{
						char clearchar[GAMEDATA_PACKAGE_SIZE];
						recv(*(it->second), clearchar, GAMEDATA_PACKAGE_SIZE, 0);
					}
					if (host->DoWeHaveSignalNow())
					{
						break;
					}
				}
				delete(host);
			}
			else
			{
				F_Timer *client = new F_Timer(50);
				while (TestSocketRWeadable(p_PiptoServer, true, 0, 2))
				{
					char clearchar[GAMEDATA_PACKAGE_SIZE];
					recv(p_PiptoServer, clearchar, GAMEDATA_PACKAGE_SIZE, 0);
					if (client->DoWeHaveSignalNow())
					{
						break;
					}

				}
				delete(client);
			}
			while (state == RenderStage_InGame || state == RenderStage_GamePause)
			{


				netmutex.lock();
				if (Ishost)
				{


					//receive and process and Send data to all  the time lag will be 2ms
					for (std::map<std::string, F_Character*>::iterator it = AllSources->m_NPCs.begin(); it != AllSources->m_NPCs.end(); it++)
					{
						std::string name = it->first;
						if (name != AllSources->m_MName)
						{
							SOCKET *tmpsoc = PLayersSockets[name];
							F_Character *tmpcha = AllSources->m_NPCs[name];
							while (TestSocketRWeadable(*tmpsoc, true, 0, 2))
							{
								char tmp[GAMEDATA_PACKAGE_SIZE];
								//rceive data
								recv(*tmpsoc, tmp, GAMEDATA_PACKAGE_SIZE, 0);
								std::string receivedData = tmp;
								std::string playername = E_F_GameDataToCharacter(NULL, receivedData);
								//if we found the user successfully
								if (AllSources->m_NPCs.find(playername) != AllSources->m_NPCs.end())
								{
									F_Character *tempcha = AllSources->m_NPCs[playername];
									E_F_GameDataToCharacter(tempcha, receivedData);
								}

							}
						}

					}

					//calculate all effects here

					for (std::map<std::string, F_Character*>::iterator it = AllSources->m_NPCs.begin(); it != AllSources->m_NPCs.end(); it++)
					{
						std::string currentName = it->first;
						F_Character *currentCha = it->second;
						D3DXVECTOR3 origin;
						D3DXVECTOR3 direct;
						shot* tmpshot = currentCha->m_NetShot;
						if (tmpshot->m_active) //get one task 
						{
							origin = tmpshot->m_shotPos;
							direct = tmpshot->m_shotVec;
							tmpshot->m_active = false;
							//find your enemy
							for (std::string name : currentCha->m_IsRed ? AllSources->m_TeamBlueNames : AllSources->m_TeamRedNames)
							{
								F_Character *Enemy = AllSources->m_NPCs[name];
								//you hurt one emnemy then this bullet is useless
								if (Enemy->m_Anim_instance->TestOneShot(NULL, origin, direct))
								{
									Enemy->m_Hp -= 10;
									if (it->first == AllSources->m_MName)
									{
										HitAudio->PlaysoundTotheend();
									}
								}

							}
						}
						else
						{

						}
					}



					//then Send all processed data to every person

					std::string TotalData = "";
					for (std::map<std::string, F_Character*>::iterator it = AllSources->m_NPCs.begin(); it != AllSources->m_NPCs.end(); it++)
					{
						TotalData += (E_F_GameDataToString(it->second) + '\n');
					}
					for (std::map<std::string, SOCKET*>::iterator it = PLayersSockets.begin(); it != PLayersSockets.end(); it++)
					{
						send(*(it->second), TotalData.c_str(), GAMEDATA_PACKAGE_SIZE, 0);
					}
					int deadRedN = 0;
					int deadBlueN = 0;
					for (std::map<std::string, F_Character*>::iterator it = AllSources->m_NPCs.begin(); it != AllSources->m_NPCs.end(); it++)
					{
						if (it->second->m_IsRed&&it->second->m_Hp <= 0)
						{
							deadRedN++;
						}
						if (it->second->m_IsRed == false && it->second->m_Hp <= 0)
						{
							deadBlueN++;
						}
					}
					if (deadRedN == AllSources->m_TeamRedNames.size())
					{
						AllSources->m_redAllKilled = true;
					}
					if (deadBlueN == AllSources->m_TeamBlueNames.size())
					{
						AllSources->m_blueAllKilled = true;
					}
					if (AllSources->m_blueAllKilled || AllSources->m_redAllKilled)
					{
						AllSources->m_gamestate = RenderStage_GameOverUI;

						F_Character* myself = AllSources->m_NPCs[AllSources->m_MName];
						F_Ui_Item *result;
						if (AllSources->m_blueAllKilled && !AllSources->m_redAllKilled)
						{
							if (myself->m_IsRed)
							{
								WinAudio->PlaysoundTotheend();
							}
							else
							{
								LoseAudio->PlaysoundTotheend();
							}
							result = new F_Ui_Item(1000, 500 + 2 * F_Ui_Item::m_perH, 1260, 500 + 3 * F_Ui_Item::m_perH, Ui_Label, IDC_GAME_RESULT, "NEWMATCHBUTTON", L"REDWIN", g_pd3dDevice);
						}
						else if (!AllSources->m_blueAllKilled && AllSources->m_redAllKilled)
						{
							if (!myself->m_IsRed)
							{
								WinAudio->PlaysoundTotheend();
							}
							else
							{
								LoseAudio->PlaysoundTotheend();
							}
							result = new F_Ui_Item(1000, 500 + 2 * F_Ui_Item::m_perH, 1260, 500 + 3 * F_Ui_Item::m_perH, Ui_Label, IDC_GAME_RESULT, "NEWMATCHBUTTON", L"BLUEWIN", g_pd3dDevice);
						}
						else
						{
							result = new F_Ui_Item(1000, 500 + 2 * F_Ui_Item::m_perH, 1260, 500 + 3 * F_Ui_Item::m_perH, Ui_Label, IDC_GAME_RESULT, "NEWMATCHBUTTON", L"DRAW", g_pd3dDevice);
						}
						ShowCursor(true);
						ShowCursor(true);


						AllSources->m_UiResources[SRC_GAMEOVERUI]->Additem(result);
						//send results to all players
						if (AllSources->m_redAllKilled && !AllSources->m_blueAllKilled)
						{
							//blue win
							for (std::map<std::string, SOCKET*>::iterator it = PLayersSockets.begin(); it != PLayersSockets.end(); it++)
							{
								char result[50] = "BLUEWIN";
								send(*(it->second), result, GAMEDATA_PACKAGE_SIZE, 0);
							}
						}
						else if (!AllSources->m_redAllKilled && AllSources->m_blueAllKilled)
						{
							//red win
							for (std::map<std::string, SOCKET*>::iterator it = PLayersSockets.begin(); it != PLayersSockets.end(); it++)
							{
								char result[50] = "REDWIN";
								send(*(it->second), result, GAMEDATA_PACKAGE_SIZE, 0);
							}
						}
						else if (AllSources->m_redAllKilled && AllSources->m_blueAllKilled)
						{
							//DRAW
							for (std::map<std::string, SOCKET*>::iterator it = PLayersSockets.begin(); it != PLayersSockets.end(); it++)
							{
								char result[50] = "DRAW";
								send(*(it->second), result, GAMEDATA_PACKAGE_SIZE, 0);
							}
						}
						else
						{
							;
						}
					}
					Sleep(3);
				}
				else //if we are clients players
				{


					std::string currentName = AllSources->m_MName;
					F_Character *currentCha = AllSources->m_NPCs[currentName];
					D3DXVECTOR3 origin;
					D3DXVECTOR3 direct;
					shot* tmpshot = currentCha->m_NetShot;
					if (tmpshot->m_active) //get one task 
					{
						origin = tmpshot->m_shotPos;
						direct = tmpshot->m_shotVec;

						//find your enemy
						for (std::string name : currentCha->m_IsRed ? AllSources->m_TeamBlueNames : AllSources->m_TeamRedNames)
						{
							F_Character *Enemy = AllSources->m_NPCs[name];
							//you hurt one emnemy then this bullet is useless
							if (Enemy->m_Anim_instance->TestOneShot(NULL, origin, direct))
							{
								HitAudio->PlaysoundTotheend();
							}

						}
					}

					//send data
					std::string mydata;
					mydata = (E_F_GameDataToString(AllSources->m_NPCs[AllSources->m_MName]));
					send(p_PiptoServer, mydata.c_str(), GAMEDATA_PACKAGE_SIZE, 0);
					currentCha->m_NetShot->m_active = false;

					//receive data
					while (TestSocketRWeadable(p_PiptoServer, true, 0, 2))
					{
						char tmpdata[GAMEDATA_PACKAGE_SIZE];
						recv(p_PiptoServer, tmpdata, GAMEDATA_PACKAGE_SIZE, 0);
						std::string alldata = tmpdata;
						//serverlog << alldata;
						//serverlog.flush();
						//we received result
						if (alldata.size() < 10)
						{
							ShowCursor(true);
							ShowCursor(true);
							AllSources->m_gamestate = RenderStage_GameOverUI;
							F_Character* myself = AllSources->m_NPCs[AllSources->m_MName];
							F_Ui_Item *result;
							if (alldata == "REDWIN")
							{
								if (myself->m_IsRed)
								{
									WinAudio->PlaysoundTotheend();
								}
								else
								{
									LoseAudio->PlaysoundTotheend();
								}
								result = new F_Ui_Item(1000, 500 + 2 * F_Ui_Item::m_perH, 1260, 500 + 3 * F_Ui_Item::m_perH, Ui_Label, IDC_GAME_RESULT, "NEWMATCHBUTTON", L"REDWIN", g_pd3dDevice);
							}
							if (alldata == "BLUEWIN")
							{
								if (!myself->m_IsRed)
								{
									WinAudio->PlaysoundTotheend();
								}
								else
								{
									LoseAudio->PlaysoundTotheend();
								}
								result = new F_Ui_Item(1000, 500 + 2 * F_Ui_Item::m_perH, 1260, 500 + 3 * F_Ui_Item::m_perH, Ui_Label, IDC_GAME_RESULT, "NEWMATCHBUTTON", L"BLUEWIN", g_pd3dDevice);
							}
							if (alldata == "DRAW")
							{
								result = new F_Ui_Item(1000, 500 + 2 * F_Ui_Item::m_perH, 1260, 500 + 3 * F_Ui_Item::m_perH, Ui_Label, IDC_GAME_RESULT, "NEWMATCHBUTTON", L"DRAW", g_pd3dDevice);
							}


							AllSources->m_UiResources[SRC_GAMEOVERUI]->Additem(result);
						}
						else
						{
							std::string tmpstr1 = "";
							for (char per : alldata)
							{
								if (per == '\n')
								{
									std::string playername = E_F_GameDataToCharacter(NULL, tmpstr1);
									//if we found the user successfully
									if (AllSources->m_NPCs.find(playername) != AllSources->m_NPCs.end())
									{
										F_Character *tempcha = AllSources->m_NPCs[playername];
										E_F_GameDataToCharacter(tempcha, tmpstr1);
									}

									tmpstr1 = "";
								}
								else
								{
									tmpstr1 += per;
								}
							}
						}
					}



					int deadRedN = 0;
					int deadBlueN = 0;
					for (std::map<std::string, F_Character*>::iterator it = AllSources->m_NPCs.begin(); it != AllSources->m_NPCs.end(); it++)
					{
						if (it->second->m_IsRed&&it->second->m_Hp <= 0)
						{
							deadRedN++;
						}
						if (it->second->m_IsRed == false && it->second->m_Hp <= 0)
						{
							deadBlueN++;
						}
					}
					if (deadRedN == AllSources->m_TeamRedNames.size())
					{
						AllSources->m_redAllKilled = true;
					}
					if (deadBlueN == AllSources->m_TeamBlueNames.size())
					{
						AllSources->m_blueAllKilled = true;
					}
					if (AllSources->m_blueAllKilled || AllSources->m_redAllKilled)
					{
						AllSources->m_gamestate = RenderStage_GameOverUI;

						F_Character* myself = AllSources->m_NPCs[AllSources->m_MName];
						F_Ui_Item *result;
						if (AllSources->m_blueAllKilled && !AllSources->m_redAllKilled)
						{
							if (myself->m_IsRed)
							{
								WinAudio->PlaysoundTotheend();
							}
							else
							{
								LoseAudio->PlaysoundTotheend();
							}
							result = new F_Ui_Item(1000, 500 + 2 * F_Ui_Item::m_perH, 1260, 500 + 3 * F_Ui_Item::m_perH, Ui_Label, IDC_GAME_RESULT, "NEWMATCHBUTTON", L"REDWIN", g_pd3dDevice);
						}
						else if (!AllSources->m_blueAllKilled && AllSources->m_redAllKilled)
						{
							if (!myself->m_IsRed)
							{
								WinAudio->PlaysoundTotheend();
							}
							else
							{
								LoseAudio->PlaysoundTotheend();
							}
							result = new F_Ui_Item(1000, 500 + 2 * F_Ui_Item::m_perH, 1260, 500 + 3 * F_Ui_Item::m_perH, Ui_Label, IDC_GAME_RESULT, "NEWMATCHBUTTON", L"BLUEWIN", g_pd3dDevice);
						}
						else
						{
							result = new F_Ui_Item(1000, 500 + 2 * F_Ui_Item::m_perH, 1260, 500 + 3 * F_Ui_Item::m_perH, Ui_Label, IDC_GAME_RESULT, "NEWMATCHBUTTON", L"DRAW", g_pd3dDevice);
						}
						ShowCursor(true);
						ShowCursor(true);


						AllSources->m_UiResources[SRC_GAMEOVERUI]->Additem(result);
						Sleep(240);


					}
					Sleep(30);
				}

				state = AllSources->m_gamestate;
				netmutex.unlock();
			}
		}
		else if (state == RenderStage_MainUI)
		{
			Sleep(500);//Do nothing!
		}
		else if (state == RenderStage_GameOverUI)
		{
			if (Ishost)
			{

				Sleep(400);
			}



		}
		else
			;
		netmutex.lock();
		state = AllSources->m_gamestate;
		netmutex.unlock();
		Sleep(100);
	}
	WSACleanup();
}
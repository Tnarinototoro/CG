#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include <d3d9.h>
#include<d3dx9.h>
#include <tchar.h>
#include <time.h> 
#include <dinput.h>   
#include<string>
#include<iostream>
#include<fstream>
#include <vector>
#include<D3D10.h>
#include<D3DX10.h>
#include <cstdlib>
#include<assert.h>
#include<thread>
#include<list>
#include <xaudio2.h>
#include <strsafe.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <conio.h>
#include<map>
#include<mutex>
#pragma comment(lib,"d3d10.lib")
#pragma comment(lib,"d3dx10.lib")
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib, "dinput8.lib")   
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib, "winmm.lib") 

//Window size constant
#define WINDOW_WIDTH	1920					
#define WINDOW_HEIGHT	1080		
#define WINDOW_TITLE	_T("My_First_Fps_Game") 
#define DIRECTINPUT_VERSION 0x0800  
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }      
#define SAFE_DELETE(p)  { if(p) { delete (p); (p)=NULL; } }	
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif // !SAFE_DELETE_ARRAY
#ifndef V
#define V(x)           { hr = (x); if( FAILED(hr) ) {   } }
#endif

// Ui Global constans0
#define	RenderStage_MainUI 1 //main UI
#define	RenderStage_JoinGameUI 2 //Wait for other players Ui
#define RenderStage_InGame 3 // Game rendering
#define RenderStage_GameOverUI 4 //Gameover Ui to show which team wins
#define	RenderStage_GamePause 5 //When you press ESC ,you will enter Game Pause Ui
#define RenderStage_HostGameUI 28
#define RenderStage_QuitGame 29
//Ui items ID
#define IDC_GAMESTART 6 //when you press this you will enter game
#define IDC_QUITGAME 7 // you will quit game 
#define IDC_FULLSCREEN 8 //toggle fullscreen
#define IDC_JOINMATCH 9 //enter the RenderStatge_
#define IDC_NEWMATCH 10
#define IDC_LABEL_HOST 35
#define IDC_LABEL_GUEST 36
#define IDC_TEAMRED 37
#define IDC_TEAMBLUE 38
#define IDC_INVITE 39
#define IDC_CONN 40
#define IDC_PLAYER_RED_ID_BASE 1000
#define IDC_PLAYER_BLUE_ID_BASE 1020
#define IDC_GAME_RESULT 8000
//Ui message Identifier
#define GUI_MESSAGE_CLICKED 11
//Ui resources Identifier
#define SRC_MAINUI 12
#define SRC_PAUSEUI 13
#define SRC_ROOMUI 14
#define SRC_GAMEOVERUI 15
//Audio Resoureces Identifier
#define BGM_ONBUTTON 16
#define BGM_CLICKBUTTON 17
#define BGM_BGM 18
#define BGM_WIN_START 41
#define BGM_WIN_LOOP 42
#define BGM_LOSE_START 43
#define BGM_LOSE_LOOP 44
//Animation Resources Identifier
#define ANM_JOG 19
#define ANM_IDLE 20
#define ANM_WALK 21
//Camera Type Identifier
#define CAM_FREE 22
#define CAM_FPS 23
//Gun Message Type Identifier
#define LOAD_CLIP_FAIL_CLIP_FULL 24
#define LOAD_CLIP_FAIL_BACK_AMMO_SCANTY 25

//communicator mode definition
#define SIG_READYTO_ACCEPT 30
#define SIG_CONNECTING 31
#define PART_RED 32
#define PART_BLUE 33
#define COM_PREPARING 34
//Physics Parameters
#define GAVITY_G 9.832
//Animation index fail identifier
#define ANIMINDEX_FAIL 999
//NetWork Definitions 
#define PODCAST_PORT 2002
#define GAMEDATA_EXCHANGE_PORT 27018
#define RADIO_IP "230.1.1.99"
#define GAMEDATA_PACKAGE_SIZE 2048

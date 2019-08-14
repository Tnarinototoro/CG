#pragma once
#include"Runtime.h"
#include"F_Audio.h"
F_Audio* BackgroundMusic;
F_Audio* LoseAudio;
F_Audio* WinAudio;
F_Audio* HitAudio;
void  ProcessGui(int id, int state, int& gms); //function to process GUI messages 
struct BackGroundVertex
{
	FLOAT _x, _y, _z,_rhw;
	DWORD _color;
	FLOAT _u, _v;
	BackGroundVertex(FLOAT x, FLOAT y, FLOAT z,FLOAT rhw,DWORD color, FLOAT u, FLOAT v) :_x(x), _y(y),
		_z(z),_rhw(rhw),_color(color),_u(u), _v(v) {};
};
#define D3DFVF_BackGroundVertex (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
enum Ui_Type
{
	Ui_Label,
	Ui_Button,
};

class F_Ui_Item
{

private:
	RECT ValidateRect;
	int Type;
	int Id;
	std::string Name;
	bool Touchable;
	int contentLength;
	bool ClickedOnce;
	ID3DXFont *font;
	WCHAR Textcontent[50];
	F_Audio *AudioOnbutton;
	F_Audio *ClickButtonAudio;
	LPDIRECT3DDEVICE9 device;
	D3DCOLOR LabelColor;
	int playAudiotime;
	bool Isabove;
	bool IsActive;
	
public:
	static int m_redX;  //left of the start of red team
	static int m_baseY;//top base value
	static int m_blueX; //left of the start of the blue team
	static int m_perH; //every player per character label hiehgt
	static int m_perW; //every palyer per character label width
	F_Ui_Item(int left, int top, int right, int bottom, int Type, int id, std::string name, LPWSTR content, LPDIRECT3DDEVICE9 device);
	~F_Ui_Item();
	void F_UI_Item_Draw(LPDIRECT3DDEVICE9 device, int mousex, int mousey,bool L_buttondwn);
	int GetType()
	{
		return Type;
	}
	void SetItemRect(RECT tmp)
	{
		this->ValidateRect = tmp;
	}
	bool GetTouchable()
	{
		return Touchable;
	}
	int GetContentTextLength()
	{
		return contentLength;
	}
    BOOL AboveItem(int m_x, int m_y)
	{
		int vl = ValidateRect.left;
		int rt = ValidateRect.right;
		int tp = ValidateRect.top;
		int bt = ValidateRect.bottom;
		if (m_x >= vl && m_x <= rt && m_y <= bt && m_y >= tp)
		{
			return true;
		}
		else
			return false;
	}
	RECT GetItemRect()
	{
		return ValidateRect;
	}
	int GetId()
	{
		return Id;
	}
	void SetActive(bool active)
	{
		this->IsActive = active;
	}
	void SetLabelColor(D3DCOLOR color)
	{
		this->LabelColor = color;
	}
	void SetContent(LPCWSTR content) { this->contentLength = swprintf_s(Textcontent, L"%s", content); }
};
int F_Ui_Item::m_redX = 200;
int F_Ui_Item::m_baseY = 200;
int F_Ui_Item::m_blueX = 1500;
int F_Ui_Item::m_perH = 50;
int F_Ui_Item::m_perW = 20;
F_Ui_Item::F_Ui_Item(int left,int top,int right,int bottom,int Type,int id,std::string name, LPWSTR content,LPDIRECT3DDEVICE9 device)
{
	IsActive = true;
	this->device = device;
	LabelColor = D3DCOLOR_RGBA(204, 204, 204, 255);
	Isabove = false;
	//init font
	if (FAILED(D3DXCreateFont(this->device,50, 20, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		L"Arial", &font)))//50 is h 20 is w
	{
		MessageBox(NULL, L"Item Init failed!", L"Error!", 0);
	}
	//according to the type we init the item
	switch (Type)
	{
	case Ui_Label:
		Touchable = false;
		break;
	case Ui_Button:
		Touchable = true;
		break;
	default:
		break;
	}
	ValidateRect.left = left;
	ValidateRect.right = right;
	ValidateRect.top = top;
	ValidateRect.bottom = bottom;
	this->Name=name;
	this->Id = id;
	this->contentLength= swprintf_s(Textcontent, L"%s", content);
	AudioOnbutton = new F_Audio();
	AudioOnbutton->InitSound(AudioOnbutton->Getm_Xaudio(), L"bgm\\OnButton.wav");
	playAudiotime = 0;
	ClickButtonAudio = new F_Audio();
	ClickButtonAudio->InitSound(ClickButtonAudio->Getm_Xaudio(), L"bgm\\ClickButton.wav");
	this->ClickedOnce = false;
}
void F_Ui_Item::F_UI_Item_Draw(LPDIRECT3DDEVICE9 device,int mousex,int mousey,bool ld)
{

	if (IsActive)
	{
		if (Touchable)
		{
			BOOL Isabove = this->AboveItem(mousex, mousey);


			if (Isabove)
			{
				RECT temp = ValidateRect;
				temp.left += 20;
				temp.right += 20;
				font->DrawText(NULL, Textcontent, contentLength, &temp, DT_TOP | DT_LEFT, D3DCOLOR_RGBA(0, 123, 255, 250));
				if (playAudiotime == 0)
				{
					AudioOnbutton->PlaysoundTotheend();
					playAudiotime = 1;

				}
				if (ld)
				{
					ClickButtonAudio->GetSourceVoice()->SetVolume(2);
					ClickButtonAudio->PlaysoundTotheend();

				}
				else
				{
					
					ClickButtonAudio->ClearPendingbuffer();
				}



			}
			else
			{
				font->DrawText(NULL, Textcontent, contentLength, &ValidateRect, DT_TOP | DT_LEFT, D3DCOLOR_RGBA(0, 239, 136, 250));
				if (playAudiotime == 1)
					AudioOnbutton->ClearPendingbuffer();
				playAudiotime = 0;
			}
		}
		else
		{
			font->DrawText(NULL, Textcontent, contentLength, &ValidateRect, DT_TOP | DT_LEFT, LabelColor);
		}
	}
	else
	{
		;
	}
}
F_Ui_Item::~F_Ui_Item()
{
	this->font->Release();
	this->device->Release();
}
//Ui Manager Class
class F_Ui
{
	std::map<int, F_Ui_Item*> ItemsManager;
	LPDIRECT3DVERTEXBUFFER9	BackgroundVertexBuffer;		//vertex buffer of Background
	LPDIRECT3DTEXTURE9			BackgroundTexture;			//5个纹理接口对象
	LPDIRECT3DDEVICE9 device;
	bool HasBackPic;
public:
	F_Ui(LPCWCHAR backgroundname, LPDIRECT3DDEVICE9 outdevice);
	void Additem(F_Ui_Item *item);
	void RenderUi();
	F_Ui();
	~F_Ui();
	std::map<int, F_Ui_Item*> & GetItems()
	{
		return ItemsManager;
	}
	LPDIRECT3DDEVICE9 GetDevice()
	{
		return device;
	}

private:

};
F_Ui::F_Ui()
{
	BackgroundTexture = NULL;
	device = NULL;
	BackgroundVertexBuffer = NULL;
	
}
F_Ui::F_Ui(LPCWCHAR backgroundname,LPDIRECT3DDEVICE9 outdevice)
{
	this->device = outdevice;
	//we have no background!
	if (!backgroundname)
	{
		HasBackPic = false;
	}
	else
	{
		HasBackPic = true;
		//init background picture
		D3DXCreateTextureFromFile(device, backgroundname, &BackgroundTexture);
		BackGroundVertex vertice[] =
		{
			{ 0.0f,0.0f,0.0f,1,D3DCOLOR_XRGB(255,255,255),0.0f,0.0f},//left top
			{WINDOW_WIDTH,0.0f,0.0f,1,D3DCOLOR_XRGB(255,255,255),1.0f,0.0f} ,//right top
			{0.0f,WINDOW_HEIGHT,0.0f,1,D3DCOLOR_XRGB(255,255,255),0.0f,1.0f}, //left bottom
			{WINDOW_WIDTH,WINDOW_HEIGHT,0.0f,1,D3DCOLOR_XRGB(255,255,255),1.0f,1.0f} //right bottom
		};
		device->CreateVertexBuffer(sizeof(vertice), 0,
			D3DFVF_BackGroundVertex, D3DPOOL_MANAGED, &BackgroundVertexBuffer, 0);
		void* pVertices;
		BackgroundVertexBuffer->Lock(0, 4 * sizeof(BackGroundVertex), (void**)&pVertices, 0);
		memcpy(pVertices, vertice, sizeof(vertice));
		BackgroundVertexBuffer->Unlock();
	}
	
}
void F_Ui::RenderUi()
{
	if (HasBackPic)
	{
		device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		device->SetStreamSource(0, BackgroundVertexBuffer, 0, sizeof(BackGroundVertex));
		device->SetFVF(D3DFVF_BackGroundVertex);
		device->SetTexture(0, BackgroundTexture);
		device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	}
	else
	{
		//Do nothing!
		;
	}

}

F_Ui::~F_Ui()
{
	if (HasBackPic)
	{
		this->BackgroundTexture->Release();
		this->BackgroundVertexBuffer->Release();
		this->device->Release();
	}
	else
		;//Have no resources to be released!
}
void F_Ui::Additem(F_Ui_Item *item)
{
	this->ItemsManager.insert(std::pair<int, F_Ui_Item*>(item->GetId(), item));
}
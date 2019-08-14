#pragma once
#include"Runtime.h"
#include"F_Input.h"
class StaticObject
{
	DWORD	ObjectNumMtrls; //number of materaials
	D3DXVECTOR3 Position; //center point computed by function d3dxcomputebounndingsphere
	D3DVECTOR ScaleFactor;
	D3DVECTOR RotateFactor;
	LPDIRECT3DDEVICE9 device;
	ID3DX10Mesh *object;
	LPD3DXMESH	ObjectMesh; 
	D3DMATERIAL9*	ObjectMaterials;
	LPDIRECT3DTEXTURE9*	ObjectTextures;
	D3DXMATRIX Position_Trans;
	D3DXMATRIX Scale_Trans;
	D3DXMATRIX Rotate_Trans;
	D3DXMATRIX World_Trans;
	float m_SphereRadius;
	bool ModelLoaded;
	bool Isrender;
	int up;
	float v;
public:
	VOID UpdateObject(float deltTime)
	{
		if (Position.y <=400&&Position.y>=200)
		{
			if (up == 1)
			{
				Position.y += (deltTime * v);
				v -= 0.02;
				if (v <= 0)
				{
					v = 2;
				}
			}
			else
			{
				Position.y -= (deltTime * v);
				v -= 0.02;
				if (v <= 0)
				{
					v = 2;
				}
			}
		}
		else if(Position.y>400)
		{
			Position.y = 400;
			up = 0;
		}
		else
		{
			Position.y = 200;
			up = 1;
		}
		this->PositionTransForm(Position);
	}
	D3DVECTOR GetPositon()
	{
		return Position;
	}
	VOID GetObjectMesh(LPD3DXMESH *pMesh) { *pMesh = this->ObjectMesh; }
	VOID SetVisble(BOOL pIsrender) { this->Isrender = pIsrender; }
	BOOL GetVisible() { return this->Isrender; }
	VOID inline GetWorldMatrix(D3DXMATRIX *pMatWorld)
	{
		*pMatWorld = this->World_Trans;
	}
	VOID SetWorldMatrix(D3DXMATRIX world) { this->World_Trans = world; }
	D3DVECTOR GetScaleFactor()
	{
		return ScaleFactor;
	}
	D3DVECTOR GetRotateFactor()
	{
		return RotateFactor;
	}
	void StaticObjectInit(IDirect3DDevice9 * device)
	{
		this->device = device;
		ModelLoaded = false;
		PositionTransForm(0, 0, 0);
		ScaleTransForm(1, 1, 1);
		RotateTransform(0, 0, 0);
		Isrender = true;

	}
	void LoadModel(LPCWSTR filepath)
	{
		LPD3DXBUFFER pAdjBuffer = NULL;
		LPD3DXBUFFER pMtrlBuffer = NULL;
		D3DXLoadMeshFromX(filepath, D3DXMESH_MANAGED, device,
			&pAdjBuffer, &pMtrlBuffer, NULL, &ObjectNumMtrls, &ObjectMesh);
		D3DXMATERIAL *pMtrls = (D3DXMATERIAL*)pMtrlBuffer->GetBufferPointer();
		ObjectMaterials = new D3DMATERIAL9[ObjectNumMtrls];
		ObjectTextures = new LPDIRECT3DTEXTURE9[ObjectNumMtrls];
		for (DWORD i = 0; i<ObjectNumMtrls; i++)
		{
			ObjectMaterials[i] = pMtrls[i].MatD3D;
			ObjectMaterials[i].Diffuse.a = 0.5f;
			ObjectMaterials[i].Ambient = ObjectMaterials[i].Diffuse;
			ObjectTextures[i] = NULL;
			char path[50];
			sprintf_s(path,50, "Textures\\%s", pMtrls[i].pTextureFilename);
			D3DXCreateTextureFromFileA(device, path, &ObjectTextures[i]);
		}
		BYTE* v = 0;
		ObjectMesh->LockVertexBuffer(0, (void**)&v);
		
		if (FAILED
		(
			D3DXComputeBoundingSphere((D3DXVECTOR3 *)v, ObjectMesh->GetNumVertices(),
			D3DXGetFVFVertexSize(ObjectMesh->GetFVF()), &Position, &m_SphereRadius)))
		{
			MessageBox(NULL, L"Caculating sphere error!", L"error", 0);
		}
		SAFE_RELEASE(pAdjBuffer);
		SAFE_RELEASE(pMtrlBuffer);
	}
	void PositionTransForm(FLOAT x,FLOAT y,FLOAT z)
	{
		D3DXMatrixIdentity(&Position_Trans);
		D3DXMatrixTranslation(&Position_Trans, x, y, z);
		Position.x =x;
		Position.y =y;
		Position.z =z;
	}
	void PositionTransForm(D3DXVECTOR3 Position)
	{
		D3DXMatrixIdentity(&Position_Trans);
		D3DXMatrixTranslation(&Position_Trans, Position.x, Position.y, Position.z);
		this->Position = Position;
	}
	void ScaleTransForm(FLOAT sx, FLOAT sy, FLOAT sz)
	{
		D3DXMatrixIdentity(&Scale_Trans);
		D3DXMatrixScaling(&Scale_Trans, sx, sy, sz);
		ScaleFactor.x =sx;
		ScaleFactor.y =sy;
		ScaleFactor.z =sz;

	}
	void RotateTransform(FLOAT rx, FLOAT ry, FLOAT rz)
	{
		D3DXMATRIX  X;
		D3DXMATRIX  Y;
		D3DXMATRIX  Z;
		D3DXMatrixIdentity(&Rotate_Trans);
		D3DXMatrixIdentity(&X);
		D3DXMatrixIdentity(&Y);
		D3DXMatrixIdentity(&Z);
		D3DXMatrixRotationX(&X, rx);
		D3DXMatrixRotationX(&Y, ry);
		D3DXMatrixRotationX(&Z, rz);
		Rotate_Trans = X*Y*Z;
		RotateFactor.x = rx;
		RotateFactor.y = ry;
		RotateFactor.z = rz;
	}
	void SetTransForm()
	{
		World_Trans = Rotate_Trans * Scale_Trans*Position_Trans;
		device->SetTransform(D3DTS_WORLD,&World_Trans);
	}
	void JustSetWolrdMatrixComplex()
	{
		device->SetTransform(D3DTS_WORLD, &World_Trans);
	}
	void DrawObject()
	{
		if (Isrender)
		{
			for (DWORD i = 0; i < ObjectNumMtrls; i++)
			{

				device->SetMaterial(&ObjectMaterials[i]);
				device->SetTexture(0, ObjectTextures[i]);
				ObjectMesh->DrawSubset(i);
			}
		}
	}
	void ReactToInput(F_Input* Myinput,int Space)
	{
		if (Myinput->IskeyPressed(KeyBoardType, DIK_1))
			device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		if (Myinput->IskeyPressed(KeyBoardType, DIK_2))
			device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	/*	if (Myinput->IskeyPressed(KeyBoardType, DIK_A))
		{
			Position.x -= Space/20;
		}
		if (Myinput->IskeyPressed(KeyBoardType, DIK_D))
		{
			Position.x += Space/20;
		}
		if (Myinput->IskeyPressed(KeyBoardType, DIK_W))Position.z += Space/20;
		if (Myinput->IskeyPressed(KeyBoardType, DIK_S))Position.z -= Space/20;*/
	


		//if (Myinput->IskeyPressed(MouseType, 1))
		//{
		//	RotateFactor.x += Myinput->GetMouseMovement(1)* -0.01f;
		//	RotateFactor.y += Myinput->GetMouseMovement(0)* 0.01f;
		//}

		if (Myinput->IskeyPressed(KeyBoardType, DIK_UP)) RotateFactor.x += 0.005f;
		if (Myinput->IskeyPressed(KeyBoardType, DIK_DOWN)) RotateFactor.x -= 0.005f;
		if (Myinput->IskeyPressed(KeyBoardType, DIK_LEFT)) RotateFactor.y -= 0.005f;
		if (Myinput->IskeyPressed(KeyBoardType, DIK_RIGHT))RotateFactor.y += 0.005f;

		//PositionTransForm(Position.x, Position.y, Position.z);
		RotateTransform(RotateFactor.x, RotateFactor.y, RotateFactor.z);
	}
	void Setheight(float hi)
	{
		Position.y = hi;
	}
	~StaticObject();
	StaticObject();
};
StaticObject::StaticObject()
{
	device = NULL;
	ObjectMesh = NULL;
	ObjectMaterials = NULL;
	ObjectTextures = NULL;
	up = 1;
	v = 2;

}
StaticObject::~StaticObject()
{
	SAFE_RELEASE(device);
	SAFE_RELEASE(ObjectMesh);
	SAFE_DELETE(ObjectMaterials);
	SAFE_DELETE(ObjectTextures);
}
//declaration
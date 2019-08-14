#include "Runtime.h"
#include"F_Camera.h"
#include"StaticObject.h"
struct D3DVERTEX
{
	D3DXVECTOR3 p;
	D3DXVECTOR3 n;
	FLOAT tu, tv;
	static const DWORD FVF;
};
const DWORD                 D3DVERTEX::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
struct INTERSECTION
{
	DWORD dwFace;   //mesh face that was intersected
	FLOAT fBary1, fBary2; //ÖØÐÄ×ø±ê barycentric coords of intersection point
	FLOAT fDist;   //distance from ray origin to intersection  
	FLOAT tu, tv; //texture coords of intersection
	INTERSECTION(DWORD face, FLOAT f1, FLOAT f2, FLOAT dis):dwFace(face),fBary1(f1),fBary2(f2),
		fDist(dis){}
	INTERSECTION() {}
};
//function s Declaration
//VOID Tans2DmousePosTo3DPos(HWND hwnd, POINT ptCursor, D3DXVECTOR3 *Orig, D3DXVECTOR3 *Dir, D3DXMATRIX *ObjectworldTr,F_Camera* Cam); //Tansform the Picking ray form view xyz to world xyz
bool IntersectTriangle(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, D3DXVECTOR3& v0, D3DXVECTOR3& v1, D3DXVECTOR3& v2, FLOAT* t, FLOAT* u, FLOAT* v);
bool IntersectTriangle(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir,
	D3DXVECTOR3& v0, D3DXVECTOR3& v1, D3DXVECTOR3& v2,
	FLOAT* t, FLOAT* u, FLOAT* v)
{
	// Find vectors for two edges sharing vert0
	D3DXVECTOR3 edge1 = v1 - v0;
	D3DXVECTOR3 edge2 = v2 - v0;

	// Begin calculating determinant - also used to calculate U parameter
	D3DXVECTOR3 pvec;
	D3DXVec3Cross(&pvec, &dir, &edge2);

	// If determinant is near zero, ray lies in plane of triangle
	FLOAT det = D3DXVec3Dot(&edge1, &pvec);

	D3DXVECTOR3 tvec;
	if (det > 0)
	{
		tvec = orig - v0;
	}
	else
	{
		tvec = v0 - orig;
		det = -det;
	}

	if (det < 0.0001f)
		return FALSE;

	// Calculate U parameter and test bounds
	*u = D3DXVec3Dot(&tvec, &pvec);
	if (*u < 0.0f || *u > det)
		return FALSE;

	// Prepare to test V parameter
	D3DXVECTOR3 qvec;
	D3DXVec3Cross(&qvec, &tvec, &edge1);

	// Calculate V parameter and test bounds
	*v = D3DXVec3Dot(&dir, &qvec);
	if (*v < 0.0f || *u + *v > det)
		return FALSE;

	// Calculate t, scale parameters, ray intersects triangle
	*t = D3DXVec3Dot(&edge2, &qvec);
	FLOAT fInvDet = 1.0f / det;
	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	return TRUE;
}
//This method is a simplyfied function of detecting whether the given Ray has intersected with the mesh
INTERSECTION* SimpleCollisionDetect(LPDIRECT3DDEVICE9 device,HWND hwnd, StaticObject *object,F_Camera* camera,bool UseDxInter)
{
	D3DXVECTOR3 OriginRayPos;
	D3DXVECTOR3 RayDirection;
	camera->GetCameraPosition(&OriginRayPos);
	camera->GetLookVector(&RayDirection);
	D3DXMATRIX objworld;
	object->GetWorldMatrix(&objworld);
	D3DXMATRIX re;
	D3DXMatrixInverse(&re, 0, &objworld);
	D3DXVec3TransformCoord(&OriginRayPos, &OriginRayPos, &re);
	D3DXVec3TransformNormal(&RayDirection, &RayDirection, &re);
	D3DXVec3Normalize(&RayDirection, &RayDirection);
	LPD3DXMESH pMesh;
	object->GetObjectMesh(&pMesh);
	if (UseDxInter)
	{
		BOOL bHit;
		DWORD dwFace;
		FLOAT fBary1, fBary2, fDist;

		D3DXIntersect(pMesh, &OriginRayPos, &RayDirection, &bHit, &dwFace, &fBary1, &fBary2, &fDist,
			NULL, NULL);
		if (bHit)
		{
			
	
			 
			return new INTERSECTION(dwFace, fBary1, fBary2, fDist);
		}
		else
		{
			return NULL;
		}
	}
}

// this class is just a simplyfied timer
class F_Timer
{
	DWORD64 StartTime;
	DWORD64 LastSignal;
	DWORD64 CurrentTime;
	DWORD64 SumofTime;
	DWORD64 InterVal;
public:
	F_Timer(DWORD64 inTervalTime)
	{
		this->InterVal = inTervalTime;
		this->StartTime = GetTickCount64();
		this->LastSignal = StartTime;
	}
	BOOL DoWeHaveSignalNow()
	{
		CurrentTime = GetTickCount64();
		SumofTime = CurrentTime - StartTime;
		if (CurrentTime - LastSignal >= InterVal)
		{
			LastSignal = CurrentTime;
			return true;
		}
		return false;
	}
	~F_Timer()
	{
		;
	}
};
//The Segmentation below is coded as the function to curtail the Moveable area of mouse,and hide the cursor to make the game look vivid 
void LockMouse2Client(HWND hwnd)
{
	
	POINT lt, rb;
	RECT rect;
	GetClientRect(hwnd, &rect);  //Get window Rect
	lt.x = rect.left;
	lt.y = rect.top;
	rb.x = rect.right;
	rb.y = rect.bottom;
	ClientToScreen(hwnd, &lt);
	ClientToScreen(hwnd, &rb);
	//Reset the screen coordinate
	rect.left = lt.x;
	rect.top = lt.y;
	rect.right = rb.x;
	rect.bottom = rb.y;
	//limit the cursor
	ClipCursor(&rect);
}




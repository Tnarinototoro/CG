#pragma once
#include"Runtime.h"
class F_Camera
{
private:
	
	D3DXVECTOR3				m_vRightVector;       
	D3DXVECTOR3				m_vUpVector;           
	D3DXVECTOR3				m_vLookVector;         
	D3DXVECTOR3				m_vCameraPosition;       
	D3DXVECTOR3				m_vTargetPosition;       
	D3DXMATRIX				m_matView;         
	D3DXMATRIX				m_matProj;           
	LPDIRECT3DDEVICE9		m_pd3dDevice;  
	FLOAT delAngleRight;   //to limit the angles of right vector
	INT m_Type;
public:
	INT GetType() { return m_Type; }
	VOID SetType(int ptype) { this->m_Type = ptype; }
	VOID CalculateViewMatrix(D3DXMATRIX *pMatrix); 
	VOID ReactToInput(F_Input* input, FLOAT hLimit, float nbei);
	VOID GetProjMatrix(D3DXMATRIX *pMatrix) { *pMatrix = m_matProj; }  
	VOID GetCameraPosition(D3DXVECTOR3 *pVector) { *pVector = m_vCameraPosition; }
	VOID GetLookVector(D3DXVECTOR3 *pVector) { *pVector = m_vLookVector; } 
	VOID GetUpVector(D3DXVECTOR3* pVector) { *pVector = m_vUpVector; }
	VOID GetRightVector(D3DXVECTOR3 *pVector) { *pVector = m_vRightVector; }
	VOID SetTargetPosition(D3DXVECTOR3 *pLookat = NULL); 
	VOID GetTagertPosition(D3DXVECTOR3* out) { *out = m_vTargetPosition; }
	VOID SetCameraPosition(D3DXVECTOR3 *pVector = NULL); 
	VOID SetViewMatrix(D3DXMATRIX *pMatrix = NULL);  
	VOID GetViewMatrix(D3DXMATRIX *pView);
	VOID SetProjMatrix(D3DXMATRIX *pMatrix = NULL); 
	VOID SetCameraHeightY(FLOAT h)
	{
		this->m_vCameraPosition.y = h;
	}

public:
	
	VOID MoveAlongRightVec(FLOAT fUnits);  
	VOID MoveAlongUpVec(FLOAT fUnits);     
	VOID MoveAlongLookVec(FLOAT fUnits);   

											
	VOID RotationRightVec(FLOAT fAngle);   
	VOID RotationUpVec(FLOAT fAngle);      
	VOID RotationLookVec(FLOAT fAngle);

public:
	F_Camera(IDirect3DDevice9 *pd3dDevice,int pType); 
	virtual ~F_Camera(void);  

};
F_Camera::F_Camera(IDirect3DDevice9 *pd3dDevice,int pType)
{
	m_pd3dDevice = pd3dDevice;
	m_vRightVector = D3DXVECTOR3(1.0f, 0.0f, 0.0f);   
	m_vUpVector = D3DXVECTOR3(0.0f, 1.0f, 0.0f); 
	m_vLookVector = D3DXVECTOR3(0.0f, 0.0f, 1.0f); 
	m_vCameraPosition = D3DXVECTOR3(0.0f, 0.0f, -250.0f);   
	m_vTargetPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	delAngleRight = 0.0f;
	this->m_Type = pType;

}
VOID F_Camera::CalculateViewMatrix(D3DXMATRIX *pMatrix)
{
	
	D3DXVec3Normalize(&m_vLookVector, &m_vLookVector); 
	D3DXVec3Cross(&m_vUpVector, &m_vLookVector, &m_vRightVector);    
	D3DXVec3Normalize(&m_vUpVector, &m_vUpVector);               
	D3DXVec3Cross(&m_vRightVector, &m_vUpVector, &m_vLookVector);   
	D3DXVec3Normalize(&m_vRightVector, &m_vRightVector);         
	pMatrix->_11 = m_vRightVector.x;          
	pMatrix->_12 = m_vUpVector.x;             
	pMatrix->_13 = m_vLookVector.x;           
	pMatrix->_14 = 0.0f;
	
	pMatrix->_21 = m_vRightVector.y;           
	pMatrix->_22 = m_vUpVector.y;             
	pMatrix->_23 = m_vLookVector.y;           
	pMatrix->_24 = 0.0f;
	
	pMatrix->_31 = m_vRightVector.z;           
	pMatrix->_32 = m_vUpVector.z;              
	pMatrix->_33 = m_vLookVector.z;           
	pMatrix->_34 = 0.0f;
	
	pMatrix->_41 = -D3DXVec3Dot(&m_vRightVector, &m_vCameraPosition);  
	pMatrix->_42 = -D3DXVec3Dot(&m_vUpVector, &m_vCameraPosition);       
	pMatrix->_43 = -D3DXVec3Dot(&m_vLookVector, &m_vCameraPosition);   
	pMatrix->_44 = 1.0f;
}
VOID F_Camera::SetTargetPosition(D3DXVECTOR3 *pLookat)
{
	if (pLookat != NULL)  m_vTargetPosition = (*pLookat);
	else m_vTargetPosition = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

	m_vLookVector = m_vTargetPosition - m_vCameraPosition;
	D3DXVec3Normalize(&m_vLookVector, &m_vLookVector);

												
	D3DXVec3Cross(&m_vUpVector, &m_vLookVector, &m_vRightVector);
	D3DXVec3Normalize(&m_vUpVector, &m_vUpVector);
	D3DXVec3Cross(&m_vRightVector, &m_vUpVector, &m_vLookVector);
	D3DXVec3Normalize(&m_vRightVector, &m_vRightVector);
}
VOID F_Camera::SetCameraPosition(D3DXVECTOR3 *pVector)
{
	D3DXVECTOR3 V = D3DXVECTOR3(0.0f, 200.0f, -250.0f);
	m_vCameraPosition = pVector ? (*pVector) : V;		
}
VOID F_Camera::SetViewMatrix(D3DXMATRIX *pMatrix)
{
	if (pMatrix) m_matView = *pMatrix;
	else CalculateViewMatrix(&m_matView);
	m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_matView);
	m_vRightVector = D3DXVECTOR3(m_matView._11, m_matView._12, m_matView._13);
	m_vUpVector = D3DXVECTOR3(m_matView._21, m_matView._22, m_matView._23);
	m_vLookVector = D3DXVECTOR3(m_matView._31, m_matView._32, m_matView._33);
}
VOID F_Camera::SetProjMatrix(D3DXMATRIX *pMatrix)
{
	if (pMatrix != NULL) m_matProj = *pMatrix;
	else D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4.0f, (float)((double)WINDOW_WIDTH / WINDOW_HEIGHT), 1.0f, 30000.0f);
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
}
VOID F_Camera::MoveAlongRightVec(FLOAT fUnits)
{

	m_vCameraPosition += m_vRightVector * fUnits;
	m_vTargetPosition += m_vRightVector * fUnits;
}
VOID F_Camera::MoveAlongUpVec(FLOAT fUnits)
{
	m_vCameraPosition += m_vUpVector * fUnits;
	m_vTargetPosition += m_vUpVector * fUnits;
}
VOID F_Camera::MoveAlongLookVec(FLOAT fUnits)
{

	m_vCameraPosition += m_vLookVector * fUnits;
	m_vTargetPosition += m_vLookVector * fUnits;
}
VOID F_Camera::RotationRightVec(FLOAT fAngle)
{
	delAngleRight += fAngle;
	if (delAngleRight <= D3DX_PI / 2 && delAngleRight >=- D3DX_PI / 3)
	{
		D3DXMATRIX R;
		D3DXMatrixRotationAxis(&R, &m_vRightVector, fAngle);
		D3DXVec3TransformCoord(&m_vUpVector, &m_vUpVector, &R);
		D3DXVec3TransformCoord(&m_vLookVector, &m_vLookVector, &R);
		m_vRightVector.y = 0;
		m_vTargetPosition = m_vLookVector * D3DXVec3Length(&m_vCameraPosition);

	}
	else
	{
		if (delAngleRight > 0)
			delAngleRight = D3DX_PI / 2;
		else
			delAngleRight = -D3DX_PI / 3;
	}

}
VOID F_Camera::RotationUpVec(FLOAT fAngle)
{
	D3DXMATRIX R;
	D3DXMatrixRotationAxis(&R, &m_vUpVector, fAngle);
	D3DXVec3TransformCoord(&m_vRightVector, &m_vRightVector, &R);
	D3DXVec3TransformCoord(&m_vLookVector, &m_vLookVector, &R);

	m_vTargetPosition = m_vLookVector * D3DXVec3Length(&m_vCameraPosition);
}
VOID F_Camera::RotationLookVec(FLOAT fAngle)
{
	D3DXMATRIX R;
	D3DXMatrixRotationAxis(&R, &m_vLookVector, fAngle);
	D3DXVec3TransformCoord(&m_vRightVector, &m_vRightVector, &R);
	D3DXVec3TransformCoord(&m_vUpVector, &m_vUpVector, &R);

	m_vTargetPosition = m_vLookVector * D3DXVec3Length(&m_vCameraPosition);
}
F_Camera::~F_Camera()
{
	;
}
VOID F_Camera::ReactToInput(F_Input* input,FLOAT hLimit,float nbei)
{
	
	if (this->m_Type == CAM_FREE)
	{
		if (input->IskeyPressed(KeyBoardType, DIK_A))  this->MoveAlongRightVec(-5*nbei);
		if (input->IskeyPressed(KeyBoardType, DIK_D))  this->MoveAlongRightVec(5 * nbei);
		if (input->IskeyPressed(KeyBoardType, DIK_W))  this->MoveAlongLookVec(5 * nbei);
		if (input->IskeyPressed(KeyBoardType, DIK_S))  this->MoveAlongLookVec(-5 * nbei);

	}
	else
	{
		if (input->IskeyPressed(KeyBoardType, DIK_A))  this->MoveAlongRightVec(-5 * nbei);
		if (input->IskeyPressed(KeyBoardType, DIK_D))  this->MoveAlongRightVec(5 * nbei);
		if (input->IskeyPressed(KeyBoardType, DIK_W))  this->MoveAlongLookVec(5 * nbei);
		if (input->IskeyPressed(KeyBoardType, DIK_S))  this->MoveAlongLookVec(-5 * nbei);
		this->SetCameraHeightY(hLimit);
	}
	this->RotationUpVec(input->GetMouseMovement(0)* 0.001f);
	this->RotationRightVec(input->GetMouseMovement(1) * 0.001f);

	//鼠标滚轮控制观察点收缩操作
	static FLOAT fPosZ = 0.0f;
	fPosZ += input->GetMouseMovement(2)*0.03f;
	//鼠标控制右向量和上向量的旋转
	//计算并设置取景变换矩阵
	D3DXMATRIX matView;
	this->CalculateViewMatrix(&matView);
	this->m_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);
}
VOID F_Camera::GetViewMatrix(D3DXMATRIX *pView) { *pView = m_matView; }



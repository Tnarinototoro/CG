#pragma once
#include"Runtime.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <fstream>
VOID ComputeNomals(D3DXVECTOR3* p0, D3DXVECTOR3* p1, D3DXVECTOR3 *p2, D3DXVECTOR3 *out)
{
	D3DXVECTOR3 u = *p1 - *p0;
	D3DXVECTOR3 v = *p2 - *p0;
	D3DXVec3Cross(out, &u, &v);
	D3DXVec3Normalize(out, out);
}
float Lerp(float a, float b, float t)
{
	return a - (a*t) + (b*t);
}
class F_Terrain
{
private:
	LPDIRECT3DDEVICE9				m_pd3dDevice;			//D3D�豸
	LPDIRECT3DTEXTURE9			m_pTexture;				//����
	LPDIRECT3DINDEXBUFFER9	m_pIndexBuffer;			//���㻺��
	LPDIRECT3DVERTEXBUFFER9	m_pVertexBuffer;		//��������

	int								m_nCellsPerRow;		// ÿ�еĵ�Ԫ����
	int								m_nCellsPerCol;			// ÿ�еĵ�Ԫ����
	int								m_nVertsPerRow;		// ÿ�еĶ�����
	int								m_nVertsPerCol;			// ÿ�еĶ�����
	int								m_nNumVertices;		// ��������
	FLOAT						m_fTerrainWidth;		// ���εĿ��
	FLOAT						m_fTerrainDepth;		// ���ε����
	FLOAT						m_fCellSpacing;			// ��Ԫ��ļ��
	FLOAT						m_fHeightScale;			// �߶�����ϵ��
	std::vector<FLOAT>   m_vHeightInfo;			// ���ڴ�Ÿ߶���Ϣ
	D3DMATERIAL9 m_landMaterial;
												//����һ�����ε�FVF�����ʽ
	struct TERRAINVERTEX
	{
		D3DXVECTOR3 _pos;
		D3DXVECTOR3 _normal;
		FLOAT _u, _v;
		TERRAINVERTEX(D3DXVECTOR3 pos,D3DXVECTOR3 normal, FLOAT u, FLOAT v)
			:_pos(pos), _normal(normal), _u(u), _v(v) {}
		static const DWORD FVF = D3DFVF_XYZ |D3DFVF_NORMAL|D3DFVF_TEX1;
	};

public:
	F_Terrain(IDirect3DDevice9 *pd3dDevice); //���캯��
	virtual ~F_Terrain(void);		//��������
	FLOAT GetHeightY(float xr, float zr)
	{
		//Detect whether the object is in the map
		if (std::abs(xr) < m_fTerrainWidth / 2 && std::abs(zr) < m_fTerrainDepth / 2)
		{
			FLOAT height = 0.0f;
			float z = (-zr + (m_fTerrainDepth / 2.0));
			z/= m_fCellSpacing;
			float x = (xr + (m_fTerrainWidth / 2.0f));
			x/= m_fCellSpacing;
			float col = ::floorf(x);
			float row = ::floorf(z);
			float A= m_vHeightInfo[row*m_nCellsPerRow+col];
			float B= m_vHeightInfo[row*m_nCellsPerRow + col+1];
			float C= m_vHeightInfo[(row+1)*m_nCellsPerRow + col];
			float D= m_vHeightInfo[(row+1)*m_nCellsPerRow + col+1];
			float dx = x - col;
			float dz = z - row;
			if (dz < 1.0f - dx)
			{
				float uy = B - A;
				float vy = C - A;
				height = A + Lerp(0.0f, uy, dx) + Lerp(0.0f, vy, dz);
				return height;
			}
			else
			{
				float uy = C - D;
				float vy = B - D;
				height= D + Lerp(0.0f, uy,1.0f- dx) + Lerp(0.0f, vy,1.0f-dz);
				return height;
			}
		}
		else
			return 600;
	}
	FLOAT GetfSpace()
	{
		return this->m_fCellSpacing;
	}
	//To get whether you are out of the terrain
	BOOL Out_terrain(float x, float z)
	{
		return (std::fabs(x) > m_fTerrainWidth / 2 || std::fabs(z) > m_fTerrainWidth / 2);
	}
public:
	BOOL LoadTerrainFromFile(wchar_t *pRawFileName, wchar_t *pTextureFile);		//���ļ����ظ߶�ͼ������ĺ���
	BOOL InitTerrain(INT nRows, INT nCols, FLOAT fSpace, FLOAT fScale);  //���γ�ʼ������
	BOOL RenderTerrain(D3DXMATRIX *pMatWorld, BOOL bDrawFrame = FALSE);  //������Ⱦ����
};


//-----------------------------------------------------------------------------
// Desc: ���캯��
//-----------------------------------------------------------------------------
F_Terrain::F_Terrain(IDirect3DDevice9* pd3dDevice)
{
	//��������Ա��������ֵ
	m_pd3dDevice = pd3dDevice;
	m_pTexture = NULL;
	m_pIndexBuffer = NULL;
	m_pVertexBuffer = NULL;
	m_nCellsPerRow = 0;
	m_nCellsPerCol = 0;
	m_nVertsPerRow = 0;
	m_nVertsPerCol = 0;
	m_nNumVertices = 0;
	m_fTerrainWidth = 0.0f;
	m_fTerrainDepth = 0.0f;
	m_fCellSpacing = 0.0f;
	m_fHeightScale = 0.0f;
	::ZeroMemory(&m_landMaterial, sizeof(m_landMaterial)); m_landMaterial.Ambient = D3DXCOLOR(0.5f, 0.5f, 0.7f, 1.0f);
	m_landMaterial.Diffuse = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);
	m_landMaterial.Specular = D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.3f);
	m_landMaterial.Emissive = D3DXCOLOR(0.3f, 0.0f, 0.1f, 1.0f);
}


//--------------------------------------------------------------------------------------
// Name: TerrainClass::LoadTerrainFromFile()
// Desc: ���ص��θ߶���Ϣ�Լ�����
//--------------------------------------------------------------------------------------
BOOL F_Terrain::LoadTerrainFromFile(wchar_t *pRawFileName, wchar_t *pTextureFile)
{
	// ���ļ��ж�ȡ�߶���Ϣ
	std::ifstream inFile;
	inFile.open(pRawFileName, std::ios::binary);   //�ö����Ƶķ�ʽ���ļ�

	inFile.seekg(0, std::ios::end);							//���ļ�ָ���ƶ����ļ�ĩβ
	std::vector<BYTE> inData(inFile.tellg());			//��ģ�嶨��һ��vector<BYTE>���͵ı���inData����ʼ������ֵΪ��������ǰλ�ã�����������С

	inFile.seekg(std::ios::beg);								//���ļ�ָ���ƶ����ļ��Ŀ�ͷ��׼����ȡ�߶���Ϣ
	inFile.read((char*)&inData[0], inData.size());	//�ؼ���һ������ȡ�����߶���Ϣ
	inFile.close();													//�������������Թر��ļ���

	m_vHeightInfo.resize(inData.size());					//��m_vHeightInfo�ߴ�ȡΪ�������ĳߴ�
															//������������������inData�е�ֵ����m_vHeightInfo
	for (unsigned int i = 0; i<inData.size(); i++)
		m_vHeightInfo[i] = inData[i];

	if (FAILED(D3DXCreateTextureFromFile(m_pd3dDevice, pTextureFile, &m_pTexture)))
		return FALSE;

	return TRUE;
}
BOOL F_Terrain::InitTerrain(INT nRows, INT nCols, FLOAT fSpace, FLOAT fScale)
{
	m_nCellsPerRow = nRows;  //ÿ�еĵ�Ԫ����Ŀ
	m_nCellsPerCol = nCols;  //ÿ�еĵ�Ԫ����Ŀ
	m_fCellSpacing = fSpace;	//��Ԫ���ļ��
	m_fHeightScale = fScale; //�߶�����ϵ��
	m_fTerrainWidth = nRows * fSpace;  //���εĿ��
	m_fTerrainDepth = nCols * fSpace;  //���ε����
	m_nVertsPerRow = m_nCellsPerCol + 1;  //ÿ�еĶ�����
	m_nVertsPerCol = m_nCellsPerRow + 1; //ÿ�еĶ�����
	m_nNumVertices = m_nVertsPerRow * m_nVertsPerCol;  //��������

													   // ͨ��һ��forѭ��������ѵ���ԭʼ�߶ȳ�������ϵ�����õ����ź�ĸ߶�
	for (unsigned int i = 0; i<m_vHeightInfo.size(); i++)
		m_vHeightInfo[i] *= m_fHeightScale;
	
	if (FAILED(m_pd3dDevice->CreateVertexBuffer(m_nNumVertices * sizeof(TERRAINVERTEX),
		D3DUSAGE_WRITEONLY, TERRAINVERTEX::FVF, D3DPOOL_MANAGED, &m_pVertexBuffer, 0)))
		return FALSE;

	TERRAINVERTEX *pVertices = NULL;
	m_pVertexBuffer->Lock(0, 0, (void**)&pVertices, 0);

	FLOAT fStartX = -m_fTerrainWidth / 2.0f, fEndX = m_fTerrainWidth / 2.0f;		//ָ����ʼ��ͽ������X����ֵ
	FLOAT fStartZ = m_fTerrainDepth / 2.0f, fEndZ = -m_fTerrainDepth / 2.0f;	//ָ����ʼ��ͽ������Z����ֵ
	FLOAT fCoordU = 10.0f / (FLOAT)m_nCellsPerRow;     //ָ������ĺ�����ֵ
	FLOAT fCoordV = 10.0f / (FLOAT)m_nCellsPerCol;		//ָ�������������ֵ

	int nIndex = 0, i = 0, j = 0;
	for (float z = fStartZ; z >= fEndZ; z -= m_fCellSpacing, i++)		//Z���귽������ʼ���㵽���������м�ı���
	{
		j = 0;
		for (float x = fStartX; x <= fEndX; x += m_fCellSpacing, j++)	//X���귽������ʼ���㵽���������м�ı���
		{
			D3DXVECTOR3 tmp;
			nIndex = i * m_nCellsPerRow + j;		//ָ����ǰ�����ڶ��㻺���е�λ��
			pVertices[nIndex] = TERRAINVERTEX(D3DXVECTOR3(x, m_vHeightInfo[nIndex], z),tmp, j*fCoordU, i*fCoordV); //�Ѷ���λ�������ڸ߶�ͼ�ж�Ӧ�ĸ�����������Լ��������긳ֵ��������ǰ�Ķ���
			nIndex++;											//�������Լ�1
		}
	}

	m_pVertexBuffer->Unlock();

	
	if (FAILED(m_pd3dDevice->CreateIndexBuffer(m_nNumVertices * 6 * sizeof(WORD),
		D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIndexBuffer, 0)))
		return FALSE;
	
	WORD* pIndices = NULL;
	m_pIndexBuffer->Lock(0, 0, (void**)&pIndices, 0);
	
	nIndex = 0;
	for (int row = 0; row < m_nCellsPerRow - 1; row++)   
	{
		for (int col = 0; col < m_nCellsPerCol - 1; col++)  
		{
			//calculate abc normals
			D3DXVECTOR3 pa = pVertices[row * m_nCellsPerRow + col]._pos;
			D3DXVECTOR3 pb = pVertices[row * m_nCellsPerRow + col + 1]._pos;
			D3DXVECTOR3 pc = pVertices[(row + 1) * m_nCellsPerRow + col]._pos;
			D3DXVECTOR3 pd = pVertices[(row + 1) * m_nCellsPerRow + col + 1]._pos;
			D3DXVECTOR3 out1;
			ComputeNomals(&pa, &pb, &pc, &out1);
			D3DXVECTOR3 out2;
			ComputeNomals(&pc, &pb, &pd, &out2);
			//calculate cbd normal
			D3DXVECTOR3 out3 = (out1 + out2);
			D3DXVec3Normalize(&out3, &out3);
			pVertices[row * m_nCellsPerRow + col]._normal = out1;  // A 
			pVertices[(row + 1) * m_nCellsPerRow + col]._normal = out3;  //C
			pVertices[row * m_nCellsPerRow + col + 1]._normal = out3;  //B
			pVertices[(row + 1) * m_nCellsPerRow + col + 1]._normal = out2; //D
			
			//������CBD����������
			pIndices[nIndex] = row * m_nCellsPerRow + col;			//����A
			pIndices[nIndex + 1] = row * m_nCellsPerRow + col + 1;  //����B
			pIndices[nIndex + 2] = (row + 1) * m_nCellsPerRow + col;	//����C
			pIndices[nIndex + 3] = (row + 1) * m_nCellsPerRow + col;		//����C
			pIndices[nIndex + 4] = row * m_nCellsPerRow + col + 1;		//����B
			pIndices[nIndex + 5] = (row + 1) * m_nCellsPerRow + col + 1;//����D
																		//������һ����Ԫ����������6
			nIndex += 6;  //�����Լ�6
		}
	}
	m_pIndexBuffer->Unlock();

	return TRUE;
}
BOOL F_Terrain::RenderTerrain(D3DXMATRIX *pMatWorld, BOOL bRenderFrame)
{
	m_pd3dDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(TERRAINVERTEX));
	m_pd3dDevice->SetFVF(TERRAINVERTEX::FVF);//ָ������ʹ�õ������ʽ�ĺ�����
	m_pd3dDevice->SetIndices(m_pIndexBuffer);//������������  
	m_pd3dDevice->SetTexture(0, m_pTexture);//��������
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);  //�򿪹���
	m_pd3dDevice->SetMaterial(&m_landMaterial);
	m_pd3dDevice->SetTransform(D3DTS_WORLD, pMatWorld); //�����������
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
		m_nNumVertices, 0, m_nNumVertices * 2);		//���ƶ���


	m_pd3dDevice->SetTexture(0, 0);	//�����ÿ�

									
	if (bRenderFrame)  //���Ҫ��Ⱦ���߿�Ļ�
	{
		m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME); //�����ģʽ��Ϊ�߿����
		m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
			m_nNumVertices, 0, m_nNumVertices * 2);	//���ƶ���  
		m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);	//�����ģʽ����ʵ�����
	}
	return TRUE;
}
F_Terrain::~F_Terrain(void)
{
	SAFE_RELEASE(m_pTexture);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pVertexBuffer);
}
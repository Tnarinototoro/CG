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
	LPDIRECT3DDEVICE9				m_pd3dDevice;			//D3D设备
	LPDIRECT3DTEXTURE9			m_pTexture;				//纹理
	LPDIRECT3DINDEXBUFFER9	m_pIndexBuffer;			//顶点缓存
	LPDIRECT3DVERTEXBUFFER9	m_pVertexBuffer;		//索引缓存

	int								m_nCellsPerRow;		// 每行的单元格数
	int								m_nCellsPerCol;			// 每列的单元格数
	int								m_nVertsPerRow;		// 每行的顶点数
	int								m_nVertsPerCol;			// 每列的顶点数
	int								m_nNumVertices;		// 顶点总数
	FLOAT						m_fTerrainWidth;		// 地形的宽度
	FLOAT						m_fTerrainDepth;		// 地形的深度
	FLOAT						m_fCellSpacing;			// 单元格的间距
	FLOAT						m_fHeightScale;			// 高度缩放系数
	std::vector<FLOAT>   m_vHeightInfo;			// 用于存放高度信息
	D3DMATERIAL9 m_landMaterial;
												//定义一个地形的FVF顶点格式
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
	F_Terrain(IDirect3DDevice9 *pd3dDevice); //构造函数
	virtual ~F_Terrain(void);		//析构函数
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
	BOOL LoadTerrainFromFile(wchar_t *pRawFileName, wchar_t *pTextureFile);		//从文件加载高度图和纹理的函数
	BOOL InitTerrain(INT nRows, INT nCols, FLOAT fSpace, FLOAT fScale);  //地形初始化函数
	BOOL RenderTerrain(D3DXMATRIX *pMatWorld, BOOL bDrawFrame = FALSE);  //地形渲染函数
};


//-----------------------------------------------------------------------------
// Desc: 构造函数
//-----------------------------------------------------------------------------
F_Terrain::F_Terrain(IDirect3DDevice9* pd3dDevice)
{
	//给各个成员变量赋初值
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
// Desc: 加载地形高度信息以及纹理
//--------------------------------------------------------------------------------------
BOOL F_Terrain::LoadTerrainFromFile(wchar_t *pRawFileName, wchar_t *pTextureFile)
{
	// 从文件中读取高度信息
	std::ifstream inFile;
	inFile.open(pRawFileName, std::ios::binary);   //用二进制的方式打开文件

	inFile.seekg(0, std::ios::end);							//把文件指针移动到文件末尾
	std::vector<BYTE> inData(inFile.tellg());			//用模板定义一个vector<BYTE>类型的变量inData并初始化，其值为缓冲区当前位置，即缓冲区大小

	inFile.seekg(std::ios::beg);								//将文件指针移动到文件的开头，准备读取高度信息
	inFile.read((char*)&inData[0], inData.size());	//关键的一步，读取整个高度信息
	inFile.close();													//操作结束，可以关闭文件了

	m_vHeightInfo.resize(inData.size());					//将m_vHeightInfo尺寸取为缓冲区的尺寸
															//遍历整个缓冲区，将inData中的值赋给m_vHeightInfo
	for (unsigned int i = 0; i<inData.size(); i++)
		m_vHeightInfo[i] = inData[i];

	if (FAILED(D3DXCreateTextureFromFile(m_pd3dDevice, pTextureFile, &m_pTexture)))
		return FALSE;

	return TRUE;
}
BOOL F_Terrain::InitTerrain(INT nRows, INT nCols, FLOAT fSpace, FLOAT fScale)
{
	m_nCellsPerRow = nRows;  //每行的单元格数目
	m_nCellsPerCol = nCols;  //每列的单元格数目
	m_fCellSpacing = fSpace;	//单元格间的间距
	m_fHeightScale = fScale; //高度缩放系数
	m_fTerrainWidth = nRows * fSpace;  //地形的宽度
	m_fTerrainDepth = nCols * fSpace;  //地形的深度
	m_nVertsPerRow = m_nCellsPerCol + 1;  //每行的顶点数
	m_nVertsPerCol = m_nCellsPerRow + 1; //每列的顶点数
	m_nNumVertices = m_nVertsPerRow * m_nVertsPerCol;  //顶点总数

													   // 通过一个for循环，逐个把地形原始高度乘以缩放系数，得到缩放后的高度
	for (unsigned int i = 0; i<m_vHeightInfo.size(); i++)
		m_vHeightInfo[i] *= m_fHeightScale;
	
	if (FAILED(m_pd3dDevice->CreateVertexBuffer(m_nNumVertices * sizeof(TERRAINVERTEX),
		D3DUSAGE_WRITEONLY, TERRAINVERTEX::FVF, D3DPOOL_MANAGED, &m_pVertexBuffer, 0)))
		return FALSE;

	TERRAINVERTEX *pVertices = NULL;
	m_pVertexBuffer->Lock(0, 0, (void**)&pVertices, 0);

	FLOAT fStartX = -m_fTerrainWidth / 2.0f, fEndX = m_fTerrainWidth / 2.0f;		//指定起始点和结束点的X坐标值
	FLOAT fStartZ = m_fTerrainDepth / 2.0f, fEndZ = -m_fTerrainDepth / 2.0f;	//指定起始点和结束点的Z坐标值
	FLOAT fCoordU = 10.0f / (FLOAT)m_nCellsPerRow;     //指定纹理的横坐标值
	FLOAT fCoordV = 10.0f / (FLOAT)m_nCellsPerCol;		//指定纹理的纵坐标值

	int nIndex = 0, i = 0, j = 0;
	for (float z = fStartZ; z >= fEndZ; z -= m_fCellSpacing, i++)		//Z坐标方向上起始顶点到结束顶点行间的遍历
	{
		j = 0;
		for (float x = fStartX; x <= fEndX; x += m_fCellSpacing, j++)	//X坐标方向上起始顶点到结束顶点行间的遍历
		{
			D3DXVECTOR3 tmp;
			nIndex = i * m_nCellsPerRow + j;		//指定当前顶点在顶点缓存中的位置
			pVertices[nIndex] = TERRAINVERTEX(D3DXVECTOR3(x, m_vHeightInfo[nIndex], z),tmp, j*fCoordU, i*fCoordV); //把顶点位置索引在高度图中对应的各个顶点参数以及纹理坐标赋值给赋给当前的顶点
			nIndex++;											//索引数自加1
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
			
			//三角形CBD的三个顶点
			pIndices[nIndex] = row * m_nCellsPerRow + col;			//顶点A
			pIndices[nIndex + 1] = row * m_nCellsPerRow + col + 1;  //顶点B
			pIndices[nIndex + 2] = (row + 1) * m_nCellsPerRow + col;	//顶点C
			pIndices[nIndex + 3] = (row + 1) * m_nCellsPerRow + col;		//顶点C
			pIndices[nIndex + 4] = row * m_nCellsPerRow + col + 1;		//顶点B
			pIndices[nIndex + 5] = (row + 1) * m_nCellsPerRow + col + 1;//顶点D
																		//处理完一个单元格，索引加上6
			nIndex += 6;  //索引自加6
		}
	}
	m_pIndexBuffer->Unlock();

	return TRUE;
}
BOOL F_Terrain::RenderTerrain(D3DXMATRIX *pMatWorld, BOOL bRenderFrame)
{
	m_pd3dDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(TERRAINVERTEX));
	m_pd3dDevice->SetFVF(TERRAINVERTEX::FVF);//指定我们使用的灵活顶点格式的宏名称
	m_pd3dDevice->SetIndices(m_pIndexBuffer);//设置索引缓存  
	m_pd3dDevice->SetTexture(0, m_pTexture);//设置纹理
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);  //打开光照
	m_pd3dDevice->SetMaterial(&m_landMaterial);
	m_pd3dDevice->SetTransform(D3DTS_WORLD, pMatWorld); //设置世界矩阵
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
		m_nNumVertices, 0, m_nNumVertices * 2);		//绘制顶点


	m_pd3dDevice->SetTexture(0, 0);	//纹理置空

									
	if (bRenderFrame)  //如果要渲染出线框的话
	{
		m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME); //把填充模式设为线框填充
		m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
			m_nNumVertices, 0, m_nNumVertices * 2);	//绘制顶点  
		m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);	//把填充模式调回实体填充
	}
	return TRUE;
}
F_Terrain::~F_Terrain(void)
{
	SAFE_RELEASE(m_pTexture);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pVertexBuffer);
}
#pragma once
#include"Runtime.h"
#include "F_Terrain.h"
enum ANIM_RENDER_METHOD
{
	D3DNONINDEXED,
	D3DINDEXED,
	SOFTWARE,
	D3DINDEXEDVS,
	D3DINDEXEDHLSLVS,
	NONE
}; // the way to render animation
WCHAR g_wszShaderSource[4][30] =
{
	L"skinmesh1.vsh",
	L"skinmesh2.vsh",
	L"skinmesh3.vsh",
	L"skinmesh4.vsh"
};
//Some Global variables!
UINT                        g_NumBoneMatricesMax;
D3DXMATRIXA16*              g_pBoneMatrices;
ANIM_RENDER_METHOD                      g_SkinningMethod = D3DNONINDEXED; // Current skinning method
																		  //You can take it as a piece of your bone! 
struct D3DXFRAME_DERIVED : public D3DXFRAME
{
	D3DXMATRIXA16 CombinedTransformationMatrix;
};
//You can take it as the mesh on wrapping your bone!
struct D3DXMESHCONTAINER_DERIVED : public D3DXMESHCONTAINER
{
	LPDIRECT3DTEXTURE9 * ppTextures; //array of txtures,entries are NULL if no Textures specified
									 //SkinMesh info
	LPD3DXMESH pOriginMesh;//last time drawn mesh
	LPD3DXATTRIBUTERANGE pAttributeTable;//weight of the matrices influencing the mesh
	DWORD NumAttributeGroups;//How many groups influence yourmesh!
	DWORD NumInfl;
	LPD3DXBUFFER pBoneCombinationBuf;
	D3DXMATRIX** ppBoneMatrixPtrs;
	D3DXMATRIX* pBoneOffsetMatrices;
	DWORD NumPaletteEntries;
	bool UseSoftwareVP; //use soft rendering or not
	DWORD iAttributeSW; //  used to denote the split between SW and HW if necessary for non-indexed skinning
};
class AllocateHierarchy : public ID3DXAllocateHierarchy
{
public:
	STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);
	STDMETHOD(CreateMeshContainer)(THIS_
		LPCSTR Name,
		CONST D3DXMESHDATA *pMeshData,
		CONST D3DXMATERIAL *pMaterials,
		CONST D3DXEFFECTINSTANCE *pEffectInstances,
		DWORD NumMaterials,
		CONST DWORD *pAdjacency,
		LPD3DXSKININFO pSkinInfo,
		LPD3DXMESHCONTAINER *ppNewMeshContainer);
	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);
	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase);

	AllocateHierarchy()
	{
	}

};





HRESULT GenerateSkinnedMesh(IDirect3DDevice9* pd3dDevice, D3DXMESHCONTAINER_DERIVED* pMeshContainer);
//--------------------------------------------------------------------------------------
// Name: AllocateName()
// Desc: Allocates memory for a string to hold the name of a frame or mesh
//--------------------------------------------------------------------------------------
HRESULT AllocateName(LPCSTR Name, LPSTR* pNewName)
{
	UINT cbLength;

	if (Name != NULL)
	{
		cbLength = (UINT)strlen(Name) + 1;
		*pNewName = new CHAR[cbLength];
		if (*pNewName == NULL)
			return E_OUTOFMEMORY;
		memcpy(*pNewName, Name, cbLength * sizeof(CHAR));
	}
	else
	{
		*pNewName = NULL;
	}

	return S_OK;
}
//--------------------------------------------------------------------------------------
// Name: CAllocateHierarchy::CreateFrame()
// Desc: 
//--------------------------------------------------------------------------------------
HRESULT AllocateHierarchy::CreateFrame(LPCSTR Name, LPD3DXFRAME* ppNewFrame)
{
	HRESULT hr = S_OK;
	D3DXFRAME_DERIVED* pFrame;

	*ppNewFrame = NULL;

	pFrame = new D3DXFRAME_DERIVED;
	if (pFrame == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto e_Exit;
	}

	hr = AllocateName(Name, &pFrame->Name);
	if (FAILED(hr))
		goto e_Exit;

	// initialize other data members of the frame
	D3DXMatrixIdentity(&pFrame->TransformationMatrix);
	D3DXMatrixIdentity(&pFrame->CombinedTransformationMatrix);

	pFrame->pMeshContainer = NULL;
	pFrame->pFrameSibling = NULL;
	pFrame->pFrameFirstChild = NULL;

	*ppNewFrame = pFrame;
	pFrame = NULL;

e_Exit:
	delete pFrame;
	return hr;
}
//--------------------------------------------------------------------------------------
// Name: CAllocateHierarchy::CreateMeshContainer()
// Desc: 
//--------------------------------------------------------------------------------------
HRESULT AllocateHierarchy::CreateMeshContainer
(
	LPCSTR Name,
	CONST D3DXMESHDATA *pMeshData,
	CONST D3DXMATERIAL *pMaterials,
	CONST D3DXEFFECTINSTANCE *pEffectInstances,
	DWORD NumMaterials,
	CONST DWORD *pAdjacency,
	LPD3DXSKININFO pSkinInfo,
	LPD3DXMESHCONTAINER *ppNewMeshContainer)
{
	HRESULT hr;
	D3DXMESHCONTAINER_DERIVED *pMeshContainer = NULL;
	UINT NumFaces;
	UINT iMaterial;
	UINT iBone, cBones;
	LPDIRECT3DDEVICE9 pd3dDevice = NULL;

	LPD3DXMESH pMesh = NULL;

	*ppNewMeshContainer = NULL;

	// this sample does not handle patch meshes, so fail when one is found
	if (pMeshData->Type != D3DXMESHTYPE_MESH)
	{
		hr = E_FAIL;
		goto e_Exit;
	}

	// get the pMesh interface pointer out of the mesh data structure
	pMesh = pMeshData->pMesh;

	// this sample does not FVF compatible meshes, so fail when one is found
	if (pMesh->GetFVF() == 0)
	{
		hr = E_FAIL;
		goto e_Exit;
	}

	// allocate the overloaded structure to return as a D3DXMESHCONTAINER
	pMeshContainer = new D3DXMESHCONTAINER_DERIVED;
	if (pMeshContainer == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto e_Exit;
	}
	memset(pMeshContainer, 0, sizeof(D3DXMESHCONTAINER_DERIVED));

	// make sure and copy the name.  All memory as input belongs to caller, interfaces can be addref'd though
	hr = AllocateName(Name, &pMeshContainer->Name);
	if (FAILED(hr))
		goto e_Exit;

	pMesh->GetDevice(&pd3dDevice);
	NumFaces = pMesh->GetNumFaces();

	// if no normals are in the mesh, add them
	if (!(pMesh->GetFVF() & D3DFVF_NORMAL))
	{
		pMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;

		// clone the mesh to make room for the normals
		hr = pMesh->CloneMeshFVF(pMesh->GetOptions(),
			pMesh->GetFVF() | D3DFVF_NORMAL,
			pd3dDevice, &pMeshContainer->MeshData.pMesh);
		if (FAILED(hr))
			goto e_Exit;

		// get the new pMesh pointer back out of the mesh container to use
		// NOTE: we do not release pMesh because we do not have a reference to it yet
		pMesh = pMeshContainer->MeshData.pMesh;

		// now generate the normals for the pmesh
		D3DXComputeNormals(pMesh, NULL);
	}
	else  // if no normals, just add a reference to the mesh for the mesh container
	{
		pMeshContainer->MeshData.pMesh = pMesh;
		pMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;

		pMesh->AddRef();
	}

	// allocate memory to contain the material information.  This sample uses
	//   the D3D9 materials and texture names instead of the EffectInstance style materials
	pMeshContainer->NumMaterials = max(1, NumMaterials);
	pMeshContainer->pMaterials = new D3DXMATERIAL[pMeshContainer->NumMaterials];
	pMeshContainer->ppTextures = new LPDIRECT3DTEXTURE9[pMeshContainer->NumMaterials];
	pMeshContainer->pAdjacency = new DWORD[NumFaces * 3];
	if ((pMeshContainer->pAdjacency == NULL) || (pMeshContainer->pMaterials == NULL))
	{
		hr = E_OUTOFMEMORY;
		goto e_Exit;
	}

	memcpy(pMeshContainer->pAdjacency, pAdjacency, sizeof(DWORD) * NumFaces * 3);
	memset(pMeshContainer->ppTextures, 0, sizeof(LPDIRECT3DTEXTURE9) * pMeshContainer->NumMaterials);

	// if materials provided, copy them
	if (NumMaterials > 0)
	{
		memcpy(pMeshContainer->pMaterials, pMaterials, sizeof(D3DXMATERIAL) * NumMaterials);

		for (iMaterial = 0; iMaterial < NumMaterials; iMaterial++)
		{
			if (pMeshContainer->pMaterials[iMaterial].pTextureFilename != NULL)
			{
				WCHAR strTexturePath[MAX_PATH];

				MultiByteToWideChar(CP_ACP, 0, pMeshContainer->pMaterials[iMaterial].pTextureFilename, -1, strTexturePath, MAX_PATH);
				if (FAILED(D3DXCreateTextureFromFile(pd3dDevice, strTexturePath,
					&pMeshContainer->ppTextures[iMaterial])))
					pMeshContainer->ppTextures[iMaterial] = NULL;

				// don't remember a pointer into the dynamic memory, just forget the name after loading
				pMeshContainer->pMaterials[iMaterial].pTextureFilename = NULL;
			}
		}
	}
	else // if no materials provided, use a default one
	{
		pMeshContainer->pMaterials[0].pTextureFilename = NULL;
		memset(&pMeshContainer->pMaterials[0].MatD3D, 0, sizeof(D3DMATERIAL9));
		pMeshContainer->pMaterials[0].MatD3D.Diffuse.r = 0.5f;
		pMeshContainer->pMaterials[0].MatD3D.Diffuse.g = 0.5f;
		pMeshContainer->pMaterials[0].MatD3D.Diffuse.b = 0.5f;
		pMeshContainer->pMaterials[0].MatD3D.Specular = pMeshContainer->pMaterials[0].MatD3D.Diffuse;
	}

	// if there is skinning information, save off the required data and then setup for HW skinning
	if (pSkinInfo != NULL)
	{
		// first save off the SkinInfo and original mesh data
		pMeshContainer->pSkinInfo = pSkinInfo;
		pSkinInfo->AddRef();

		pMeshContainer->pOriginMesh = pMesh;
		pMesh->AddRef();

		// Will need an array of offset matrices to move the vertices from the figure space to the bone's space
		cBones = pSkinInfo->GetNumBones();
		pMeshContainer->pBoneOffsetMatrices = new D3DXMATRIX[cBones];
		if (pMeshContainer->pBoneOffsetMatrices == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto e_Exit;
		}

		// get each of the bone offset matrices so that we don't need to get them later
		for (iBone = 0; iBone < cBones; iBone++)
		{
			pMeshContainer->pBoneOffsetMatrices[iBone] = *(pMeshContainer->pSkinInfo->GetBoneOffsetMatrix(iBone));
		}

		// GenerateSkinnedMesh will take the general skinning information and transform it to a HW friendly version
		hr = GenerateSkinnedMesh(pd3dDevice, pMeshContainer);
		if (FAILED(hr))
			goto e_Exit;
	}

	*ppNewMeshContainer = pMeshContainer;
	pMeshContainer = NULL;

e_Exit:
	SAFE_RELEASE(pd3dDevice);

	// call Destroy function to properly clean up the memory allocated 
	if (pMeshContainer != NULL)
	{
		DestroyMeshContainer(pMeshContainer);
	}

	return hr;
}

HRESULT AllocateHierarchy::DestroyFrame(LPD3DXFRAME pFrameToFree)
{
	SAFE_DELETE_ARRAY(pFrameToFree->Name);
	SAFE_DELETE(pFrameToFree);
	return S_OK;
}
//--------------------------------------------------------------------------------------
// Name: CAllocateHierarchy::DestroyMeshContainer()
// Desc: 
//--------------------------------------------------------------------------------------
HRESULT AllocateHierarchy::DestroyMeshContainer(LPD3DXMESHCONTAINER pMeshContainerBase)
{
	UINT iMaterial;
	D3DXMESHCONTAINER_DERIVED* pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;

	SAFE_DELETE_ARRAY(pMeshContainer->Name);
	SAFE_DELETE_ARRAY(pMeshContainer->pAdjacency);
	SAFE_DELETE_ARRAY(pMeshContainer->pMaterials);
	SAFE_DELETE_ARRAY(pMeshContainer->pBoneOffsetMatrices);

	// release all the allocated textures
	if (pMeshContainer->ppTextures != NULL)
	{
		for (iMaterial = 0; iMaterial < pMeshContainer->NumMaterials; iMaterial++)
		{
			SAFE_RELEASE(pMeshContainer->ppTextures[iMaterial]);
		}
	}

	SAFE_DELETE_ARRAY(pMeshContainer->ppTextures);
	SAFE_DELETE_ARRAY(pMeshContainer->ppBoneMatrixPtrs);
	SAFE_RELEASE(pMeshContainer->pBoneCombinationBuf);
	SAFE_RELEASE(pMeshContainer->MeshData.pMesh);
	SAFE_RELEASE(pMeshContainer->pSkinInfo);
	SAFE_RELEASE(pMeshContainer->pOriginMesh);
	SAFE_DELETE(pMeshContainer);
	return S_OK;
}
//--------------------------------------------------------------------------------------
// Called either by CreateMeshContainer when loading a skin mesh, or when 
// changing methods.  This function uses the pSkinInfo of the mesh 
// container to generate the desired drawable mesh and bone combination 
// table.
//--------------------------------------------------------------------------------------
HRESULT GenerateSkinnedMesh(IDirect3DDevice9* pd3dDevice, D3DXMESHCONTAINER_DERIVED* pMeshContainer)
{
	HRESULT hr = S_OK;
	D3DCAPS9 d3dCaps;
	pd3dDevice->GetDeviceCaps(&d3dCaps);

	if (pMeshContainer->pSkinInfo == NULL)
		return hr;


	SAFE_RELEASE(pMeshContainer->MeshData.pMesh);
	SAFE_RELEASE(pMeshContainer->pBoneCombinationBuf);

	// if non-indexed skinning mode selected, use ConvertToBlendedMesh to generate drawable mesh
	if (g_SkinningMethod == D3DNONINDEXED)
	{

		hr = pMeshContainer->pSkinInfo->ConvertToBlendedMesh
		(
			pMeshContainer->pOriginMesh,
			D3DXMESH_MANAGED | D3DXMESHOPT_VERTEXCACHE,
			pMeshContainer->pAdjacency,
			NULL, NULL, NULL,
			&pMeshContainer->NumInfl,
			&pMeshContainer->NumAttributeGroups,
			&pMeshContainer->pBoneCombinationBuf,
			&pMeshContainer->MeshData.pMesh
		);
		if (FAILED(hr))
			goto e_Exit;


		// If the device can only do 2 matrix blends, ConvertToBlendedMesh cannot approximate all meshes to it
		// Thus we split the mesh in two parts: The part that uses at most 2 matrices and the rest. The first is
		// drawn using the device's HW vertex processing and the rest is drawn using SW vertex processing.
		LPD3DXBONECOMBINATION rgBoneCombinations = reinterpret_cast<LPD3DXBONECOMBINATION>(
			pMeshContainer->pBoneCombinationBuf->GetBufferPointer());

		// look for any set of bone combinations that do not fit the caps
		for (pMeshContainer->iAttributeSW = 0; pMeshContainer->iAttributeSW < pMeshContainer->NumAttributeGroups;
			pMeshContainer->iAttributeSW++)
		{
			DWORD cInfl = 0;

			for (DWORD iInfl = 0; iInfl < pMeshContainer->NumInfl; iInfl++)
			{
				if (rgBoneCombinations[pMeshContainer->iAttributeSW].BoneId[iInfl] != UINT_MAX)
				{
					++cInfl;
				}
			}

			if (cInfl > d3dCaps.MaxVertexBlendMatrices)
			{
				break;
			}
		}

		// if there is both HW and SW, add the Software Processing flag
		if (pMeshContainer->iAttributeSW < pMeshContainer->NumAttributeGroups)
		{
			LPD3DXMESH pMeshTmp;

			hr = pMeshContainer->MeshData.pMesh->CloneMeshFVF(D3DXMESH_SOFTWAREPROCESSING |
				pMeshContainer->MeshData.pMesh->GetOptions(),
				pMeshContainer->MeshData.pMesh->GetFVF(),
				pd3dDevice, &pMeshTmp);
			if (FAILED(hr))
			{
				goto e_Exit;
			}

			pMeshContainer->MeshData.pMesh->Release();
			pMeshContainer->MeshData.pMesh = pMeshTmp;
			pMeshTmp = NULL;
		}
	}
	// if indexed skinning mode selected, use ConvertToIndexedsBlendedMesh to generate drawable mesh
	else if (g_SkinningMethod == D3DINDEXED)
	{
		DWORD NumMaxFaceInfl;
		DWORD Flags = D3DXMESHOPT_VERTEXCACHE;

		LPDIRECT3DINDEXBUFFER9 pIB;
		hr = pMeshContainer->pOriginMesh->GetIndexBuffer(&pIB);
		if (FAILED(hr))
			goto e_Exit;

		hr = pMeshContainer->pSkinInfo->GetMaxFaceInfluences(pIB,
			pMeshContainer->pOriginMesh->GetNumFaces(),
			&NumMaxFaceInfl);
		pIB->Release();
		if (FAILED(hr))
			goto e_Exit;

		// 12 entry palette guarantees that any triangle (4 independent influences per vertex of a tri)
		// can be handled
		NumMaxFaceInfl = min(NumMaxFaceInfl, 12);

		if (d3dCaps.MaxVertexBlendMatrixIndex + 1 < NumMaxFaceInfl)
		{
			// HW does not support indexed vertex blending. Use SW instead
			pMeshContainer->NumPaletteEntries = min(256, pMeshContainer->pSkinInfo->GetNumBones());
			pMeshContainer->UseSoftwareVP = true;
			Flags |= D3DXMESH_SYSTEMMEM;
		}
		else
		{
			// using hardware - determine palette size from caps and number of bones
			// If normals are present in the vertex data that needs to be blended for lighting, then 
			// the number of matrices is half the number specified by MaxVertexBlendMatrixIndex.
			pMeshContainer->NumPaletteEntries = min((d3dCaps.MaxVertexBlendMatrixIndex + 1) / 2,
				pMeshContainer->pSkinInfo->GetNumBones());
			pMeshContainer->UseSoftwareVP = false;
			Flags |= D3DXMESH_MANAGED;
		}

		hr = pMeshContainer->pSkinInfo->ConvertToIndexedBlendedMesh
		(
			pMeshContainer->pOriginMesh,
			Flags,
			pMeshContainer->NumPaletteEntries,
			pMeshContainer->pAdjacency,
			NULL, NULL, NULL,
			&pMeshContainer->NumInfl,
			&pMeshContainer->NumAttributeGroups,
			&pMeshContainer->pBoneCombinationBuf,
			&pMeshContainer->MeshData.pMesh);
		if (FAILED(hr))
			goto e_Exit;
	}
	// if vertex shader indexed skinning mode selected, use ConvertToIndexedsBlendedMesh to generate drawable mesh
	else if ((g_SkinningMethod == D3DINDEXEDVS) || (g_SkinningMethod == D3DINDEXEDHLSLVS))
	{
		// Get palette size
		// First 9 constants are used for other data.  Each 4x3 matrix takes up 3 constants.
		// (96 - 9) /3 i.e. Maximum constant count - used constants 
		UINT MaxMatrices = 26;
		pMeshContainer->NumPaletteEntries = min(MaxMatrices, pMeshContainer->pSkinInfo->GetNumBones());

		DWORD Flags = D3DXMESHOPT_VERTEXCACHE;
		if (d3dCaps.VertexShaderVersion >= D3DVS_VERSION(1, 1))
		{
			pMeshContainer->UseSoftwareVP = false;
			Flags |= D3DXMESH_MANAGED;
		}
		else
		{
			pMeshContainer->UseSoftwareVP = true;
			Flags |= D3DXMESH_SYSTEMMEM;
		}

		SAFE_RELEASE(pMeshContainer->MeshData.pMesh);

		hr = pMeshContainer->pSkinInfo->ConvertToIndexedBlendedMesh
		(
			pMeshContainer->pOriginMesh,
			Flags,
			pMeshContainer->NumPaletteEntries,
			pMeshContainer->pAdjacency,
			NULL, NULL, NULL,
			&pMeshContainer->NumInfl,
			&pMeshContainer->NumAttributeGroups,
			&pMeshContainer->pBoneCombinationBuf,
			&pMeshContainer->MeshData.pMesh);
		if (FAILED(hr))
			goto e_Exit;


		// FVF has to match our declarator. Vertex shaders are not as forgiving as FF pipeline
		DWORD NewFVF = (pMeshContainer->MeshData.pMesh->GetFVF() & D3DFVF_POSITION_MASK) | D3DFVF_NORMAL |
			D3DFVF_TEX1 | D3DFVF_LASTBETA_UBYTE4;
		if (NewFVF != pMeshContainer->MeshData.pMesh->GetFVF())
		{
			LPD3DXMESH pMesh;
			hr = pMeshContainer->MeshData.pMesh->CloneMeshFVF(pMeshContainer->MeshData.pMesh->GetOptions(), NewFVF,
				pd3dDevice, &pMesh);
			if (!FAILED(hr))
			{
				pMeshContainer->MeshData.pMesh->Release();
				pMeshContainer->MeshData.pMesh = pMesh;
				pMesh = NULL;
			}
		}

		D3DVERTEXELEMENT9 pDecl[MAX_FVF_DECL_SIZE];
		LPD3DVERTEXELEMENT9 pDeclCur;
		hr = pMeshContainer->MeshData.pMesh->GetDeclaration(pDecl);
		if (FAILED(hr))
			goto e_Exit;

		// the vertex shader is expecting to interpret the UBYTE4 as a D3DCOLOR, so update the type 
		//   NOTE: this cannot be done with CloneMesh, that would convert the UBYTE4 data to float and then to D3DCOLOR
		//          this is more of a "cast" operation
		pDeclCur = pDecl;
		while (pDeclCur->Stream != 0xff)
		{
			if ((pDeclCur->Usage == D3DDECLUSAGE_BLENDINDICES) && (pDeclCur->UsageIndex == 0))
				pDeclCur->Type = D3DDECLTYPE_D3DCOLOR;
			pDeclCur++;
		}

		hr = pMeshContainer->MeshData.pMesh->UpdateSemantics(pDecl);
		if (FAILED(hr))
			goto e_Exit;

		// allocate a buffer for bone matrices, but only if another mesh has not allocated one of the same size or larger
		if (g_NumBoneMatricesMax < pMeshContainer->pSkinInfo->GetNumBones())
		{
			g_NumBoneMatricesMax = pMeshContainer->pSkinInfo->GetNumBones();

			// Allocate space for blend matrices
			delete[] g_pBoneMatrices;
			g_pBoneMatrices = new D3DXMATRIXA16[g_NumBoneMatricesMax];
			if (g_pBoneMatrices == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto e_Exit;
			}
		}

	}
	// if software skinning selected, use GenerateSkinnedMesh to create a mesh that can be used with UpdateSkinnedMesh
	else if (g_SkinningMethod == SOFTWARE)
	{
		hr = pMeshContainer->pOriginMesh->CloneMeshFVF(D3DXMESH_MANAGED, pMeshContainer->pOriginMesh->GetFVF(),
			pd3dDevice, &pMeshContainer->MeshData.pMesh);
		if (FAILED(hr))
			goto e_Exit;

		hr = pMeshContainer->MeshData.pMesh->GetAttributeTable(NULL, &pMeshContainer->NumAttributeGroups);
		if (FAILED(hr))
			goto e_Exit;

		delete[] pMeshContainer->pAttributeTable;
		pMeshContainer->pAttributeTable = new D3DXATTRIBUTERANGE[pMeshContainer->NumAttributeGroups];
		if (pMeshContainer->pAttributeTable == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto e_Exit;
		}

		hr = pMeshContainer->MeshData.pMesh->GetAttributeTable(pMeshContainer->pAttributeTable, NULL);
		if (FAILED(hr))
			goto e_Exit;

		// allocate a buffer for bone matrices, but only if another mesh has not allocated one of the same size or larger
		if (g_NumBoneMatricesMax < pMeshContainer->pSkinInfo->GetNumBones())
		{
			g_NumBoneMatricesMax = pMeshContainer->pSkinInfo->GetNumBones();

			// Allocate space for blend matrices
			delete[] g_pBoneMatrices;
			g_pBoneMatrices = new D3DXMATRIXA16[g_NumBoneMatricesMax];
			if (g_pBoneMatrices == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto e_Exit;
			}
		}
	}
	else  // invalid g_SkinningMethod value
	{
		// return failure due to invalid skinning method value
		hr = E_INVALIDARG;
		goto e_Exit;
	}

e_Exit:
	return hr;
}









//Here is The Animator Class ,Its function is to Entitize a Animator Which stores An Animation Sequence
//And the flow  of this class is like this ->
//Render-> Update->Render and so on
//It is especially fouced that we should better put all our update funcitons in update ,and Rendering is Rendering.
//One last thing,you have to prevent yourself from using some uneccessary Matrix transform in rendering and update
class F_Animator
{
private:
	LPD3DXFRAME                 m_pFrameRoot;
	ID3DXAnimationController*   m_pAnimController;
	D3DXVECTOR3                 m_vObjectCenter;        // Center of bounding sphere of object
	FLOAT                       m_fObjectRadius;        // Radius of bounding sphere of object
	UINT                        m_NumBoneMatricesMax = 0;
	IDirect3DVertexShader9*     m_pIndexedVertexShader[4];
	D3DXVECTOR3 ForwardAngle;                 //The angle you will go To
	D3DXVECTOR3 FacingAngle;                   //The  angle you are facing
	D3DXMATRIXA16               m_matView;              // View matrix
	D3DXMATRIXA16               m_matProj;              // Projection matrix
	D3DXMATRIXA16               m_matProjT;             // Transpose of projection matrix (for asm shader)
	DWORD                       m_dwBehaviorFlags;      // Behavior flags of the 3D device
	D3DXMATRIXA16               m_World;                //world transformation
	D3DXMATRIXA16               m_rotate, m_rotateX, m_rotateY, m_rotateZ;               //angle tansformation 
	D3DXMATRIXA16               m_scale;                //scalling transformaiton  usually at default setting
	LPD3DXANIMATIONSET         ms_Jog;
	LPD3DXANIMATIONSET         ms_Walk;
	LPD3DXANIMATIONSET          ms_Loiter;
	D3DXMATRIXA16 m_outcomematrix;
	DWORD m_ixJog, m_ixWalk, m_ixLoiter;
	D3DXVECTOR3 Origin, Direct;
	bool Checkrunning, CheckResult;
public:
	F_Animator();
	~F_Animator();
	DWORD GetJogindex() { return m_ixJog; }
	DWORD GetLoiterindex() { return m_ixLoiter; }
	DWORD GetWalkindex() { return m_ixWalk; }

	void F_Animator_Init(LPDIRECT3DDEVICE9 device, LPCWSTR fileName); //init the aniator
	void Update(float fElapsedTime, IDirect3DDevice9* device, float pace, F_Terrain *map);
	void Render(IDirect3DDevice9* pd3dDevice);
	void DrawMeshContainer(IDirect3DDevice9* pd3dDevice, LPD3DXMESHCONTAINER pMeshContainerBase, LPD3DXFRAME pFrameBase);
	void DrawFrame(IDirect3DDevice9* pd3dDevice, LPD3DXFRAME pFrame);
	HRESULT SetupBoneMatrixPointersOnMesh(LPD3DXMESHCONTAINER pMeshContainer);
	HRESULT SetupBoneMatrixPointers(LPD3DXFRAME pFrame);
	void UpdateFrameMatrices(LPD3DXFRAME pFrameBase, LPD3DXMATRIX pParentMatrix);
	void UpdateSkinningMethod(LPD3DXFRAME pFrameBase, LPDIRECT3DDEVICE9 deice);
	void ReleaseAttributeTable(LPD3DXFRAME pFrameBase);

	BOOL TestOneShot(LPDIRECT3DDEVICE9 device, D3DXVECTOR3 origin, D3DXVECTOR3 direct)
	{
		this->setTwostate(true, false);
		this->Origin = origin;
		this->Direct = direct;
		DrawFrame(NULL, m_pFrameRoot);
		if (CheckResult)
		{
			setTwostate(false, false);
			return true;
		}
		else
		{
			setTwostate(false, false);
			return false;
		}
	}
	VOID SetAnimatorPostion(D3DVECTOR Position)
	{
		this->m_vObjectCenter = Position;
	}
	VOID SetAnimatorPostion(FLOAT x, FLOAT y, FLOAT z)
	{
		this->m_vObjectCenter.x = x;
		this->m_vObjectCenter.y = y;
		this->m_vObjectCenter.z = z;
	}
	D3DXVECTOR3 GetPosition()
	{
		return m_vObjectCenter;
	}
	void setTwostate(bool inputrunning, bool inputresult)
	{
		this->Checkrunning = inputrunning;
		this->CheckResult = inputresult;
	}
	D3DXVECTOR3 GetForwarAngle()
	{
		return ForwardAngle;
	}
	VOID SetForwardAngle(D3DXVECTOR3 angle)
	{
		this->ForwardAngle = angle;
	}
	D3DXVECTOR3 GetFacingAngle()
	{
		return this->FacingAngle;
	}
	VOID SetFacingAngle(D3DXVECTOR3 angle)
	{
		FacingAngle = angle;
	}

	void GetAnimationController(ID3DXAnimationController** ppAC)
	{
		m_pAnimController->AddRef();
		*ppAC = m_pAnimController;
	}
	DWORD GetAnimationSetIndex(char sString[]);
	void LoadAnimationSetFrom();
	void SetAnimSetByindex(DWORD index);


};

//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void F_Animator::Update(float fElapsedTime, IDirect3DDevice9* device, float pace, F_Terrain *map)
{
	IDirect3DDevice9* pd3dDevice = device;
	float h = map->GetHeightY(m_vObjectCenter.x, m_vObjectCenter.z);
	m_vObjectCenter.y = h;
	// Setup overall world matrix
	FacingAngle.y = 0;
	ForwardAngle.y = 0;
	D3DXVec3Normalize(&ForwardAngle, &ForwardAngle);
	D3DXVec3Normalize(&FacingAngle, &FacingAngle);
	m_vObjectCenter.z += pace * (ForwardAngle.z);
	m_vObjectCenter.x += pace * (ForwardAngle.x);

	D3DXMatrixTranslation(&m_World, m_vObjectCenter.x, h, (m_vObjectCenter.z));
	//set rotation matrix
	D3DXMatrixIdentity(&m_rotateY);
	m_rotateY._11 = FacingAngle.z;
	m_rotateY._13 = -FacingAngle.x;
	m_rotateY._31 = FacingAngle.x;
	m_rotateY._33 = FacingAngle.z;
	D3DXMatrixRotationX(&m_rotateX, -D3DX_PI / 2);  //this is a default operation to make sure the model is facing the Postive Z axis  
	m_rotate = m_rotateX * m_rotateY;
	D3DXMatrixScaling(&m_scale, 0.2, 0.2, 0.2);
	D3DXMATRIXA16 re = m_rotate * m_scale* m_World;
	if (m_pAnimController != NULL)
		m_pAnimController->AdvanceTime(fElapsedTime, NULL);
	UpdateFrameMatrices(m_pFrameRoot, &re);

	D3DXMatrixRotationX(&m_rotateX, D3DX_PI / 2);
	D3DXMATRIXA16 world2;
	D3DXMatrixTranslation(&world2, -FacingAngle.x * 15, 75, -FacingAngle.z * 15);
	D3DXMATRIXA16 scale;
	D3DXMatrixScaling(&scale, 1.28, 1.28, 1.28);
	m_outcomematrix = scale * m_rotateX * re*world2;
}
//--------------------------------------------------------------------------------------
// Called to render a mesh in the hierarchy
//--------------------------------------------------------------------------------------
void F_Animator::DrawMeshContainer(IDirect3DDevice9* pd3dDevice, LPD3DXMESHCONTAINER pMeshContainerBase, LPD3DXFRAME pFrameBase)
{
	HRESULT hr;
	D3DXMESHCONTAINER_DERIVED* pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;
	D3DXFRAME_DERIVED* pFrame = (D3DXFRAME_DERIVED*)pFrameBase;
	UINT iMaterial;
	UINT NumBlend;
	UINT iAttrib;
	DWORD AttribIdPrev;
	LPD3DXBONECOMBINATION pBoneComb;
	UINT iMatrixIndex;
	UINT iPaletteEntry;
	D3DXMATRIXA16 matTemp;
	D3DCAPS9 d3dCaps;
	if (!Checkrunning)
		pd3dDevice->GetDeviceCaps(&d3dCaps);

	// first check for skinning
	if (pMeshContainer->pSkinInfo != NULL)
	{
		if (g_SkinningMethod == D3DNONINDEXED)
		{

			if (!Checkrunning)
			{
				AttribIdPrev = UNUSED32;
				pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(pMeshContainer->pBoneCombinationBuf->GetBufferPointer
				());

				// Draw using default vtx processing of the device (typically HW)
				for (iAttrib = 0; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
				{
					NumBlend = 0;
					for (DWORD i = 0; i < pMeshContainer->NumInfl; ++i)
					{
						if (pBoneComb[iAttrib].BoneId[i] != UINT_MAX)
						{
							NumBlend = i;
						}
					}

					if (d3dCaps.MaxVertexBlendMatrices >= NumBlend + 1)
					{
						// first calculate the world matrices for the current set of blend weights and get the accurate count of the number of blends
						for (DWORD i = 0; i < pMeshContainer->NumInfl; ++i)
						{
							iMatrixIndex = pBoneComb[iAttrib].BoneId[i];
							if (iMatrixIndex != UINT_MAX)
							{
								D3DXMatrixMultiply(&matTemp, &pMeshContainer->pBoneOffsetMatrices[iMatrixIndex],
									pMeshContainer->ppBoneMatrixPtrs[iMatrixIndex]);
								V(pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(i), &matTemp));
							}
						}

						V(pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, NumBlend));

						// lookup the material used for this subset of faces
						if ((AttribIdPrev != pBoneComb[iAttrib].AttribId) || (AttribIdPrev == UNUSED32))
						{
							V(pd3dDevice->SetMaterial(&pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D)
							);
							V(pd3dDevice->SetTexture(0, pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId]));
							AttribIdPrev = pBoneComb[iAttrib].AttribId;
						}

						// draw the subset now that the correct material and matrices are loaded
						V(pMeshContainer->MeshData.pMesh->DrawSubset(iAttrib));


					}
				}

				// If necessary, draw parts that HW could not handle using SW
				if (pMeshContainer->iAttributeSW < pMeshContainer->NumAttributeGroups)
				{
					AttribIdPrev = UNUSED32;
					V(pd3dDevice->SetSoftwareVertexProcessing(TRUE));
					for (iAttrib = pMeshContainer->iAttributeSW; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
					{
						NumBlend = 0;
						for (DWORD i = 0; i < pMeshContainer->NumInfl; ++i)
						{
							if (pBoneComb[iAttrib].BoneId[i] != UINT_MAX)
							{
								NumBlend = i;
							}
						}

						if (d3dCaps.MaxVertexBlendMatrices < NumBlend + 1)
						{
							// first calculate the world matrices for the current set of blend weights and get the accurate count of the number of blends
							for (DWORD i = 0; i < pMeshContainer->NumInfl; ++i)
							{
								iMatrixIndex = pBoneComb[iAttrib].BoneId[i];
								if (iMatrixIndex != UINT_MAX)
								{
									D3DXMatrixMultiply(&matTemp, &pMeshContainer->pBoneOffsetMatrices[iMatrixIndex],
										pMeshContainer->ppBoneMatrixPtrs[iMatrixIndex]);
									V(pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(i), &matTemp));
								}
							}

							V(pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, NumBlend));

							// lookup the material used for this subset of faces
							if ((AttribIdPrev != pBoneComb[iAttrib].AttribId) || (AttribIdPrev == UNUSED32))
							{
								V(pd3dDevice->SetMaterial(&pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D
								));
								V(pd3dDevice->SetTexture(0, pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId]));
								AttribIdPrev = pBoneComb[iAttrib].AttribId;
							}

							// draw the subset now that the correct material and matrices are loaded
							V(pMeshContainer->MeshData.pMesh->DrawSubset(iAttrib));

						}
					}
					V(pd3dDevice->SetSoftwareVertexProcessing(FALSE));
				}
				V(pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, 0));


				for (iAttrib = 0; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
				{
					if ((AttribIdPrev != pBoneComb[iAttrib].AttribId) || (AttribIdPrev == UNUSED32))
					{
						V(pd3dDevice->SetMaterial(&pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D)
						);
						V(pd3dDevice->SetTexture(0, pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId]));
						AttribIdPrev = pBoneComb[iAttrib].AttribId;
					}


				}



			}
			if (Checkrunning && !CheckResult)//check running if we find no collision
			{
				D3DXVECTOR3 OriginRayPos = Origin;
				D3DXVECTOR3 RayDirection = Direct;
				D3DXMATRIX objworld = m_outcomematrix;
				D3DXMATRIX re;
				D3DXMatrixInverse(&re, 0, &objworld);
				D3DXVec3TransformCoord(&OriginRayPos, &OriginRayPos, &re);
				D3DXVec3TransformNormal(&RayDirection, &RayDirection, &re);
				D3DXVec3Normalize(&RayDirection, &RayDirection);
				BOOL bHit;
				DWORD dwFace;
				FLOAT fBary1, fBary2, fDist;

				D3DXIntersect(pMeshContainer->pOriginMesh, &OriginRayPos, &RayDirection, &bHit, &dwFace, &fBary1, &fBary2, &fDist,
					NULL, NULL);
				if (bHit)
				{

					CheckResult = true;
				}
				else
				{

				}
			}
			if (Checkrunning&&CheckResult)
			{
				;
			}
		}

		else if (g_SkinningMethod == D3DINDEXED)  //decrypde
		{

			// if hw doesn't support indexed vertex processing, switch to software vertex processing
			if (pMeshContainer->UseSoftwareVP)
			{
				// If hw or pure hw vertex processing is forced, we can't render the
				// mesh, so just exit out.  Typical applications should create
				// a device with appropriate vertex processing capability for this
				// skinning method.
				if (m_dwBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
					return;

				V(pd3dDevice->SetSoftwareVertexProcessing(TRUE));
			}

			// set the number of vertex blend indices to be blended
			if (pMeshContainer->NumInfl == 1)
			{
				V(pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_0WEIGHTS));
			}
			else
			{
				V(pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, pMeshContainer->NumInfl - 1));
			}

			if (pMeshContainer->NumInfl)
				V(pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE));

			// for each attribute group in the mesh, calculate the set of matrices in the palette and then draw the mesh subset
			pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(pMeshContainer->pBoneCombinationBuf->GetBufferPointer
			());
			for (iAttrib = 0; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
			{
				// first calculate all the world matrices
				for (iPaletteEntry = 0; iPaletteEntry < pMeshContainer->NumPaletteEntries; ++iPaletteEntry)
				{
					iMatrixIndex = pBoneComb[iAttrib].BoneId[iPaletteEntry];
					if (iMatrixIndex != UINT_MAX)
					{
						D3DXMatrixMultiply(&matTemp, &pMeshContainer->pBoneOffsetMatrices[iMatrixIndex],
							pMeshContainer->ppBoneMatrixPtrs[iMatrixIndex]);
						V(pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(iPaletteEntry), &matTemp));
					}
				}

				// setup the material of the mesh subset - REMEMBER to use the original pre-skinning attribute id to get the correct material id
				V(pd3dDevice->SetMaterial(&pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D));
				V(pd3dDevice->SetTexture(0, pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId]));

				// finally draw the subset with the current world matrix palette and material state
				V(pMeshContainer->MeshData.pMesh->DrawSubset(iAttrib));
			}

			// reset blending state
			V(pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE));
			V(pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, 0));

			// remember to reset back to hw vertex processing if software was required
			if (pMeshContainer->UseSoftwareVP)
			{
				V(pd3dDevice->SetSoftwareVertexProcessing(FALSE));
			}
		}
		else if (g_SkinningMethod == D3DINDEXEDVS)
		{
			// Use COLOR instead of UBYTE4 since Geforce3 does not support it
			// vConst.w should be 3, but due to COLOR/UBYTE4 issue, mul by 255 and add epsilon
			D3DXVECTOR4 vConst(1.0f, 0.0f, 0.0f, 765.01f);

			if (pMeshContainer->UseSoftwareVP)
			{
				// If hw or pure hw vertex processing is forced, we can't render the
				// mesh, so just exit out.  Typical applications should create
				// a device with appropriate vertex processing capability for this
				// skinning method.
				if (m_dwBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
					return;

				V(pd3dDevice->SetSoftwareVertexProcessing(TRUE));
			}

			V(pd3dDevice->SetVertexShader(m_pIndexedVertexShader[pMeshContainer->NumInfl - 1]));

			pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(pMeshContainer->pBoneCombinationBuf->GetBufferPointer
			());
			for (iAttrib = 0; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
			{
				// first calculate all the world matrices
				for (iPaletteEntry = 0; iPaletteEntry < pMeshContainer->NumPaletteEntries; ++iPaletteEntry)
				{
					iMatrixIndex = pBoneComb[iAttrib].BoneId[iPaletteEntry];
					if (iMatrixIndex != UINT_MAX)
					{
						D3DXMatrixMultiply(&matTemp, &pMeshContainer->pBoneOffsetMatrices[iMatrixIndex],
							pMeshContainer->ppBoneMatrixPtrs[iMatrixIndex]);
						D3DXMatrixMultiplyTranspose(&matTemp, &matTemp, &m_matView);
						V(pd3dDevice->SetVertexShaderConstantF(iPaletteEntry * 3 + 9, (float*)&matTemp, 3));
					}
				}

				// Sum of all ambient and emissive contribution
				D3DXCOLOR color1(pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D.Ambient);
				D3DXCOLOR color2(.25, .25, .25, 1.0);
				D3DXCOLOR ambEmm;
				D3DXColorModulate(&ambEmm, &color1, &color2);
				ambEmm += D3DXCOLOR(pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D.Emissive);

				// set material color properties 
				V(pd3dDevice->SetVertexShaderConstantF(8,
					(float*)&(
						pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D.Diffuse), 1));
				V(pd3dDevice->SetVertexShaderConstantF(7, (float*)&ambEmm, 1));
				vConst.y = pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D.Power;
				V(pd3dDevice->SetVertexShaderConstantF(0, (float*)&vConst, 1));

				V(pd3dDevice->SetTexture(0, pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId]));

				// finally draw the subset with the current world matrix palette and material state
				V(pMeshContainer->MeshData.pMesh->DrawSubset(iAttrib));
			}

			// remember to reset back to hw vertex processing if software was required
			if (pMeshContainer->UseSoftwareVP)
			{
				V(pd3dDevice->SetSoftwareVertexProcessing(FALSE));
			}
			V(pd3dDevice->SetVertexShader(NULL));
		}
		else if (g_SkinningMethod == SOFTWARE)
		{
			D3DXMATRIX Identity;
			DWORD cBones = pMeshContainer->pSkinInfo->GetNumBones();
			DWORD iBone;
			PBYTE pbVerticesSrc;
			PBYTE pbVerticesDest;

			// set up bone transforms
			for (iBone = 0; iBone < cBones; ++iBone)
			{
				D3DXMatrixMultiply
				(
					&g_pBoneMatrices[iBone],                 // output
					&pMeshContainer->pBoneOffsetMatrices[iBone],
					pMeshContainer->ppBoneMatrixPtrs[iBone]
				);
			}

			// set world transform
			D3DXMatrixIdentity(&Identity);
			V(pd3dDevice->SetTransform(D3DTS_WORLD, &Identity));

			V(pMeshContainer->pOriginMesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&pbVerticesSrc));
			V(pMeshContainer->MeshData.pMesh->LockVertexBuffer(0, (LPVOID*)&pbVerticesDest));

			// generate skinned mesh
			pMeshContainer->pSkinInfo->UpdateSkinnedMesh(g_pBoneMatrices, NULL, pbVerticesSrc, pbVerticesDest);

			V(pMeshContainer->pOriginMesh->UnlockVertexBuffer());
			V(pMeshContainer->MeshData.pMesh->UnlockVertexBuffer());

			for (iAttrib = 0; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
			{
				V(pd3dDevice->SetMaterial(&(
					pMeshContainer->pMaterials[pMeshContainer->pAttributeTable[iAttrib].AttribId].MatD3D)));
				V(pd3dDevice->SetTexture(0,
					pMeshContainer->ppTextures[pMeshContainer->pAttributeTable[iAttrib].AttribId]));
				V(pMeshContainer->MeshData.pMesh->DrawSubset(pMeshContainer->pAttributeTable[iAttrib].AttribId));
			}
		}
		else // bug out as unsupported mode
		{
			return;
		}
	}
	else  // standard mesh, just draw it after setting material properties
	{
		if (!Checkrunning)
		{
			V(pd3dDevice->SetTransform(D3DTS_WORLD, &pFrame->CombinedTransformationMatrix));

			for (iMaterial = 0; iMaterial < pMeshContainer->NumMaterials; iMaterial++)
			{
				V(pd3dDevice->SetMaterial(&pMeshContainer->pMaterials[iMaterial].MatD3D));
				V(pd3dDevice->SetTexture(0, pMeshContainer->ppTextures[iMaterial]));
				V(pMeshContainer->MeshData.pMesh->DrawSubset(iMaterial));
			}
		}
	}
}
//--------------------------------------------------------------------------------------
// Called to render a frame in the hierarchy
//--------------------------------------------------------------------------------------
void F_Animator::DrawFrame(IDirect3DDevice9* pd3dDevice, LPD3DXFRAME pFrame)
{
	LPD3DXMESHCONTAINER pMeshContainer;

	pMeshContainer = pFrame->pMeshContainer;
	while (pMeshContainer != NULL)
	{
		DrawMeshContainer(pd3dDevice, pMeshContainer, pFrame);

		pMeshContainer = pMeshContainer->pNextMeshContainer;
	}

	if (pFrame->pFrameSibling != NULL)
	{
		DrawFrame(pd3dDevice, pFrame->pFrameSibling);
	}

	if (pFrame->pFrameFirstChild != NULL)
	{
		DrawFrame(pd3dDevice, pFrame->pFrameFirstChild);
	}
}












//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, DXUT will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void  F_Animator::Render(IDirect3DDevice9* pd3dDevice)
{
	// Begin the scene
	DrawFrame(pd3dDevice, m_pFrameRoot);
}

//--------------------------------------------------------------------------------------
// Called to setup the pointers for a given bone to its transformation matrix
//--------------------------------------------------------------------------------------
HRESULT F_Animator::SetupBoneMatrixPointersOnMesh(LPD3DXMESHCONTAINER pMeshContainerBase)
{
	UINT iBone, cBones;
	D3DXFRAME_DERIVED* pFrame;

	D3DXMESHCONTAINER_DERIVED* pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;

	// if there is a skinmesh, then setup the bone matrices
	if (pMeshContainer->pSkinInfo != NULL)
	{
		cBones = pMeshContainer->pSkinInfo->GetNumBones();

		pMeshContainer->ppBoneMatrixPtrs = new D3DXMATRIX*[cBones];
		if (pMeshContainer->ppBoneMatrixPtrs == NULL)
			return E_OUTOFMEMORY;

		for (iBone = 0; iBone < cBones; iBone++)
		{
			pFrame = (D3DXFRAME_DERIVED*)D3DXFrameFind(m_pFrameRoot,
				pMeshContainer->pSkinInfo->GetBoneName(iBone));
			if (pFrame == NULL)
				return E_FAIL;

			pMeshContainer->ppBoneMatrixPtrs[iBone] = &pFrame->CombinedTransformationMatrix;
		}
	}

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Called to setup the pointers for a given bone to its transformation matrix
//--------------------------------------------------------------------------------------
HRESULT F_Animator::SetupBoneMatrixPointers(LPD3DXFRAME pFrame)
{
	HRESULT hr;

	if (pFrame->pMeshContainer != NULL)
	{
		hr = SetupBoneMatrixPointersOnMesh(pFrame->pMeshContainer);
		if (FAILED(hr))
			return hr;
	}

	if (pFrame->pFrameSibling != NULL)
	{
		hr = SetupBoneMatrixPointers(pFrame->pFrameSibling);
		if (FAILED(hr))
			return hr;
	}

	if (pFrame->pFrameFirstChild != NULL)
	{
		hr = SetupBoneMatrixPointers(pFrame->pFrameFirstChild);
		if (FAILED(hr))
			return hr;
	}

	return S_OK;
}




//--------------------------------------------------------------------------------------
// update the frame matrices
//--------------------------------------------------------------------------------------
void F_Animator::UpdateFrameMatrices(LPD3DXFRAME pFrameBase, LPD3DXMATRIX pParentMatrix)
{
	D3DXFRAME_DERIVED* pFrame = (D3DXFRAME_DERIVED*)pFrameBase;

	if (pParentMatrix != NULL)
		D3DXMatrixMultiply(&pFrame->CombinedTransformationMatrix, &pFrame->TransformationMatrix, pParentMatrix);
	else
		pFrame->CombinedTransformationMatrix = pFrame->TransformationMatrix;

	if (pFrame->pFrameSibling != NULL)
	{
		UpdateFrameMatrices(pFrame->pFrameSibling, pParentMatrix);
	}

	if (pFrame->pFrameFirstChild != NULL)
	{
		UpdateFrameMatrices(pFrame->pFrameFirstChild, &pFrame->CombinedTransformationMatrix);
	}
}


//--------------------------------------------------------------------------------------
// update the skinning method
//--------------------------------------------------------------------------------------
void F_Animator::UpdateSkinningMethod(LPD3DXFRAME pFrameBase, LPDIRECT3DDEVICE9 device)
{
	D3DXFRAME_DERIVED* pFrame = (D3DXFRAME_DERIVED*)pFrameBase;
	D3DXMESHCONTAINER_DERIVED* pMeshContainer;

	pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pFrame->pMeshContainer;

	while (pMeshContainer != NULL)
	{
		GenerateSkinnedMesh(device, pMeshContainer);

		pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainer->pNextMeshContainer;
	}

	if (pFrame->pFrameSibling != NULL)
	{
		UpdateSkinningMethod(pFrame->pFrameSibling, device);
	}

	if (pFrame->pFrameFirstChild != NULL)
	{
		UpdateSkinningMethod(pFrame->pFrameFirstChild, device);
	}
}
void F_Animator::ReleaseAttributeTable(LPD3DXFRAME pFrameBase)
{
	D3DXFRAME_DERIVED* pFrame = (D3DXFRAME_DERIVED*)pFrameBase;
	D3DXMESHCONTAINER_DERIVED* pMeshContainer;

	pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pFrame->pMeshContainer;

	while (pMeshContainer != NULL)
	{
		delete[] pMeshContainer->pAttributeTable;

		pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainer->pNextMeshContainer;
	}

	if (pFrame->pFrameSibling != NULL)
	{
		ReleaseAttributeTable(pFrame->pFrameSibling);
	}

	if (pFrame->pFrameFirstChild != NULL)
	{
		ReleaseAttributeTable(pFrame->pFrameFirstChild);
	}
}
F_Animator::F_Animator()
{
	m_pFrameRoot = NULL;
	m_pAnimController = NULL;
	m_NumBoneMatricesMax = 0;
	m_pIndexedVertexShader[4] = { NULL };
	ForwardAngle.x = 1;
	ForwardAngle.z = 1;
	FacingAngle.x = 1;
	FacingAngle.z = 1;
	m_NumBoneMatricesMax = 0;
	g_pBoneMatrices = NULL;
	Checkrunning = false;
	CheckResult = false;
}
F_Animator::~F_Animator()
{
	this->ReleaseAttributeTable(m_pFrameRoot);
}
void F_Animator::F_Animator_Init(LPDIRECT3DDEVICE9 device, LPCWSTR fileName)
{
	// Load the mesh
	AllocateHierarchy Alloc;
	D3DXLoadMeshHierarchyFromX(fileName, D3DXMESH_MANAGED, device,
		&Alloc, NULL, &m_pFrameRoot, &m_pAnimController);
	SetupBoneMatrixPointers(m_pFrameRoot);
	D3DXFrameCalculateBoundingSphere(m_pFrameRoot, &m_vObjectCenter, &m_fObjectRadius);
	this->LoadAnimationSetFrom();
	this->SetAnimSetByindex(m_ixLoiter);

}
DWORD F_Animator::GetAnimationSetIndex(char sString[])
{
	HRESULT hr;
	LPD3DXANIMATIONCONTROLLER pAC;
	this->GetAnimationController(&pAC);
	LPD3DXANIMATIONSET pAS;
	DWORD dwRet = ANIMINDEX_FAIL;
	for (DWORD i = 0; i < pAC->GetNumAnimationSets(); ++i)
	{
		hr = pAC->GetAnimationSet(i, &pAS);
		if (FAILED(hr))
			continue;

		if (pAS->GetName() &&
			!strncmp(pAS->GetName(), sString, min(strlen(pAS->GetName()), strlen(sString))))
		{
			dwRet = i;
			pAS->Release();
			break;
		}

		pAS->Release();
	}
	pAC->Release();
	return dwRet;
}
void F_Animator::SetAnimSetByindex(DWORD index)
{
	LPD3DXANIMATIONCONTROLLER pAC;
	this->GetAnimationController(&pAC);
	if (index == this->m_ixJog)
	{
		pAC->SetTrackAnimationSet(0, ms_Jog);
		pAC->SetTrackEnable(0, true);
	}
	else if (index == this->m_ixWalk)
	{
		pAC->SetTrackAnimationSet(0, ms_Walk);
		pAC->SetTrackEnable(0, true);


	}
	else if (index == this->m_ixLoiter)
	{
		pAC->SetTrackAnimationSet(0, ms_Loiter);
		pAC->SetTrackEnable(0, true);
	}
	else
	{
		MessageBox(NULL, L"oops animation set wrong!", L"you are in trouble now!!", 0);
	}
	pAC->Release();
}
void F_Animator::LoadAnimationSetFrom()
{
	LPD3DXANIMATIONCONTROLLER pAC;
	this->GetAnimationController(&pAC);
	m_ixLoiter = this->GetAnimationSetIndex("Loiter");
	m_ixJog = this->GetAnimationSetIndex("Jog");
	m_ixWalk = this->GetAnimationSetIndex("Walk");

	pAC->GetAnimationSet(m_ixLoiter, &ms_Loiter);
	pAC->GetAnimationSet(m_ixWalk, &ms_Walk);
	pAC->GetAnimationSet(m_ixJog, &ms_Jog);
	pAC->Release();
}
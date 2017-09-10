#include "SceneDelegate.h"

#include <fstream>

#include "pxr/imaging/hdSt/camera.h"
#include "pxr/imaging/cameraUtil/conformWindow.h"
#include "pxr/imaging/pxOsd/tokens.h"

#include "pxr/imaging/hdx/renderTask.h"

#include "pxr/base/gf/range3f.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/base/vt/array.h"


SceneDelegate::SceneDelegate(pxr::HdRenderIndex *parentIndex, pxr::SdfPath const &delegateID)
 : pxr::HdSceneDelegate(parentIndex, delegateID)
{
	cameraPath = pxr::SdfPath("/camera");
	GetRenderIndex().InsertSprim(pxr::HdPrimTypeTokens->camera, this, cameraPath);
	pxr::GfFrustum frustum;
	frustum.SetPosition(pxr::GfVec3d(0, 0, 3));
	SetCamera(frustum.ComputeViewMatrix(), frustum.ComputeProjectionMatrix());

	// single triangle
	GetRenderIndex().InsertRprim(pxr::HdPrimTypeTokens->mesh, this, pxr::SdfPath("/triangle") );
	// add a shader
	GetRenderIndex().InsertSprim(pxr::HdPrimTypeTokens->shader, this, pxr::SdfPath("/shader") );
}

void
SceneDelegate::AddRenderTask(pxr::SdfPath const &id)
{
	GetRenderIndex().InsertTask<pxr::HdxRenderTask>(this, id);
	_ValueCache &cache = _valueCacheMap[id];
	cache[pxr::HdTokens->children] = pxr::VtValue(pxr::SdfPathVector());
	cache[pxr::HdTokens->collection]
			= pxr::HdRprimCollection(pxr::HdTokens->geometry, pxr::HdTokens->smoothHull);
}

void
SceneDelegate::AddRenderSetupTask(pxr::SdfPath const &id)
{
	GetRenderIndex().InsertTask<pxr::HdxRenderSetupTask>(this, id);
	_ValueCache &cache = _valueCacheMap[id];
	pxr::HdxRenderTaskParams params;
	params.enableLighting = true;
	params.camera = cameraPath;
	params.viewport = pxr::GfVec4f(0, 0, 512, 512);
	cache[pxr::HdTokens->children] = pxr::VtValue(pxr::SdfPathVector());
	cache[pxr::HdTokens->params] = pxr::VtValue(params);
}

void SceneDelegate::SetCamera(pxr::GfMatrix4d const &viewMatrix, pxr::GfMatrix4d const &projMatrix)
{
	SetCamera(cameraPath, viewMatrix, projMatrix);
}

void SceneDelegate::SetCamera(pxr::SdfPath const &cameraId, pxr::GfMatrix4d const &viewMatrix, pxr::GfMatrix4d const &projMatrix)
{
	_ValueCache &cache = _valueCacheMap[cameraId];
	cache[pxr::HdStCameraTokens->windowPolicy] = pxr::VtValue(pxr::CameraUtilFit);
	cache[pxr::HdStCameraTokens->worldToViewMatrix] = pxr::VtValue(viewMatrix);
	cache[pxr::HdStCameraTokens->projectionMatrix] = pxr::VtValue(projMatrix);

	GetRenderIndex().GetChangeTracker().MarkSprimDirty(cameraId, pxr::HdStCamera::AllDirty);
}


pxr::VtValue SceneDelegate::Get(pxr::SdfPath const &id, const pxr::TfToken &key)
{
	std::cout << "[" << id.GetString() <<"][" << key << "]" << std::endl;
	_ValueCache *vcache = pxr::TfMapLookupPtr(_valueCacheMap, id);
	pxr::VtValue ret;
	if (vcache && pxr::TfMapLookup(*vcache, key, &ret)) {
		return ret;
	}

	if (id == pxr::SdfPath("/triangle") && key == pxr::HdShaderTokens->surfaceShader)
	{
		return pxr::VtValue(pxr::SdfPath("/shader"));
	}

	if (key == pxr::HdTokens->points)
	{
		pxr::VtVec3fArray points;

		points.push_back(pxr::GfVec3f(0,0,0));
		points.push_back(pxr::GfVec3f(1,0,0));
		points.push_back(pxr::GfVec3f(0,1,0));
		return pxr::VtValue(points);
	}

	if (key == pxr::HdTokens->color)
	{
		return pxr::VtValue(pxr::GfVec4f(0.5f, 0.5f, 0.5f, 1.0));
	}

	return pxr::VtValue();
}

bool SceneDelegate::GetVisible(pxr::SdfPath const &id)
{
	std::cout << "[" << id.GetString() <<"][Visible]" << std::endl;
	return true;
}

pxr::GfRange3d SceneDelegate::GetExtent(pxr::SdfPath const &id)
{
	std::cout << "[" << id.GetString() <<"][Extent]" << std::endl;
	return pxr::GfRange3d(pxr::GfVec3d(-1,-1,-1), pxr::GfVec3d(1,1,1));
}

pxr::GfMatrix4d SceneDelegate::GetTransform(pxr::SdfPath const &id)
{
	std::cout << "[" << id.GetString() <<"][Transform]" << std::endl;
	return pxr::GfMatrix4d(1.0f);
}

pxr::HdMeshTopology SceneDelegate::GetMeshTopology(pxr::SdfPath const &id)
{
	std::cout << "[" << id.GetString() <<"][Topology]" << std::endl;
	pxr::VtArray<int> vertCountsPerFace;
	pxr::VtArray<int> verts;
	vertCountsPerFace.push_back(3);
	verts.push_back(0);
	verts.push_back(1);
	verts.push_back(2);

	pxr::HdMeshTopology triangleTopology(pxr::PxOsdOpenSubdivTokens->none, pxr::HdTokens->rightHanded, vertCountsPerFace, verts);
	return triangleTopology;
}

pxr::TfTokenVector SceneDelegate::GetPrimVarVertexNames(pxr::SdfPath const &id)
{
	std::cout << "[" << id.GetString() <<"][PrimVarVertexNames]" << std::endl;

	pxr::TfTokenVector names;
	names.push_back(pxr::HdTokens->points);

	return names;
}

pxr::TfTokenVector SceneDelegate::GetPrimVarConstantNames(pxr::SdfPath const& id)
{
	pxr::TfTokenVector names;
	names.push_back(pxr::HdTokens->color);
	return names;
}

std::string SceneDelegate::GetSurfaceShaderSource(pxr::SdfPath const &shaderId)
{
	std::cout << "[" << shaderId.GetString() <<"][GetSurfaceShaderSource]" << std::endl;

	return "vec4 surfaceShader(vec4 Peye, vec3 Neye, vec4 color, vec4 patchCoord) { return vec4(0.2, 0.5, 0.2, 1) * color; }";
}

std::string SceneDelegate::GetDisplacementShaderSource(pxr::SdfPath const &shaderId)
{
	std::cout << "[" << shaderId.GetString() <<"][GetDisplacementShaderSource]" << std::endl;
	return "vec4 displacementShader(int  a, vec4 b, vec3 c, vec4 d) { return b; }";
}

pxr::VtValue SceneDelegate::GetSurfaceShaderParamValue(pxr::SdfPath const &shaderId, const pxr::TfToken &paramName)
{
	std::cout << "[" << shaderId.GetString() << "." << paramName <<"][GetSurfaceShaderParamValue]" << std::endl;
	return  pxr::VtValue();
}

pxr::HdShaderParamVector SceneDelegate::GetSurfaceShaderParams(pxr::SdfPath const &shaderId)
{
	std::cout << "[" << shaderId.GetString() <<"][GetSurfaceShaderParams]" << std::endl;
	pxr::HdShaderParamVector r;
	return r;
}

pxr::SdfPathVector SceneDelegate::GetSurfaceShaderTextures(pxr::SdfPath const &shaderId)
{
	std::cout << "[" << shaderId.GetString() <<"][GetSurfaceShaderTextures]" << std::endl;
	return pxr::SdfPathVector();
}

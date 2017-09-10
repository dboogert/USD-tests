#pragma once

#include "pxr/imaging/hd/sceneDelegate.h"

#include "pxr/base/gf/frustum.h"
#include "pxr/base/vt/array.h"

#include "tiny_obj_loader.h"

class SceneDelegate : public pxr::HdSceneDelegate
{
public:
	SceneDelegate(pxr::HdRenderIndex *parentIndex, pxr::SdfPath const& delegateID);
	
	void AddRenderTask(pxr::SdfPath const &id);
	void AddRenderSetupTask(pxr::SdfPath const &id);

	void SetCamera(pxr::GfMatrix4d const &viewMatrix, pxr::GfMatrix4d const &projMatrix);
	void SetCamera(pxr::SdfPath const &cameraId, pxr::GfMatrix4d const &viewMatrix, pxr::GfMatrix4d const &projMatrix);

	// interface Hydra uses query information from the SceneDelegate when processing an 
	// Prim in the RenderIndex
	pxr::VtValue Get(pxr::SdfPath const &id, const pxr::TfToken &key) override;
	bool GetVisible(pxr::SdfPath const &id) override;

	pxr::GfRange3d GetExtent(pxr::SdfPath const &id) override;
	pxr::GfMatrix4d GetTransform(pxr::SdfPath const &id) override;

	pxr::HdMeshTopology GetMeshTopology(pxr::SdfPath const &id) override;
	pxr::TfTokenVector GetPrimVarVertexNames(pxr::SdfPath const &id) override;

	pxr::TfTokenVector GetPrimVarVaryingNames(pxr::SdfPath const& id) override;
	pxr::TfTokenVector GetPrimVarFacevaryingNames(pxr::SdfPath const& id) override;
	pxr::TfTokenVector GetPrimVarUniformNames(pxr::SdfPath const& id) override;
	pxr::TfTokenVector GetPrimVarConstantNames(pxr::SdfPath const& id) override;

private:
	// per location (SdfPath) cache of dictionaries (TfToken -> VtValue) 
	typedef pxr::TfHashMap<pxr::TfToken, pxr::VtValue, pxr::TfToken::HashFunctor> _ValueCache;
	typedef pxr::TfHashMap<pxr::SdfPath, _ValueCache, pxr::SdfPath::Hash> _ValueCacheMap;
	_ValueCacheMap _valueCacheMap;

	tinyobj::attrib_t attribs;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	pxr::VtArray<int> vertCountsPerFace;
	pxr::VtArray<int> verts;

	pxr::VtVec3fArray normals;
	pxr::VtVec3fArray points;

	bool hasNormals;
	bool hasUVs;

	// location of Camera (setup in ctor)
	pxr::SdfPath cameraPath;
};
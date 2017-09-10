#pragma once

#include "pxr/imaging/hd/sceneDelegate.h"


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
	pxr::TfTokenVector GetPrimVarConstantNames(pxr::SdfPath const& id) override;

	std::string GetSurfaceShaderSource(pxr::SdfPath const &shaderId) override;
	std::string GetDisplacementShaderSource(pxr::SdfPath const &shaderId) override;
	pxr::VtValue GetSurfaceShaderParamValue(pxr::SdfPath const &shaderId, const pxr::TfToken &paramName) override;
	pxr::HdShaderParamVector GetSurfaceShaderParams(pxr::SdfPath const &shaderId) override;
	pxr::SdfPathVector GetSurfaceShaderTextures(pxr::SdfPath const &shaderId) override;

	pxr::HdTextureResource::ID GetTextureResourceID(pxr::SdfPath const &textureId) override;

	pxr::HdTextureResourceSharedPtr GetTextureResource(pxr::SdfPath const &textureId) override;

private:
	// per location (SdfPath) cache of dictionaries (TfToken -> VtValue) 
	typedef pxr::TfHashMap<pxr::TfToken, pxr::VtValue, pxr::TfToken::HashFunctor> _ValueCache;
	typedef pxr::TfHashMap<pxr::SdfPath, _ValueCache, pxr::SdfPath::Hash> _ValueCacheMap;
	_ValueCacheMap _valueCacheMap;

	// location of Camera (setup in ctor)
	pxr::SdfPath cameraPath;

	std::string shaderSource;
	pxr::GlfTextureHandleRefPtr textureHandle;
};
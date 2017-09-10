#include "SceneDelegate.h"

#include "pxr/imaging/hdSt/camera.h"
#include "pxr/imaging/cameraUtil/conformWindow.h"
#include "pxr/imaging/pxOsd/tokens.h"

#include "pxr/imaging/hdx/renderTask.h"

#include "pxr/base/gf/range3f.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/base/vt/array.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

SceneDelegate::SceneDelegate(pxr::HdRenderIndex *parentIndex, pxr::SdfPath const &delegateID)
 : pxr::HdSceneDelegate(parentIndex, delegateID)
{
	cameraPath = pxr::SdfPath("/camera");
	GetRenderIndex().InsertSprim(pxr::HdPrimTypeTokens->camera, this, cameraPath);
	pxr::GfFrustum frustum;
	frustum.SetPosition(pxr::GfVec3d(0, 1.5, 250.0));
	frustum.SetNearFar(pxr::GfRange1d(0.1, 1000.0));
	frustum.SetRotation(pxr::GfRotation(pxr::GfVec3d(1, 0, 0), 0));
	SetCamera(frustum.ComputeViewMatrix(), frustum.ComputeProjectionMatrix());

	// load the .obj file

	std::string error;

	bool result = tinyobj::LoadObj(&attribs, &shapes, &materials, &error, "teapot/teapot.obj");
	std::cout << result  << std::endl;

	std::cout << "num positions:" << attribs.vertices.size() / 3 << std::endl;

	pxr::SdfPath path ( std::string("/object") );
	GetRenderIndex().InsertRprim(pxr::HdPrimTypeTokens->mesh, this, path);

	hasNormals = attribs.normals.size() > 0;
	hasUVs = attribs.texcoords.size() > 0;


	for (size_t v = 0; v < attribs.vertices.size() / 3; ++v)
	{
		points.push_back( pxr::GfVec3f(attribs.vertices[v * 3 + 0], attribs.vertices[v * 3 + 1], attribs.vertices[v* 3 + 2]));
	}

	for(size_t s = 0; s < shapes.size(); ++s)
	{
		for (size_t i = 0; i < shapes[s].mesh.indices.size(); ++i)
		{
			verts.push_back( shapes[s].mesh.indices[i].vertex_index );

			if (hasNormals)
			{
				int normalIndex = shapes[s].mesh.indices[i].normal_index;
				pxr::GfVec3f n( attribs.normals[3 * normalIndex+0], attribs.normals[3 * normalIndex+1], attribs.normals[3 * normalIndex+2]);
				normals.push_back(n);
			}
		}

		for (size_t i = 0; i < shapes[s].mesh.num_face_vertices.size(); ++i )
		{
			vertCountsPerFace.push_back( shapes[s].mesh.num_face_vertices[i] );
		}
	}
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
	params.camera = cameraPath;
	params.enableLighting = true;
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

	if (key == pxr::HdShaderTokens->surfaceShader)
	{
		return pxr::VtValue();
	}

	if (key == pxr::HdTokens->points)
	{
		return pxr::VtValue(points);
	}

	if (key == pxr::HdTokens->normals)
	{
		return pxr::VtValue(normals);
	}

	if (key == pxr::HdTokens->color)
	{
		return pxr::VtValue(pxr::GfVec4f(0.2, 0.4, 0.2, 1.0));
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
	// todo return .obj BBox.
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
	// todo return correct obj topology
	std::cout << "[" << id.GetString() <<"][Topology]" << std::endl;

	std::cout << "\tnum polygons: " << verts.size() << std::endl;
	std::cout << "\tnum indices: " << vertCountsPerFace.size() << std::endl;

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

pxr::TfTokenVector SceneDelegate::GetPrimVarVaryingNames(pxr::SdfPath const& id)
{
	pxr::TfTokenVector names;
	return names;
}

pxr::TfTokenVector SceneDelegate::GetPrimVarFacevaryingNames(pxr::SdfPath const& id)
{
	pxr::TfTokenVector names;
	if (hasNormals)
	{
		names.push_back(pxr::HdTokens->normals);
	}

	if (hasUVs)
	{
		//names.push_back(pxr::HdTokens->surfaceShaderParams);
	}

	return names;
}

pxr::TfTokenVector SceneDelegate::GetPrimVarUniformNames(pxr::SdfPath const& id)
{
	pxr::TfTokenVector names;
	return names;
}

pxr::TfTokenVector SceneDelegate::GetPrimVarConstantNames(pxr::SdfPath const& id)
{
	pxr::TfTokenVector names;
	names.push_back(pxr::HdTokens->color);
	return names;
}

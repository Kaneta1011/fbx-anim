#ifndef R_MODELLOADER_HPP
#define R_MODELLOADER_HPP

#include "rTypes.hpp"
#include "rModel.hpp"

#include <fbxsdk.h>
#include <wx/wx.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/sstream.h>

class rModelLoader{
public:
	rModelLoader();
	~rModelLoader();
	bool Load(const rString& path, rModel* model);
	void SetLogPath(const wxString& logPath);

	void Clear();
private:

	void WalkScene(rModel* model);
	void WalkNode(FbxNode* node, rModel* model);
	
	void ProcessSkeletonRoot(FbxNode* node, rModel* model);
	void ProcessSkeletonNode(FbxNode* node, rModel* model, rBone* parentBone);
	void LogSkeleton(FbxNode* node);

	bool NodeAttributeIsType(FbxNode* node, FbxNodeAttribute::EType type);

	void Indent(size_t depth, wxTextOutputStream& logStream);
	wxString LogPathForObject(FbxObject* obj);

	FbxAMatrix GetGlobalDefaultPosition(FbxNode* pNode);

private:

	FbxManager* fbxManager;
	FbxScene* fbxScene;

	wxString m_logPath;
};

#endif
#include "rwxModelPreview.hpp"

BEGIN_EVENT_TABLE(rwxModelPreview, wxPanel)
	EVT_DATAVIEW_SELECTION_CHANGED(rwxModelPreview::rwxPREVIEW_SKELETON_TREE , rwxModelPreview::OnSkeletonTreeBoneSelect)
	EVT_DATAVIEW_ITEM_ACTIVATED(rwxModelPreview::rwxPREVIEW_SKELETON_TREE , rwxModelPreview::OnSkeletonTreeBoneActivated)

	EVT_BUTTON(rwxModelAnimationPanel::rwxANIM_PLAY_BUTTON, rwxModelPreview::OnAnimPlayButton)
	EVT_BUTTON(rwxModelAnimationPanel::rwxANIM_PAUSE_BUTTON, rwxModelPreview::OnAnimPauseButton)
	EVT_BUTTON(rwxModelAnimationPanel::rwxANIM_STOP_BUTTON, rwxModelPreview::OnAnimStopButton)
	EVT_CHOICE(rwxModelAnimationPanel::rwxANIM_ANIMATION_SELECTOR, rwxModelPreview::OnAnimSelect)

	EVT_TIMER(1000, rwxModelPreview::OnTick)
END_EVENT_TABLE()

rwxModelPreview::rwxModelPreview(rEngine* engine, wxWindow* parent, wxWindowID id)
	:wxPanel(parent, id)
{
	m_engine = engine;
	m_model = NULL;

	m_timer.SetOwner(this, 1000);
	m_isPlaying = false;

	m_boneInfo = new rwxBoneInfo(wxTheApp->GetTopWindow(), wxID_ANY);

	InitModelPreview();
	m_view->SetFocus();
}

void rwxModelPreview::InitModelPreview(){
	wxSplitterWindow* m_splitter = new wxSplitterWindow(this, wxID_ANY);

	m_skeletonTree = new wxDataViewTreeCtrl(m_splitter, rwxPREVIEW_SKELETON_TREE, wxDefaultPosition, wxDefaultSize, wxDV_MULTIPLE);
	m_skeletonTree->SetMinSize(wxSize(150, -1));
	m_skeletonTree->Connect(wxEVT_CHAR, wxKeyEventHandler(rwxModelPreview::OnSkeletonTreeKeyEvent), 0,this);
	
	wxPanel* rightPanel = new wxPanel(m_splitter, wxID_ANY);
	m_view = new rwxModelView(m_engine , rightPanel , wxID_ANY);
	m_animPanel = new rwxModelAnimationPanel(rightPanel, wxID_ANY);

	wxBoxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
		panelSizer->Add(m_view, 1, wxEXPAND);
		panelSizer->Add(m_animPanel, 0, wxEXPAND);
	rightPanel->SetSizer(panelSizer);
	
	m_splitter->SplitVertically(m_skeletonTree,rightPanel, 150);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
	mainSizer->Add(m_splitter , 1 , wxEXPAND);
	SetSizer(mainSizer);
}

void rwxModelPreview::SetModel(rModel* model){
	m_model = model;
	
	if (m_model)
		m_model->CalculateBoundingBox();

	m_boneInfo->ClearBoneAttrs();
	BuildSkeletonTree(m_model);
	m_view->SetModel(m_model);
	m_animPanel->SetModel(m_model);
}

void rwxModelPreview::BuildSkeletonTree(rModel* model){
	m_skeletonTree->DeleteAllItems();
	m_boneMap.clear();

	wxDataViewItem root(NULL);

	rSkeleton* skeleton = model->GetSkeleton();
	CreateSkeletonTreeNode(root , skeleton->GetRoot());
}

void rwxModelPreview::CreateSkeletonTreeNode(wxDataViewItem& parentItem, rBone* bone){
	wxDataViewItem item;

	if (bone){
		size_t childCount = bone->NumChildren();

		if (childCount == 0)
			item = m_skeletonTree->AppendItem(parentItem, bone->Name());
		else{
			item = m_skeletonTree->AppendContainer(parentItem, bone->Name());
			for (size_t i = 0; i < childCount; i++)
				CreateSkeletonTreeNode(item , bone->GetChild(i));
		}

		m_boneMap[item] = bone;
	}
}

void rwxModelPreview::OnSkeletonTreeBoneSelect(wxDataViewEvent& event){
	rBone* bone = m_boneMap[event.GetItem()];

	m_view->SetSelectedBone(bone);
	m_boneInfo->SetBone(bone);
}

void rwxModelPreview::OnAnimPlayButton(wxCommandEvent& event){
	wxLongLong milis = wxGetLocalTimeMillis();

	if (!m_isPlaying){
		m_timer.Start(1000 / 30);
		m_gameTime.Start(milis.GetLo());
		m_isPlaying = true;
	}

	if (m_model){
		wxString selectedAnim = m_animPanel->GetSelectedAnimation();
		if (selectedAnim != wxEmptyString){
			wxLogMessage("Play Animation: " + selectedAnim);
			m_model->PlayAnimation(selectedAnim);
		}

	}
}

void rwxModelPreview::OnAnimPauseButton(wxCommandEvent& event){
	m_isPlaying = false;

	if (m_model && m_model->CurrentAnimation())
		wxLogMessage("Pause Animation: " +  m_model->CurrentAnimation()->Name());
}

void rwxModelPreview::OnAnimSelect(wxCommandEvent& event){
	wxString animName =  event.GetString();
	wxLogMessage("Select Animation: " + animName);

	if (m_model)
		m_model->PlayAnimation(animName);
}

void rwxModelPreview::OnAnimStopButton(wxCommandEvent& event){
	wxLogMessage("Stop Animation");
	m_isPlaying = false;

	if (m_model)
		m_model->StopAnimation();

	m_timer.Stop();
}

void rwxModelPreview::OnTick(wxTimerEvent& event){
	wxLongLong milis = wxGetLocalTimeMillis();
	m_gameTime.Update(milis.GetLo());
	
	if (m_isPlaying){
		m_model->Update(m_gameTime);
		m_view->Refresh();
	}
}

void rwxModelPreview::OnSkeletonTreeKeyEvent(wxKeyEvent& event){
	int code = event.GetKeyCode();
	
	if (code == WXK_DELETE || code == WXK_BACK){
		bool deleted = false;
		wxDataViewItemArray selections;
		m_skeletonTree->GetSelections(selections);
		for (size_t i =0; i < selections.size(); i++){
			wxDataViewItem selection = selections[i];

			if (m_skeletonTree->IsContainer(selection))
				continue;

			wxString boneName = m_skeletonTree->GetItemText(selection);

			m_boneMap.erase(selection);
			m_skeletonTree->DeleteItem(selection);
			m_model->GetSkeleton()->DeleteBone(boneName);
			deleted = true;
		}

		if (deleted){
			m_view->Refresh();
			m_boneInfo->ClearBoneAttrs();
		}
	}
	event.Skip();
}

void rwxModelPreview::OnSkeletonTreeBoneActivated(wxDataViewEvent& event){
	m_boneInfo->Show(true);
}
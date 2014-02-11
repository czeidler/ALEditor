/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "editor/ALMEditor.h"

#include <Autolock.h>
#include <Window.h>

#include <ALMLayout.h>

#include <CustomizableNodeFactory.h>

#include "CustomizableNodeView.h"
#include "EditorWindow.h"
#include "LayoutArchive.h"
#include "LayoutEditView.h"
#include "OverlapManager.h"


class LayerWindow : public BWindow
{
public:
	LayerWindow(BALMEditor* editor, BALM::LayoutEditView* editView)
		:
		BWindow(BRect(350, 50, 1200, 700), "Layers", B_TITLED_WINDOW, 0),

		fEditor(editor)
	{
		BALMLayout* layout = new BALMLayout(10.);
		SetLayout(layout);
		
		fNodeView = new CustomizableNodeView(
			CustomizableRoster::DefaultRoster());
		layout->AddView(fNodeView, layout->Left(), layout->Top(),
			layout->Right(), layout->Bottom());
		
		editView->StartWatching(fNodeView, kCustomizableSelected);
		fNodeView->StartWatching(editView, kCustomizableSelected);
	}

	bool QuitRequested()
	{
		BMessage layout;
		fNodeView->StoreLayout(&layout);
		fEditor->SetLayerLayout(layout);
		fEditor->StopEdit();
		return true;
	}

	CustomizableNodeView* GetLayerView()
	{
		return fNodeView;
	}

private:
			BALMEditor*			fEditor;
			CustomizableNodeView* fNodeView;
};


BALMEditor::trash_item::trash_item(Customizable* customizable)
{
	raw_pointer = customizable;
	trash = customizable;
}


BALMEditor::BALMEditor(BALMLayout* layout)
	:
	fLayout(layout),
	fEditView(NULL),
	fLayerWindow(NULL),
	fEnableLayerWindow(false),
	fCreationMode(false),
	fShowXTabs(false),
	fShowYTabs(false),
	fFreePlacement(false)
{
	fOverlapManager = new BALM::OverlapManager(layout);
}


BALMEditor::~BALMEditor()
{
	StopEdit();
}


void
BALMEditor::StartEdit()
{
	BAutolock _(fLock);

	fEditView = new LayoutEditView(this);

	fLayout->AddView(fEditView, fLayout->Left(), fLayout->Top(),
		fLayout->Right(), fLayout->Bottom());
	Area* area = fLayout->AreaFor(fEditView);
	area->SetLeftInset(0);
	area->SetTopInset(0);
	area->SetRightInset(0);
	area->SetBottomInset(0);

	fEditWindow = new EditWindow(this, fEditView);
	fEditWindow->Show();
	fEditWindowMessenger = BMessenger(NULL, fEditWindow);

 	if (fEnableLayerWindow) {
		LayerWindow* layerWindow = new LayerWindow(this, fEditView);
		fLayerWindow = layerWindow;
		layerWindow->GetLayerView()->RestoreLayout(&fLayerLayout);
		fLayerWindowMessenger = BMessenger(NULL, fLayerWindow);
		fLayerWindow->Show();
 	}
}


void
BALMEditor::StopEdit()
{
	BAutolock _(fLock);
	BMessenger(fEditView).SendMessage(LayoutEditView::kQuitMsg);

	fEditWindowMessenger.SendMessage(B_QUIT_REQUESTED);
	fLayerWindowMessenger.SendMessage(B_QUIT_REQUESTED);
}


/*! removes the LayoutEditView and add it later. */
class LayoutEditViewDisabler {
public:
	LayoutEditViewDisabler(BALMLayout* layout, BALM::LayoutEditView* editView)
		:
		fLayout(layout),
		fEditView(editView)
	{
		fEditView->RemoveSelf();
	}

	~LayoutEditViewDisabler()
	{
		fLayout->AddView(fEditView, fLayout->Left(), fLayout->Top(),
		fLayout->Right(), fLayout->Bottom());
		Area* area = fLayout->AreaFor(fEditView);
		area->SetLeftInset(0);
		area->SetTopInset(0);
		area->SetRightInset(0);
		area->SetBottomInset(0);
	}

private:
			BALMLayout*			fLayout;
			BALM::LayoutEditView*	fEditView;
};


void
BALMEditor::ClearLayout()
{
	LayoutEditViewDisabler _(fLayout, fEditView);
	LayoutArchive archiver(fLayout);
	return archiver.ClearLayout();	
}


status_t
BALMEditor::SaveToFile(BFile* file, const BMessage* message)
{
	LayoutEditViewDisabler _(fLayout, fEditView);
	LayoutArchive archiver(fLayout);
	return archiver.SaveToFile(file, message);	
}


status_t
BALMEditor::RestoreFromFile(BFile* file, bool restoreComponents)
{
	LayoutEditViewDisabler _(fLayout, fEditView);
	LayoutArchive archiver(fLayout);
	return archiver.RestoreFromFile(file, restoreComponents);	
}


status_t
BALMEditor::SaveToAppFile(const char* attribute, const BMessage* message)
{
	LayoutEditViewDisabler _(fLayout, fEditView);
	LayoutArchive archiver(fLayout);
	return archiver.SaveToAppFile(attribute, message);	
}


status_t
BALMEditor::RestoreFromAppFile(const char* attribute, bool restoreComponents)
{
	LayoutEditViewDisabler _(fLayout, fEditView);
	LayoutArchive archiver(fLayout);
	return archiver.RestoreFromAppFile(attribute, restoreComponents);	
}


status_t
BALMEditor::SaveToAttribute(BNode* node, const char* attribute,
	const BMessage* message)
{
	LayoutEditViewDisabler _(fLayout, fEditView);
	LayoutArchive archiver(fLayout);
	return archiver.SaveToAttribute(node, attribute, message);	
}


status_t
BALMEditor::RestoreFromAttribute(BNode* node, const char* attribute,
	bool restoreComponents)
{
	LayoutEditViewDisabler _(fLayout, fEditView);
	LayoutArchive archiver(fLayout);
	return archiver.RestoreFromAttribute(node, attribute, restoreComponents);
}


BALMLayout*
BALMEditor::Layout()
{
	return fLayout;
}


void
BALMEditor::UpdateEditWindow()
{
	if (fEditWindow == NULL)
		return;
	fEditWindow->UpdateEditWindow();
}


void
BALMEditor::SetLayerLayout(const BMessage& archive)
{
	BAutolock _(fLock);
	fLayerLayout = archive;
}


void
BALMEditor::SetShowXTabs(bool show)
{
	BAutolock _(fLock);
	fShowXTabs = show;

	BMessage message(LayoutEditView::kShowXTabs);
	message.AddBool("show", show);
	BMessenger(fEditView).SendMessage(&message);
}


void
BALMEditor::SetShowYTabs(bool show)
{
	BAutolock _(fLock);
	fShowYTabs = show;

	BMessage message(LayoutEditView::kShowYTabs);
	message.AddBool("show", show);
	BMessenger(fEditView).SendMessage(&message);
}


void
BALMEditor::SetFreePlacement(bool freePlacement)
{
	fFreePlacement = freePlacement;
}


bool
BALMEditor::FreePlacement()
{
	return fFreePlacement;
}


bool
BALMEditor::ShowXTabs()
{
	return fShowXTabs;
}


bool
BALMEditor::ShowYTabs()
{
	return fShowYTabs;
}


void
BALMEditor::SetEnableLayerWindow(bool enabled)
{
	fEnableLayerWindow = enabled;
}


void
BALMEditor::SetEnableCreationMode(bool enabled)
{
	fCreationMode = enabled;
}


bool
BALMEditor::IsCreationMode()
{
	return fCreationMode;
}


bool
BALMEditor::Trash(Customizable* customizable)
{
	BAutolock _(fLock);
	trash_item* item = new trash_item(customizable);
	if (item == NULL)
		return false;
	fTrash.AddItem(item);
	fTrashWatcher.SendMessage(kTrashUpdated);
	return true;
}


BReference<Customizable>
BALMEditor::UnTrash(Customizable* customizable)
{
	BAutolock _(fLock);
	int32 count = fTrash.CountItems();
	for (int32 i = 0; i < count; i++) {
		trash_item* item = fTrash.ItemAt(i);
		BReference<Customizable> ref = item->trash;
		if (ref.Get() == customizable) {
			fTrash.RemoveItemAt(i);	
			delete item;
			fTrashWatcher.SendMessage(kTrashUpdated);
			return ref;
		}
		ref = item->weak_trash.GetReference();
		if (ref.Get() == NULL)
			continue;
		fTrash.RemoveItemAt(i);	
		delete item;
		fTrashWatcher.SendMessage(kTrashUpdated);
		return ref;
	}
	return NULL;
}


bool
BALMEditor::DeleteFromTrash(Customizable* customizable)
{
	BAutolock _(fLock);
	int32 count = fTrash.CountItems();
	for (int32 i = 0; i < count; i++) {
		trash_item* item = fTrash.ItemAt(i);
		if (item->trash.Get() == customizable) {
			item->weak_trash = item->trash;
			item->trash = NULL;
			if (!item->weak_trash.IsAlive()) {
				fTrash.RemoveItemAt(i);
				delete item;
				fTrashWatcher.SendMessage(kTrashUpdated);
				return true;
			}
			fTrashWatcher.SendMessage(kTrashUpdated);
			return true;
		}
		BReference<Customizable> ref = item->weak_trash.GetReference();
		if (ref.Get() != customizable)
			continue;
		fTrash.RemoveItemAt(i);
		delete item;
		fTrashWatcher.SendMessage(kTrashUpdated);
		return true;
	}
	return false;
}


void
BALMEditor::GetTrash(BArray<BWeakReference<Customizable> >& list)
{
	BAutolock _(fLock);
	int32 count = fTrash.CountItems();
	for (int32 i = 0; i < count; i++) {
		trash_item* item = fTrash.ItemAt(i);
		BWeakReference<Customizable> ref;
		if (item->trash.Get() == NULL)
			ref = item->weak_trash;
		else
			ref = item->trash;

		list.AddItem(ref);
	}
}

			
void
BALMEditor::SetTrashWatcher(BMessenger target)
{
	BAutolock _(fLock);
	fTrashWatcher = target;
}


BALM::OverlapManager&
BALMEditor::GetOverlapManager()
{
	return *fOverlapManager;
}


BString
BALMEditor::ProposeIdentifier(IViewContainer* container)
{
	BString objectName = container->ObjectName();
	BString identifier;

	// TODO: make that more efficient
	int32 id = 0;
	while (true) {
		id++;
		identifier = objectName;
		identifier << id;

		bool identifierTaken = false;
		for (int32 i = 0; i < fLayout->CountAreas(); i++) {
			Area* area = fLayout->AreaAt(i);

			BView* view = area->Item()->View();
			CustomizableView* customizable = dynamic_cast<CustomizableView*>(view);
			if (customizable == NULL) {
				BLayoutItem* item = area->Item();
				customizable = dynamic_cast<CustomizableView*>(item);
				if (customizable == NULL)
					continue;
			}
	
			if (identifier == customizable->Identifier()) {
				identifierTaken = true;
				break;
			}
		}
		if (!identifierTaken)
			break;
	}
	return identifier;
}

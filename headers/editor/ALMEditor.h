/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	ALM_EDITOR_H
#define	ALM_EDITOR_H


#include <Locker.h>
#include <Messenger.h>
#include <Window.h>

#include <ArrayContainer.h>
#include <Customizable.h>


const int32 kMsgLayoutEdited = '&LEd';


class BFile;
class BNode;

class IViewContainer;


namespace BALM {


class BALMLayout;
class EditWindow;
class LayoutEditView;
class OverlapManager;


enum tash_messages {
	kTrashUpdated = '&tra',
	kUnTrashComponent
};


class BALMEditor {
public:
								BALMEditor(BALMLayout* layout);
								~BALMEditor();

			void				StartEdit();
			void				StopEdit();

			void				ClearLayout();
			status_t			SaveToFile(BFile* file,
									const BMessage* message);
			status_t			RestoreFromFile(BFile* file,
									bool restoreComponents = true);

			status_t			SaveToAppFile(const char* attribute,
									const BMessage* message);
			status_t			RestoreFromAppFile(const char* attribute,
									bool restoreComponents = true);

			status_t			SaveToAttribute(BNode* node,
									const char* attribute,
									const BMessage* message);
			status_t			RestoreFromAttribute(BNode* node,
									const char* attribute,
									bool restoreComponents = true);

			BALMLayout*			Layout();
			void				UpdateEditWindow();

			void				SetLayerLayout(const BMessage& archive);

			void				SetShowXTabs(bool show);
			void				SetShowYTabs(bool show);
			bool				ShowXTabs();
			bool				ShowYTabs();
			void				SetEnableLayerWindow(bool enabled);
			void				SetEnableCreationMode(bool enabled);
			bool				IsCreationMode();

			void				SetFreePlacement(bool freePlacement);
			bool				FreePlacement();

			bool				Trash(Customizable* customizable);
			BReference<Customizable>	UnTrash(Customizable* customizable);
			bool				DeleteFromTrash(Customizable* customizable);

			void				GetTrash(
									BArray<BWeakReference<Customizable> >& l);
			
			void				SetTrashWatcher(BMessenger target);

			OverlapManager&		GetOverlapManager();

			BString				ProposeIdentifier(IViewContainer* container);
private:
	class trash_item {
	public:
								trash_item(Customizable* customizable);
		Customizable*					raw_pointer;
		BReference<Customizable>		trash;
		BWeakReference<Customizable>	weak_trash;		
	};

			BALMLayout*			fLayout;
			
			LayoutEditView*		fEditView;
			EditWindow*			fEditWindow;
			BMessenger			fEditWindowMessenger;

			BWindow*			fLayerWindow;
			BMessage			fLayerLayout;
			BMessenger			fLayerWindowMessenger;
			bool				fEnableLayerWindow;
			bool				fCreationMode;

			BLocker				fLock;

			bool				fShowXTabs;
			bool				fShowYTabs;
			bool				fFreePlacement;

			BObjectList<trash_item>	fTrash;
			BMessenger			fTrashWatcher;

			OverlapManager*		fOverlapManager;
};


}	// namespace BALM


using BALM::BALMEditor;


#endif // ALM_EDITOR_H

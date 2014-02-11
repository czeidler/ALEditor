/*
 * Copyright 2007-2008, Christof Lutteroth, lutteroth@cs.auckland.ac.nz
 * Copyright 2007-2008, James Kim, jkim202@ec.auckland.ac.nz
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	EDIT_FORM_H
#define	EDIT_FORM_H


#include <Application.h>
#include <CheckBox.h>
#include <File.h>
#include <FilePanel.h>
#include <List.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <ObjectList.h>
#include <StringView.h>
#include <SupportDefs.h>
#include <TabView.h>
#include <TextControl.h>
#include <View.h>
#include <Window.h>

#include <Tab.h>


class FactoryDragListView;


namespace BALM {

class Area;
class LayoutEditView;
class BALMLayout;
class BALMEditor;

typedef BObjectList<XTab> XTabList;
typedef BObjectList<YTab> YTabList;


/**
 * Edit form window when in edit mode.
 */
class EditWindow : public BWindow {

public:
	enum {
	kMsgShowXTabBox,
	kMsgShowYTabBox,
	kMsgFreePlacementBox,
	kMsgLoadDialog,
	kMsgSaveDialog,
	kMsgClearLayout,
	kMsgLoadLayout,
	kMsgSaveLayout,
	};
	
public:
								EditWindow(BALMEditor* editor,
									LayoutEditView* editView);
								~EditWindow();

			void				UpdateEditWindow();

protected:
			void				MessageReceived(BMessage* message);
			bool				QuitRequested();

private:
			void				_InitializeComponent();
			BView*				_CreateAreaPropertyView(Area* area);
			BView*				_CreateNoItemSelectedView();

			void				_SetTabAreaContent(BView* view);

private:
			BALMEditor*			fEditor;
			LayoutEditView*		fEditView;
			BALMLayout* 		fALMEngine;
			BList				fInvisibleControls;

			// Keep lists of XTabs and YTabs to refer to index later
			XTabList			fXTabs;
			YTabList			fYTabs;

			BFilePanel*			fOpenPanel;
			BFilePanel*			fSavePanel;

			BMenuBar*			fMainMenu;

			BView*				fAreaView;

			FactoryDragListView*	fFactoryDragListView;
			BTabView*			fEditWindowTabContent;
			BTab*				fAreaPage;
			BTab*				fConstraintPage;
			BTab*				fComponentsPage;

			BCheckBox*			fShowXTabBox;
			BCheckBox*			fShowYTabBox;

			BCheckBox*			fFreePlacementBox;
};

}	// namespace BALM

using BALM::EditWindow;

#endif	// EDIT_FORM_H

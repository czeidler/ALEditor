/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "CustomizableRosterView.h"

#include <ListItem.h>
#include <SpaceLayoutItem.h>
#include <StringView.h>
#include <Window.h>

#include "ALMLayout.h"
#include "ALMLayoutBuilder.h"

#include "CustomizableNodeFactory.h"


InstalledCustomizableView::InstalledCustomizableView()
	:
	BView("InstalledCustomizableView", B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	BALMLayout* layout = new BALMLayout(5, 5);
	layout->SetInsets(5);
	SetLayout(layout);

	fInstalledComponentsListView = new BListView;
	fInstalledComponentsListView->SetExplicitPreferredSize(BSize(-1, -1));
	fCreateComponent = new BButton("Create", "Create",
		new BMessage(kMsgCreateComponent));
	fCreateComponent->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT,
		B_ALIGN_VERTICAL_CENTER));

	BALMLayoutBuilder(layout)
		.Add(fInstalledComponentsListView, layout->Left(), layout->Top(),
			layout->Right())
		.StartingAt(fInstalledComponentsListView)
			.AddBelow(fCreateComponent, layout->Bottom());

	CustomizableRoster* roster = CustomizableRoster::DefaultRoster();
	CustomizableAddOnList installedComponents;
	roster->GetInstalledCustomizableList(installedComponents);
	for (int32 i = 0; i < installedComponents.CountItems(); i++) {
		BString component = installedComponents.ItemAt(i)->Name();
		fInstalledComponents.push_back(component);
		fInstalledComponentsListView->AddItem(new BStringItem(component));
	}
}


void
InstalledCustomizableView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMsgCreateComponent:
		{
			int32 index = fInstalledComponentsListView->CurrentSelection();
			if (index < 0 || (unsigned int)index >= fInstalledComponents.size())
				break;
			BString& component = fInstalledComponents[index];
			CustomizableRoster* roster = CustomizableRoster::DefaultRoster();
			roster->InstantiateCustomizable(component);
			Looper()->PostMessage(B_QUIT_REQUESTED);
			break;
		}

		default:
			BView::MessageReceived(message);
	}
}


void
InstalledCustomizableView::AttachedToWindow()
{
	fCreateComponent->SetTarget(this);
}


const uint32 kMsgComponentSelected = '&cse';
const uint32 kMsgAddComponent = '&aco';
const uint32 kMsgRemoveComponent = '&rco';


CustomizableRosterView::CustomizableRosterView()
	:
	BView("CustomizableRosterView", B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE)
{
	fRoster = CustomizableRoster::DefaultRoster();

	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	fCustomizableListView = new BListView;
	fRoster->StartWatching(this);
	_LoadCustomizableList();
	fCustomizableListView->SetExplicitPreferredSize(BSize(B_SIZE_UNLIMITED,
		B_SIZE_UNLIMITED));
	fCustomizableListView->SetSelectionMessage(
		new BMessage(kMsgComponentSelected));

	fAddComponent = new BButton("Add", "Add",
		new BMessage(kMsgAddComponent));
	fRemoveComponent = new BButton("Remove", "Remove",
		new BMessage(kMsgRemoveComponent));

	BStringView* label = new BStringView("Components", "List of Components:");
	BALMLayout* layout = new BALMLayout(5, 5);
	BALMLayoutBuilder(this, layout)
		.SetInsets(5)
		.Add(label, layout->Left(), layout->Top())
		.StartingAt(label)
			.Push()
				.AddToRight(BSpaceLayoutItem::CreateGlue())
				.AddToRight(fAddComponent)
				.AddToRight(fRemoveComponent, layout->Right())
			.Pop()
			.AddBelow(fCustomizableListView, layout->Bottom(),
				layout->Left() ,layout->Right());
}


CustomizableRosterView::~CustomizableRosterView()
{
	fRoster->StopWatching(this);
}


void
CustomizableRosterView::AttachedToWindow()
{
	fCustomizableListView->SetTarget(this);
	fAddComponent->SetTarget(this);
	fRemoveComponent->SetTarget(this);
}


void
CustomizableRosterView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_CUSTOMIZABLE_ADDED:
		case B_CUSTOMIZABLE_REMOVED:
		case B_SOCKET_CONNECTED:
		case B_SOCKET_DISCONNECTED:
			_LoadCustomizableList();
			break;

		case kMsgComponentSelected:
		{
			break;
		}

		case kMsgAddComponent:
		{
			BWindow* window = new BWindow(BRect(20, 20, 250, 300),
				"Add Component", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, 0);
			window->CenterIn(Window()->Frame());
			BALMLayout* layout = new BALMLayout(10.);
			window->SetLayout(layout);

			layout->AddView(new InstalledCustomizableView, layout->Left(),
				layout->Top(), layout->Right(), layout->Bottom());
			window->Show();
			break;
		}

		case kMsgRemoveComponent:
		{
			BArray<BWeakReference<Customizable> > allCustomizable;
			fRoster->GetCustomizableList(allCustomizable);
			int32 index = fCustomizableListView->CurrentSelection();
			if (index < 0 || index >= (int32)allCustomizable.CountItems())
				break;
			BReference<Customizable> customizable
				= allCustomizable.ItemAt(index).GetReference();
			if (customizable == NULL)
				break;

			if (customizable->CountOwnConnections() == 0)
//TODO fix this BReferencable
//				customizable->Roster()->ReleaseCustomizable(customizable);				
			break;
		}

		default:
			BView::MessageReceived(message);
	}
}


void
CustomizableRosterView::_LoadCustomizableList()
{
	fCustomizableListView->MakeEmpty();

	BArray<BWeakReference<Customizable> > allCustomizable;
	fRoster->GetCustomizableList(allCustomizable);
	for (int32 i = 0; i < allCustomizable.CountItems(); i++) {
		BReference<Customizable> customizable
			= allCustomizable.ItemAt(i).GetReference();
		if (customizable == NULL)
			continue;
		BString label = customizable->ObjectName();
		if (customizable->CountOwnConnections() == 0)
			label += " NC";
		fCustomizableListView->AddItem(new BStringItem(label));
	}
}

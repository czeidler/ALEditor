/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "CustomizableEditView.h"

#include <Looper.h>
#include <MenuItem.h>
#include <PopUpMenu.h>

#include "ALMLayout.h"

#include <CustomizableRoster.h>


template <class Type>
class ListItemHandlerInterface {
public:
	virtual	void				MouseDown(Type* view, BPoint point) {}
};


ConnectionListView::ConnectionListView(CustomizableEditView* parent)
	:
	BOutlineListView("ConnectionListView"),
	fParent(parent)
{
}


void
ConnectionListView::MouseDown(BPoint point)
{
	BOutlineListView::MouseDown(point);

	int32 index = IndexOf(point);
	if (index < 0)
		return;
	ListItemHandlerInterface<BOutlineListView>* item
		= dynamic_cast<ListItemHandlerInterface<BOutlineListView>*>(ItemAt(index));
	item->MouseDown(this, point);
}


class LabelItem : public BStringItem,
	public ListItemHandlerInterface<BOutlineListView> {
public:
	LabelItem(const char* label)
		:
		BStringItem(label)
	{
	}
};


class SocketItem : public BStringItem,
	public ListItemHandlerInterface<BOutlineListView> {
public:
	SocketItem(const char* label, Customizable* customizable,
		Customizable::Socket* socket)
		:
		BStringItem(label),
		fCustomizable(customizable),
		fSocket(socket)
	{
	}

	void
	MouseDown(BOutlineListView* view, BPoint point)
	{
		uint32 buttons;
		view->GetMouse(&point, &buttons);
		point = view->ConvertToScreen(point);
		if ((buttons & B_SECONDARY_MOUSE_BUTTON) == 0)
			return;
		BPopUpMenu rightClickMenu(fSocket->Name());
		BMenu* compatibleConnections = new BMenu("Connect To");
		BMenuItem* connectItem = new BMenuItem(compatibleConnections);
		BALM::CustomizableList list;
		fCustomizable->Roster()->GetCompatibleConnections(fSocket, list);
		for (int32 i = 0; i < list.CountItems(); i++) {
			Customizable* customizable = list.ItemAt(i);
			compatibleConnections->AddItem(new BMenuItem(
				customizable->ObjectName(),	NULL));
		}
		rightClickMenu.AddItem(connectItem);
		BMenuItem* result = rightClickMenu.Go(point);
		int32 index = compatibleConnections->IndexOf(result);
		if (index < 0)
			return;
		Customizable* connectCustomizable = list.ItemAt(index);
		fCustomizable->Connect(fSocket, connectCustomizable);
	}

private:
	Customizable*				fCustomizable;
	Customizable::Socket*		fSocket;
};


class ConnectionItem : public BStringItem,
	public ListItemHandlerInterface<BOutlineListView> {
public:
	ConnectionItem(const char* label, Customizable* customizable,
		Customizable* connected, Customizable::Socket* socket,
		bool backwards = false)
		:
		BStringItem(label),
		fBackwards(backwards),
		fCustomizable(customizable),
		fConnectedCustomizable(connected),
		fSocket(socket)
	{
	}

	void
	MouseDown(BOutlineListView* view, BPoint point)
	{
		BMessage* message = view->Looper()->CurrentMessage();
		int32 buttons = message->FindInt32("buttons");
		int32 clicks = message->FindInt32("clicks");

		if ((buttons & B_SECONDARY_MOUSE_BUTTON) != 0) {
			BPopUpMenu rightClickMenu(fSocket->Name());
			BMenuItem* removeItem = new BMenuItem("Remove", NULL);
			rightClickMenu.AddItem(removeItem);
			point = view->ConvertToScreen(point);
			BMenuItem* result = rightClickMenu.Go(point);
			if (removeItem != result)
				return;
			fCustomizable->Disconnect(fSocket, fConnectedCustomizable);
		} else if (clicks == 2) {
			CustomizableEditView* parent
				= ((ConnectionListView*)view)->Parent();
			if (fBackwards)
				parent->SetTo(fCustomizable);
			else
				parent->SetTo(fConnectedCustomizable);
		}
	}

private:
	bool						fBackwards;
	Customizable*				fCustomizable;
	Customizable*				fConnectedCustomizable;
	Customizable::Socket*		fSocket;
};


CustomizableEditView::CustomizableEditView()
	:
	BView("CustomizableEditView", B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	fCustomizable(NULL)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	BALMLayout* layout = new BALMLayout(5, 5);
	SetLayout(layout);

	fConnectionListView = new ConnectionListView(this);

	layout->AddView(fConnectionListView, layout->Left(), layout->Top(),
		layout->Right(), layout->Bottom());
}


CustomizableEditView::~CustomizableEditView()
{
	SetTo(NULL);
}


void
CustomizableEditView::SetTo(Customizable* component)
{
	fCustomizable = component;
	fConnectionListView->MakeEmpty();
	if (fCustomizable == NULL)
		return;
	fCustomizable->Roster()->StartWatching(this);

	BStringItem* socketRoot = new LabelItem("Sockets:");
	fConnectionListView->AddItem(socketRoot);

	for (int32 i = fCustomizable->CountSockets() - 1; i >= 0; i--) {
		Customizable::Socket* socket = fCustomizable->SocketAt(i);
		BString label = socket->Name();
		label += " (";
		label += socket->Interface();
		label += ")";
		SocketItem* socketItem = new SocketItem(label, fCustomizable, socket);
		fConnectionListView->AddUnder(socketItem, socketRoot);
		_FillSocketConnections(socket, socketItem);
	}

	BStringItem* connectionRoot = new LabelItem("Connected To:");
	fConnectionListView->AddItem(connectionRoot);
	for (int32 i = fCustomizable->CountOwnConnections() - 1; i >= 0; i--) {
		Customizable::Socket* socket = fCustomizable->OwnConnectionAt(i);
		BString label = socket->Parent()->ObjectName();
		label += " (";
		label += socket->Name();
		label += ")";
		ConnectionItem* item = new ConnectionItem(label, socket->Parent(),
			fCustomizable, socket, true);
		fConnectionListView->AddUnder(item, connectionRoot);
	}
}


void
CustomizableEditView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_SOCKET_CONNECTED:
		case B_SOCKET_DISCONNECTED:
			SetTo(fCustomizable);
			break;

		default:
			BView::MessageReceived(message);
	}
}


void
CustomizableEditView::_FillSocketConnections(Customizable::Socket* socket,
	BStringItem* socketItem)
{
	for (int32 i = socket->CountConnections() - 1; i >= 0; i--) {
		Customizable* connection = socket->ConnectionAt(i);
		fConnectionListView->AddUnder(new ConnectionItem(connection->ObjectName(),
			fCustomizable, connection, socket), socketItem);
	}
}

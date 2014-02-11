/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include <TrashView.h>


TrashView::TrashView(BALMEditor* editor)
	:
	fEditor(editor)
{
	_LoadTrash();
}


bool
TrashView::InitiateDrag(BPoint point, int32 index, bool wasSelected)
{
	BReference<Customizable> customizable
		= fTrashList.ItemAt(index).GetReference();
	if (customizable.Get() == NULL)
		return false;
	BMessage dragMessage(BALM::kUnTrashComponent);
	dragMessage.AddPointer("component", customizable.Get());

	DragMessage(&dragMessage, ItemFrame(index));
	return true;
}


void
TrashView::AttachedToWindow()
{
	fEditor->SetTrashWatcher(BMessenger(this));
}


void
TrashView::MessageReceived(BMessage* message)
{
	switch (message->what) {
	case BALM::kTrashUpdated:
		_LoadTrash();
		break;

	default:
		BListView::MessageReceived(message);
	};
}


void
TrashView::_LoadTrash()
{
	MakeEmpty();
	fTrashList.MakeEmpty();

	fEditor->GetTrash(fTrashList);
	for (int32 i = 0; i < fTrashList.CountItems(); i++) {
		BReference<Customizable> customizable
			= fTrashList.ItemAt(i).GetReference();
		if (customizable.Get() == NULL)
			continue;
		AddItem(new BStringItem(customizable->ObjectName()));
	}
}

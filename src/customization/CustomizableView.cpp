/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include <CustomizableView.h>


CustomizableView::CustomizableView(BView* parent, bool own)
	:
	fView(parent),
	fLayoutItem(NULL),
	fOwnObject(own)
{
}


CustomizableView::CustomizableView(BLayoutItem* item, bool own)
	:
	fView(NULL),
	fLayoutItem(item),
	fOwnObject(own)
{
}

CustomizableView::~CustomizableView()
{
	if (fView && fOwnObject) {
		fView->RemoveSelf();
		delete fView;
	}
	if (fLayoutItem && fOwnObject) {
		fLayoutItem->Layout()->RemoveItem(fLayoutItem);
		delete fLayoutItem;
	}
}


BString
CustomizableView::Identifier()
{
	if (fView != NULL)
		return fView->Name();
	return fName;
}


void
CustomizableView::SetIdentifier(const BString& id)
{
	if (fView != NULL)
		return fView->SetName(id);
	else
		fName = id;
}


BViewLocalPointer
CustomizableView::View()
{
	return fView;
}


BViewLocalPointer
CustomizableView::CreateConfigView()
{
	return NULL;
}


BLayoutItemLocalPointer
CustomizableView::LayoutItem()
{
	return fLayoutItem;
}


void
CustomizableView::RemoveSelf()
{
	if (fView)
		fView->RemoveSelf();
	else if (fLayoutItem)
		fLayoutItem->Layout()->RemoveItem(fLayoutItem);
}


BSize
CustomizableView::PreferredSize()
{
	if (fView)
		return fView->PreferredSize();
	else if (fLayoutItem)
		return fLayoutItem->PreferredSize();

	return BSize(-1, -1);
}


BSize
CustomizableView::MinSize()
{
	if (fView)
		return fView->MinSize();
	else if (fLayoutItem)
		return fLayoutItem->MinSize();

	return BSize(-1, -1);
}


BRect
CustomizableView::Frame()
{
	if (fView)
		return fView->Frame();
	else if (fLayoutItem)
		return fLayoutItem->Frame();

	return BRect();
}


BLayout*
CustomizableView::Layout()
{
	if (fView)
		return fView->GetLayout();
	else if (fLayoutItem)
		return fLayoutItem->Layout();

	return NULL;
}

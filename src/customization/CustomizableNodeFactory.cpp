/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include <CustomizableNodeFactory.h>

#include <Picture.h>
#include <StringItem.h>

#include <Misc.h>

#include "ALMLayout.h"


static const float kLeftInset = 4;


class BIconItem : public BStringItem{
public:
								BIconItem(const char* text,
									const BPicture& picture, BRect iconFrame);

	virtual void				Update(BView* owner, const BFont* font);

	virtual	void				DrawItem(BView* owner, BRect frame,
									bool complete = false);

protected:
			void				DrawItemWithTextOffset(BView* owner,
									BRect frame, bool complete,
									float textOffset);

private:
			BPicture			fPicture;
			BRect				fFrame;
};


BIconItem::BIconItem(const char* text, const BPicture& picture, BRect iconFrame)
	:
	BStringItem(text),
	fPicture(picture),
	fFrame(iconFrame)
{
}


void
BIconItem::Update(BView* owner, const BFont* font)
{
	BStringItem::Update(owner, font);

	float height = Height();
	if (height < fFrame.Height())
		SetHeight(fFrame.Height() + 2);
	SetWidth(Width() + fFrame.Width() + 4);
}


void
BIconItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	float iconSize = fFrame.Width();
	DrawItemWithTextOffset(owner, frame, complete, iconSize + 4);

	BRect iconFrame(frame.left + kLeftInset, frame.top,
		frame.left + kLeftInset + iconSize - 1, frame.top + iconSize - 1);
	owner->SetDrawingMode(B_OP_OVER);
	owner->DrawPicture(&fPicture, iconFrame.LeftTop());
	owner->SetDrawingMode(B_OP_COPY);
}


void
BIconItem::DrawItemWithTextOffset(BView* owner, BRect frame,
	bool complete, float textOffset)
{
	if (IsSelected() || complete) {
		rgb_color color;
		if (IsSelected())
			color = ui_color(B_MENU_SELECTED_BACKGROUND_COLOR);
		else
			color = owner->ViewColor();
		owner->SetHighColor(color);
		owner->SetLowColor(color);
		owner->FillRect(frame);
	} else
		owner->SetLowColor(owner->ViewColor());

	BString text = Text();
	if (IsEnabled())
		owner->SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
	else
		owner->SetHighColor(tint_color(owner->LowColor(), B_DARKEN_3_TINT));

	owner->MovePenTo(frame.left + kLeftInset + textOffset,
		frame.top + BaselineOffset());
	owner->DrawString(text.String());
}


FactoryDragListView::FactoryDragListView(CustomizableRoster* roster)
	:
	fRoster(roster)
{
}


void
FactoryDragListView::AttachedToWindow()
{
	fRoster->GetInstalledCustomizableList(fInstalledComponents);
	for (int32 i = 0; i < fInstalledComponents.CountItems(); i++) {
		BALM::CustomizableAddOn* addOn = fInstalledComponents.ItemAt(i);
		BPicture* picture;
		BRect pictureFrame;
		if (addOn->IconPicture(this, &picture, pictureFrame))
			AddItem(new BIconItem(addOn->Name(), *picture, pictureFrame));
		else
			AddItem(new BStringItem(addOn->Name()));
	}
}


void
FactoryDragListView::MouseMoved(BPoint where, uint32 transit,
	const BMessage* message)
{
	fCurrentMousePosition = where;
	BListView::MouseMoved(where, transit, message);	
}


bool
FactoryDragListView::InitiateDrag(BPoint point, int32 index, bool wasSelected)
{
	BALM::CustomizableAddOn* addOn = fInstalledComponents.ItemAt(index);
	BMessage dragMessage(kMsgCreateComponent);
	dragMessage.AddString("component", addOn->Name());

	BRect frame;
	BBitmap* bitmap = BALM::EditorHelper::CreateBitmap(addOn, frame, NULL);
	if (bitmap != NULL) {
		DragMessage(&dragMessage, bitmap, BPoint(frame.Width() / 2,
			frame.Height() / 2));
	} else {
		frame.OffsetTo(fCurrentMousePosition.x - frame.Width() / 2,
			fCurrentMousePosition.y - frame.Height() / 2);
		DragMessage(&dragMessage, frame);
	}

	return true;
}


CustomizableFactoryWindow::CustomizableFactoryWindow(CustomizableRoster* roster)
	:
	BWindow(BRect(20, 220, 250, 500), "Add Component", B_FLOATING_WINDOW, 0)
{
	BALMLayout* layout = new BALMLayout(10.);
	SetLayout(layout);
	FactoryDragListView* listView = new FactoryDragListView(roster);
	layout->AddView(listView, layout->Left(), layout->Top(),
		layout->Right(), layout->Bottom());
}

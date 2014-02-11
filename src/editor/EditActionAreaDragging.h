/*
 * Copyright 2011-2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	EDIT_ACTION_DRAGGING_H
#define	EDIT_ACTION_DRAGGING_H

#include <Debug.h>
#include <Misc.h>

#include "EditAction.h"
#include "LayoutEditView.h"


using namespace BALM;


class InsertHorizontalActionBase : public EditAction {
public:
	InsertHorizontalActionBase(BALMLayout* layout, BMessage* prevLayout,
		int32 xTab, BArray<Area*> areas, area_side direction,
		vertical_alignment alignment)
		:
		EditAction(layout, prevLayout),
		fXTabIndex(xTab),
		fAreas(areas),
		fInsertDirection(direction),
		fAlignment(alignment)
	{
	}

	virtual bool Perform()
	{
		if (fXTabIndex < 0 || fAreas.CountItems() == 0)
			return false;

		BReference<XTab> left;
		BReference<XTab> right;
		BReference<YTab> top;
		BReference<YTab> bottom;

		XTab* tab = fALMLayout->XTabAt(fXTabIndex, true);
		if (tab == NULL)
			debugger("Ups");

		if (fInsertDirection == kRight) {
			left = tab;
			right = fALMLayout->AddXTab();
			right->SetValue(tab->Value());
			for (int32 i = 0; i < fAreas.CountItems(); i++)
				fAreas.ItemAt(i)->SetLeft(right);
		} else {
			right = tab;
			left =  fALMLayout->AddXTab();
			left->SetValue(tab->Value());
			for (int32 i = 0; i < fAreas.CountItems(); i++)
				fAreas.ItemAt(i)->SetRight(left);
		}

		top = fAreas.ItemAt(0)->Top();
		bottom = fAreas.ItemAt(0)->Bottom();
		for (int32 i = 1; i < fAreas.CountItems(); i++) {
			Area* area = fAreas.ItemAt(i);
			if (area->Top()->Value() < top->Value())
				top = area->Top();
			if (area->Bottom()->Value() > top->Value())
				bottom = area->Bottom();
		}
		ASSERT(top->Value() <= bottom->Value());

		if (fAreas.CountItems() == 1) {
			Area* area = fAreas.ItemAt(0);
			// must have an extent to no confuse the overlap algorithm!
			if (fAlignment == B_ALIGN_TOP) {
				bottom = fALMLayout->AddYTab();
				bottom->SetValue(area->Bottom()->Value());
			} else if (fAlignment == B_ALIGN_BOTTOM) {
				top = fALMLayout->AddYTab();
				top->SetValue(area->Top()->Value());
			}
		}

		return InsertObject(left, top, right, bottom);
	}

protected:
	virtual bool				InsertObject(XTab* left, YTab* top, XTab* right,
									YTab* bottom) = 0;

			int32				fXTabIndex;
			BArray<Area*>		fAreas;
			area_side			fInsertDirection;
			vertical_alignment	fAlignment;
};


class InsertVerticalActionBase : public EditAction {
public:
	InsertVerticalActionBase(BALMLayout* layout, BMessage* prevLayout,
		int32 yTab, BArray<Area*> areas, area_side direction,
		alignment alignment)
		:
		EditAction(layout, prevLayout),
		fYTabIndex(yTab),
		fAreas(areas),
		fInsertDirection(direction),
		fAlignment(alignment)
	{
	}

	virtual bool Perform()
	{
		if (fYTabIndex < 0 || fAreas.CountItems() == 0)
			return false;

		BReference<XTab> left;
		BReference<XTab> right;
		BReference<YTab> top;
		BReference<YTab> bottom;

		YTab* tab = fALMLayout->YTabAt(fYTabIndex, true);
		if (tab == NULL)
			debugger("Ups");

		if (fInsertDirection == kBottom) {
			top = tab;
			bottom = fALMLayout->AddYTab();
			bottom->SetValue(tab->Value());
			for (int32 i = 0; i < fAreas.CountItems(); i++)
				fAreas.ItemAt(i)->SetTop(bottom);
		} else {
			bottom = tab;
			top =  fALMLayout->AddYTab();
			top->SetValue(tab->Value());
			for (int32 i = 0; i < fAreas.CountItems(); i++)
				fAreas.ItemAt(i)->SetBottom(top);
		}

		left = fAreas.ItemAt(0)->Left();
		right = fAreas.ItemAt(0)->Right();
		for (int32 i = 1; i < fAreas.CountItems(); i++) {
			Area* area = fAreas.ItemAt(i);
			if (area->Left()->Value() < left->Value())
				left = area->Left();
			if (area->Right()->Value() > right->Value())
				right = area->Right();
		}
		ASSERT(right->Value() >= left->Value());

		if (fAreas.CountItems() == 1) {
			Area* area = fAreas.ItemAt(0);
			// must have an extent to no confuse the overlap algorithm!
			if (fAlignment == B_ALIGN_LEFT) {
				right = fALMLayout->AddXTab();
				right->SetValue(area->Right()->Value());
			} else if (fAlignment == B_ALIGN_RIGHT) {
				left = fALMLayout->AddXTab();
				left->SetValue(area->Left()->Value());
			}
		}

		return InsertObject(left, top, right, bottom);
	}

protected:
	virtual bool				InsertObject(XTab* left, YTab* top, XTab* right,
									YTab* bottom) = 0;

			int32				fYTabIndex;
			BArray<Area*>		fAreas;
			area_side			fInsertDirection;
			alignment			fAlignment;
};


class LayoutEditView::DragState : public State {
public:
								DragState(LayoutEditView* view,
									Area* selectedArea);

	virtual bool				MouseMoved(BPoint point, uint32 transit,
									const BMessage* message);
	virtual	void				Draw(BRect updateRect);
	virtual bool	 			MouseUp(BPoint point);

protected:
	virtual	BRect				DragObjectFrame(BPoint mousePosition,
									BPoint dragOffset);
	virtual	BSize				DragObjectPrefSize() = 0;
	virtual BSize				DragObjectMinSize() = 0;
	virtual	EditAction*			CreateHInsertionAction() = 0;
	virtual	EditAction*			CreateVInsertionAction() = 0;
	virtual EditAction*			CreateIntoEmptyAreaAction() = 0;
	virtual EditAction*			CreateSwapAction() = 0;

protected:
			BRect				fDragFrame;
			BPoint				fDropOffset;

			bool				fPerformed;

			Area*				fSelectedArea;
			Area*				fMouseOverArea;

			//! insert between:
			int32				fXTab;
			int32				fYTab;
			area_side			fInsertDirection;
			BArray<Area*>		fAreas;
			BAlignment			fInsertBetweenAlignment;

			InsertionIntoEmptyArea	fInsertionIntoEmptyArea;
};


class LayoutEditView::DragAreaState : public LayoutEditView::DragState {
public:
	DragAreaState(LayoutEditView* view, Area* selectedArea, BPoint dragStart,
		BPoint dragOffset)
		:
		LayoutEditView::DragState(view, selectedArea)
	{
		fDropOffset = dragOffset;
		fDragFrame = DragObjectFrame(dragStart, fDropOffset);
	}

	virtual void EnterState()
	{
		BMessage dragMessage(uint32(0));
		fView->DragMessage(&dragMessage, fDragFrame);

		set_cursor(kMoveCursor);
	}

protected:
	virtual	BRect				DragObjectFrame(BPoint mousePosition,
									BPoint dragOffset);
	virtual	BSize				DragObjectPrefSize();
	virtual BSize				DragObjectMinSize();
	virtual	EditAction*			CreateHInsertionAction();
	virtual	EditAction*			CreateVInsertionAction();
	virtual EditAction*			CreateIntoEmptyAreaAction();
	virtual EditAction*			CreateSwapAction();
};


#endif // EDIT_ACTION_DRAGGING_H

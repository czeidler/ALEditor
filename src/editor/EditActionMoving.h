/*
 * Copyright 2011-2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	EDIT_ACTION_MOVING_H
#define	EDIT_ACTION_MOVING_H


#include "EditActionAreaDragging.h"


class InsertAreaHorizontalAction : public InsertHorizontalActionBase {
public:
	InsertAreaHorizontalAction(BALMLayout* layout, BMessage* prevLayout,
		Area* area,	int32 xTab, BArray<Area*> areas, area_side direction,
		vertical_alignment alignment)
		:
		InsertHorizontalActionBase(layout, prevLayout, xTab, areas, direction,
			alignment),
		fSelectedArea(area)
	{
	}

	virtual bool Perform()
	{
		BALM::area_ref sourceAreaRef(fSelectedArea);

		if (InsertHorizontalActionBase::Perform() == false)
			return false;
		
		AreaRemoval::FillEmptySpace(fALMLayout, sourceAreaRef.left,
			sourceAreaRef.top, sourceAreaRef.right, sourceAreaRef.bottom);
		return true;
	}

protected:
	virtual bool InsertObject(XTab* left, YTab* top, XTab* right, YTab* bottom)
	{
		fSelectedArea->SetLeft(left);
		fSelectedArea->SetTop(top);
		fSelectedArea->SetRight(right);
		fSelectedArea->SetBottom(bottom);
		return true;
	}
	
private:
			Area*				fSelectedArea;
};


class InsertAreaVerticalAction : public InsertVerticalActionBase {
public:
	InsertAreaVerticalAction(BALMLayout* layout, BMessage* prevLayout,
		Area* area, int32 xTab, BArray<Area*> areas, area_side direction,
		alignment alignment)
		:
		InsertVerticalActionBase(layout, prevLayout, xTab, areas,
			direction, alignment),
		fSelectedArea(area)
	{
	}

	virtual bool Perform()
	{
		BALM::area_ref sourceAreaRef(fSelectedArea);

		if (InsertVerticalActionBase::Perform() == false)
			return false;
		
		AreaRemoval::FillEmptySpace(fALMLayout, sourceAreaRef.left,
			sourceAreaRef.top, sourceAreaRef.right, sourceAreaRef.bottom);
		return true;
	}

protected:
	virtual bool InsertObject(XTab* left, YTab* top, XTab* right, YTab* bottom)
	{
		fSelectedArea->SetLeft(left);
		fSelectedArea->SetTop(top);
		fSelectedArea->SetRight(right);
		fSelectedArea->SetBottom(bottom);
		return true;
	}
	
private:
			Area*				fSelectedArea;
};


class MoveAction : public EditAction {
public:
	MoveAction(BALMLayout* layout, BMessage* prevLayout, Area* from,
		InsertionIntoEmptyArea& insertionIntoEmptyArea)
		:
		EditAction(layout, prevLayout),
		fFromArea(from),
		fInsertionIntoEmptyArea(insertionIntoEmptyArea)
	{
	}

	virtual bool Perform()
	{
		area_ref fromRef(fFromArea);
		
		area_ref toRef = fInsertionIntoEmptyArea.CreateTargetArea();

		fFromArea->SetLeft(toRef.left);
		fFromArea->SetTop(toRef.top);
		fFromArea->SetRight(toRef.right);
		fFromArea->SetBottom(toRef.bottom);

		AreaRemoval::FillEmptySpace(fALMLayout, fromRef.left, fromRef.top,
			fromRef.right, fromRef.bottom);

		return true;
	}

	virtual	const char* Name()
	{
		return "move";
	}

protected:
			Area*				fFromArea;
			InsertionIntoEmptyArea	fInsertionIntoEmptyArea;
};


class LayoutEditView::DragOutsideState : public LayoutEditView::State {
public:
	DragOutsideState(LayoutEditView* view, Area* area, BPoint dragOffset)
		:
		LayoutEditView::State(view),
		fArea(area),
		fDragOffset(dragOffset)
	{
	}

	virtual bool MouseMoved(BPoint point, uint32 transit,
		const BMessage* message)
	{
		if (transit == B_ENTERED_VIEW) {
			fView->_SetState(new LayoutEditView::DragAreaState(fView,
				fArea, point, fDragOffset));
		}
		return true;
	}

	virtual bool MouseUp(BPoint point)
	{
		fView->TrashArea(fArea);
		return true;
	}

protected:
			Area*				fArea;
			BPoint				fDragOffset;
};


#endif // EDIT_ACTION_MOVING_H

/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "EditActionResizing.h"


template <>
bool
InsertTabAction<XTab>::Perform()
{
	bool status = false;

	XTab* oldTab = NULL;
	if (fSide == kLeft) {
		BReference<XTab> xTab = fALMLayout->AddXTab();
		xTab->SetValue(fArea->Left()->Value());
		oldTab = fArea->Left();
		fArea->SetLeft(xTab);
		status = true;
	}
	if (fSide == kRight) {
		BReference<XTab> xTab = fALMLayout->AddXTab();
		xTab->SetValue(fArea->Right()->Value());
		oldTab = fArea->Right();
		fArea->SetRight(xTab);
		status = true;
	}

	if (status) {
		TabConnections connections;
		AreaRemoval areaRemoval(&connections, fALMLayout);
		areaRemoval.AreaDetachedX(fArea, fSide, oldTab);
	}
	return status;
}


template <>
bool
InsertTabAction<YTab>::Perform()
{
	bool status = false;

	YTab* oldTab = NULL;
	if (fSide == kTop) {
		BReference<YTab> yTab = fALMLayout->AddYTab();
		yTab->SetValue(fArea->Top()->Value());
		oldTab = fArea->Top();
		fArea->SetTop(yTab);
		status = true;
	}
	if (fSide == kBottom) {
		BReference<YTab> yTab = fALMLayout->AddYTab();
		yTab->SetValue(fArea->Bottom()->Value());
		oldTab = fArea->Bottom();
		fArea->SetBottom(yTab);
		status = true;
	}

	if (status) {
		TabConnections connections;
		AreaRemoval areaRemoval(&connections, fALMLayout);
		areaRemoval.AreaDetachedY(fArea, fSide, oldTab);
	}
	return status;
}


template <>
bool
ResizeGroupAction<XTab>::Perform()
{
	XTab* toTab = fALMLayout->XTabAt(fMoveToTab, true);
	if (fSide == kLeft) {
		for (int32 i = 0; i < fAreas.CountItems(); i++)
			fAreas.ItemAt(i)->SetLeft(toTab);
		return true;
	} else if (fSide == kRight) {
		for (int32 i = 0; i < fAreas.CountItems(); i++)
			fAreas.ItemAt(i)->SetRight(toTab);
		return true;
	}
	return false;
}


template <>
bool
ResizeGroupAction<YTab>::Perform()
{
	YTab* toTab = fALMLayout->YTabAt(fMoveToTab, true);
	if (fSide == kTop) {
		for (int32 i = 0; i < fAreas.CountItems(); i++)
			fAreas.ItemAt(i)->SetTop(toTab);
		return true;
	} else if (fSide == kBottom) {
		for (int32 i = 0; i < fAreas.CountItems(); i++)
			fAreas.ItemAt(i)->SetBottom(toTab);
		return true;
	}
	return false;
}


template <>
bool
InsertGroupTabAction<XTab>::Perform()
{
	if (fSide == kLeft) {
		BReference<XTab> xTab = fALMLayout->AddXTab();
		xTab->SetValue(fAreas.ItemAt(0)->Left()->Value());
		for (int32 i = 0; i < fAreas.CountItems(); i++)
			fAreas.ItemAt(i)->SetLeft(xTab);
		return true;
	} else if (fSide == kRight) {
		BReference<XTab> xTab = fALMLayout->AddXTab();
		xTab->SetValue(fAreas.ItemAt(0)->Right()->Value());
		for (int32 i = 0; i < fAreas.CountItems(); i++)
			fAreas.ItemAt(i)->SetRight(xTab);
		return true;
	}
	return false;
}


template <>
bool
InsertGroupTabAction<YTab>::Perform()
{
	if (fSide == kTop) {
		BReference<YTab> yTab = fALMLayout->AddYTab();
		yTab->SetValue(fAreas.ItemAt(0)->Top()->Value());
		for (int32 i = 0; i < fAreas.CountItems(); i++)
			fAreas.ItemAt(i)->SetTop(yTab);
		return true;
	} else if (fSide == kBottom) {
		BReference<YTab> yTab = fALMLayout->AddYTab();
		yTab->SetValue(fAreas.ItemAt(0)->Bottom()->Value());
		for (int32 i = 0; i < fAreas.CountItems(); i++)
			fAreas.ItemAt(i)->SetBottom(yTab);
		return true;
	}
	return false;
}


#define RESIZE_TO_TABS_IN_AREA 1


LayoutEditView::ResizeState::ResizeState(LayoutEditView* view, Area* area,
	area_side xSide, area_side ySide)
	:
	State(view),
	fSelectedArea(area),
	fDragSideX(xSide),
	fDragSideY(ySide),
	fMouseOverXTab(-1),
	fMouseOverYTab(-1),
	fDetachX(false),
	fDetachY(false),
	fHardResizeToX(-1),
	fHardResizeToY(-1),
	fShowXTabs(false),
	fShowYTabs(false),
	fResizeCursor(NULL),
	fDetachCursor(NULL)
{
}


void
LayoutEditView::ResizeState::EnterState()
{
	fView->SetShowXTabs(false);
	fView->SetShowYTabs(false);

	if (fDragSideX != kNoSide && fDragSideY != kNoSide) {
		bool nwse = (fDragSideY == kTop && fDragSideX == kLeft)
			|| (fDragSideY == kBottom && fDragSideX == kRight);
		bool nesw = (fDragSideY == kTop && fDragSideX == kRight)
			|| (fDragSideY == kBottom && fDragSideX == kLeft);
		if (nwse)
			fResizeCursor = kLeftTopRightBottomCursor;
		else if (nesw)
			fResizeCursor = kLeftBottomRightTopCursor;
		fShowXTabs = true;
		fShowYTabs = true;
	} else if (fDragSideX != kNoSide) {
		fShowXTabs = true;
		fResizeCursor = kLeftRightCursor;
	} else if (fDragSideY != kNoSide) {
		fShowYTabs = true;
		fResizeCursor = kUpDownCursor;
	}

	set_cursor(fResizeCursor);
}


bool
LayoutEditView::ResizeState::MouseUp(BPoint point)
{
	if (fMouseOverXTab >= 0 || fMouseOverYTab >= 0) {
		XTab* selectedX;
		YTab* selectedY;
		_SelectedTabs(&selectedX, &selectedY);
		fView->PerformAction(new ResizeAction(fView->fALMEngine,
			fView->CurrentLayout(), fSelectedArea, selectedX, selectedY,
			fMouseOverXTab, fMouseOverYTab));
	}
	if (fDetachX) {
		fView->PerformAction(new InsertTabAction<XTab>(fView->fALMEngine,
			fView->CurrentLayout(), fSelectedArea, fDragSideX));
	}
	if (fDetachY) {
		fView->PerformAction(new InsertTabAction<YTab>(fView->fALMEngine,
			fView->CurrentLayout(), fSelectedArea, fDragSideY));
	}

	if (fHardResizeToX > 0) {
		fView->PerformAction(new HardResizeAction(fView->fALMEngine,
			fView->CurrentLayout(), fSelectedArea, fDragSideX, fHardResizeToX));
	}

	if (fHardResizeToY > 0) {
		fView->PerformAction(new HardResizeAction(fView->fALMEngine,
			fView->CurrentLayout(), fSelectedArea, fDragSideY, fHardResizeToY));
	}

	BALMEditor* editor = fView->Editor();
	fView->SetShowXTabs(editor->ShowXTabs());
	fView->SetShowYTabs(editor->ShowYTabs());
	return true;
}


class CursorSetter {
public:
	CursorSetter(const uint8* cursor)
		:
		fCursor(cursor)
	{}

	~CursorSetter()
	{
		if (fCursor != NULL)
			set_cursor(fCursor);
	}

	void SetCursor(const uint8* cursor)
	{
		fCursor = cursor;
	}
private:
	const	uint8*				fCursor;
};


bool
LayoutEditView::ResizeState::MouseMoved(BPoint point, uint32 transit,
	const BMessage* message)
{
	CursorSetter cursorSetter(fResizeCursor);

	XTab* selectedX;
	YTab* selectedY;
	_SelectedTabs(&selectedX, &selectedY);

	fMouseOverXTab = -1;
	fMouseOverYTab = -1;

	fDetachX = false;
	fDetachY = false;
	fHardResizeToX = -1;
	fHardResizeToY = -1;

	BALMLayout*	layout = fView->fALMEngine;

	XTab* xTab = NULL;
	if (selectedX != NULL)
		xTab = fView->GetXTabNearPoint(point, kTolerance);
	YTab* yTab = NULL;
	if (selectedY != NULL)
		yTab = fView->GetYTabNearPoint(point, kTolerance);

	BRect areaFrame = fSelectedArea->Frame();
	if (xTab && xTab != selectedX) {
		if (fDragSideX == kLeft && xTab != fSelectedArea->Right()
			&& xTab->Value() < areaFrame.right) {
			if (xTab->Value() < areaFrame.left) {
				BRect increase(ceilf(xTab->Value()), ceilf(areaFrame.top),
					floorf(areaFrame.left),	floorf(areaFrame.bottom));
				increase.InsetBy(1, 1);
				if (fView->fTakenSpace.Intersects(increase) == false)
					fMouseOverXTab = layout->IndexOf(xTab, true);
			} else {
#if RESIZE_TO_TABS_IN_AREA
				fMouseOverXTab = layout->IndexOf(xTab, true);
#else
				if (point.y < areaFrame.top || point.y > areaFrame.bottom)
					fMouseOverXTab = layout->IndexOf(xTab, true);
#endif
			}
		} else if (fDragSideX == kRight && xTab != fSelectedArea->Left()
			&& xTab->Value() > areaFrame.left) {
			if (xTab->Value() > areaFrame.right) {
				BRect increase(ceilf(areaFrame.right), ceilf(areaFrame.top),
					floorf(xTab->Value()), floorf(areaFrame.bottom));
				increase.InsetBy(1, 1);
				if (fView->fTakenSpace.Intersects(increase) == false)
					fMouseOverXTab = layout->IndexOf(xTab, true);
			} else {
#if RESIZE_TO_TABS_IN_AREA
				fMouseOverXTab = layout->IndexOf(xTab, true);
#else
				if (point.y < areaFrame.top || point.y > areaFrame.bottom)
					fMouseOverXTab = layout->IndexOf(xTab, true);
#endif
			}
		}
	}
	if (yTab && yTab != selectedY) {
		if (fDragSideY == kTop && yTab != fSelectedArea->Bottom()
			&& yTab->Value() < areaFrame.bottom) {
			if (yTab->Value() < areaFrame.top) {
				BRect increase(ceilf(areaFrame.left), ceilf(yTab->Value()),
					floorf(areaFrame.right), floorf(areaFrame.top));
				increase.InsetBy(1, 1);
				if (fView->fTakenSpace.Intersects(increase) == false)
					fMouseOverYTab = layout->IndexOf(yTab, true);
			} else {
#if RESIZE_TO_TABS_IN_AREA
				fMouseOverYTab = layout->IndexOf(yTab, true);
#else
			if (point.x < areaFrame.left || point.x > areaFrame.right)
					fMouseOverYTab = layout->IndexOf(yTab, true);
#endif
			}
		} else if (fDragSideY == kBottom && yTab != fSelectedArea->Top()
			&& yTab->Value() > areaFrame.top) {
			if (yTab->Value() > areaFrame.bottom) {
				BRect increase(ceilf(areaFrame.left), ceilf(areaFrame.bottom),
					floorf(areaFrame.right), floorf(yTab->Value()));
				increase.InsetBy(1, 1);
				if (fView->fTakenSpace.Intersects(increase) == false)
					fMouseOverYTab = layout->IndexOf(yTab, true);
			} else {
#if RESIZE_TO_TABS_IN_AREA
				fMouseOverYTab = layout->IndexOf(yTab, true);
#else
				if (point.x < areaFrame.left || point.x > areaFrame.right)
					fMouseOverYTab = layout->IndexOf(yTab, true);
#endif
			}
		}
	}

	if (fMouseOverXTab >= 0 || fMouseOverYTab >= 0) {
		if (fView->TestAction(new ResizeAction(fView->fALMEngine,
			fView->CurrentLayout(), fSelectedArea, selectedX, selectedY,
			fMouseOverXTab, fMouseOverYTab)) == true) {
			return true;
		}
		fMouseOverXTab = -1;
		fMouseOverYTab = -1;
	}

	// check if we should detach the tab
	float kHardResizeThreshold = 25;
	if (!fView->fEditor->FreePlacement())
		kHardResizeThreshold = 99999925;

	BSize minSize = fSelectedArea->Item()->MinSize();
	BRect frame = fSelectedArea->Frame();
	if (fDragSideX == kLeft && fView->ConnectedTo(fSelectedArea, kRight)) {
		float distance = fabs(point.x - frame.left);
		float newWidth = frame.right - point.x;
		if ((newWidth > minSize.width && distance > kHardResizeThreshold)
			|| (newWidth < 0 && distance > kHardResizeThreshold))
			fHardResizeToX = point.x;
		else if (fabs(distance) > kDetachThreshold)
			fDetachX = true;
	} else if (fDragSideX == kRight && fView->ConnectedTo(fSelectedArea,
		kLeft)) {
		float distance = fabs(frame.right - point.x);
		float newWidth = point.x - frame.left;
		if ((newWidth > minSize.width && distance > kHardResizeThreshold)
			|| (newWidth < 0 && distance > kHardResizeThreshold))
			fHardResizeToX = point.x;
		else if (distance > kDetachThreshold)
			fDetachX = true;
	}
	if (fDragSideY == kTop && fView->ConnectedTo(fSelectedArea, kBottom)) {
		float distance = fabs(point.y - frame.top);
		float newHeight = frame.bottom - point.y;
		if ((newHeight > minSize.height && distance > kHardResizeThreshold)
			|| (newHeight < 0 && distance > kHardResizeThreshold))
			fHardResizeToY = point.y;
		else if (distance > kDetachThreshold)
			fDetachY = true;
	} else if (fDragSideY == kBottom && fView->ConnectedTo(
		fSelectedArea, kTop)) {
		float distance = fabs(frame.bottom - point.y);
		float newHeight = point.y - frame.top;
		if ((newHeight > minSize.height && distance > kHardResizeThreshold)
			|| (newHeight < 0 && distance > kHardResizeThreshold))
			fHardResizeToY = point.y;
		else if (distance > kDetachThreshold)
			fDetachY = true;
	}

	if (!fView->fEditor->FreePlacement()) {
		fHardResizeToX = -1;
		fHardResizeToY = -1;
	}

	// detach not allowed when dragging a corner
	if (fDragSideX != kNoSide && fDragSideY != kNoSide) {
		fDetachX = false;
		fDetachY = false;
		return true;
	}

	if (fDetachX) {
		if (fDragSideX == kLeft) {
			if (fSelectedArea->Left()->Value() > point.x)
				fDetachCursor = kLeftArrowCursor;
			else
				fDetachCursor = kRightArrowCursor;
		} else {
			if (fSelectedArea->Right()->Value() > point.x)
				fDetachCursor = kLeftArrowCursor;
			else
				fDetachCursor = kRightArrowCursor;
		}
		cursorSetter.SetCursor(fDetachCursor);
	} else if (fDetachY) {
		if (fDragSideY == kTop) {
			if (fSelectedArea->Top()->Value() > point.y)
				fDetachCursor = kUpArrowCursor;
			else
				fDetachCursor = kDownArrowCursor;
		} else {
			if (fSelectedArea->Bottom()->Value() > point.y)
				fDetachCursor = kUpArrowCursor;
			else
				fDetachCursor = kDownArrowCursor;
		}
		cursorSetter.SetCursor(fDetachCursor);
	}
	
	if (fDragSideY == kBottom)
			fDetachCursor = kUpArrowCursor;
		else
			fDetachCursor = kDownArrowCursor;

	if (fDetachX) {
		if (fView->TestAction(new InsertTabAction<XTab>(fView->fALMEngine,
			fView->CurrentLayout(), fSelectedArea, fDragSideX)) == false)
			fDetachX = false;
	}

	if (fDetachY) {
		if (fView->TestAction(new InsertTabAction<YTab>(fView->fALMEngine,
			fView->CurrentLayout(), fSelectedArea, fDragSideY)) == false)
			fDetachY = false;
	}

	return true;
}


void
LayoutEditView::ResizeState::Draw(BRect updateRect)
{
	// draw tabs
	if (fShowXTabs)
		_DrawXResizeTabs();
	if (fShowYTabs)
		_DrawYResizeTabs();

	// draw fSelectedArea selection rectangle
	fView->DrawHighLightArea(fSelectedArea, 2, kSelectedColor);

	if (fDragSideX != kNoSide) {
		XTab* mouseOverXTab = fView->fALMEngine->XTabAt(fMouseOverXTab,
			true);
		if (mouseOverXTab != NULL)
			fView->DrawTab(mouseOverXTab, kResizeWidth, kDestinationColor);
	}
	if (fDragSideY != kNoSide) {
		YTab* mouseOverYTab = fView->fALMEngine->YTabAt(fMouseOverYTab,
			true);
		if (mouseOverYTab != NULL)
			fView->DrawTab(mouseOverYTab, kResizeWidth, kDestinationColor);
	}

	// hard resize
	fView->SetPenSize(kTabWidth);
	fView->SetHighColor(kSuggestionColor);
	if (fHardResizeToX > 0) {
		BRect frame = fSelectedArea->Frame();
		fView->StrokeLine(BPoint(fHardResizeToX, frame.top),
			BPoint(fHardResizeToX, frame.bottom));
	}
	if (fHardResizeToY > 0) {
		BRect frame = fSelectedArea->Frame();
		fView->StrokeLine(BPoint(frame.left, fHardResizeToY),
			BPoint(frame.right, fHardResizeToY));
	}
}


void
LayoutEditView::ResizeState::_DrawXResizeTabs()
{
	for (int32 i = 0; i < fView->fALMEngine->CountXTabs(); i++) {
		XTab* tab = fView->fALMEngine->XTabAt(i);
		#if RESIZE_TO_TABS_IN_AREA
		fView->DrawTab(tab, kTabWidth, kSuggestionColor);
		#else
		float tabPosition = tab->Value();
		BRect frame = fSelectedArea->Frame();
		if (tabPosition >= frame.left && tabPosition <= frame.right) {
			fView->SetPenSize(kTabWidth);
			fView->SetHighColor(kSuggestionColor);
			fView->StrokeLine(BPoint(tabPosition, 0), BPoint(tabPosition,
				frame.top - kTabWidth / 2));
			fView->StrokeLine(BPoint(tabPosition, frame.bottom + kTabWidth / 2),
				BPoint(tabPosition, fView->Bounds().Height() - kTabWidth / 2));
		} else
			fView->DrawTab(tab, kTabWidth, kSuggestionColor);
		#endif
	}
}


void
LayoutEditView::ResizeState::_DrawYResizeTabs()
{
	for (int32 i = 0; i < fView->fALMEngine->CountYTabs(); i++) {
		YTab* tab = fView->fALMEngine->YTabAt(i);
		#if RESIZE_TO_TABS_IN_AREA
		fView->DrawTab(tab, kTabWidth, kSuggestionColor);
		#else
		float tabPosition = tab->Value();
		BRect frame = fSelectedArea->Frame();
		if (tabPosition >= frame.top && tabPosition <= frame.bottom) {
			fView->SetPenSize(kTabWidth);
			fView->SetHighColor(kSuggestionColor);
			fView->StrokeLine(BPoint(0, tabPosition),
				BPoint(frame.left - kTabWidth / 2, tabPosition));
			fView->StrokeLine(BPoint(frame.right + kTabWidth / 2, tabPosition),
				BPoint(fView->Bounds().Width() - kTabWidth / 2, tabPosition));
		} else
			fView->DrawTab(tab, kTabWidth, kSuggestionColor);
		#endif
	}
}


void
LayoutEditView::ResizeState::_SelectedTabs(XTab** selectedX, YTab** selectedY)
{
	*selectedX = NULL;
	*selectedY = NULL;
	if (fDragSideX == kLeft)
		*selectedX = fSelectedArea->Left();
	else if (fDragSideX == kRight)
		*selectedX = fSelectedArea->Right();
	if (fDragSideY == kTop)
		*selectedY = fSelectedArea->Top();
	else if (fDragSideY == kBottom)
		*selectedY = fSelectedArea->Bottom();
}

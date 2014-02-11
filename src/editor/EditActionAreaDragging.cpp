/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "EditActionAreaDragging.h"

#include "EditActionInserting.h"
#include "EditActionMoving.h"
#include "EditActionSwapping.h"
#include "GroupDetection.h"


LayoutEditView::DragState::DragState(LayoutEditView* view, Area* selectedArea)
	:
	LayoutEditView::State(view),

	fPerformed(false),

	fSelectedArea(selectedArea),
	fMouseOverArea(NULL),

	fXTab(-1),
	fYTab(-1)
{
}	


bool
LayoutEditView::DragState::MouseMoved(BPoint point, uint32 transit,
	const BMessage* message)
{
	fDragFrame = DragObjectFrame(point, fDropOffset);
	if (transit == B_OUTSIDE_VIEW)
		return true;
	if (transit == B_EXITED_VIEW && fSelectedArea != NULL) {
		CustomizableView* customizable = to_customizable_view(
			fSelectedArea->Item());
		if (customizable != NULL) {
			fView->_SetState(new LayoutEditView::DragOutsideState(fView,
				fSelectedArea, fDropOffset));
			return true;
		}
	}

	fInsertBetweenAlignment = BAlignment(B_ALIGN_USE_FULL_WIDTH,
		B_ALIGN_USE_FULL_HEIGHT);
	BSize prefSize = DragObjectPrefSize();
	BSize minSize = DragObjectMinSize();
	fInsertionIntoEmptyArea.SetTo(fView, prefSize, minSize);
	fInsertionIntoEmptyArea.Reset();
	fXTab = -1;
	fYTab = -1;
	fMouseOverArea = NULL;

	const float kAlignmentThreshold = 10;

	BALMLayout* layout = fView->fALMEngine;
	// first handle insert between tab and area(s)
	XTab* xTab = fView->GetXTabNearPoint(point, kTolerance);
	if (xTab != NULL) {
		Area* area = fView->FindArea(point);
		if (area != NULL) {
			InsertDetection insertDetection(layout,
				fView->Editor()->GetOverlapManager().GetTabConnections());
			insertDetection.GetInsertAreas(xTab, fDragFrame, fAreas,
				fInsertDirection);
			fAreas.RemoveItem(fView->fALMEngine->AreaFor(fView));
			fAreas.RemoveItem(fSelectedArea);
			if (fAreas.CountItems() == 1) {
				BRect frame = fAreas.ItemAt(0)->Frame();
				if (prefSize.height < frame.Height() - kAlignmentThreshold) {
					if (point.y - frame.top < frame.bottom - point.y)
						fInsertBetweenAlignment.SetVertical(B_ALIGN_TOP);
					else
						fInsertBetweenAlignment.SetVertical(B_ALIGN_BOTTOM);
				}
			}
			fXTab = fView->fALMEngine->IndexOf(xTab, true);
			if (fView->TestAction(CreateHInsertionAction())) {
				set_cursor(default_resize_ew_data);
				return true;
			}
			fXTab = -1;
		}
	}

	YTab* yTab = fView->GetYTabNearPoint(point, kTolerance);
	if (yTab != NULL) {
		Area* area = fView->FindArea(point);
		if (area != NULL) {
			InsertDetection insertDetection(layout,
				fView->Editor()->GetOverlapManager().GetTabConnections());
			insertDetection.GetInsertAreas(yTab, fDragFrame, fAreas,
				fInsertDirection);
			fAreas.RemoveItem(fView->fALMEngine->AreaFor(fView));
			fAreas.RemoveItem(fSelectedArea);
			if (fAreas.CountItems() == 1) {
				BRect frame = fAreas.ItemAt(0)->Frame();
				if (prefSize.width < frame.Width() - kAlignmentThreshold) {
					if (point.x - frame.left < frame.right - point.x)
						fInsertBetweenAlignment.SetHorizontal(B_ALIGN_LEFT);
					else
						fInsertBetweenAlignment.SetHorizontal(B_ALIGN_RIGHT);
				}
			}
			fYTab = fView->fALMEngine->IndexOf(yTab, true);
			if (fView->TestAction(CreateVInsertionAction())) {
				set_cursor(default_resize_ns_data);
				return true;
			}
			fYTab = -1;
		}
	}

	set_cursor(kMoveCursor);

	// Can't be insert search for a empty area
	if (fInsertionIntoEmptyArea.FindOptimalArea(point, fDragFrame)) {
		if (fView->TestAction(CreateIntoEmptyAreaAction()))
			return true;
		fInsertionIntoEmptyArea.Reset();
	}

	// swap two areas?
	if (fSelectedArea != NULL) {
		fMouseOverArea = fView->FindArea(point);
		if (fMouseOverArea != NULL && fView->TestAction(CreateSwapAction())
			== true)
			return true;
		fMouseOverArea = NULL;
	}

	return true;
}


void
LayoutEditView::DragState::Draw(BRect updateRect)
{
	// Draw fSelectedArea selection rectangle
	if (fSelectedArea)
		fView->DrawHighLightArea(fSelectedArea, 2, kSelectedColor);
	// swap area
	if (fMouseOverArea)
		fView->DrawHighLightArea(fMouseOverArea, kDestinationPen,
			kDestinationColor);

	// inserting between
	if (fXTab >= 0) {
		XTab* tab = fView->Layout()->XTabAt(fXTab, true);
		if (tab == NULL)
			debugger("Ups");
		fView->DrawTab(tab, kDestinationPen, kDestinationColor);

		for (int32 i = 0; i < fAreas.CountItems(); i++) {
			Area* area = fAreas.ItemAt(i);
			fView->DrawHighLightArea(area, kDestinationPen,
				kDestinationColor);
		}
	} else if (fYTab >= 0) {
		YTab* tab = fView->Layout()->YTabAt(fYTab, true);
		if (tab == NULL)
			debugger("Ups");
		fView->DrawTab(tab, kDestinationPen, kDestinationColor);

		for (int32 i = 0; i < fAreas.CountItems(); i++) {
			Area* area = fAreas.ItemAt(i);
			fView->DrawHighLightArea(area, kDestinationPen,
				kDestinationColor);
		}
	} else if (fInsertionIntoEmptyArea.HasArea())
		fInsertionIntoEmptyArea.Draw();

	fView->SetPenSize(1);
	rgb_color color = {100, 100, 100};
	fView->SetHighColor(color);
	fView->StrokeRect(fDragFrame);
}


bool
LayoutEditView::DragState::MouseUp(BPoint point)
{
	if (fXTab < 0 && fYTab < 0
		&& !fInsertionIntoEmptyArea.HasArea() && fMouseOverArea == NULL)
		return true;

	if (fXTab >= 0) {
		if (fAreas.CountItems() == 0)
			debugger("Invalid Areas.");
		fView->PerformAction(CreateHInsertionAction());
		fPerformed = true;
	} else if (fYTab >= 0) {
		if (fAreas.CountItems() == 0)
			debugger("Invalid Areas.");
		fView->PerformAction(CreateVInsertionAction());
		fPerformed = true;
	} else if (fInsertionIntoEmptyArea.HasArea()) {
		fView->PerformAction(CreateIntoEmptyAreaAction());
		fPerformed = true;
	} else if (fSelectedArea != NULL && fMouseOverArea != NULL) {
		fView->PerformAction(CreateSwapAction());
		fPerformed = true;
	}

	fXTab = -1;
	fYTab = -1;
	return true;
}


BRect
LayoutEditView::DragState::DragObjectFrame(BPoint mousePosition,
	BPoint dragOffset)
{
	BRect frame = BALM::EditorHelper::GetDragFrame(DragObjectPrefSize(),
		DragObjectMinSize());

	frame.OffsetTo(mousePosition.x - frame.Width() / 2,
		mousePosition.y - frame.Height() / 2);
	return frame;
}


BRect
LayoutEditView::DragAreaState::DragObjectFrame(BPoint mousePosition,
	BPoint dragOffset)
{
	BRect frame = fSelectedArea->Frame();
	frame.OffsetTo(mousePosition);
	frame.OffsetBy(-dragOffset);

	// reduce to pref drag frame
	BRect prefFrame = BALM::EditorHelper::GetDragFrame(DragObjectPrefSize(),
		DragObjectMinSize());
	BPoint inset;
	if (frame.Width() > prefFrame.Width())
		inset.x = (frame.Width() - prefFrame.Width()) / 2;
	if (frame.Height() > prefFrame.Height())
		inset.y = (frame.Height() - prefFrame.Height()) / 2;
	frame.InsetBy(inset);
	if (!frame.Contains(mousePosition)) {
		frame = prefFrame;
		frame.OffsetTo(mousePosition.x - frame.Width() / 2,
			mousePosition.y - frame.Height() / 2);
	}

	return frame;
}


BSize
LayoutEditView::DragAreaState::DragObjectPrefSize()
{
	return fSelectedArea->Item()->PreferredSize();
}


BSize
LayoutEditView::DragAreaState::DragObjectMinSize()
{
	return fSelectedArea->Item()->MinSize();
}


EditAction*
LayoutEditView::DragAreaState::CreateHInsertionAction()
{
	return new InsertAreaHorizontalAction(fView->fALMEngine,
		fView->CurrentLayout(), fSelectedArea, fXTab, fAreas,
		fInsertDirection, fInsertBetweenAlignment.Vertical());
}


EditAction*
LayoutEditView::DragAreaState::CreateVInsertionAction()
{
	return new InsertAreaVerticalAction(fView->fALMEngine,
		fView->CurrentLayout(), fSelectedArea, fYTab, fAreas,
		fInsertDirection, fInsertBetweenAlignment.Horizontal());
}


EditAction*
LayoutEditView::DragAreaState::CreateIntoEmptyAreaAction()
{
	return new MoveAction(fView->fALMEngine, fView->CurrentLayout(),
		fSelectedArea, fInsertionIntoEmptyArea);
}


EditAction*
LayoutEditView::DragAreaState::CreateSwapAction()
{
	return new SwapAction(fView->fALMEngine, fView->CurrentLayout(),
		fSelectedArea, fMouseOverArea);
}

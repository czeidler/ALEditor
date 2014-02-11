/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "EditAction.h"

#include <Misc.h>

#include <ALMEditor.h>


using namespace BALM;


bool BALM::fuzzy_equal(float x1, float x2)
{
	if (fabs(x1 - x2) < 0.1)
		return true;
	return false;
}


class InnerAreaTabs {
public:
	InnerAreaTabs(TabConnections* connections)
		:
		fConnections(connections)
	{
	}

	XTab* FindHorizontalInnerTab(float pos, float tolerance, area_ref& area)
	{
		XTab* tab1;
		float dist1 = _FindInnerTab<XTab, YTab, top_search>(pos, tolerance,
			&tab1, area.top);

		XTab* tab2;
		float dist2 = _FindInnerTab<XTab, YTab, bottom_search>(pos, tolerance,
			&tab2, area.bottom);

		if (dist1 < dist2)
			return tab1;
		return tab2;
	}

	YTab* FindVerticalInnerTab(float pos, float tolerance, area_ref& area)
	{
		YTab* tab1;
		float dist1 = _FindInnerTab<YTab, XTab, left_search>(pos, tolerance,
			&tab1, area.left);

		YTab* tab2;
		float dist2 = _FindInnerTab<YTab, XTab, right_search>(pos, tolerance,
			&tab2, area.right);

		if (dist1 < dist2)
			return tab1;
		return tab2;
	}

private:
	template<class TabType, class OrthTab, typename SearchDirection>
	float _FindInnerTab(float pos, float tolerance, TabType** tab,
		OrthTab* border)
	{
		*tab = NULL;
		float minDist = HUGE_VAL;

		SearchDirection search;
		std::map<OrthTab*, tab_links<OrthTab> >& linkMap
			= search.LinkMap(fConnections);
		tab_links<OrthTab>& links = linkMap[border];

		BObjectList<Area>& areas = search.AreasSearchDirection(links);
		for (int32 i = 0; i < areas.CountItems(); i++) {
			Area* area = areas.ItemAt(i);
			float dist1 = fabs(pos - search.OrthTab1(area)->Value());
			float dist2 = fabs(pos - search.OrthTab2(area)->Value());
			TabType* bestTab = NULL;
			if (dist1 > tolerance && dist2 > tolerance)
				continue;
			float min;
			if (dist1 < dist2) {
				min = dist1;
				bestTab = search.OrthTab1(area);
			} else {
				min = dist2;
				bestTab = search.OrthTab2(area);
			}
			if (min < minDist) {
				minDist = min;
				*tab = bestTab;
			}
		}
		return minDist;
	}

private:
			TabConnections*		fConnections;
};


InsertionIntoEmptyArea::InsertionIntoEmptyArea()
	:
	fView(NULL)
{
	Reset();	
}


void
InsertionIntoEmptyArea::SetTo(LayoutEditView* view, BSize prefSize,
	BSize minSize)
{
	fView = view;
	fPreferredSize = prefSize;
	fMinSize = minSize;
}


void
InsertionIntoEmptyArea::Reset()
{
	fEmptyArea.Unset();
	fAvailableArea.Unset();
	fFreePosition = BPoint(-1, -1);
}


bool
InsertionIntoEmptyArea::HasArea()
{
	// left and right == NULL and position set -> free insertion
	// left or right set -> insertion at existing tabs
	if (fEmptyArea.left >= 0 || fEmptyArea.right >= 0)
		return true;
	if (fFreePosition.x >= 0)
		return true;
	return false;
}


bool
InsertionIntoEmptyArea::FindOptimalArea(BPoint point, BRect dragFrame,
	Area* movedArea)
{
	BALM::area_ref emptyArea;
	BALM::area_ref availableSpace;
	bool result = FindOptimalAreaFor(point, dragFrame, fPreferredSize,
		fMinSize, emptyArea, availableSpace, movedArea);
	if (!result)
		 return false;

	fEmptyArea.SetTo(emptyArea, fView->Layout());
	fAvailableArea.SetTo(availableSpace, fView->Layout());
	fFreePosition = dragFrame.LeftTop();
	return true;
}


void
InsertionIntoEmptyArea::Draw()
{
	area_ref availableAreaRef = fAvailableArea.to_ref(fView->Layout());
	fView->DrawAreaWithInnerTabs(availableAreaRef);

	BRect frame = TargetFrame();

	fView->SetHighColor(kMarkedTabColor);
	fView->SetPenSize(1);
	area_ref ref = fEmptyArea.to_ref(fView->Layout());
	BRect outerFrame = availableAreaRef.Frame();
	if (ref.left != NULL) {
		fView->StrokeLine(BPoint(ref.left->Value(), outerFrame.top),
			BPoint(ref.left->Value(), outerFrame.bottom));
	}
	if (ref.right != NULL) {
		fView->StrokeLine(BPoint(ref.right->Value(), outerFrame.top),
			BPoint(ref.right->Value(), outerFrame.bottom));
	}
	if (ref.top != NULL) {
		fView->StrokeLine(BPoint(outerFrame.left, ref.top->Value()),
			BPoint(outerFrame.right, ref.top->Value()));
	}
	if (ref.bottom != NULL) {
		fView->StrokeLine(BPoint(outerFrame.left, ref.bottom->Value()),
			BPoint(outerFrame.right, ref.bottom->Value()));
	}	

	fView->DrawHighLightArea(frame, kDestinationPen, kDestinationColor);
}


area_ref
InsertionIntoEmptyArea::CreateTargetArea()
{
	BALMLayout* layout = fView->Layout();
	area_ref ref = fEmptyArea.to_ref(layout);

	BRect frame = TargetFrame();
	
	if (ref.left == NULL) {
		ref.left = layout->AddXTab();
		ref.left->SetValue(frame.left);
	}
	if (ref.top == NULL) {
		ref.top = layout->AddYTab();
		ref.top->SetValue(frame.top);
	}
	if (ref.right == NULL) {
		ref.right = layout->AddXTab();
		ref.right->SetValue(frame.right);
	}
	if (ref.bottom == NULL) {
		ref.bottom = layout->AddYTab();
		ref.bottom->SetValue(frame.bottom);
	}

	const double kPenalty = 5;
	// free positioning at freePosition
	if (fEmptyArea.left == -1 && fEmptyArea.right == -1) {
		Constraint* constraint = layout->AddConstraint(1, ref.left,
			LinearProgramming::kEQ, fFreePosition.x, kPenalty, kPenalty);
		constraint->SetLabel("_EditHelper");
	}
	if (fEmptyArea.top == -1 && fEmptyArea.bottom == -1) {
		Constraint* constraint = layout->AddConstraint(1, ref.top,
			LinearProgramming::kEQ, fFreePosition.y, kPenalty, kPenalty);
		constraint->SetLabel("_EditHelper");
	}
	return ref;
}


BRect
InsertionIntoEmptyArea::TargetFrame()
{
	float xSpacing, ySpacing;
	fView->Layout()->GetSpacing(&xSpacing, &ySpacing);

	area_ref emptyAreaRef = fEmptyArea.to_ref(fView->Layout());

	BRect frame;
	if (emptyAreaRef.left == NULL && emptyAreaRef.right == NULL) {
		frame.left = fFreePosition.x;
		frame.right = frame.left + (fPreferredSize.width + xSpacing);
	} else {
		if (emptyAreaRef.left != NULL)
			frame.left = emptyAreaRef.left->Value();
		if (emptyAreaRef.right != NULL)
			frame.right = emptyAreaRef.right->Value();
		if (emptyAreaRef.left == NULL)
			frame.left = frame.right - (fPreferredSize.width + xSpacing);
		else if (emptyAreaRef.right == NULL)
			frame.right = frame.left + (fPreferredSize.width + xSpacing);
	}
	if (emptyAreaRef.top == NULL && emptyAreaRef.bottom == NULL) {
		frame.top = fFreePosition.y;
		frame.bottom = frame.top + (fPreferredSize.height + ySpacing);
	} else {
		if (emptyAreaRef.top != NULL)
			frame.top = emptyAreaRef.top->Value();
		if (emptyAreaRef.bottom != NULL)
			frame.bottom = emptyAreaRef.bottom->Value();
		if (emptyAreaRef.top == NULL)
			frame.top = frame.bottom - (fPreferredSize.height + ySpacing);
		else if (emptyAreaRef.bottom == NULL)
			frame.bottom = frame.top + (fPreferredSize.height + ySpacing);
	}
	return frame;
}


bool
InsertionIntoEmptyArea::FindOptimalAreaFor(const BPoint& point,	BRect dragFrame,
	BSize target, BSize minSize, area_ref& ref, area_ref& maximalArea,
	Area* movedArea)
{
	if (!fView->FindEmptyArea(point, maximalArea, movedArea))
		return false;

	BRect currentFrame;
	if (movedArea != NULL)
		currentFrame = movedArea->Frame();
	MaximizeEmptyArea(maximalArea, dragFrame, currentFrame);

	ref = maximalArea;

	const float kSnapDistance = 10;

	// first reduce the area again if we are close to tab
	BRect emptyFrame = ref.Frame();
	float interestingWidth = dragFrame.Width();
	if (target.width >= 0 && emptyFrame.Width() > interestingWidth
		&& dragFrame.right + kSnapDistance < emptyFrame.right
		&& dragFrame.left - kSnapDistance > emptyFrame.left) {
		XTab* betterLeft = fView->GetXTabNearPoint(dragFrame.LeftTop(),
			kSnapDistance);
		if (betterLeft != NULL && (movedArea == NULL
			|| (betterLeft != movedArea->Left()
				&& betterLeft != movedArea->Right()))) {
			if (emptyFrame.right - betterLeft->Value() >= interestingWidth)
				ref.left = betterLeft;
		} else {
			XTab* betterRight = fView->GetXTabNearPoint(dragFrame.RightTop(),
				kSnapDistance);
			if (betterRight != NULL && (movedArea == NULL
				|| (betterRight != movedArea->Left()
					&& betterRight != movedArea->Right()))) {
				if (betterRight->Value() - emptyFrame.left >= interestingWidth)
					ref.right = betterRight;
			}
		}
	}
	float interestingHeight = dragFrame.Height();
	if (target.height >= 0 && emptyFrame.Height() > interestingHeight
		&& dragFrame.bottom + kSnapDistance < emptyFrame.bottom
		&& dragFrame.top - kSnapDistance > emptyFrame.top) {
		YTab* betterTop = fView->GetYTabNearPoint(dragFrame.LeftTop(),
			kSnapDistance);
		if (betterTop != NULL && (movedArea == NULL
			|| (betterTop != movedArea->Top()
			&& betterTop != movedArea->Bottom()))) {
			if (emptyFrame.bottom - betterTop->Value() >= interestingHeight)
				ref.top = betterTop;
		} else {
			YTab* betterBottom = fView->GetYTabNearPoint(dragFrame.RightBottom(),
				kSnapDistance);
			if (betterBottom != NULL && (movedArea == NULL
				|| (betterBottom != movedArea->Top()
					&& betterBottom != movedArea->Bottom()))) {
				if (betterBottom->Value() - emptyFrame.top >= interestingHeight)
					ref.bottom = betterBottom;
			}
		}
	}

	bool xSnapped = true;
	bool ySnapped = true;

	const float kNewTabSnapDistance = 15;
	// snap to area border
	emptyFrame = ref.Frame();
	InnerAreaTabs innerAreaTabs(fView->fOverlapManager.GetTabConnections());
	if (target.width >= 0
		&& emptyFrame.Width() - target.width > kNewTabSnapDistance) {
		if (dragFrame.left - emptyFrame.left < kNewTabSnapDistance) {
			ref.right = innerAreaTabs.FindHorizontalInnerTab(
				emptyFrame.left + target.width, kNewTabSnapDistance,
				maximalArea);
			if (ref.right != NULL && ref.right->Value() <= ref.left->Value())
				ref.right = NULL;
		} else if (emptyFrame.right - dragFrame.right < kNewTabSnapDistance) {
			ref.left = innerAreaTabs.FindHorizontalInnerTab(
				emptyFrame.right - target.width, kNewTabSnapDistance,
				maximalArea);
			if (ref.left != NULL && ref.right->Value() <= ref.left->Value())
				ref.left = NULL;
		} else
			xSnapped = false;
	}

	if (target.height >= 0
		&& emptyFrame.Height() - target.height > kNewTabSnapDistance) {
		if (dragFrame.top - emptyFrame.top < kNewTabSnapDistance) {
			ref.bottom = innerAreaTabs.FindVerticalInnerTab(
				emptyFrame.top + target.height, kNewTabSnapDistance,
				maximalArea);
			if (ref.bottom != NULL && ref.bottom->Value() <= ref.top->Value())
				ref.bottom = NULL;
		} else if (emptyFrame.bottom - dragFrame.bottom < kNewTabSnapDistance) {
			ref.top = innerAreaTabs.FindVerticalInnerTab(
				emptyFrame.bottom - target.height, kNewTabSnapDistance,
				maximalArea);
			if (ref.top != NULL && ref.bottom->Value() <= ref.top->Value())
				ref.top = NULL;
		} else
			ySnapped = false;
	}

	//free placement
	if (!fView->Editor()->FreePlacement())
		return true;

	if (!xSnapped) {
		ref.left = NULL;
		ref.right = NULL;
	}
	if (!ySnapped) {
		ref.top = NULL;
		ref.bottom = NULL;
	}

	return true;
}


void
InsertionIntoEmptyArea::MaximizeEmptyArea(area_ref& ref, BRect target, BRect ignore)
{
	BALMLayout* layout = fView->Layout();
	BRegion takenSpace = fView->fTakenSpace;
	takenSpace.Exclude(ignore);
	area_info areaInfo(ref, layout);
	// try to match the target

	XTab* left = ref.left;
	while (left->Value() > target.left) {
		left = layout->XTabAt(areaInfo.left - 1, true);
		if (left == NULL)
			break;
		area_ref newRef = ref;
		newRef.left = left;
		BRect frame = newRef.Frame();
		frame.InsetBy(1, 1);
		if (takenSpace.Intersects(frame))
			break;
		areaInfo.left -= 1;
		ref.left = left;
	}
	XTab* right = ref.right;
	while (right->Value() < target.right) {
		right = layout->XTabAt(areaInfo.right + 1, true);
		if (right == NULL)
			break;
		area_ref newRef = ref;
		newRef.right = right;
		BRect frame = newRef.Frame();
		frame.InsetBy(1, 1);
		if (takenSpace.Intersects(frame))
			break;
		areaInfo.right += 1;
		ref.right = right;
	}
	YTab* top = ref.top;
	while (top->Value() > target.top) {
		top = layout->YTabAt(areaInfo.top - 1, true);
		if (top == NULL)
			break;
		area_ref newRef = ref;
		newRef.top = top;
		BRect frame = newRef.Frame();
		frame.InsetBy(1, 1);
		if (takenSpace.Intersects(frame))
			break;
		areaInfo.top -= 1;
		ref.top = top;
	}
	YTab* bottom = ref.bottom;
	while (bottom->Value() < target.bottom) {
		bottom = layout->YTabAt(areaInfo.bottom + 1, true);
		if (bottom == NULL)
			break;
		area_ref newRef = ref;
		newRef.bottom = bottom;
		BRect frame = newRef.Frame();
		frame.InsetBy(1, 1);
		if (takenSpace.Intersects(frame))
			break;
		areaInfo.bottom += 1;
		ref.bottom = bottom;
	}

	// maximize further
	while (true) {
		BRect previousFrame = ref.Frame();
		
		left = layout->XTabAt(areaInfo.left - 1, true);
		right = layout->XTabAt(areaInfo.right + 1, true);
		top = layout->YTabAt(areaInfo.top - 1, true);
		bottom = layout->YTabAt(areaInfo.bottom + 1, true);

		float leftDelta = -1;
		if (left != NULL) {
			BRect newFrame(previousFrame);
			newFrame.left = left->Value();
			newFrame.InsetBy(1, 1);
			if (!takenSpace.Intersects(newFrame)) {
				leftDelta = previousFrame.Height()
					* (previousFrame.left - left->Value());
			}
		}
		float rightDelta = -1;
		if (right != NULL) {
			BRect newFrame(previousFrame);
			newFrame.right = right->Value();
			newFrame.InsetBy(1, 1);
			if (!takenSpace.Intersects(newFrame)) {
				rightDelta = previousFrame.Height()
					* (right->Value() - previousFrame.right);
			}
		}
		float topDelta = -1;
		if (top != NULL) {
			BRect newFrame(previousFrame);
			newFrame.top = top->Value();
			newFrame.InsetBy(1, 1);
			if (!takenSpace.Intersects(newFrame)) {
				topDelta = previousFrame.Width()
					* (previousFrame.top - top->Value());
			}
		}
		float bottomDelta = -1;
		if (bottom != NULL) {
			BRect newFrame(previousFrame);
			newFrame.bottom = bottom->Value();
			newFrame.InsetBy(1, 1);
			if (!takenSpace.Intersects(newFrame)) {
				bottomDelta = previousFrame.Width()
					* (bottom->Value() - previousFrame.bottom);
			}
		}
		if (leftDelta > 0 && leftDelta > rightDelta && leftDelta > topDelta
			&& leftDelta > bottomDelta) {
			ref.left = left;
			areaInfo.left = layout->IndexOf(left, true);
		} else if (rightDelta > 0 && rightDelta > topDelta
			&& rightDelta > bottomDelta) {
			ref.right = right;
			areaInfo.right = layout->IndexOf(right, true);
		} else if (topDelta > 0 && topDelta > bottomDelta) {
			ref.top = top;
			areaInfo.top = layout->IndexOf(top, true);
		} else if (bottomDelta > 0) {
			ref.bottom = bottom;
			areaInfo.bottom = layout->IndexOf(bottom, true);
		}

		BRect newFrame = ref.Frame();
		if (fuzzy_equal(previousFrame.Width(), newFrame.Width())
			&& fuzzy_equal(previousFrame.Height(), newFrame.Height()))
			break;
	}
}


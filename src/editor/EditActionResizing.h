/*
* Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	EDIT_ACTION_RESIZING_H
#define	EDIT_ACTION_RESIZING_H


#include <ALMEditor.h>

#include "EditAction.h"
#include "GroupDetection.h"
#include "LayoutEditView.h"


using namespace BALM;


const float kDetachThreshold = 10;
const float kResizeWidth = 2;


class ResizeAction : public EditAction {
public:
	ResizeAction(BALMLayout* layout, BMessage* prevLayout, Area* area,
		XTab* xFrom, YTab* yFrom, int32 xTo, int32 yTo)
		:
		EditAction(layout, prevLayout),
		fFromArea(area),
		fFromXTab(layout->IndexOf(xFrom, true)),
		fFromYTab(layout->IndexOf(yFrom, true)), 
		fToXTab(xTo),
		fToYTab(yTo)
	{
	}

	virtual bool
	Perform()
	{
		TabConnections connections;
		AreaRemoval areaRemoval(&connections, fALMLayout);

		BReference<XTab> xFrom = fALMLayout->XTabAt(fFromXTab, true);
		BReference<YTab> yFrom = fALMLayout->YTabAt(fFromYTab, true);
		BReference<XTab> xTo = fALMLayout->XTabAt(fToXTab, true);
		BReference<YTab> yTo = fALMLayout->YTabAt(fToYTab, true);
		// If there is XTab selected, replace it with targetX
		if (xFrom != NULL && xTo != NULL) {
			XTab* oldTab = NULL;
			if (xFrom.Get() == fFromArea->Left()) {
				oldTab = fFromArea->Left();
				fFromArea->SetLeft(xTo);
				areaRemoval.AreaDetachedX(fFromArea, kLeft, oldTab);
			} else {
				oldTab = fFromArea->Right();
				fFromArea->SetRight(xTo);
				areaRemoval.AreaDetachedX(fFromArea, kRight, oldTab);
			}
		}

		// If there is YTab selected, replace it with targetY
		if (yFrom != NULL && yTo != NULL) {
			YTab* oldTab = NULL;
			if (yFrom.Get() == fFromArea->Top()) {
				oldTab = fFromArea->Top();
				fFromArea->SetTop(yTo);
				areaRemoval.AreaDetachedY(fFromArea, kTop, oldTab);
			} else {
				oldTab = fFromArea->Bottom();
				fFromArea->SetBottom(yTo);
				areaRemoval.AreaDetachedY(fFromArea, kBottom, oldTab);
			}
		}

		return true;
	}

protected:
			Area*				fFromArea;
			int32				fFromXTab;
			int32				fFromYTab;
			int32				fToXTab;
			int32				fToYTab;
};


class HardResizeAction : public EditAction {
public:
	HardResizeAction(BALMLayout* layout, BMessage* prevLayout, Area* area,
		area_side side, float tabPosition)
		:
		EditAction(layout, prevLayout),
		fArea(area),
		fSide(side),
		fTabPosition(tabPosition)
	{
	}

	virtual bool Perform()
	{
		Constraint* constraint = NULL;
		if (fSide == kLeft) {
			BReference<XTab> tab = fALMLayout->AddXTab();
			fArea->SetLeft(tab);
			tab->SetValue(fArea->Right()->Value());
			float extend = fArea->Right()->Value() - fTabPosition;
			constraint = fALMLayout->AddConstraint(1, fArea->Right(), -1,
				fArea->Left(), LinearProgramming::kEQ, extend);
		}
		if (fSide == kRight) {
			BReference<XTab> tab = fALMLayout->AddXTab();
			fArea->SetRight(tab);
			tab->SetValue(fArea->Left()->Value());
			float extend = fTabPosition - fArea->Left()->Value();
			constraint = fALMLayout->AddConstraint(1, fArea->Right(), -1,
				fArea->Left(), LinearProgramming::kEQ, extend);
		}
		if (fSide == kTop) {
			BReference<YTab> tab = fALMLayout->AddYTab();
			fArea->SetTop(tab);
			tab->SetValue(fArea->Bottom()->Value());
			float extend = fArea->Bottom()->Value() - fTabPosition;
			constraint = fALMLayout->AddConstraint(1, fArea->Bottom(), -1,
				fArea->Top(), LinearProgramming::kEQ, extend);
		}
		if (fSide == kBottom) {
			BReference<YTab> tab = fALMLayout->AddYTab();
			fArea->SetBottom(tab);
			tab->SetValue(fArea->Top()->Value());
			float extend = fTabPosition - fArea->Top()->Value();
			constraint = fALMLayout->AddConstraint(1, fArea->Bottom(), -1,
				fArea->Top(), LinearProgramming::kEQ, extend);
		}
		if (constraint != NULL)
			constraint->SetLabel("_EditHelper");
		return true;
	}

private:
			Area*				fArea;
			area_side			fSide;
			float				fTabPosition;
};


template <class TYPE>
class InsertTabAction : public EditAction {
public:
	InsertTabAction(BALMLayout* layout, BMessage* prevLayout, Area* area,
		area_side side)
		:
		EditAction(layout, prevLayout),
		fArea(area),
		fSide(side)
	{
	}

	virtual bool Perform();

protected:
			Area*				fArea;
			area_side			fSide;
};


template <class TYPE>
class ResizeGroupAction : public EditAction {
public:
	ResizeGroupAction(BALMLayout* layout, BMessage* prevLayout,
		BObjectList<Area>& areas, area_side side, int32 toTab)
		:
		EditAction(layout, prevLayout),
		fAreas(areas),
		fSide(side),
		fMoveToTab(toTab)
	{
	}

	virtual bool Perform();

protected:
			BObjectList<Area>	fAreas;
			area_side			fSide;
			int32				fMoveToTab;
};


template <class TYPE>
class InsertGroupTabAction : public EditAction {
public:
	InsertGroupTabAction(BALMLayout* layout, BMessage* prevLayout,
		BObjectList<Area>& areas, area_side side)
		:
		EditAction(layout, prevLayout),
		fAreas(areas),
		fSide(side)
	{
	}

	virtual bool Perform();

protected:
			BObjectList<Area>	fAreas;
			area_side			fSide;
};


class LayoutEditView::ResizeState : public State {
public:
								ResizeState(LayoutEditView* view, Area* area,
									area_side xSide, area_side ySide);
	

	virtual void				EnterState();
	virtual bool				MouseUp(BPoint point);
	virtual bool				MouseMoved(BPoint point, uint32 transit,
									const BMessage* message);
	virtual void				Draw(BRect updateRect);

protected:
			void				_DrawXResizeTabs();
			void				_DrawYResizeTabs();
	
			void				_SelectedTabs(XTab** selectedX,
									YTab** selectedY);

			Area*				fSelectedArea;
			area_side			fDragSideX;
			area_side			fDragSideY;
			int32				fMouseOverXTab;
			int32				fMouseOverYTab;

			bool				fDetachX;
			bool				fDetachY;
			float				fHardResizeToX;
			float				fHardResizeToY;

			bool				fShowXTabs;
			bool				fShowYTabs;

	const	uint8*				fResizeCursor;
	const	uint8*				fDetachCursor;
};


class LayoutEditView::DragXTabState : public State {
public:
	DragXTabState(LayoutEditView* view, int32 tabIndex, Area* seed)
		:
		State(view),
		fSelectedTab(tabIndex),
		fSeedArea(seed)
	{
		XTab* tab = fView->fALMEngine->XTabAt(fSelectedTab, true);
		if (fSeedArea->Left() == tab)
			fSide = kLeft;
		else
			fSide = kRight;
		GroupDetection groupDetection(
			fView->fOverlapManager.GetTabConnections());
		groupDetection.FindAdjacentAreas(fSeedArea, fSide, fAreaGroup);
	}

	virtual void EnterState()
	{
		fView->SetShowXTabs(true);
		set_cursor(default_resize_ew_data);
	}

	virtual bool MouseMoved(BPoint point, uint32 transit,
		const BMessage* message)
	{
		// check if we should detach the tab
		fDetach = false;
		fLastPoint = point;
		XTab* selectedTab = fView->fALMEngine->XTabAt(fSelectedTab, true);

		if (_GroupConnectedToBorders()) {
			if (fSide == kLeft
				&& point.x - selectedTab->Value() >  kDetachThreshold)
				fDetach = true;
			else if (fSide == kRight
				&& selectedTab->Value() - point.x >  kDetachThreshold)
				fDetach = true;
		}

		if (fDetach) {
			if (fView->TestAction(new InsertGroupTabAction<XTab>(
				fView->fALMEngine, fView->CurrentLayout(), fAreaGroup, fSide))
					== false)
				fDetach = false;
		}

		if (fDetach)
			return true;

		// connect?
		fConnectToTab = -1;
		XTab* xTab = fView->GetBestXTab(selectedTab, point);
		if (xTab == NULL || xTab == selectedTab)
			return true;
		BRect increase;
		increase.top = ceilf(GroupDetection::Top(fAreaGroup)->Value());
		increase.bottom = floorf(GroupDetection::Bottom(fAreaGroup)->Value());
		if (fSide == kLeft) {
			if (xTab->Value() <= selectedTab->Value()) {
				increase.left = ceilf(xTab->Value());
				increase.right = floorf(selectedTab->Value());
				increase.InsetBy(1, 1);
				if (fView->fTakenSpace.Intersects(increase) == true)
					xTab = NULL;
			} else if (xTab->Value() >= GroupDetection::LeftmostRight(
				fAreaGroup)->Value())
				xTab = NULL;
		} else if (fSide == kRight) {
			if (xTab->Value() >= selectedTab->Value()) {
				increase.left = ceilf(selectedTab->Value());
				increase.right = floorf(xTab->Value());
				increase.InsetBy(1, 1);
				if (fView->fTakenSpace.Intersects(increase) == true)
					xTab = NULL;
			} else if (xTab->Value() <= GroupDetection::RightmostLeft(
				fAreaGroup)->Value())
				xTab = NULL;
		}
		if (xTab == NULL)
			return false;

		fConnectToTab = fView->fALMEngine->IndexOf(xTab, true);
		// and test if solvable
		if (fView->TestAction(new ResizeGroupAction<XTab>(fView->fALMEngine,
			fView->CurrentLayout(), fAreaGroup, fSide, fConnectToTab)) == false)
			fConnectToTab = -1;

		return true;
	}

	virtual bool MouseUp(BPoint point)
	{
		if (fDetach) {
			fView->PerformAction(new InsertGroupTabAction<XTab>(
				fView->fALMEngine, fView->CurrentLayout(), fAreaGroup, fSide));
		} else if (fConnectToTab >= 0) {
			fView->PerformAction(new ResizeGroupAction<XTab>(fView->fALMEngine,
				fView->CurrentLayout(), fAreaGroup, fSide, fConnectToTab));
		}

		BALMEditor* editor = fView->Editor();
		fView->SetShowXTabs(editor->ShowXTabs());
		fView->SetShowYTabs(editor->ShowYTabs());
		return true;
	}

	virtual void Draw(BRect updateRect)
	{
		XTab* selectedTab = fView->fALMEngine->XTabAt(fSelectedTab, true);
		fView->DrawTab(selectedTab, 2, kSelectedColor);

		if (fDetach) {
			fView->StrokeLine(BPoint(fLastPoint.x,
				fView->fALMEngine->Top()->Value()),
				BPoint(fLastPoint.x, fView->fALMEngine->Bottom()->Value()));	
		} else {
			XTab* tab = fView->fALMEngine->XTabAt(fConnectToTab, true);
			if (tab != NULL)
				fView->DrawTab(tab, kResizeWidth, kDestinationColor);
		}
	}

protected:
	virtual bool _GroupConnectedToBorders()
	{
		bool leftConnected = false;
		bool rightConnected = false;
		for (int32 i = 0; i < fAreaGroup.CountItems(); i++) {
			Area* area = fAreaGroup.ItemAt(i);
			if (!leftConnected && fView->ConnectedToLeftBorder(area))
				leftConnected = true;
			if (!rightConnected && fView->ConnectedToRightBorder(area))
				rightConnected = true;
			if (leftConnected && rightConnected)
				return true;
		}
		return false;
	}

			int32				fSelectedTab;
			Area*				fSeedArea;
			area_side			fSide;

			BObjectList<Area>	fAreaGroup;

			bool				fDetach;
			BPoint				fLastPoint;

			int32				fConnectToTab;
};


class LayoutEditView::DragYTabState : public State {
public:
	DragYTabState(LayoutEditView* view, int32 tabIndex, Area* seed)
		:
		State(view),
		fSelectedTab(tabIndex),
		fSeedArea(seed)
	{
		YTab* tab = fView->fALMEngine->YTabAt(tabIndex, true);
		if (fSeedArea->Top() == tab)
			fSide = kTop;
		else
			fSide = kBottom;
		GroupDetection groupDetection(
			fView->fOverlapManager.GetTabConnections());
		groupDetection.FindAdjacentAreas(fSeedArea, fSide, fAreaGroup);
	}

	virtual void EnterState()
	{
		fView->SetShowYTabs(true);
		set_cursor(default_resize_ns_data);
	}

	virtual bool MouseMoved(BPoint point, uint32 transit,
		const BMessage* message)
	{
		// check if we should detach the tab
		fDetach = false;
		fLastPoint = point;
		YTab* selectedTab = fView->fALMEngine->YTabAt(fSelectedTab, true);

		if (_GroupConnectedToBorders()) {
			if (fSide == kTop
				&& point.y - selectedTab->Value() >  kDetachThreshold)
				fDetach = true;
			else if (fSide == kBottom
				&& selectedTab->Value() - point.y >  kDetachThreshold)
				fDetach = true;
		}

		if (fDetach) {
			if (fView->TestAction(new InsertGroupTabAction<YTab>(
				fView->fALMEngine, fView->CurrentLayout(), fAreaGroup, fSide))
					== false)
				fDetach = false;
		}

		if (fDetach)
			return true;

		// connect?
		fConnectToTab = -1;
		YTab* yTab = fView->GetBestYTab(selectedTab, point);
		if (yTab == NULL || yTab == selectedTab)
			return true;
		BRect increase;
		increase.left = ceilf(GroupDetection::Left(fAreaGroup)->Value());
		increase.right = floorf(GroupDetection::Right(fAreaGroup)->Value());
		if (fSide == kTop) {
			if (yTab->Value() <= selectedTab->Value()) {
				increase.top = ceilf(yTab->Value());
				increase.bottom = floorf(selectedTab->Value());
				increase.InsetBy(1, 1);
				if (fView->fTakenSpace.Intersects(increase) == true)
					yTab = NULL;
			} else if (yTab->Value() >= GroupDetection::TopmostBottom(
				fAreaGroup)->Value())
				yTab = NULL;
		} else if (fSide == kBottom) {
			if (yTab->Value() >= selectedTab->Value()) {
				increase.top = ceilf(selectedTab->Value());
				increase.bottom = floorf(yTab->Value());
				increase.InsetBy(1, 1);
				if (fView->fTakenSpace.Intersects(increase) == true)
					yTab = NULL;
			} else if (yTab->Value() <= GroupDetection::BottommostTop(
				fAreaGroup)->Value())
				yTab = NULL;
		}
		if (yTab == NULL)
			return false;

		fConnectToTab = fView->fALMEngine->IndexOf(yTab, true);
		// and test if solvable
		if (fView->TestAction(new ResizeGroupAction<YTab>(fView->fALMEngine,
			fView->CurrentLayout(), fAreaGroup, fSide, fConnectToTab)) == false)
			fConnectToTab = -1;

		return true;
	}

	virtual bool MouseUp(BPoint point)
	{
		if (fDetach) {
			fView->PerformAction(new InsertGroupTabAction<YTab>(
				fView->fALMEngine, fView->CurrentLayout(), fAreaGroup, fSide));
		} else if (fConnectToTab >= 0) {
			fView->PerformAction(new ResizeGroupAction<YTab>(fView->fALMEngine,
				fView->CurrentLayout(), fAreaGroup, fSide, fConnectToTab));
		}

		BALMEditor* editor = fView->Editor();
		fView->SetShowXTabs(editor->ShowXTabs());
		fView->SetShowYTabs(editor->ShowYTabs());
		return true;
	}

	virtual void Draw(BRect updateRect)
	{
		YTab* selectedTab = fView->fALMEngine->YTabAt(fSelectedTab, true);
		fView->DrawTab(selectedTab, 2, kSelectedColor);

		if (fDetach) {
			fView->StrokeLine(BPoint(fView->fALMEngine->Left()->Value(),
				fLastPoint.y),
				BPoint(fView->fALMEngine->Right()->Value(), fLastPoint.y));
		} else {
			YTab* tab = fView->fALMEngine->YTabAt(fConnectToTab, true);
			if (tab != NULL)
				fView->DrawTab(tab, kResizeWidth, kDestinationColor);
		}
	}

protected:
	virtual bool _GroupConnectedToBorders()
	{
		bool topConnected = false;
		bool bottomConnected = false;
		for (int32 i = 0; i < fAreaGroup.CountItems(); i++) {
			Area* area = fAreaGroup.ItemAt(i);
			if (!topConnected && fView->ConnectedToTopBorder(area))
				topConnected = true;
			if (!bottomConnected && fView->ConnectedToBottomBorder(area))
				bottomConnected = true;
			if (topConnected && bottomConnected)
				return true;
		}
		return false;
	}

			int32				fSelectedTab;
			Area*				fSeedArea;
			area_side			fSide;

			BObjectList<Area>	fAreaGroup;

			bool				fDetach;
			BPoint				fLastPoint;

			int32				fConnectToTab;
};


#endif // EDIT_ACTION_RESIZING_H

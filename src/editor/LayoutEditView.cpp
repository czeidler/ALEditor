/*
 * Copyright 2007-2008, Christof Lutteroth, lutteroth@cs.auckland.ac.nz
 * Copyright 2007-2008, James Kim, jkim202@ec.auckland.ac.nz
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "LayoutEditView.h"

#include <Cursor.h>
#include <Looper.h>
#include <Message.h>
#include <Region.h>
#include <Size.h>

#include <AutoDeleter.h>

#include "ALMEditor.h"
#include "CustomizableNodeFactory.h"
#include "CustomizableNodeView.h"
#include "EditActionAreaDragging.h"
#include "EditActionInserting.h"
#include "EditActionMisc.h"
#include "EditActionResizing.h"
#include "LayoutArchive.h"


using namespace BALM;


CustomizableView* BALM::to_customizable_view(BLayoutItem* item)
{
	CustomizableView* customizable = dynamic_cast<CustomizableView*>(
		item->View());
	if (customizable == NULL)
		customizable = dynamic_cast<CustomizableView*>(item);
	return customizable;
}


class EditMessageFilter : public BMessageFilter {
public:
	EditMessageFilter(LayoutEditView* newTarget)
		:
		BMessageFilter(B_ANY_DELIVERY, B_REMOTE_SOURCE),

		fTarget(newTarget)
	{

	}

	filter_result
	Filter(BMessage* message, BHandler** target)
	{
		if (fTarget == *target)
			return B_DISPATCH_MESSAGE;

		switch (message->what) {
			case B_MOUSE_MOVED:
			case B_MOUSE_UP:
			case B_MOUSE_DOWN:
			{
				BView* view = dynamic_cast<BView*>(*target);
				if (view == NULL)
					break;

				// convert the mouse position to the layout system
				BPoint where;
				message->FindPoint("be:view_where", &where);
				BPoint editViewLeftTopScreen = fTarget->ConvertToScreen(
					fTarget->Frame().LeftTop());
				BPoint whereScreen = view->ConvertToScreen(where);
				where = whereScreen - editViewLeftTopScreen;
 				message->ReplacePoint("be:view_where", where);
 
				*target = fTarget;
				break;
			}
		}
		return B_DISPATCH_MESSAGE;
	}

private:
			LayoutEditView*		fTarget;
};


const float kEnlargedAreaInset = 3;

const uint32 kMsgUndo = '&Udo';
const uint32 kMsgRedo = '&Rdo';


LayoutEditView::LayoutEditView(BALMEditor* editor)
	:
	BView("LayoutEditView", B_WILL_DRAW | B_FRAME_EVENTS
		| B_FULL_UPDATE_ON_RESIZE),

	fEditor(editor),
	fALMEngine(editor->Layout()),
	fMessageFilter(NULL),

	fSelectedArea(NULL),
	fSelectedXTab(NULL),
	fSelectedYTab(NULL),
	fShowXTabs(false),
	fShowYTabs(false),

	fRightClickMenu(NULL),
	fHAlignmentMenu(NULL),
	fVAlignmentMenu(NULL),

	fState(NULL),

	fOverlapManager(editor->GetOverlapManager()),
	fEditAnimation(fALMEngine, this)
{
	fInformant = new ToolTipInformant(this);

	SetViewColor(B_TRANSPARENT_COLOR);
	SetExplicitMinSize(BSize(0, 0));

	SetShowXTabs(fEditor->ShowXTabs());
	SetShowYTabs(fEditor->ShowYTabs());


	_SetState(new(std::nothrow) MouseOverState(this));
}


LayoutEditView::~LayoutEditView()
{
	fEditor->StopEdit();

	delete fInformant;
	delete fMessageFilter;
}


void
LayoutEditView::AttachedToWindow()
{
	fSelectedArea = NULL;
	fSelectedXTab = NULL;
	fSelectedYTab = NULL;

	_StoreAction(NULL);

	// right click menu
	fRightClickMenu = new BPopUpMenu("Edit Menu", false, false,
		B_ITEMS_IN_COLUMN);
	fRemoveContent = new BMenuItem("Remove Area Content",
		new BMessage(kRemoveComponentMsg));
	fAddTab = new BMenuItem("Add Tab", new BMessage(kAddTabMsg));
	_UpdateRightClickMenu(NULL);

	if (fMessageFilter == NULL)
		fMessageFilter = new EditMessageFilter(this);
	if (LockLooper()) {
		BWindow* window = Window();
		window->AddCommonFilter(fMessageFilter);
		window->AddHandler(&fEditAnimation);

		window->AddShortcut('z', 0, new BMessage(kMsgUndo), this);
		window->AddShortcut('z', B_SHIFT_KEY, new BMessage(kMsgRedo), this);
		UnlockLooper();
	}

	_InvalidateAreaData();
	fALMEngine->Solver()->Solve();

	fOverlapManager.ConnectAreas();
}


void
LayoutEditView::DetachedFromWindow()
{
	fOverlapManager.DisconnectAreas();

	_SetState(NULL);

	delete fRightClickMenu;

	if (LockLooper()) {
		BWindow* window = Window();
		window->RemoveCommonFilter(fMessageFilter);
		window->RemoveHandler(&fEditAnimation);
		
		window->RemoveShortcut('z', 0);
		window->RemoveShortcut('z', B_SHIFT_KEY);
		UnlockLooper();
	}
}


BALMLayout*
LayoutEditView::Layout()
{
	return fALMEngine;
}


BALMEditor*
LayoutEditView::Editor()
{
	return fEditor;
}


Area*
LayoutEditView::SelectedArea() const
{
	return fSelectedArea;
}


void
LayoutEditView::SetSelectedArea(Area* area)
{
	fSelectedArea = area;
}


void
LayoutEditView::SetShowXTabs(bool show)
{
	fShowXTabs = show;
	Invalidate();
}


void
LayoutEditView::SetShowYTabs(bool show)
{
	fShowYTabs = show;
	Invalidate();
}


bool
LayoutEditView::Undo()
{
	history_entry* entry = fHistory.CurrentEvent();
	if (entry == NULL)
		debugger("we have no history!");
	if (entry->action == NULL)
		return false;
	EditAction* action = entry->action;

	fOverlapManager.DisconnectAreas();
	bool status = action->Undo();
	fOverlapManager.ConnectAreas();
	if (!status) {
		_ResetHistory();
		return false;
	}

	// need to solve the layout to to a proper animation
	LinearProgramming::ResultType resultType = fALMEngine->ValidateLayout();
	if (resultType == LinearProgramming::kInfeasible) {
		action->Perform();
		debugger("should not happen");
		return false;
	}

	_InvalidateAreaData();

	BWindow* window = Window();
	if (window != NULL)
		window->PostMessage(kMsgLayoutEdited);

	Invalidate();
	InvalidateLayout();

	fSelectedArea = NULL;

	fHistory.MoveBackward();
	return true;
}


bool
LayoutEditView::Redo()
{
	history_entry* entry = fHistory.MoveForward();
	if (entry == NULL)
		return false;
	EditAction* action = entry->action;

	fOverlapManager.DisconnectAreas();
	bool result = action->Perform();
	fOverlapManager.ConnectAreas();
	if (result != true) {
		_ResetHistory();
		return false;
	}

	// need to solve the layout to to a proper animation
	LinearProgramming::ResultType resultType = fALMEngine->ValidateLayout();
	if (resultType == LinearProgramming::kInfeasible) {
		action->Undo();
		debugger("should not happen");
		return false;
	}

	BWindow* window = Window();
	if (window != NULL)
		window->PostMessage(kMsgLayoutEdited);

	Invalidate();
	InvalidateLayout();

	fSelectedArea = NULL;

	return true;
}


BMessage*
LayoutEditView::CurrentLayout()
{
	return &fHistory.CurrentEvent()->layout;
}


bool
LayoutEditView::PerformAction(EditAction* action)
{
	ObjectDeleter<EditAction> actionDeleter(action);

	fEditAnimation.CaptureStartpoint();
	// Disable the layout invalidation till we start the animation.
	fALMEngine->DisableLayoutInvalidation();

	fOverlapManager.DisconnectAreas();
	bool result = action->Perform();

	fOverlapManager.FillTabConnections();
	_CheckTempEditConstraints();
	fOverlapManager.ConnectAreas(false);

	if (result != true) {
		debugger("should not happen");
		return false;
	}

	// need to solve the layout to to a proper animation
	LinearProgramming::ResultType resultType = fALMEngine->ValidateLayout();
	if (resultType == LinearProgramming::kInfeasible) {
		action->Undo();
		debugger("should not happen");
		return false;
	}

	_StoreAction(action);
	_InvalidateAreaData();

	fEditAnimation.Animate();
	// the animation takes care of the invalidation now
	fALMEngine->EnableLayoutInvalidation();

	BWindow* window = Window();
	if (window != NULL)
		window->PostMessage(kMsgLayoutEdited);

	Invalidate();
	InvalidateLayout();
	actionDeleter.Detach();
	return true;
}


bool
LayoutEditView::TestAction(EditAction* action, bool deleteAction)
{
	ObjectDeleter<EditAction> _(action);
	if (!deleteAction)
		_.Detach();
		
	fOverlapManager.DisconnectAreas();
	bool result = action->Perform();
	if (result != true) {
		fOverlapManager.ConnectAreas();
		return false;
	}

	bool possible = true;

	fOverlapManager.FillTabConnections();
	_CheckTempEditConstraints();
	fOverlapManager.ConnectAreas(false);

	LinearProgramming::ResultType resultType = fALMEngine->ValidateLayout();
	if (resultType == LinearProgramming::kInfeasible)
		possible = false;

	fOverlapManager.DisconnectAreas();
	action->Undo();
	fOverlapManager.ConnectAreas();

//TODO this is only necessary for the bad resize action and can be removed after fixing it
fALMEngine->ValidateLayout();

	if (possible == false) {
		BString message = "Can't perform ";
		message += action->Name();
		message += " operation here.";
		fInformant->Error(message);
	}
	return possible;
}


bool
LayoutEditView::TestAndPerformAction(EditAction* action)
{
	if (!TestAction(action, false)) {
		delete action;
		return false;
	}
	return PerformAction(action);
}


bool
LayoutEditView::TrashArea(Area* area)
{
	if (area == NULL)
		return false;

	CustomizableView* customizable = to_customizable_view(area->Item());
	if (customizable == NULL)
		return false;

	bool status = PerformAction(new RemoveCustomizableViewAction(fEditor,
		CurrentLayout(), customizable));
	if (status == false)
		return false;

	SetSelectedArea(NULL);
	return true;
}


bool
LayoutEditView::ConnectedToLeftBorder(Area* area)
{
	return fOverlapManager.GetTabConnections()->ConnectedToLeftBorder(
		area->Left(), fALMEngine);
}


bool
LayoutEditView::ConnectedToTopBorder(Area* area)
{
	return fOverlapManager.GetTabConnections()->ConnectedToTopBorder(
		area->Top(), fALMEngine);
}


bool
LayoutEditView::ConnectedToRightBorder(Area* area)
{
	return fOverlapManager.GetTabConnections()->ConnectedToRightBorder(
		area->Right(), fALMEngine);
}


bool
LayoutEditView::ConnectedToBottomBorder(Area* area)
{
	return fOverlapManager.GetTabConnections()->ConnectedToBottomBorder(
		area->Bottom(), fALMEngine);
}


bool
LayoutEditView::ConnectedTo(Area* area, area_side side)
{
	return fOverlapManager.GetTabConnections()->ConnectedTo(
		area, side);
}


void
LayoutEditView::KeyDown(const char* bytes, int32 numBytes)
{
	if (bytes[0] == B_DELETE) {
		Area* area = SelectedArea();
		TrashArea(area);
	}
	BView::KeyDown(bytes, numBytes);
}


void
LayoutEditView::MouseDown(BPoint point)
{
	MakeFocus();

	_ToALMLayoutCoordinates(point);

	SetMouseEventMask(B_POINTER_EVENTS);

	int32 buttons = 0;
	if (Looper() != NULL && Looper()->CurrentMessage() != NULL)
		Looper()->CurrentMessage()->FindInt32("buttons", &buttons);

	if (buttons == B_PRIMARY_MOUSE_BUTTON) {
		Area* oldSelectedArea = fSelectedArea;
		fSelectedArea = _FindTooSmallArea(point);
		if (fSelectedArea != NULL) {
			_SetState(new(std::nothrow) DragAreaState(this, fSelectedArea,
				point, BPoint(0, 0)));
		} else {
			fSelectedArea = FindArea(point);
			_NotifyAreaSelected(fSelectedArea);

			if (fSelectedArea != NULL) {
				fSelectedXTab = GetAreaXTabNearPoint(fSelectedArea, point,
					kTolerance);
				fSelectedYTab = GetAreaYTabNearPoint(fSelectedArea, point,
					kTolerance);
				if (fSelectedXTab != NULL || fSelectedYTab != NULL) {
					if (oldSelectedArea != fSelectedArea) {
#if GROUP_TAB_RESIZE
						// drag tab
						if (fSelectedXTab != NULL) {
							_SetState(new(std::nothrow) DragXTabState(this,
								fALMEngine->IndexOf(fSelectedXTab, true),
								fSelectedArea));
							fSelectedArea = NULL;
						} else if (fSelectedYTab != NULL) {
							_SetState(new(std::nothrow) DragYTabState(this,
								fALMEngine->IndexOf(fSelectedYTab, true),
								fSelectedArea));
							fSelectedArea = NULL;
						}
#endif
					} else {
						// resize area
						area_side xSide = kNoSide;
						area_side ySide = kNoSide;
						if (fSelectedArea->Left() == fSelectedXTab)
							xSide = kLeft;
						if (fSelectedArea->Right() == fSelectedXTab)
							xSide = kRight;
						if (fSelectedArea->Top() == fSelectedYTab)
							ySide = kTop;
						if (fSelectedArea->Bottom() == fSelectedYTab)
							ySide = kBottom;
						_SetState(new(std::nothrow) ResizeState(this,
							fSelectedArea, xSide, ySide));
					}
				} else {
					// drag area
					BRect areaFrame = fSelectedArea->Frame();
					BPoint dropOffset = point - areaFrame.LeftTop();
					_SetState(new(std::nothrow) DragAreaState(this,
						fSelectedArea, point, dropOffset));
				}
			}
		}
		_NotifyAreaSelected(fSelectedArea);
		Invalidate();
		fEditor->UpdateEditWindow();
	}
	
	// If right button is clicked, edit menu
	if (buttons == B_SECONDARY_MOUSE_BUTTON) {
		fSelectedArea = FindArea(point);
		_UpdateRightClickMenu(fSelectedArea);
		fLastMenuPosition = point;
		ConvertToScreen(&point);
		fRightClickMenu->Go(point, true, true, false);
	}
}


void
LayoutEditView::MouseUp(BPoint point)
{
	_ToALMLayoutCoordinates(point);

	if (fState != NULL && fState->MouseUp(point) == true)
		_SetState(new(std::nothrow) MouseOverState(this));

	Invalidate();
	fEditor->UpdateEditWindow();
}


void
LayoutEditView::MouseMoved(BPoint point, uint32 transit,
	const BMessage* message)
{
	if (transit == B_EXITED_VIEW || transit == B_OUTSIDE_VIEW) {
		BCursor cursor(B_CURSOR_SYSTEM_DEFAULT);
		be_app->SetCursor(&cursor);
	}

	_ToALMLayoutCoordinates(point);

	if (message != NULL && message->what == kMsgCreateComponent) {
		CustomizableRoster* roster = CustomizableRoster::DefaultRoster();
		BString component;
		message->FindString("component", &component);
		BReference<Customizable> customizable = roster->InstantiateCustomizable(
			component, message);
		IViewContainer* view = dynamic_cast<IViewContainer*>(
			customizable.Get());
		if (view != NULL)
			_SetState(new CreateAndInsertState(this, component, NULL));
	} else if (message != NULL && message->what == kUnTrashComponent) {
		Customizable* customizable;
		message->FindPointer("component", (void**)&customizable);
		IViewContainer* view = dynamic_cast<IViewContainer*>(customizable);
		if (view != NULL)
			_SetState(new UnTrashInsertState(fEditor, this, customizable));
	}
	if (fState != NULL)
		fState->MouseMoved(point, transit, message);

	Invalidate();
}


Area*
LayoutEditView::FindItemArea(BPoint point)
{
	float hSpacing;
	float vSpacing;
	fALMEngine->GetSpacing(&hSpacing, &vSpacing);
	for (int32 i = 0; i < fALMEngine->CountItems(); i++) {
		Area* area = fALMEngine->AreaAt(i);
		if (area->Item()->View() == this)
			continue;
		BRect frameRect = area->Frame();
		frameRect.InsetBy(- hSpacing / 2, - vSpacing / 2);

		float leftInset;
		float topInset;
		fALMEngine->GetInsets(&leftInset, &topInset, NULL, NULL);
		frameRect.OffsetBy(-leftInset, -topInset);
		if (frameRect.Contains(point))
			return area;
	}
	return NULL;
}


Area*
LayoutEditView::FindArea(BPoint point)
{
	for (int32 i = 0; i < fALMEngine->CountItems(); i++) {
		Area* area = fALMEngine->AreaAt(i);
		if (area->Item()->View() == this)
			continue;
		BRect frameRect = _AreaFrame(area);
		if (frameRect.Contains(point))
			return area;
	}
	return NULL;
}


bool
LayoutEditView::FindEmptyArea(const BPoint& point, area_ref& ref, Area* ignore)
{
	if (point.x <= 0 || point.y <= 0)
		return false;
	if (ignore != NULL) {
		BRegion takenSpace = fTakenSpace;
		takenSpace.Exclude(ignore->Frame());
		if (takenSpace.Contains(point))
			return false;
	} else if (fTakenSpace.Contains(point))
		return false;

	for (int32 i = 1; i < fALMEngine->CountXTabs(); i++) {
		XTab* tab = fALMEngine->XTabAt(i, true);
		if (point.x < tab->Value()) {
			ref.left = fALMEngine->XTabAt(i - 1, true);
			ref.right = tab;
			break;
		}
	}
	for (int32 i = 1; i < fALMEngine->CountYTabs(); i++) {
		YTab* tab = fALMEngine->YTabAt(i, true);
		if (point.y < tab->Value()) {
			ref.top = fALMEngine->YTabAt(i - 1);
			ref.bottom = tab;
			break;
		}
	}
	if (ref.left.Get() == NULL || ref.right.Get() == NULL
		|| ref.top.Get() == NULL || ref.bottom.Get() == NULL) {
		ref.left = NULL;
		ref.top = NULL;
		ref.right = NULL;
		ref.bottom = NULL;
		return false;
	}
	return true;
}


XTab*
LayoutEditView::GetXTabNearPoint(BPoint p, int32 tolerance)
{
	XTab* selectedTab = NULL;
	double proximity = tolerance;

	for (int32 i = 0; i < fALMEngine->CountXTabs(); i++) {
		XTab* tab = fALMEngine->XTabAt(i, true);
		float tabPosition = _TabPosition(tab);
		if (fabs(tabPosition - p.x) < proximity) {
			selectedTab = tab;
			proximity = fabs(tabPosition - p.x);
		} else if (selectedTab != NULL)
			break;
	}

	return selectedTab;
}


YTab*
LayoutEditView::GetYTabNearPoint(BPoint p, int32 tolerance)
{
	YTab* selectedTab = NULL;
	double proximity = tolerance;

	for (int32 i = 0; i < fALMEngine->CountYTabs(); i++) {
		YTab* tab = fALMEngine->YTabAt(i, true);
		float tabPosition = _TabPosition(tab);
		if (fabs(tabPosition - p.y) < proximity) {
			selectedTab = tab;
			proximity = fabs(tabPosition - p.y);
		} else if (selectedTab != NULL)
			break;
	}

	return selectedTab;
}


XTab*
LayoutEditView::GetAreaXTabNearPoint(Area* area, BPoint point, int32 tolerance)
{
	if (fabs(_TabPosition(area->Left()) - point.x) < tolerance)
		return area->Left();
	if (fabs(_TabPosition(area->Right()) - point.x) < tolerance)
		return area->Right();

	return NULL;
}


YTab*
LayoutEditView::GetAreaYTabNearPoint(Area* area, BPoint point, int32 tolerance)
{
	if (fabs(_TabPosition(area->Top()) - point.y) < tolerance)
		return area->Top();
	if (fabs(_TabPosition(area->Bottom()) - point.y) < tolerance)
		return area->Bottom();

	return NULL;
}


XTab*
LayoutEditView::GetBestXTab(XTab* searchStart, BPoint p)
{
	XTab* selectedTab = NULL;
	float minDist = HUGE_VAL;

	float searchTabPos = _TabPosition(searchStart);
	if (fabs(searchTabPos - p.x) < kTolerance)
		return NULL;
	if (searchTabPos < p.x) {
		// search right
		for (int32 i = 0; i < fALMEngine->CountXTabs(); i++) {
			XTab* tab = fALMEngine->XTabAt(i, true);
			if (tab == searchStart)
				continue;
			float tabPosition = _TabPosition(tab);
			if (tabPosition < searchTabPos)
				continue;

			float distance = fabs(tabPosition - p.x);
			if (minDist < distance)
				break;
			minDist = distance;
			selectedTab = tab;
		}
	} else {
		// search left
		for (int32 i = fALMEngine->CountXTabs() - 1; i >= 0; i--) {
			XTab* tab = fALMEngine->XTabAt(i, true);
			if (tab == searchStart)
				continue;
			float tabPosition = _TabPosition(tab);
			if (tabPosition > searchTabPos)
				continue;

			float distance = fabs(tabPosition - p.x);
			if (minDist < distance)
				break;
			minDist = distance;
			selectedTab = tab;
		}
	}
	return selectedTab;
}


YTab*
LayoutEditView::GetBestYTab(YTab* searchStart, BPoint p)
{
	YTab* selectedTab = NULL;
	double minDist = HUGE_VAL;

	float searchTabPos = _TabPosition(searchStart);
	if (fabs(searchTabPos - p.y) < kTolerance)
		return NULL;
	if (searchTabPos < p.y) {
		// search down
		for (int32 i = 0; i < fALMEngine->CountYTabs(); i++) {
			YTab* tab = fALMEngine->YTabAt(i, true);
			if (tab == searchStart)
				continue;
			float tabPosition = _TabPosition(tab);
			if (tabPosition < searchTabPos)
				continue;

			float distance = fabs(tabPosition - p.y);
			if (minDist < distance)
				break;
			minDist = distance;
			selectedTab = tab;
		}
	} else {
		// search up
		for (int32 i = fALMEngine->CountYTabs() - 1; i >= 0; i--) {
			YTab* tab = fALMEngine->YTabAt(i, true);
			if (tab == searchStart)
				continue;
			float tabPosition = _TabPosition(tab);
			if (tabPosition > searchTabPos)
				continue;

			float distance = fabs(tabPosition - p.y);
			if (minDist < distance)
				break;
			minDist = distance;
			selectedTab = tab;
		}
	}

	return selectedTab;
}


void
LayoutEditView::DrawHighLightArea(const Area* area, float penSize,
	const rgb_color& color)
{
	DrawHighLightArea(_AreaFrame(area), penSize, color);
}


void
LayoutEditView::DrawHighLightArea(const area_ref& ref, float penSize,
	const rgb_color& color)
{
	BRect areaFrame(ref.left->Value(), ref.top->Value(), ref.right->Value(),
		ref.bottom->Value());
	_AreaFrame(areaFrame);
	DrawHighLightArea(areaFrame, penSize, color);
}


void
LayoutEditView::DrawHighLightArea(BRect frame, float penSize,
	const rgb_color& color)
{
	SetPenSize(penSize);
	SetHighColor(color);
	_EnlargeTooSmallArea(frame);
	frame.InsetBy(2, 2);
	StrokeRect(frame);
}


void
LayoutEditView::DrawResizeKnops(const Area* area)
{
	SetPenSize(1);
	rgb_color color = {0, 0, 0};
	SetHighColor(color);
	BRect areaFrame = _AreaFrame(area);
	areaFrame.InsetBy(2, 2);
	if (areaFrame.Width() <= 2 * kEnlargedAreaInset
		|| areaFrame.Height() <= 2 * kEnlargedAreaInset)
		return;

	const float kKnopSize = 4;

	BPoint knopSizeHalf(kKnopSize / 2, kKnopSize / 2);

	BPoint middle = areaFrame.LeftTop();
	BPoint leftTop = middle - knopSizeHalf;
	BPoint rightBottom = middle + knopSizeHalf;
	StrokeRect(BRect(leftTop, rightBottom));

	middle = areaFrame.LeftBottom();
	leftTop = middle - knopSizeHalf;
	rightBottom = middle + knopSizeHalf;
	StrokeRect(BRect(leftTop, rightBottom));

	middle = areaFrame.RightTop();
	leftTop = middle - knopSizeHalf;
	rightBottom = middle + knopSizeHalf;
	StrokeRect(BRect(leftTop, rightBottom));

	middle = areaFrame.RightBottom();
	leftTop = middle - knopSizeHalf;
	rightBottom = middle + knopSizeHalf;
	StrokeRect(BRect(leftTop, rightBottom));

	
	middle = BPoint(areaFrame.left + areaFrame.Width() / 2, areaFrame.top);
	leftTop = middle - knopSizeHalf;
	rightBottom = middle + knopSizeHalf;
	StrokeRect(BRect(leftTop, rightBottom));

	middle = BPoint(areaFrame.left + areaFrame.Width() / 2, areaFrame.bottom);
	leftTop = middle - knopSizeHalf;
	rightBottom = middle + knopSizeHalf;
	StrokeRect(BRect(leftTop, rightBottom));

	middle = BPoint(areaFrame.left, areaFrame.top + areaFrame.Height() / 2);
	leftTop = middle - knopSizeHalf;
	rightBottom = middle + knopSizeHalf;
	StrokeRect(BRect(leftTop, rightBottom));

	middle = BPoint(areaFrame.right, areaFrame.top + areaFrame.Height() / 2);
	leftTop = middle - knopSizeHalf;
	rightBottom = middle + knopSizeHalf;
	StrokeRect(BRect(leftTop, rightBottom));
}


void
LayoutEditView::DrawTab(const XTab* tab, float penSize, const rgb_color& color)
{
	SetPenSize(penSize);
	SetHighColor(color);
	float tabPosition = _TabPosition(tab);
	StrokeLine(BPoint(tabPosition, 0), BPoint(tabPosition,
		Bounds().Height() - penSize / 2));
}


void
LayoutEditView::DrawTab(const YTab* tab, float penSize, const rgb_color& color)
{
	SetPenSize(penSize);
	SetHighColor(color);
	float tabPosition = _TabPosition(tab);
	StrokeLine(BPoint(0, tabPosition), BPoint(Bounds().Width() - penSize / 2,
		tabPosition));
}


void
LayoutEditView::DrawXTabConnections(XTab* tab)
{
	SetHighColor(kMarkedTabColor);
	SetPenSize(1);
	const int space = 3;

	TabConnections* tabConnection = fOverlapManager.GetTabConnections();
	std::map<XTab*, tab_links<XTab> >& linkMap
		= tabConnection->GetXTabLinkMap();
	tab_links<XTab>& links = linkMap[tab];
	BPoint start;
	BPoint end;
	start.x = tab->Value() - 1;
	end.x = tab->Value() - 1;		
	for (int32 i = 0; i < links.areas1.CountItems(); i++) {
		Area* area = links.areas1.ItemAt(i);
		start.y = area->Top()->Value() + space;
		end.y = area->Bottom()->Value() - space;
		StrokeLine(start, end);
		
	}

	start.x = tab->Value() + 1;
	end.x = tab->Value() + 1;
	for (int32 i = 0; i < links.areas2.CountItems(); i++) {
		Area* area = links.areas2.ItemAt(i);
		start.y = area->Top()->Value() + space;
		end.y = area->Bottom()->Value() - space;
		StrokeLine(start, end);
	}
}


void
LayoutEditView::DrawYTabConnections(YTab* tab)
{
	SetHighColor(kMarkedTabColor);
	SetPenSize(1);
	const int space = 3;

	TabConnections* tabConnection = fOverlapManager.GetTabConnections();
	std::map<YTab*, tab_links<YTab> >& linkMap
		= tabConnection->GetYTabLinkMap();
	tab_links<YTab>& links = linkMap[tab];
	BPoint start;
	BPoint end;
	start.y = tab->Value() - 1;
	end.y = tab->Value() - 1;		
	for (int32 i = 0; i < links.areas1.CountItems(); i++) {
		Area* area = links.areas1.ItemAt(i);
		start.x = area->Left()->Value() + space;
		end.x = area->Right()->Value() - space;
		StrokeLine(start, end);
		
	}

	start.y = tab->Value() + 1;
	end.y = tab->Value() + 1;
	for (int32 i = 0; i < links.areas2.CountItems(); i++) {
		Area* area = links.areas2.ItemAt(i);
		start.x = area->Left()->Value() + space;
		end.x = area->Right()->Value() - space;
		StrokeLine(start, end);
	}
}


void
LayoutEditView::DrawAreaWithInnerTabs(const area_ref& ref)
{
	const int32 kShade = 200;
	rgb_color gridColor = {kShade, kShade, kShade};
	SetDrawingMode(B_OP_OVER);
	SetHighColor(gridColor);
	SetPenSize(1);

	BRect frame = ref.Frame();

	int32 startIndex = fALMEngine->IndexOf(ref.left, true);
	for (int32 i = startIndex; i < fALMEngine->CountXTabs(); i++) {
		XTab* tab = fALMEngine->XTabAt(i, true);
		float tabPosition = tab->Value();
		if (tabPosition > frame.right)
			break;
		StrokeLine(BPoint(tabPosition, frame.top),
			BPoint(tabPosition, frame.bottom));
	}

	startIndex = fALMEngine->IndexOf(ref.top, true);
	for (int32 i = startIndex; i < fALMEngine->CountYTabs(); i++) {
		YTab* tab = fALMEngine->YTabAt(i, true);
		float tabPosition = tab->Value();
		if (tabPosition > frame.bottom)
			break;
		StrokeLine(BPoint(frame.left, tabPosition),
			BPoint(frame.right, tabPosition));
	}
}


void
LayoutEditView::DrawTooSmallArea(Area* area)
{
	BRect frame = _AreaFrame(area);
	if (!_EnlargeTooSmallArea(frame))
		return;

	rgb_color color = {0, 0, 200, 40};
	SetDrawingMode(B_OP_ALPHA);
	SetHighColor(color);
	FillRect(frame);
	SetDrawingMode(B_OP_OVER);
}


void
LayoutEditView::DrawSpacer(Area* area, BSpaceLayoutItem* spacer)
{
	rgb_color spacerColor = {20, 20, 20};
	rgb_color barColor = {100, 100, 100};

	BRect rect = _AreaFrame(area);
	const float kKnopSize = 6;
	float middleH = rect.left + rect.Width() / 2;
	float middleV = rect.top + rect.Height() / 2;

	SetHighColor(spacerColor);
	// left
	FillRect(BRect(rect.left, middleV - kKnopSize / 2,
		rect.left + kKnopSize / 2, middleV + kKnopSize / 2));
	// top
	FillRect(BRect(middleH - kKnopSize / 2, rect.top,
		middleH + kKnopSize / 2, rect.top + kKnopSize / 2));
	// right
	FillRect(BRect(rect.right - kKnopSize / 2, middleV - kKnopSize / 2,
		rect.right, middleV + kKnopSize / 2));
	// bottom
	FillRect(BRect(middleH - kKnopSize / 2, rect.bottom - kKnopSize / 2,
		middleH + kKnopSize / 2, rect.bottom));

	float lengthH = rect.Width() * 0.3;
	float lengthV = rect.Height() * 0.3;
	SetPenSize(3);
	StrokeLine(BPoint(rect.left + lengthH, middleV),
		BPoint(rect.right - kKnopSize / 2, middleV));
	StrokeLine(BPoint(middleH, rect.top + lengthV),
		BPoint(middleH, rect.bottom- kKnopSize / 2));

	SetHighColor(barColor);
	SetPenSize(1);
	StrokeLine(BPoint(rect.left + kKnopSize / 2, middleV),
		BPoint(rect.left + lengthH, middleV));
	StrokeLine(BPoint(middleH, rect.top + kKnopSize / 2),
		BPoint(middleH, rect.top + lengthV));
}


void
LayoutEditView::DrawAreaBackground(Area* area)
{
	BRect frame = area->Frame();

/*BRect areaFrame = _AreaFrame(area);
rgb_color dumb = {0, 255, 0};
SetHighColor(dumb);
StrokeRect(areaFrame);
*/
	frame.left += area->LeftInset();
	frame.top += area->TopInset();
	frame.right -= area->RightInset();
	frame.bottom -= area->BottomInset();
	BRegion region = frame;
	BLayoutItem* item = area->Item();
	if (item != NULL && item->View() != NULL) {
		BRect itemFrame = item->View()->Frame();
		const float space = 2;
		itemFrame.InsetBy(-space, -space);
		region.Exclude(itemFrame);
	} else if (dynamic_cast<BLayout*>(item) != NULL)
		region.Exclude(item->Frame());

	const int8 shade = 240;
	rgb_color backgoundColor = {shade, shade, shade};
	SetHighColor(backgoundColor);
	FillRegion(&region);

	const int8 shadeDark = 220;
	rgb_color borderColor = {shadeDark, shadeDark, shadeDark};
	SetHighColor(borderColor);
	StrokeRect(region.Frame());
}


void
LayoutEditView::DrawTakenSpace()
{
	rgb_color color = {0, 255, 0, 20};
	SetDrawingMode(B_OP_ALPHA);
	SetHighColor(color);
	FillRegion(&fTakenSpace);
	SetDrawingMode(B_OP_OVER);
}


void
LayoutEditView::DrawTempConstraints()
{
	rgb_color constraintColor = {200, 200, 0};
	SetHighColor(constraintColor);
	SetPenSize(1);

	float xSpacing, ySpacing;
	fALMEngine->GetSpacing(&xSpacing, &ySpacing);

	ConstraintList toBeRemoved;
	for (int32 i = 0; i < fALMEngine->CountConstraints(); i++) {
		Constraint* constraint = fALMEngine->ConstraintAt(i);
		if (BString(constraint->Label()) != "_EditHelper")
			continue;
		SummandList* summands = constraint->LeftSide();
		Summand* summand1 = summands->ItemAt(0);
		Summand* summand2 = summands->ItemAt(1);
		if (summand1 == NULL)
			debugger("Ups?");
		for (int32 a = 0; a < fALMEngine->CountAreas(); a++) {
			Area* area = fALMEngine->AreaAt(a);
			if (summand2 == NULL) {
				// position constraint
				XTab* xTab = static_cast<XTab*>(summand1->Var());
				if (fALMEngine->IndexOf(xTab) >= 0) {
					if (area->Left() == xTab) {
						StrokeLine(BPoint(xTab->Value() - 5,
							area->Top()->Value()),
							BPoint(xTab->Value(), area->Top()->Value()));
					}
				}
				YTab* yTab = static_cast<YTab*>(summand1->Var());
				if (fALMEngine->IndexOf(yTab) >= 0) {
					if (area->Top() == yTab) {
						StrokeLine(BPoint(area->Left()->Value(),
							yTab->Value() - 5),
							BPoint(area->Left()->Value(), yTab->Value()));
					}
				}
			} else {
				// size constraint
				XTab* xVar1 = static_cast<XTab*>(summand1->Var());
				XTab* xVar2 = static_cast<XTab*>(summand2->Var());
				YTab* yVar1 = static_cast<YTab*>(summand1->Var());
				YTab* yVar2 = static_cast<YTab*>(summand2->Var());
				if (fALMEngine->IndexOf(xVar1) >= 0
					&& fALMEngine->IndexOf(xVar2) >= 0) {
					if ((xVar1 == area->Left() && xVar2 == area->Right()) ||
						(xVar1 == area->Right() && xVar2 == area->Left())) {
						BRect frame = area->Frame();
						StrokeLine(BPoint(frame.left,
							frame.top + frame.Height() / 2), BPoint(frame.right,
							frame.top + frame.Height() / 2));
						break;
					}
				} else if (fALMEngine->IndexOf(yVar1) >= 0
					&& fALMEngine->IndexOf(yVar2) >= 0) {
					if ((yVar1 == area->Top() && yVar2 == area->Bottom()) ||
						(yVar1 == area->Bottom() && yVar2 == area->Top())) {
						BRect frame = area->Frame();
						StrokeLine(BPoint(frame.left + frame.Width() / 2,
							frame.top), BPoint(frame.left + frame.Width() / 2,
							frame.bottom));
						break;
					}
				}
			}	
		}
	}
}


void
LayoutEditView::Draw(BRect updateRect)
{
	rgb_color backgoundColor = {255, 255, 255};
	if (fALMEngine->View() != NULL)
		backgoundColor = fALMEngine->View()->ViewColor();
	SetHighColor(backgoundColor);

	BRegion background(updateRect);
	for (int32 i = 0; i < fALMEngine->CountItems(); i++) {
		BLayoutItem* item = fALMEngine->ItemAt(i);
		BView* view = item->View();
		if (view == this)
			continue;
		_RecursiveExcludeViews(item, background);
	}
	FillRegion(&background);

	// Draw selected area
	if (fSelectedArea != NULL) {
		DrawXTabConnections(fSelectedArea->Left());
		DrawXTabConnections(fSelectedArea->Right());
		DrawYTabConnections(fSelectedArea->Top());
		DrawYTabConnections(fSelectedArea->Bottom());

		DrawHighLightArea(fSelectedArea, 1, kSelectedColor);
		DrawResizeKnops(fSelectedArea);
	}

	for (int32 i = 0; i < fALMEngine->CountItems(); i++) {
		Area* area = fALMEngine->AreaAt(i);
		DrawAreaBackground(area);
	}

	// Paint tabs if the option is selected
	if (fShowXTabs) {
		for (int32 i = 0; i < fALMEngine->CountXTabs(); i++)
			DrawTab(fALMEngine->XTabAt(i), kTabWidth, kSuggestionColor);
	}
	if (fShowYTabs) {
		for (int32 i = 0; i < fALMEngine->CountYTabs(); i++)
			DrawTab(fALMEngine->YTabAt(i), kTabWidth, kSuggestionColor);
	}

	// draw spacer and to small areas
	for (int32 i = 0; i < fALMEngine->CountItems(); i++) {
		Area* area = fALMEngine->AreaAt(i);

		DrawTooSmallArea(area);

		BSpaceLayoutItem* spacer = dynamic_cast<BSpaceLayoutItem*>(
			area->Item());
		if (spacer == NULL)
			continue;
		DrawSpacer(area, spacer);
	}

	if (fState != NULL)
		fState->Draw(updateRect);

//	fOverlapManager.Draw(this);
//	DrawTakenSpace();
	DrawTempConstraints();
}


void
LayoutEditView::FrameResized(float width, float height)
{
	_InvalidateAreaData();

	_UpdateCurrentLayout();
}


void
LayoutEditView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMsgUndo:
			Undo();
			break;
		
		case kMsgRedo:
			Redo();
			break;

		case kMsgCreateComponent:
		{
			BString component;
			message->FindString("component", &component);
			//fActionHistory.PerformAction(new AddComponentAction(this, component,
			//	point));		
			break;
		}

		case kQuitMsg:
			_Quit();
			break;

		case kRemoveComponentMsg: {
			Area* area = FindArea(fLastMenuPosition);
			TrashArea(area);
			break;
		}

		case kAddTabMsg: {
			Area* area = FindArea(fLastMenuPosition);
			if (area == NULL)
				break;
			XTab* xTab = GetAreaXTabNearPoint(area, fLastMenuPosition,
				kTolerance);
			YTab* yTab = GetAreaYTabNearPoint(area, fLastMenuPosition,
				kTolerance);
			if (xTab == NULL && yTab == NULL)
				break;

			if (xTab != NULL && yTab != NULL) {
				if (fabs(xTab->Value() - fLastMenuPosition.x)
					< fabs(yTab->Value() - fLastMenuPosition.y))
					yTab = NULL;
				else
					xTab = NULL;
			}
			TabConnections* connections = fOverlapManager.GetTabConnections();
			if (xTab != NULL) {
				if (xTab == area->Left()
					&& connections->TabsAtTheLeft(xTab)) {
					PerformAction(new InsertTabAction<XTab>(fALMEngine,
						CurrentLayout(), area, kLeft));
				}
				if (xTab == area->Right()
					&& connections->TabsAtTheRight(xTab)) {
					PerformAction(new InsertTabAction<XTab>(fALMEngine,
						CurrentLayout(), area, kRight));
				}
			} else if (yTab != NULL) {
				if (yTab == area->Top()
					&& connections->TabsAtTheTop(yTab)) {
					PerformAction(new InsertTabAction<YTab>(fALMEngine,
						CurrentLayout(), area, kTop));
				}
				if (yTab == area->Bottom()
					&& connections->TabsAtTheTop(yTab)) {
					PerformAction(new InsertTabAction<YTab>(fALMEngine,
						CurrentLayout(), area, kBottom));
				}
			}
			break;
		}

		case kSetHorizontalAlignmentMsg:
		{
			Area* area;
			message->FindPointer("area", (void**)&area);
			alignment value;
			message->FindInt32("alignment", (int32*)&value);
			BAlignment align = area->Item()->Alignment();
			align.SetHorizontal(value);
			TestAndPerformAction(new SetAlignmentAction(area->Item(), align));
			break;
		}

		case kSetVerticalAlignmentMsg:
		{
			Area* area;
			message->FindPointer("area", (void**)&area);
			vertical_alignment value;
			message->FindInt32("alignment", (int32*)&value);
			BAlignment align = area->Item()->Alignment();
			align.SetVertical(value);
			TestAndPerformAction(new SetAlignmentAction(area->Item(), align));
			break;
		}

		case kShowXTabs:
			SetShowXTabs(message->FindBool("show"));
			break;
			
		case kShowYTabs:
			SetShowYTabs(message->FindBool("show"));
			break;

		case B_OBSERVER_NOTICE_CHANGE:
		{
			int32 what = message->FindInt32(B_OBSERVE_WHAT_CHANGE);
			if (what == kCustomizableSelected) {
				Customizable* customizable;
				message->FindPointer("customizable", (void**)&customizable);
				_HightlightCustomizableView(customizable);
				break;
			}
		}
		default:
			BView::MessageReceived(message);
	}
}


void
LayoutEditView::_SetState(State* state)
{
	delete fState;
	fState = state;
	if (fState)
		fState->EnterState();
	fInformant->Clear();
}


void
LayoutEditView::_AreaFrame(BRect& rect)
{
	return;
}


BRect
LayoutEditView::_AreaFrame(const Area* area)
{
	BRect areaFrame = area->Frame();
	_AreaFrame(areaFrame);

	return areaFrame;
}


Area*
LayoutEditView::_FindTooSmallArea(const BPoint& point)
{
	for (int32 i = 0; i < fALMEngine->CountItems(); i++) {
		Area* area = fALMEngine->AreaAt(i);
		if (area->Item()->View() == this)
			continue;
		BRect frame = _AreaFrame(area);
		if (!_EnlargeTooSmallArea(frame))
			continue;
		if (frame.Contains(point))
			return area;
	}
	return NULL;
}


bool
LayoutEditView::_EnlargeTooSmallArea(BRect& frame)
{
	float xInset = 0;
	float yInset = 0;
	if (frame.Width() < kEnlargedAreaInset)
		xInset = kEnlargedAreaInset;
	if (frame.Height() < kEnlargedAreaInset)
		yInset = kEnlargedAreaInset;
	if (xInset == 0 && yInset == 0)
		return false;
	frame.InsetBy(-xInset, - yInset);
	return true;
}


void
LayoutEditView::_ToALMLayoutCoordinates(BPoint& point)
{
}


float
LayoutEditView::_TabPosition(const XTab* tab) const
{
	float position = tab->Value();
	return position;
}


float
LayoutEditView::_TabPosition(const YTab* tab) const
{
	float position = tab->Value();
	return position;
}


void
LayoutEditView::_Quit()
{
	RemoveSelf();
	delete this;
}


void
LayoutEditView::_NotifyAreaSelected(Area* area)
{
	BMessage message;
	if (area != NULL) {
		Customizable* customizable = dynamic_cast<Customizable*>(
			area->Item()->View());
		if (customizable != NULL)
			message.AddPointer("customizable", customizable);
	}
	SendNotices(kCustomizableSelected, &message);
}


void
LayoutEditView::_InvalidateAreaData()
{
	fTakenSpace.Set(BRect(0, 0, 0, 0));

	for (int32 i = 0; i < fALMEngine->CountItems(); i++) {
		Area* area = fALMEngine->AreaAt(i);
		if (area->Item()->View() == this)
			continue;
		fTakenSpace.Include(area->Frame());
	}
}


void
LayoutEditView::_HightlightCustomizableView(Customizable* customizable)
{
	fSelectedArea = NULL;

	IViewContainer* viewContainer = dynamic_cast<IViewContainer*>(customizable);
	if (viewContainer == NULL)
		return;
	BView* view = viewContainer->View();
	for (int32 i = 0; i < fALMEngine->CountItems(); i++) {
		Area* area = fALMEngine->AreaAt(i);
		if (area->Item()->View() == view) {
			fSelectedArea = area;
			Invalidate();
			break;
		}
	}	
}


void
LayoutEditView::_StoreAction(EditAction* action)
{
	history_entry* entry = new history_entry(action);
	LayoutArchive layoutArchive(fALMEngine);
	layoutArchive.SaveLayout(&entry->layout, true);
	layoutArchive.SaveToAppFile("last_layout", &entry->layout);

	fHistory.AddEvent(entry);
}


void
LayoutEditView::_ResetHistory()
{
	fHistory.MakeEmpty(NULL);
	_StoreAction(NULL);
}


void
LayoutEditView::_UpdateCurrentLayout()
{
	history_entry* entry = fHistory.CurrentEvent();
	if (entry == NULL)
		debugger("we have no history!");

	LayoutArchive(fALMEngine).SaveLayout(&entry->layout, false);
}


static BMessage* make_alignment_message(int32 what, Area* area, int32 alignment)
{
	BMessage* message = new BMessage(what);
	message->AddPointer("area", area);
	message->AddInt32("alignment", alignment);
	return message;
}


void
LayoutEditView::_UpdateRightClickMenu(Area* area)
{
	fRightClickMenu->RemoveItems(0, fRightClickMenu->CountItems());

	delete fHAlignmentMenu;
	delete fVAlignmentMenu;
	fHAlignmentMenu = NULL;
	fVAlignmentMenu = NULL;

	fRightClickMenu->AddItem(fRemoveContent);
	fRightClickMenu->AddItem(fAddTab);

	if (area != NULL) {
		BMessage* message = NULL;

		fHAlignmentMenu = new BMenu("Alignment (horizontal)");
		fVAlignmentMenu = new BMenu("Alignment (vertical)");

		message = make_alignment_message(kSetHorizontalAlignmentMsg, area,
			B_ALIGN_LEFT);
		BMenuItem* hLeftItem = new BMenuItem("Left", message);
		message = make_alignment_message(kSetHorizontalAlignmentMsg, area,
			B_ALIGN_CENTER);
		BMenuItem* hCenterItem = new BMenuItem("Center", message);
		message = make_alignment_message(kSetHorizontalAlignmentMsg, area,
			B_ALIGN_RIGHT);
		BMenuItem* hRightItem = new BMenuItem("Right", message);
		message = make_alignment_message(kSetHorizontalAlignmentMsg, area,
			B_ALIGN_USE_FULL_WIDTH);
		BMenuItem* hFullWidthItem = new BMenuItem("Full Width", message);
	
		fHAlignmentMenu->AddItem(hLeftItem);
		fHAlignmentMenu->AddItem(hCenterItem);
		fHAlignmentMenu->AddItem(hRightItem);
		fHAlignmentMenu->AddItem(hFullWidthItem);

		message = make_alignment_message(kSetVerticalAlignmentMsg, area,
			B_ALIGN_TOP);
		BMenuItem* vTopItem = new BMenuItem("Top", message);
		message = make_alignment_message(kSetVerticalAlignmentMsg, area,
			B_ALIGN_MIDDLE);
		BMenuItem* vCenterItem = new BMenuItem("Center", message);
		message = make_alignment_message(kSetVerticalAlignmentMsg, area,
			B_ALIGN_BOTTOM);
		BMenuItem* vBottomItem = new BMenuItem("Bottom", message);
		message = make_alignment_message(kSetVerticalAlignmentMsg, area,
			B_ALIGN_USE_FULL_HEIGHT);
		BMenuItem* hFullHeightItem = new BMenuItem("Full Height", message);

		fVAlignmentMenu->AddItem(vTopItem);
		fVAlignmentMenu->AddItem(vCenterItem);
		fVAlignmentMenu->AddItem(vBottomItem);
		fVAlignmentMenu->AddItem(hFullHeightItem);

		fRightClickMenu->AddItem(fHAlignmentMenu);
		fRightClickMenu->AddItem(fVAlignmentMenu);

		BAlignment align = area->Item()->Alignment();
		alignment hAlignment = align.Horizontal();
		switch (hAlignment) {
		case B_ALIGN_LEFT:
			hLeftItem->SetMarked(true);
			break;
		case B_ALIGN_RIGHT:
			hRightItem->SetMarked(true);
			break;
		case B_ALIGN_USE_FULL_WIDTH:
			hFullWidthItem->SetMarked(true);
			break;
		default:
			hCenterItem->SetMarked(true);
			break;
		};
		vertical_alignment vAlignment = align.Vertical();
		switch (vAlignment) {
		case B_ALIGN_TOP:
			vTopItem->SetMarked(true);
			break;
		case B_ALIGN_BOTTOM:
			vBottomItem->SetMarked(true);
			break;
		case B_ALIGN_USE_FULL_HEIGHT:
			hFullHeightItem->SetMarked(true);
			break;
		default:
			vCenterItem->SetMarked(true);
			break;
		};

		fHAlignmentMenu->SetTargetForItems(this);
		fVAlignmentMenu->SetTargetForItems(this);
	}
	fRightClickMenu->SetTargetForItems(this);
}


void
LayoutEditView::_CheckTempEditConstraints()
{
	ConstraintList toBeRemoved;
	for (int32 i = 0; i < fALMEngine->CountConstraints(); i++) {
		Constraint* constraint = fALMEngine->ConstraintAt(i);
		if (BString(constraint->Label()) != "_EditHelper")
			continue;
		SummandList* summands = constraint->LeftSide();
		Summand* summand = summands->ItemAt(0);
		if (summand == NULL)
			debugger("Ups?");

		TabConnections* tabConnections = fOverlapManager.GetTabConnections();
		
		XTab* xTab = static_cast<XTab*>(summand->Var());
		if (fALMEngine->IndexOf(xTab) >= 0) {
			bool left = tabConnections->ConnectedToLeftBorder(xTab, fALMEngine);
			bool right = tabConnections->ConnectedToRightBorder(xTab,
				fALMEngine);
			if (xTab == fALMEngine->Left())
				right = false;
			else if (xTab == fALMEngine->Right())
				left = false;
			if (summands->CountItems() == 1 && (left || right)) {
				// position constraint
				toBeRemoved.AddItem(constraint);
			} else if (left && right) {
				// size constraint
				toBeRemoved.AddItem(constraint);				
			}
		} else {
			YTab* yTab = static_cast<YTab*>(summand->Var());
			if (fALMEngine->IndexOf(yTab) < 0)
				debugger("Should not happen");
			bool top = tabConnections->ConnectedToTopBorder(yTab, fALMEngine);
			bool bottom = tabConnections->ConnectedToBottomBorder(yTab,
				fALMEngine);
			if (yTab == fALMEngine->Top())
				bottom = false;
			else if (yTab == fALMEngine->Bottom())
				top = false;
			if (summands->CountItems() == 1 && (top || bottom)) {
				// position constraint
				toBeRemoved.AddItem(constraint);
			} else if (top && bottom) {
				// size constraint
				toBeRemoved.AddItem(constraint);
			}	
		}
	}

	for (int32 i = 0; i < toBeRemoved.CountItems(); i++)
		fALMEngine->RemoveConstraint(toBeRemoved.ItemAt(i), true);
}


void
LayoutEditView::_RecursiveExcludeViews(BLayoutItem* item, BRegion& region)
{
	BView* view = item->View();
	if (view == NULL) {
		BLayout* layout = dynamic_cast<BLayout*>(item);
		if (layout != NULL) {
			for (int32 i = 0; i < layout->CountItems(); i++)
				_RecursiveExcludeViews(layout->ItemAt(i), region);
			return;
		} else
			return;
	}
	BRect itemFrame = item->Frame();
	region.Exclude(itemFrame);
}

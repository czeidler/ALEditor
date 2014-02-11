/*
 * Copyright 2007-2008, Christof Lutteroth, lutteroth@cs.auckland.ac.nz
 * Copyright 2007-2008, James Kim, jkim202@ec.auckland.ac.nz
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	ALM_EDIT_VIEW_H
#define	ALM_EDIT_VIEW_H


#include "app/MessageFilter.h"
#include <MenuItem.h>
#include <Point.h>
#include <PopUpMenu.h>
#include <Region.h>
#include <SpaceLayoutItem.h>
#include <SupportDefs.h>
#include <View.h>

#include <ALMLayout.h>
#include <Customizable.h>
#include <CustomizableView.h>

#include "EditAnimation.h"
#include "InfoSystem.h"
#include "OverlapManager.h"


namespace BALM {

class Area;
class Column;
class Row;


class BALMEditor;
class EditWindow;


struct area_ref {
	area_ref(Area* area) {
		left = area->Left();
		right = area->Right();
		top = area->Top();
		bottom = area->Bottom();
	}

	area_ref() {
		left = NULL;
		right = NULL;
		top = NULL;
		bottom = NULL;
	}

	BRect Frame() const
	{
		BRect frame;
		if (left != NULL)
			frame.left = round(left->Value());
		if (right != NULL)
			frame.right = round(right->Value());
		if (top != NULL)
			frame.top = round(top->Value());
		if (bottom != NULL)
			frame.bottom = round(bottom->Value());
		return frame;
	}

	bool IsSet()
	{
		if (left != NULL || right != NULL)
			return true;
		return false;
	}

	void Unset()
	{
		left = NULL;
		right = NULL;
		top = NULL;
		bottom = NULL;
	}

	BReference<XTab>	left;
	BReference<XTab>	right;
	BReference<YTab>	top;
	BReference<YTab>	bottom;
};


struct area_info {
	area_info(Area* area)
	{
		BALMLayout* layout = static_cast<BALMLayout*>(area->Item()->Layout());
		left = layout->IndexOf(area->Left(), true);
		right = layout->IndexOf(area->Right(), true);
		top = layout->IndexOf(area->Top(), true);
		bottom = layout->IndexOf(area->Bottom(), true);
	}

	area_info(area_ref& ref, BALMLayout* layout)
	{
		SetTo(ref, layout);
	}

	area_info()
	{
		Unset();
	}

	area_ref to_ref(BALMLayout* layout)
	{
		area_ref ref;
		ref.left = layout->XTabAt(left, true);
		ref.top = layout->YTabAt(top, true);
		ref.right = layout->XTabAt(right, true);
		ref.bottom = layout->YTabAt(bottom, true);
		return ref;
	}

	void SetTo(area_ref& ref, BALMLayout* layout)
	{
		left = layout->IndexOf(ref.left, true);
		right = layout->IndexOf(ref.right, true);
		top = layout->IndexOf(ref.top, true);
		bottom = layout->IndexOf(ref.bottom, true);
	}

	void Unset()
	{
		left = -1;
		right = -1;
		top = -1;
		bottom = -1;
	}

	bool IsSet()
	{
		if (left >= 0 || right >= 0)
			return true;
		return false;
	}

	int32	left;
	int32	right;
	int32	top;
	int32	bottom;
};


CustomizableView* to_customizable_view(BLayoutItem* item);


template<class Type>
class HistoryManager {
public:
	HistoryManager()
		:
		fPosition(-1)
	{
		MakeEmpty(NULL);
	}

	bool AddEvent(Type* event)
	{
		bool status = fHistory.AddItem(event, fPosition + 1);
		if (!status)
			return false;
		fPosition++;

		while (fPosition + 1 < fHistory.CountItems())
			delete fHistory.RemoveItemAt(fPosition + 1);

		return true;
	}

	Type* CurrentEvent()
	{
		return fHistory.ItemAt(fPosition);
	}

	Type* MoveBackward()
	{
		Type* event = fHistory.ItemAt(fPosition);
		if (event != NULL)
			fPosition--;
		return event;
	}

	Type* MoveForward()
	{
		Type* event = fHistory.ItemAt(fPosition + 1);
		if (event != NULL)
			fPosition++;
		return event;
	}

	void MakeEmpty(BObjectList<Type>* list)
	{
		if (list != NULL)
			*list = fHistory;
		else {
			for (int32 i = 0; i < fHistory.CountItems(); i++)
				delete fHistory.ItemAt(i);
		}
		fHistory.MakeEmpty();
		fPosition = -1;			
	}

private:
	int32				fPosition;
	BObjectList<Type>	fHistory;
};


class EditAction;
class InsertionIntoEmptyArea;


#define GROUP_TAB_RESIZE 0


class LayoutEditView : public BView {
public:
	enum message_t {
		kQuitMsg = '&qui',
		kRemoveComponentMsg,
		kAddTabMsg,
		kSetHorizontalAlignmentMsg,
		kSetVerticalAlignmentMsg,
		kShowXTabs,
		kShowYTabs
	};

public:
								LayoutEditView(BALMEditor* editor);
								~LayoutEditView();

			void				AttachedToWindow();
			void				DetachedFromWindow();

			BALMLayout*			Layout();
			BALMEditor*			Editor();

			Area*				SelectedArea() const;
			void				SetSelectedArea(Area* area);

			void				SetShowXTabs(bool show);
			void				SetShowYTabs(bool show);

			bool				Undo();
			bool				Redo();

			BMessage*			CurrentLayout();
			bool				PerformAction(EditAction* action);
			bool				TestAction(EditAction* action,
									bool deleteAction = true);
			bool				TestAndPerformAction(EditAction* action);

			bool				TrashArea(Area* area);
protected:
			void				KeyDown(const char* bytes, int32 numBytes);

			void				MouseDown(BPoint point);
			void				MouseUp(BPoint point);
			void				MouseMoved(BPoint point, uint32 transit,
									const BMessage* message);
			void				DrawHighLightArea(const Area* area,
									float penSize, const rgb_color& color);
			void				DrawHighLightArea(const area_ref& ref,
									float penSize, const rgb_color& color);
			void				DrawHighLightArea(BRect frame,
									float penSize, const rgb_color& color);
			void				DrawResizeKnops(const Area* area);
			void				DrawTab(const XTab* tab, float penSize,
									const rgb_color& color);
			void				DrawTab(const YTab* tab, float penSize,
									const rgb_color& color);
			void				DrawXTabConnections(XTab* tab);
			void				DrawYTabConnections(YTab* tab);
			void				DrawAreaWithInnerTabs(const area_ref& ref);
			void				DrawTooSmallArea(Area* area);
			void				DrawSpacer(Area* area,
									BSpaceLayoutItem* spacer);
			void				DrawAreaBackground(Area* area);
			void				DrawTakenSpace();
			void				DrawTempConstraints();

			void				Draw(BRect updateRect);
			void				FrameResized(float width, float height);
			void				MessageReceived(BMessage* message);

			Area*				FindItemArea(BPoint point);
			Area*				FindArea(BPoint point);
			bool				FindEmptyArea(const BPoint& point,
									area_ref& ref, Area* ignore = NULL);

			XTab*				GetXTabNearPoint(BPoint p, int32 tolerance);
			YTab*				GetYTabNearPoint(BPoint p, int32 tolerance);
			XTab*				GetAreaXTabNearPoint(Area* area, BPoint point,
									int32 tolerance);
			YTab*				GetAreaYTabNearPoint(Area* area, BPoint point,
									int32 tolerance);

			XTab*				GetBestXTab(XTab* searchStart, BPoint p);
			YTab*				GetBestYTab(YTab* searchStart, BPoint p);

			/*! Check if the area is somehow connected to a certain side. */
			bool				ConnectedToLeftBorder(Area* area);
			bool				ConnectedToTopBorder(Area* area);
			bool				ConnectedToRightBorder(Area* area);
			bool				ConnectedToBottomBorder(Area* area);

			bool				ConnectedTo(Area* area, area_side side);

private:
			class State;
			class MouseOverState;
			class DragState;
			class DragAreaState;
			class DragSwapState;
			class DragMoveState;
			class DragInsertState;
			class DragBorderInsertState;
			class ResizeState;
			class InsertState;
			class UnTrashInsertState;
			class CreateAndInsertState;
			class DragOutsideState;
			class DragXTabState;
			class DragYTabState;
			friend class MouseOverState;
			friend class DragState;
			friend class DragAreaState;
			friend class DragSwapState;
			friend class DragMoveState;
			friend class DragInsertState;
			friend class DragBorderInsertState;
			friend class ResizeState;
			friend class InsertState;
			friend class DragXTabState;
			friend class DragYTabState;

			friend class InsertionIntoEmptyArea;

			void				_AreaFrame(BRect& rect);
			BRect				_AreaFrame(const Area* area);

			//! Find the area the user clicked on. The point might be not within
			//! the area frame, e.g. when the area is a enlarged area.
			Area*				_FindTooSmallArea(const BPoint& point);
			bool				_EnlargeTooSmallArea(BRect& frame);

			void				_ToALMLayoutCoordinates(BPoint& point);
			void				_SetState(State* state);

			float				_TabPosition(const XTab* tab) const;
			float				_TabPosition(const YTab* tab) const;

			void				_Quit();

			void				_NotifyAreaSelected(Area* area);

			//! Trigger the recalculation of the taken space.
			void				_InvalidateAreaData();

			void				_HightlightCustomizableView(
									Customizable* customizable);

			void				_StoreAction(EditAction* action);
			void				_ResetHistory();
			void				_UpdateCurrentLayout();

			void				_UpdateRightClickMenu(Area* area);

			void				_CheckTempEditConstraints();

			void				_RecursiveExcludeViews(BLayoutItem* item,
									BRegion& region);

			BALMEditor*			fEditor;
			BALMLayout*			fALMEngine;
			BMessageFilter*		fMessageFilter;

			Area*				fSelectedArea;
			XTab*				fSelectedXTab;
			YTab*				fSelectedYTab;
			bool				fShowXTabs;
			bool				fShowYTabs;
	
			BPopUpMenu*			fRightClickMenu;
			BMenuItem*			fRemoveContent;
			BMenuItem*			fAddTab;
			BMenu*				fHAlignmentMenu;
			BMenu*				fVAlignmentMenu;

			State*				fState;

			BRegion				fTakenSpace;

			Informant*			fInformant;

struct history_entry {
	history_entry(EditAction* a)
		:
		action(a)
	{}

	BMessage		layout;
	EditAction*		action;
};

			HistoryManager<history_entry>	fHistory;

			OverlapManager&		fOverlapManager;
			BPoint				fLastMenuPosition;

			EditAnimation		fEditAnimation;
};


}	// namespace BALM


#endif	// ALM_EDIT_VIEW_H

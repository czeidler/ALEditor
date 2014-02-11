/*
 * Copyright 2011-2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	EDIT_ACTION_INSERTING_H
#define	EDIT_ACTION_INSERTING_H


#include "EditAction.h"

#include <ALMEditor.h>
#include <CustomizableView.h>


class InsertNewHorizontalAction : public InsertHorizontalActionBase {
public:
	InsertNewHorizontalAction(BALMLayout* layout, BMessage* prevLayout,
		IViewContainer* item, int32 xTab, BArray<Area*> areas, area_side direction,
		vertical_alignment alignment)
		:
		InsertHorizontalActionBase(layout, prevLayout, xTab, areas, direction,
			alignment),
		fItem(item)
	{
	}

	virtual bool Undo()
	{
		BReference<IViewContainer> item = fItem.GetReference();
		if (item == NULL)
			return false;

		CustomizableRoster* roster = CustomizableRoster::DefaultRoster();
		roster->RemoveFromShelf(item);

		item->RemoveSelf();

		return EditAction::Undo();
	}

protected:
	virtual bool InsertObject(XTab* left, YTab* top, XTab* right, YTab* bottom)
	{
		BReference<IViewContainer> item = fItem.GetReference();
		if (item == NULL)
			return false;

		CustomizableRoster* roster = CustomizableRoster::DefaultRoster();
		roster->AddToShelf(item);

		if (item->View().Get() != NULL)
			fALMLayout->AddView(item->View(), left, top, right, bottom);
		else if (item->LayoutItem().Get() != NULL) {
			fALMLayout->AddItem(item->LayoutItem(), left, top, right,
				bottom);
		} else {
			roster->RemoveFromShelf(item);
			return false;
		}
		return true;
	}
	
private:
			BWeakReference<IViewContainer>	fItem;
};


class InsertNewVerticalAction : public InsertVerticalActionBase {
public:
	InsertNewVerticalAction(BALMLayout* layout, BMessage* prevLayout,
		IViewContainer* item, int32 yTab, BArray<Area*> areas,
		area_side direction, alignment alignment)
		:
		InsertVerticalActionBase(layout, prevLayout, yTab, areas, direction,
			alignment),
		fItem(item)
	{
	}

	virtual bool Undo()
	{
		BReference<IViewContainer> item = fItem.GetReference();
		if (item == NULL)
			return false;

		CustomizableRoster* roster = CustomizableRoster::DefaultRoster();
		roster->RemoveFromShelf(item);

		item->RemoveSelf();

		return EditAction::Undo();
	}

protected:
	virtual bool InsertObject(XTab* left, YTab* top, XTab* right, YTab* bottom)
	{
		BReference<IViewContainer> item = fItem.GetReference();
		if (item == NULL)
			return false;

		CustomizableRoster* roster = CustomizableRoster::DefaultRoster();
		roster->AddToShelf(item);

		if (item->View().Get() != NULL)
			fALMLayout->AddView(item->View(), left, top, right, bottom);
		else if (item->LayoutItem().Get() != NULL) {
			fALMLayout->AddItem(item->LayoutItem(), left, top, right,
				bottom);
		} else {
			roster->RemoveFromShelf(item);
			return false;
		}
		return true;
	}
	
private:
			BWeakReference<IViewContainer>	fItem;
};


class InsertNewInEmptyAreaAction : public EditAction {
public:
	InsertNewInEmptyAreaAction(BALMLayout* layout, BMessage* prevLayout,
		const BWeakReference<CustomizableView>& item,
		InsertionIntoEmptyArea& insertionIntoEmptyArea)
		:
		EditAction(layout, prevLayout),
		fItem(item),
		fInsertionIntoEmptyArea(insertionIntoEmptyArea)
	{
	}

	virtual bool Perform()
	{
		BReference<CustomizableView> item = fItem.GetReference();
		if (item == NULL)
			return false;

		CustomizableRoster* roster = CustomizableRoster::DefaultRoster();
		roster->AddToShelf(item);

		area_ref toRef = fInsertionIntoEmptyArea.CreateTargetArea();
	
		if (item->View().Get() != NULL) {
			fALMLayout->AddView(item->View(), toRef.left, toRef.top,
				toRef.right, toRef.bottom);
		} else if (item->LayoutItem().Get() != NULL) {
			fALMLayout->AddItem(item->LayoutItem(), toRef.left, toRef.top,
				toRef.right, toRef.bottom);
		} else
			return false;
		return true;
	}

	virtual bool Undo()
	{
		BReference<IViewContainer> item = fItem.GetReference();
		if (item == NULL)
			return false;

		CustomizableRoster* roster = CustomizableRoster::DefaultRoster();
		roster->RemoveFromShelf(item);

		item->RemoveSelf();

		return EditAction::Undo();
	}

protected:
			BWeakReference<CustomizableView>	fItem;
			InsertionIntoEmptyArea	fInsertionIntoEmptyArea;
};


class LayoutEditView::InsertState : public LayoutEditView::DragState {
public:
	InsertState(LayoutEditView* view, Customizable* component)
		:
		LayoutEditView::DragState(view, NULL),
		fNewComponent(component)
	{
		fViewContainer = dynamic_cast<CustomizableView*>(fNewComponent.Get());
	}

	virtual bool MouseMoved(BPoint point, uint32 transit,
		const BMessage* message)
	{
		if (transit == B_EXITED_VIEW) {
			fView->_SetState(NULL);
			return false;
		}
		return LayoutEditView::DragState::MouseMoved(point, transit, message);
	}

protected:
	virtual BSize DragObjectPrefSize()
	{
		return fViewContainer->PreferredSize();
	}

	virtual BSize DragObjectMinSize()
	{
		return fViewContainer->MinSize();
	}

	virtual	EditAction* CreateHInsertionAction()
	{
		return new InsertNewHorizontalAction(fView->fALMEngine,
			fView->CurrentLayout(), fViewContainer, fXTab, fAreas,
			fInsertDirection, fInsertBetweenAlignment.Vertical());
	}
	
	virtual	EditAction* CreateVInsertionAction()
	{
		return new InsertNewVerticalAction(fView->fALMEngine,
			fView->CurrentLayout(), fViewContainer, fYTab, fAreas,
			fInsertDirection, fInsertBetweenAlignment.Horizontal());
	}

	virtual EditAction*	CreateIntoEmptyAreaAction()
	{
		return new InsertNewInEmptyAreaAction(fView->fALMEngine,
			fView->CurrentLayout(), fViewContainer, fInsertionIntoEmptyArea);
	}

	virtual EditAction* CreateSwapAction()
	{
		return NULL;
	}

			BReference<Customizable>	fNewComponent;
			CustomizableView*	fViewContainer;
};


class LayoutEditView::UnTrashInsertState
	: public LayoutEditView::InsertState {
public:
	UnTrashInsertState(BALMEditor* editor, LayoutEditView* view,
		Customizable* customizable)
		:
		InsertState(view, customizable),

		fEditor(editor)
	{
	}

	virtual ~UnTrashInsertState()
	{
		if (fPerformed)
			fEditor->UnTrash(fNewComponent);
	}
private:
			BALMEditor*			fEditor;
};


class LayoutEditView::CreateAndInsertState
	: public LayoutEditView::InsertState {
public:
	CreateAndInsertState(LayoutEditView* view, const BString& component,
		const BMessage* message)
		:
		InsertState(view, NULL),

		fComponent(component)
	{
		CustomizableRoster* roster = CustomizableRoster::DefaultRoster();
		fNewComponent = roster->InstantiateCustomizable(component, message);
		
		fViewContainer = dynamic_cast<CustomizableView*>(fNewComponent.Get());
		if (fViewContainer != NULL) {
			fViewContainer->SetLooper(view->Looper());

			fViewContainer->SetIdentifier(view->Editor()->ProposeIdentifier(
				fViewContainer));
		}
	}

private:
			BString				fComponent;
};


#endif // EDIT_ACTION_INSERTING_H

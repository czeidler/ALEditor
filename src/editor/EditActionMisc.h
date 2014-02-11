/*
 * Copyright 2011-2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "EditAction.h"

#include <ALMEditor.h>


using namespace BALM;


class RemoveCustomizableViewAction : public EditAction {
public:
	RemoveCustomizableViewAction(BALMEditor* editor, BMessage* prevLayout,
		CustomizableView* customizable)
		:
		EditAction(editor->Layout(), prevLayout),

		fEditor(editor),
		fCustomizableView(customizable)
	{
	}

	virtual bool Perform()
	{
		BView* view = fCustomizableView->View();
		Area* area = NULL;
		if (view != NULL)
			area = fALMLayout->AreaFor(view);
		else {
			BLayoutItem* item = fCustomizableView->LayoutItem();
			if (item != NULL)
				area = fALMLayout->AreaFor(item);
		}
		if (area == NULL)
			return false;

		BReference<XTab> left = area->Left();
		BReference<YTab> top = area->Top();
		BReference<XTab> right = area->Right();
		BReference<YTab> bottom = area->Bottom();

		fCustomizableView->RemoveSelf();
		AreaRemoval::FillEmptySpace(fALMLayout, left, top, right, bottom);

		fEditor->Trash(fCustomizableView);
		return true;
	}

	virtual bool Undo()
	{
		return EditAction::Undo();
		debugger("Implement me!");
		//fALMLayout->AddView(fView);
			
		//todo implement
	}

protected:
			BALMEditor*			fEditor;
			CustomizableView*	fCustomizableView;
};


class SetAlignmentAction : public EditAction {
public:
	SetAlignmentAction(BLayoutItem* item, const BAlignment& alignment)
		:
		EditAction(NULL, NULL),
		fItem(item),
		fAlignment(alignment)
	{
		fPrevAlignment = fItem->Alignment();
	}

	virtual bool Perform()
	{
		fItem->SetExplicitAlignment(fAlignment);
		return true;
	}

	virtual bool Undo()
	{
		// TODO: thats not a real undo but the API does not allow to get the
		// explicit values
		fItem->SetExplicitAlignment(fPrevAlignment);
		return true;
	}

protected:
			BLayoutItem*		fItem;
			BAlignment			fAlignment;
			BAlignment			fPrevAlignment;
};

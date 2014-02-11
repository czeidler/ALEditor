/*
 * Copyright 2011-2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	EDIT_ACTION_SWAPPING_H
#define	EDIT_ACTION_SWAPPING_H


#include "EditActionAreaDragging.h"


class SwapAction : public EditAction {
public:
	SwapAction(BALMLayout* layout, BMessage* prevLayout, Area* from, Area* to)
		:
		EditAction(layout, prevLayout),
		fFromArea(from),
		fToArea(to)
	{
	}

	virtual bool
	Perform()
	{
		_Swap(fFromArea, fToArea);
		return true;
	}

	virtual	const char*
	Name()
	{
		return "swap";
	}

private:
	inline void _Swap(Area* from, Area* to)
	{
		BReference<XTab> left = from->Left();
		BReference<YTab> top = from->Top();
		BReference<XTab> right = from->Right();
		BReference<YTab> bottom = from->Bottom();

		from->SetLeft(to->Left());
		from->SetTop(to->Top());
		from->SetRight(to->Right());
		from->SetBottom(to->Bottom());

		to->SetLeft(left);
		to->SetTop(top);					
		to->SetRight(right);
		to->SetBottom(bottom);
	}

protected:
			Area*				fFromArea;
			Area*				fToArea;
};


#endif // EDIT_ACTION_SWAPPING_H

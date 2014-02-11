/*
 * Copyright 2011-2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	NEUTRAL_ITEM_MANAGER_H
#define	NEUTRAL_ITEM_MANAGER_H

#include <View.h>

#include <ALMLayout.h>
#include <Customizable.h>

#include <stdio.h>


namespace BALM {


class NeutralLayoutItem : public BSpaceLayoutItem {
public:
	NeutralLayoutItem()
		:
		BSpaceLayoutItem(BSize(-1, -1),
			BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED),
			BSize(-1, -1),
			BAlignment(B_ALIGN_HORIZONTAL_CENTER, B_ALIGN_VERTICAL_CENTER))
	{
	}
};


class NeutralItemManager {
public:
	NeutralItemManager(BALMLayout* layout, BView* editView)
		:
		fALMLayout(layout),
		fEditView(editView)
	{		
	}

	~NeutralItemManager()
	{
		_RemoveFiller();
	}

	void Fill()
	{
		_RemoveFiller();
		_InsertFiller();
	}

private:
	bool _InsertFiller()
	{
		BObjectList<Area> clearList;
		_Clear(fALMLayout->Top(), fALMLayout->Left(), fALMLayout->Right(),
			clearList);
		return true;
	}

	void _RemoveFiller()
	{
		for (int32 i = 0; i < fFillerList.CountItems(); i++) {
			BSpaceLayoutItem* item = fFillerList.ItemAt(i);
			fALMLayout->RemoveItem(item);
			delete item;
		}
		fFillerList.MakeEmpty();
	}

	void _Clear(YTab* top, XTab* start, XTab* end, BObjectList<Area>& clearList)
	{
		if (top == fALMLayout->Bottom())
			return;

		XTab* currentXTab = start;
		while (currentXTab != end
			&& currentXTab->Value() < end->Value()) {
			Area* area = _FindConnectedBottomArea(currentXTab, top);
			if (area == NULL) {
				// insert spacer
				XTab* tempRight = _FindNextConnectedToRight(currentXTab, top, end);
				YTab* bottomTab = _FindFreeTabBottom(top, currentXTab, tempRight);
				XTab* rightTab = tempRight;//_FindFreeTabRight(currentXTab, top, bottomTab);

				NeutralLayoutItem* item = new NeutralLayoutItem;
				area = fALMLayout->AddItem(item, currentXTab, top, rightTab,
					bottomTab);
			}
			//else if (currentXTab == start) {
				// check if it is connected to the left
				_ConnectToTheLeft(area);
			//}

			currentXTab = area->Right();
			if (clearList.HasItem(area))
				continue;
			clearList.AddItem(area);
			_Clear(area->Bottom(), area->Left(), area->Right(), clearList);
		}
	}

	Area* _FindConnectedBottomArea(XTab* left, YTab* top)
	{
		const float leftPosition = left->Value();
		for (int32 i = 0; i < fALMLayout->CountAreas(); i++) {
			Area* area = fALMLayout->AreaAt(i);
			if (area->Item()->View() == fEditView || area->Top() != top)
				continue;
			if (area->Left() == left || (area->Left()->Value() <= leftPosition
					&& area->Right()->Value() > leftPosition))
				return area;
		}
		return NULL;
	}

	XTab* _FindNextConnectedToRight(XTab* left, YTab* top, XTab* maxRight)
	{
		const float leftPos = left->Value();
		const float rightPos = maxRight->Value();
		float distance = HUGE_VAL;
		XTab* tab = NULL;
		for (int32 i = 0; i < fALMLayout->CountAreas(); i++) {
			Area* rightArea = fALMLayout->AreaAt(i);
			if (rightArea->Top() != top)
				continue;
			if (rightPos <= rightArea->Left()->Value())
				continue;
			float d = rightArea->Left()->Value() - leftPos;
			if (d < 0)
				continue;
			if (d < distance) {
				distance = d;
				tab = rightArea->Left();
			}
		}
		if (tab != NULL)
			return tab;
		return maxRight;
	}

	void _ConnectToTheLeft(Area* area)
	{
		if (area->Left() == fALMLayout->Left())
			return;

		const float topPos = area->Top()->Value();
		const float bottomPos = area->Bottom()->Value();
		int32 nAreas = fALMLayout->CountAreas();
		for (int32 i = 0; i < nAreas; i++) {
			Area* leftArea = fALMLayout->AreaAt(i);
			if (leftArea->Right() == area->Left())
				continue;
			if (_FuzzyEqual(leftArea->Right()->Value(), area->Left()->Value())
				== false)
				continue;
			if (leftArea->Bottom()->Value() < topPos
				|| leftArea->Top()->Value() > bottomPos)
				continue;
			// add spacer
			YTab* top = area->Top();
			if (leftArea->Top()->Value() > topPos)
				top = leftArea->Top();
			YTab* bottom = area->Bottom();
			if (leftArea->Bottom()->Value() < bottomPos)
				bottom = leftArea->Bottom();
			NeutralLayoutItem* item = new NeutralLayoutItem;
			Area* newArea = fALMLayout->AddItem(item, leftArea->Right(), top,
				area->Left(), bottom);
		}
	}

	bool _FuzzyEqual(float var1, float var2)
	{
		if (fabs(var1 - var2) < 0.001)
			return true;
		return false;
	}

	XTab* _FindFreeTabRight(XTab* left, YTab* top, YTab* bottom)
	{
		const float topPos = top->Value();
		const float bottomPos = bottom->Value();
		float distance = HUGE_VAL;
		XTab* tab = NULL;
		for (int32 i = 0; i < fALMLayout->CountAreas(); i++) {
			Area* rightArea = fALMLayout->AreaAt(i);
			float d = rightArea->Left()->Value() - left->Value();
			if (d < 0)
				continue;
			if (rightArea->Bottom()->Value() <= topPos
				|| rightArea->Top()->Value() >= bottomPos)
				continue;
			if (d < distance) {
				distance = d;
				tab = rightArea->Left();
			}
		}
		if (tab != NULL)
			return tab;
		return fALMLayout->Right();
	}

	YTab* _FindFreeTabBottom(YTab* top, XTab* left, XTab* right)
	{
		const float leftPos = left->Value();
		const float rightPos = right->Value();
		float distance = HUGE_VAL;
		YTab* tab = NULL;
		for (int32 i = 0; i < fALMLayout->CountAreas(); i++) {
			Area* bottomArea = fALMLayout->AreaAt(i);
			float d = bottomArea->Top()->Value() - top->Value();
			if (d < 0)
				continue;
			if (bottomArea->Right()->Value() <= leftPos
				|| bottomArea->Left()->Value() >= rightPos)
				continue;
			if (d < distance) {
				distance = d;
				tab = bottomArea->Top();
			}
		}
		if (tab != NULL)
			return tab;
		return fALMLayout->Bottom();
	}

private:
			BALMLayout*			fALMLayout;
			BView*				fEditView;
			BObjectList<BSpaceLayoutItem>	fFillerList;
};


}	// namespace BALM


#endif	// NEUTRAL_ITEM_MANAGER_H

/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	AREA_REMOVAL_H
#define	AREA_REMOVAL_H


#include "GroupDetection.h"
#include "OverlapManager.h"


namespace BALM {


class AreaRemoval {
public:
	AreaRemoval(TabConnections* connections, BALMLayout* layout)
		:
		fTabConnections(connections),
		fLayout(layout)
	{
	}

	
	static void FillEmptySpace(BALMLayout* layout, XTab* left, YTab* top,
		XTab* right, YTab* bottom)
	{
		TabConnections connections;
		AreaRemoval areaRemoval(&connections, layout);
		areaRemoval.AreaRemoved(left, top, right, bottom);
	}

	void AreaRemoved(XTab* left, YTab* top, XTab* right, YTab* bottom)
	{
		// TODO maybe do this a bit more efficient somewhere else?
		fTabConnections->Fill(fLayout);

		std::map<XTab*, tab_links<XTab> >& xConnections
			= fTabConnections->GetXTabLinkMap();
		std::map<YTab*, tab_links<YTab> >& yConnections
			= fTabConnections->GetYTabLinkMap();

		if (!fTabConnections->ConnectedToLeftBorder(left, fLayout)
			&& !fTabConnections->ConnectedToRightBorder(left, fLayout)) {
			_MoveGroups<XTab, ar_right_search>(xConnections[left].areas1);
			_MoveGroups<XTab, ar_right_search>(xConnections[left].areas2);
		}

		if (!fTabConnections->ConnectedToLeftBorder(right, fLayout)
			&& !fTabConnections->ConnectedToRightBorder(right, fLayout)) {
			_MoveGroups<XTab, ar_left_search>(xConnections[right].areas2);
			_MoveGroups<XTab, ar_left_search>(xConnections[right].areas1);
		}

		if (!fTabConnections->ConnectedToTopBorder(top, fLayout)
			&& !fTabConnections->ConnectedToBottomBorder(top, fLayout)) {
			_MoveGroups<YTab, ar_bottom_search>(yConnections[top].areas1);
			_MoveGroups<YTab, ar_bottom_search>(yConnections[top].areas2);
		}

		if (!fTabConnections->ConnectedToTopBorder(bottom, fLayout)
			&& !fTabConnections->ConnectedToBottomBorder(bottom, fLayout)) {
			_MoveGroups<YTab, ar_top_search>(yConnections[bottom].areas2);
			_MoveGroups<YTab, ar_top_search>(yConnections[bottom].areas1);
		}
	}

	void AreaDetachedX(Area* area, area_side side, XTab* oldTab)
	{
		fTabConnections->Fill(fLayout);

		BObjectList<Area> areaList;
		areaList.AddItem(area);

		std::map<XTab*, tab_links<XTab> >& connections
			= fTabConnections->GetXTabLinkMap();
		if (connections.find(oldTab) == connections.end())
			oldTab = NULL;

		if (side == kLeft) {
			_MoveGroups<XTab, ar_right_search>(areaList);
			if (oldTab != NULL) {
				_MoveGroups<XTab, ar_left_search>(
					connections[oldTab].areas1);
				_MoveGroups<XTab, ar_right_search>(
					connections[oldTab].areas2);
			}
		} else if (side == kRight) {
			_MoveGroups<XTab, ar_left_search>(areaList);
			if (oldTab != NULL) {
				_MoveGroups<XTab, ar_left_search>(
					connections[oldTab].areas1);
				_MoveGroups<XTab, ar_right_search>(
					connections[oldTab].areas2);
			}
		}
	}

	void AreaDetachedY(Area* area, area_side side, YTab* oldTab)
	{
		fTabConnections->Fill(fLayout);

		BObjectList<Area> areaList;
		areaList.AddItem(area);

		std::map<YTab*, tab_links<YTab> >& connections
			= fTabConnections->GetYTabLinkMap();
		if (connections.find(oldTab) == connections.end())
			oldTab = NULL;

		if (side == kTop) {
			_MoveGroups<YTab, ar_bottom_search>(areaList);
			if (oldTab != NULL) {
				_MoveGroups<YTab, ar_top_search>(
					connections[oldTab].areas1);
				_MoveGroups<YTab, ar_bottom_search>(
					connections[oldTab].areas2);
			}
		} else if (side == kBottom) {
			_MoveGroups<YTab, ar_top_search>(areaList);
			if (oldTab != NULL) {
				_MoveGroups<YTab, ar_top_search>(
					connections[oldTab].areas1);
				_MoveGroups<YTab, ar_bottom_search>(
					connections[oldTab].areas2);
			}
		}
	}

private:
	template<typename TabType, typename SearchDirection>
	void _MoveGroups(const BObjectList<Area>& groupSeeds)
	{
		SearchDirection search;
		for (int32 i = 0; i < groupSeeds.CountItems(); i++) {
			Area* area = groupSeeds.ItemAt(i);

			BObjectList<Area> group;
			if (!_FindDetachedGroup<TabType, SearchDirection>(area, group))
				continue;

			Area* closestInGroup;
			float distance = HUGE_VAL;
			TabType* closestTab = _FindClosestTab<TabType, SearchDirection>(
				group, &closestInGroup, distance);
			BPoint delta;
			search.MakeDelta(delta, distance);
			_MoveGroupBy(group, delta);

			// this only fixes the connected area relations
			search.UpdateConnections(fTabConnections, closestInGroup, closestTab);
		}
	}

	void UpdateConnections(TabConnections* tabConnections,
		Area* closestInGroup, YTab* closestTab)
	{
		std::map<YTab*, tab_links<YTab> >& yConnections
			= tabConnections->GetYTabLinkMap();
		tab_links<YTab>& oldLink = yConnections[closestInGroup->Top()];
		oldLink.areas2.RemoveItem(closestInGroup);
		closestInGroup->SetTop(closestTab);
		tab_links<YTab>& newLink = yConnections[closestTab];
		newLink.areas2.AddItem(closestInGroup);
	}

	//! return false if group is attached to a layout border
	template<typename TabType, typename SearchDirection>
	bool _FindDetachedGroup(Area* startArea, BObjectList<Area>& group)
	{
		SearchDirection search;
		if (search.SearchTab(startArea) == search.LayoutBorder(fLayout)
			|| search.OppositeSearchTab(startArea)
				== search.OppositeLayoutBorder(fLayout))
			return false;

		if (group.HasItem(startArea))
			return true;
		group.AddItem(startArea);

		
		if (_CollectAreas<TabType, SearchDirection>(
			search.SearchTab(startArea), group) == false)
			return false;
		if (_CollectAreas<TabType, SearchDirection>(
			search.OppositeSearchTab(startArea), group) == false)
			return false;

		return true;
	}

	template<typename TabType, typename SearchDirection>
	inline bool _CollectAreas(TabType* tab, BObjectList<Area>& group)
	{
		SearchDirection search;

		std::map<TabType*, tab_links<TabType> >& connections
			= search.LinkMap(fTabConnections);
		tab_links<TabType>& links = connections[tab];
		// areas1 
		for (int32 i = 0; i < links.areas1.CountItems(); i++) {
			Area* area = links.areas1.ItemAt(i);
			if (_FindDetachedGroup<TabType, SearchDirection>(
				area, group) == false)
				return false;
		}
		// areas2
		for (int32 i = 0; i < links.areas2.CountItems(); i++) {
			Area* area = links.areas2.ItemAt(i);
			if (_FindDetachedGroup<TabType, SearchDirection>(
				area, group) == false)
				return false;
		}
		return true;
	}

	struct ar_left_search : left_search {
		float Compare(Area* left, Area* right)
		{
			return right->Left()->Value() - left->Right()->Value();
		}

		void MakeDelta(BPoint& point, float distance)
		{
			point.x = -distance;
		}

		void UpdateConnections(TabConnections* tabConnections,
			Area* closestInGroup, XTab* closestTab)
		{
			std::map<XTab*, tab_links<XTab> >& xConnections
				= tabConnections->GetXTabLinkMap();
			tab_links<XTab>& oldLink = xConnections[closestInGroup->Left()];
			oldLink.areas2.RemoveItem(closestInGroup);
			closestInGroup->SetLeft(closestTab);
			tab_links<XTab>& newLink = xConnections[closestTab];
			newLink.areas2.AddItem(closestInGroup);
		}

	};

	struct ar_top_search : top_search {
		float Compare(Area* top, Area* bottom)
		{
			return bottom->Top()->Value() - top->Bottom()->Value();
		}

		void MakeDelta(BPoint& point, float distance)
		{
			point.y = -distance;
		}

		void UpdateConnections(TabConnections* tabConnections,
			Area* closestInGroup, YTab* closestTab)
		{
			std::map<YTab*, tab_links<YTab> >& yConnections
				= tabConnections->GetYTabLinkMap();
			tab_links<YTab>& oldLink = yConnections[closestInGroup->Top()];
			oldLink.areas2.RemoveItem(closestInGroup);
			closestInGroup->SetTop(closestTab);
			tab_links<YTab>& newLink = yConnections[closestTab];
			newLink.areas2.AddItem(closestInGroup);
		}
	};

	struct ar_right_search : right_search {
		float Compare(Area* right, Area* left)
		{
			return right->Left()->Value() - left->Right()->Value();
		}

		void MakeDelta(BPoint& point, float distance)
		{
			point.x = distance;
		}

		void UpdateConnections(TabConnections* tabConnections,
			Area* closestInGroup, XTab* closestTab)
		{
			std::map<XTab*, tab_links<XTab> >& xConnections
				= tabConnections->GetXTabLinkMap();
			tab_links<XTab>& oldLink = xConnections[closestInGroup->Right()];
			oldLink.areas1.RemoveItem(closestInGroup);
			closestInGroup->SetRight(closestTab);
			tab_links<XTab>& newLink = xConnections[closestTab];
			newLink.areas1.AddItem(closestInGroup);
		}
	};

	struct ar_bottom_search : bottom_search {
		float Compare(Area* bottom, Area* top)
		{
			return bottom->Top()->Value() - top->Bottom()->Value();
		}

		void MakeDelta(BPoint& point, float distance)
		{
			point.y = distance;
		}

		void UpdateConnections(TabConnections* tabConnections,
			Area* closestInGroup, YTab* closestTab)
		{
			std::map<YTab*, tab_links<YTab> >& yConnections
				= tabConnections->GetYTabLinkMap();
			tab_links<YTab>& oldLink = yConnections[closestInGroup->Bottom()];
			oldLink.areas1.RemoveItem(closestInGroup);
			closestInGroup->SetBottom(closestTab);
			tab_links<YTab>& newLink = yConnections[closestTab];
			newLink.areas1.AddItem(closestInGroup);
		}
	};

	template<typename SearchDirection>
	float _Distance(Area* outArea, Area* inGroupArea)
	{
		SearchDirection search;
		if (search.OrthTab1(outArea)->Value()
			>= search.OrthTab2(inGroupArea)->Value()
			|| search.OrthTab2(outArea)->Value()
			<= search.OrthTab1(inGroupArea)->Value())
			return -1;

		return search.Compare(outArea, inGroupArea);
	}

	//! return NULL means there is no Area to the left
	template<class Type, typename SearchDirection>
	Type* _FindClosestTab(const BObjectList<Area>& group, Area** groupArea,
		float& distance)
	{
		SearchDirection search;
		*groupArea = group.ItemAt(0);
		distance = HUGE_VAL;
		Type* closestTab = NULL;

		for (int32 i = 0; i < fLayout->CountAreas(); i++) {
			Area* area = fLayout->AreaAt(i);
			if (group.HasItem(area))
				continue;
			for (int32 a = 0; a < group.CountItems(); a++) {
				Area* inGroupArea = group.ItemAt(a);
				float dist = _Distance<SearchDirection>(area, inGroupArea);
				if (dist < 0)
					continue;
				if (dist < distance) {
					distance = dist;
					closestTab = search.OppositeSearchTab(area);
					*groupArea = inGroupArea;
				}
			}
		}

		if (closestTab == NULL) {
			closestTab = search.LayoutBorder(fLayout);

			distance = HUGE_VAL;
			for (int32 a = 0; a < group.CountItems(); a++) {
				Area* inGroupArea = group.ItemAt(a);
				Type* tab = search.SearchTab(inGroupArea);
				float dist = fabs(tab->Value() - closestTab->Value());
				if (dist < distance) {
					distance = dist;
					*groupArea = inGroupArea;
				}
			}
		}

		return closestTab;
	}

	void _MoveGroupBy(const BObjectList<Area>& group, const BPoint& delta)
	{
		BObjectList<XTab> xTabs;
		BObjectList<YTab> yTabs;
		for (int32 i = 0; i < group.CountItems(); i++) {
			Area* area = group.ItemAt(i);

			XTab* left = area->Left();
			if (!xTabs.HasItem(left)) {
				xTabs.AddItem(left);
				area->Left()->SetValue(left->Value() + delta.x);
			}
			XTab* right = area->Right();
			if (!xTabs.HasItem(right)) {
				xTabs.AddItem(right);
				area->Right()->SetValue(right->Value() + delta.x);
			}
			YTab* top = area->Top();
			if (!yTabs.HasItem(top)) {
				yTabs.AddItem(top);
				area->Top()->SetValue(top->Value() + delta.y);
			}
			YTab* bottom = area->Bottom();
			if (!yTabs.HasItem(bottom)) {
				yTabs.AddItem(bottom);
				area->Bottom()->SetValue(bottom->Value() + delta.y);
			}		
		}
	}

private:
			TabConnections*		fTabConnections;
			BALMLayout*			fLayout;
};


}	// namespace BALM


#endif	// AREA_REMOVAL_H

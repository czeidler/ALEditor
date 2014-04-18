/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	AREA_REMOVAL_H
#define	AREA_REMOVAL_H


#include "GroupDetection.h"
#include "OverlapManager.h"


namespace BALM {

// These functions should be/ have been members of AreaRemoval but gcc2 don't
// likes template methods in classes (?). In gcc4 it works fine.
namespace AreaRemovalHelper {

	template<typename TabType, typename SearchDirection>
	inline bool _CollectAreas(BALMLayout *layout,
		TabConnections *tabConnections, TabType* tab, BObjectList<Area>& group);
	
	//! return false if group is attached to a layout border
	template<typename TabType, typename SearchDirection>
	bool _FindDetachedGroup(BALMLayout *layout, TabConnections *tabConnections,
		Area* startArea, BObjectList<Area>& group)
	{
		SearchDirection search;
		if (search.SearchTab(startArea) == search.LayoutBorder(layout)
			|| search.OppositeSearchTab(startArea)
				== search.OppositeLayoutBorder(layout))
			return false;

		if (group.HasItem(startArea))
			return true;
		group.AddItem(startArea);

		
		if (_CollectAreas<TabType, SearchDirection>(layout, tabConnections,
			search.SearchTab(startArea), group) == false)
			return false;
		if (_CollectAreas<TabType, SearchDirection>(layout, tabConnections,
			search.OppositeSearchTab(startArea), group) == false)
			return false;

		return true;
	}
	
	template<typename TabType, typename SearchDirection>
	inline bool _CollectAreas(BALMLayout *layout, TabConnections *tabConnections,
		TabType* tab, BObjectList<Area>& group)
	{
		SearchDirection search;

		std::map<TabType*, tab_links<TabType> >& connections
			= search.LinkMap(tabConnections);
		tab_links<TabType>& links = connections[tab];
		// areas1 
		for (int32 i = 0; i < links.areas1.CountItems(); i++) {
			Area* area = links.areas1.ItemAt(i);
			if (_FindDetachedGroup<TabType, SearchDirection>(
				layout, tabConnections, area, group) == false)
				return false;
		}
		// areas2
		for (int32 i = 0; i < links.areas2.CountItems(); i++) {
			Area* area = links.areas2.ItemAt(i);
			if (_FindDetachedGroup<TabType, SearchDirection>(
				layout, tabConnections, area, group) == false)
				return false;
		}
		return true;
	}
		
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
	Type* _FindClosestTab(BALMLayout *layout, const BObjectList<Area>& group,
		Area** groupArea, float& distance)
	{
		SearchDirection search;
		*groupArea = group.ItemAt(0);
		distance = HUGE_VAL;
		Type* closestTab = NULL;

		for (int32 i = 0; i < layout->CountAreas(); i++) {
			Area* area = layout->AreaAt(i);
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
			closestTab = search.LayoutBorder(layout);

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
	
	void _MoveGroupBy(const BObjectList<Area>& group, const BPoint& delta);
	
	template<typename TabType, typename SearchDirection>
	void MoveGroups(BALMLayout *layout, TabConnections *tabConnections,
		const BObjectList<Area>& groupSeeds)
	{
		SearchDirection search;
		for (int32 i = 0; i < groupSeeds.CountItems(); i++) {
			Area* area = groupSeeds.ItemAt(i);

			BObjectList<Area> group;
			if (!_FindDetachedGroup<TabType, SearchDirection>(layout,
				tabConnections, area, group))
				continue;

			Area* closestInGroup;
			float distance = HUGE_VAL;
			TabType* closestTab = _FindClosestTab<TabType, SearchDirection>(
				layout, group, &closestInGroup, distance);
			BPoint delta;
			search.MakeDelta(delta, distance);
			_MoveGroupBy(group, delta);

			// this only fixes the connected area relations
			search.UpdateConnections(tabConnections, closestInGroup, closestTab);
		}
	}
};

class AreaRemoval {
public:
								AreaRemoval(TabConnections* connections,
									BALMLayout* layout);
	
	static	void				FillEmptySpace(BALMLayout* layout, XTab* left,
									YTab* top, XTab* right, YTab* bottom);

			void				AreaRemoved(XTab* left, YTab* top, XTab* right,
									YTab* bottom);
			void				AreaDetachedX(Area* area, area_side side,
									XTab* oldTab);
			void				AreaDetachedY(Area* area, area_side side,
									YTab* oldTab);

private:
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

private:
			TabConnections*		fTabConnections;
			BALMLayout*			fLayout;
};


}	// namespace BALM


#endif	// AREA_REMOVAL_H

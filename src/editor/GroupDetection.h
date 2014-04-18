/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	GROUP_DETECTION_H
#define	GROUP_DETECTION_H


#include <ArrayContainer.h>

#include "OverlapManager.h"
#include "LayoutEditView.h"


namespace BALM {


struct horizontal_search {
	YTab* OrthTab1(Area* area)
	{
		return area->Top();
	}

	YTab* OrthTab2(Area* area)
	{
		return area->Bottom();
	}

	YTab* OrthLayoutBorder1(BALMLayout* layout)
	{
		return layout->Top();
	}

	YTab* OrthLayoutBorder2(BALMLayout* layout)
	{
		return layout->Bottom();
	}

	float Side1(BRect& rect)
	{
		return rect.left;
	}

	float Side2(BRect& rect)
	{
		return rect.right;
	}

	float OrthSide1(BRect& rect)
	{
		return rect.top;
	}

	float OrthSide2(BRect& rect)
	{
		return rect.bottom;
	}

	float Extent(BRect& rect)
	{
		return rect.Width();
	}

	float OrthExtent(BRect& rect)
	{
		return rect.Height();
	}

	int32 CountTabs(BALMLayout* layout)
	{
		return layout->CountXTabs();
	}

	XTab* TabAt(BALMLayout* layout, int32 i)
	{
		return layout->XTabAt(i);
	}

	std::map<XTab*, tab_links<XTab> >& LinkMap(TabConnections* connections)
	{
		return connections->GetXTabLinkMap();
	}

	std::map<YTab*, tab_links<YTab> >& OrthLinkMap(TabConnections* connections)
	{
		return connections->GetYTabLinkMap();
	}
};


struct vertical_search {
	XTab* OrthTab1(Area* area)
	{
		return area->Left();
	}

	XTab* OrthTab2(Area* area)
	{
		return area->Right();
	}

	XTab* OrthLayoutBorder1(BALMLayout* layout)
	{
		return layout->Left();
	}

	XTab* OrthLayoutBorder2(BALMLayout* layout)
	{
		return layout->Right();
	}

	float Side1(BRect& rect)
	{
		return rect.top;
	}

	float Side2(BRect& rect)
	{
		return rect.bottom;
	}

	float OrthSide1(BRect& rect)
	{
		return rect.left;
	}

	float OrthSide2(BRect& rect)
	{
		return rect.right;
	}

	float Extent(BRect& rect)
	{
		return rect.Height();
	}

	float OrthExtent(BRect& rect)
	{
		return rect.Width();
	}

	int32 CountTabs(BALMLayout* layout)
	{
		return layout->CountYTabs();
	}

	YTab* TabAt(BALMLayout* layout, int32 i)
	{
		return layout->YTabAt(i);
	}

	std::map<YTab*, tab_links<YTab> >& LinkMap(TabConnections* connections)
	{
		return connections->GetYTabLinkMap();
	}

	std::map<XTab*, tab_links<XTab> >& OrthLinkMap(TabConnections* connections)
	{
		return connections->GetXTabLinkMap();
	}
};


struct left_search : horizontal_search {
	area_side SearchDirection()
	{
		return kLeft;
	}

	XTab* SearchTab(Area* area)
	{
		return area->Left();
	}
	
	XTab* OppositeSearchTab(Area* area)
	{
		return area->Right();
	}

	XTab* LayoutBorder(BALMLayout* layout)
	{
		return layout->Left();
	}

	XTab* OppositeLayoutBorder(BALMLayout* layout)
	{
		return layout->Right();
	}

	float SearchSide(BRect& rect)
	{
		return rect.left;
	}

	float OppositeSearchSide(BRect& rect)
	{
		return rect.right;
	}

	BObjectList<Area>& AreasSearchDirection(tab_links<XTab>& links)
	{
		return links.areas1;
	}

	BObjectList<Area>& AreasOppositeDirection(tab_links<XTab>& links)
	{
		return links.areas2;
	}
};

struct top_search : vertical_search {
	area_side SearchDirection()
	{
		return kTop;
	}

	YTab* SearchTab(Area* area)
	{
		return area->Top();
	}
	
	YTab* OppositeSearchTab(Area* area)
	{
		return area->Bottom();
	}
	
	YTab* LayoutBorder(BALMLayout* layout)
	{
		return layout->Top();
	}

	YTab* OppositeLayoutBorder(BALMLayout* layout)
	{
		return layout->Bottom();
	}

	float SearchSide(BRect& rect)
	{
		return rect.top;
	}

	float OppositeSearchSide(BRect& rect)
	{
		return rect.bottom;
	}

	BObjectList<Area>& AreasSearchDirection(tab_links<YTab>& links)
	{
		return links.areas1;
	}

	BObjectList<Area>& AreasOppositeDirection(tab_links<YTab>& links)
	{
		return links.areas2;
	}
};
	
struct right_search : horizontal_search{
	area_side SearchDirection()
	{
		return kRight;
	}

	XTab* SearchTab(Area* area)
	{
		return area->Right();
	}
	
	XTab* OppositeSearchTab(Area* area)
	{
		return area->Left();
	}
	
	XTab* LayoutBorder(BALMLayout* layout)
	{
		return layout->Right();
	}

	XTab* OppositeLayoutBorder(BALMLayout* layout)
	{
		return layout->Left();
	}

	float SearchSide(BRect& rect)
	{
		return rect.right;
	}

	float OppositeSearchSide(BRect& rect)
	{
		return rect.left;
	}

	BObjectList<Area>& AreasSearchDirection(tab_links<XTab>& links)
	{
		return links.areas2;
	}

	BObjectList<Area>& AreasOppositeDirection(tab_links<XTab>& links)
	{
		return links.areas1;
	}
};
	
struct bottom_search : vertical_search{
	area_side SearchDirection()
	{
		return kBottom;
	}

	YTab* SearchTab(Area* area)
	{
		return area->Bottom();
	}
	
	YTab* OppositeSearchTab(Area* area)
	{
		return area->Top();
	}

	YTab* LayoutBorder(BALMLayout* layout)
	{
		return layout->Bottom();
	}

	YTab* OppositeLayoutBorder(BALMLayout* layout)
	{
		return layout->Top();
	}

	float SearchSide(BRect& rect)
	{
		return rect.bottom;
	}

	float OppositeSearchSide(BRect& rect)
	{
		return rect.top;
	}

	BObjectList<Area>& AreasSearchDirection(tab_links<YTab>& links)
	{
		return links.areas2;
	}

	BObjectList<Area>& AreasOppositeDirection(tab_links<YTab>& links)
	{
		return links.areas1;
	}
};

// These functions should be/ have been members of GroupDetection but gcc2 don't
// likes template methods in classes (?). In gcc4 it works fine.
namespace GroupDetectionHelper {
	template<class SearchTab, typename SearchDirection, class OrthTab,
		typename OrthDirection>
		Area* _FindNeighbour(TabConnections *connections, SearchTab* tab,
			Area* area)
	{
		SearchDirection search;
		OrthDirection orthSearch;
		tab_links<SearchTab>& tabLinks = orthSearch.LinkMap(connections)[tab];
		tab_links<OrthTab>& areaLinks
			= search.LinkMap(connections)[search.SearchTab(area)];
	
		BObjectList<Area>& tabAreas = orthSearch.AreasSearchDirection(tabLinks);
		for (int32 i = 0; i < tabAreas.CountItems(); i++) {
			Area* tabArea = tabAreas.ItemAt(i);
			if (search.AreasSearchDirection(areaLinks).HasItem(tabArea))
				return tabArea;
		}
	
		return NULL;
	}
	
	template<class SearchTab, typename SearchDirection, class OrthTab,
		typename OrthDirection>
	void CollectAllNeighbours(TabConnections *connections, SearchTab* tab,
		Area* area,	BObjectList<Area>& areas)
	{
		while (true) {
			Area* next = _FindNeighbour<SearchTab, SearchDirection, OrthTab,
				OrthDirection>(connections, tab, area);
			if (next == NULL)
				break;
			areas.AddItem(next);
			area = next;
		}
	}
}
	
/*! Find all areas that are connected.*/
class GroupDetection {
public:
	GroupDetection(TabConnections* connections)
		:
		fConnections(connections)
	{
	}

	static XTab* Left(const BObjectList<Area>& areas)
	{
		float best = HUGE_VAL;
		XTab* tab = NULL;
		for (int32 i = 0; i < areas.CountItems(); i++) {
			float value = areas.ItemAt(i)->Left()->Value();
			if (value < best) {
				best = value;
				tab = areas.ItemAt(i)->Left();
			}
		}
		return tab;
	}
	
	static YTab* Top(const BObjectList<Area>& areas)
	{
		float best = HUGE_VAL;
		YTab* tab = NULL;
		for (int32 i = 0; i < areas.CountItems(); i++) {
			float value = areas.ItemAt(i)->Top()->Value();
			if (value < best) {
				best = value;
				tab = areas.ItemAt(i)->Top();
			}
		}
		return tab;
	}

	static XTab* Right(const BObjectList<Area>& areas)
	{
		float best = -HUGE_VAL;
		XTab* tab = NULL;
		for (int32 i = 0; i < areas.CountItems(); i++) {
			float value = areas.ItemAt(i)->Right()->Value();
			if (value > best) {
				best = value;
				tab = areas.ItemAt(i)->Right();
			}
		}
		return tab;
	}

	static YTab* Bottom(const BObjectList<Area>& areas)
	{
		float best = -HUGE_VAL;
		YTab* tab = NULL;
		for (int32 i = 0; i < areas.CountItems(); i++) {
			float value = areas.ItemAt(i)->Bottom()->Value();
			if (value > best) {
				best = value;
				tab = areas.ItemAt(i)->Bottom();
			}
		}
		return tab;
	}

	static XTab* LeftmostRight(const BObjectList<Area>& areas)
	{
		float best = HUGE_VAL;
		XTab* tab = NULL;
		for (int32 i = 0; i < areas.CountItems(); i++) {
			float value = areas.ItemAt(i)->Right()->Value();
			if (value < best) {
				best = value;
				tab = areas.ItemAt(i)->Right();
			}
		}
		return tab;
	}

	static YTab* TopmostBottom(const BObjectList<Area>& areas)
	{
		float best = HUGE_VAL;
		YTab* tab = NULL;
		for (int32 i = 0; i < areas.CountItems(); i++) {
			float value = areas.ItemAt(i)->Bottom()->Value();
			if (value < best) {
				best = value;
				tab = areas.ItemAt(i)->Bottom();
			}
		}
		return tab;
	}

	static XTab* RightmostLeft(const BObjectList<Area>& areas)
	{
		float best = -HUGE_VAL;
		XTab* tab = NULL;
		for (int32 i = 0; i < areas.CountItems(); i++) {
			float value = areas.ItemAt(i)->Left()->Value();
			if (value > best) {
				best = value;
				tab = areas.ItemAt(i)->Left();
			}
		}
		return tab;
	}

	static YTab* BottommostTop(const BObjectList<Area>& areas)
	{
		float best = -HUGE_VAL;
		YTab* tab = NULL;
		for (int32 i = 0; i < areas.CountItems(); i++) {
			float value = areas.ItemAt(i)->Top()->Value();
			if (value > best) {
				best = value;
				tab = areas.ItemAt(i)->Top();
			}
		}
		return tab;
	}

	void FindAdjacentAreas(Area* area, area_side side,
		BObjectList<Area>& areas)
	{
		areas.AddItem(area);

		if (side == kBottom) {
			YTab* tab = area->Bottom();
			// left
			GroupDetectionHelper::CollectAllNeighbours<YTab, left_search, XTab,
				top_search>(fConnections, tab, area, areas);
			// right
			GroupDetectionHelper::CollectAllNeighbours<YTab, right_search, XTab,
				top_search>(fConnections, tab, area, areas);
		} else if (side == kTop) {
			YTab* tab = area->Top();
			// left
			GroupDetectionHelper::CollectAllNeighbours<YTab, left_search, XTab,
				bottom_search>(fConnections, tab, area, areas);
			// right
			GroupDetectionHelper::CollectAllNeighbours<YTab, right_search, XTab,
				bottom_search>(fConnections, tab, area, areas);
		} else if (side == kLeft) {
			XTab* tab = area->Left();
			// top
			GroupDetectionHelper::CollectAllNeighbours<XTab, top_search, YTab,
				right_search>(fConnections, tab, area, areas);
			// bottom
			GroupDetectionHelper::CollectAllNeighbours<XTab, bottom_search, YTab,
				right_search>(fConnections, tab, area, areas);
		} else if (side == kRight) {
			XTab* tab = area->Right();
			// top
			GroupDetectionHelper::CollectAllNeighbours<XTab, top_search, YTab,
				left_search>(fConnections, tab, area, areas);
			// bottom
			GroupDetectionHelper::CollectAllNeighbours<XTab, bottom_search, YTab,
				left_search>(fConnections, tab, area, areas);
		}
	}

	bool GroupOwnsTab(XTab* tab, BObjectList<Area>& group)
	{
		for (int32 i = 0; i < group.CountItems(); i++) {
			Area* area = group.ItemAt(i);
			if (area->Left() == tab || area->Right() == tab)
				return true;
		}
		return false;
	}

	bool GroupOwnsTab(YTab* tab, BObjectList<Area>& group)
	{
		for (int32 i = 0; i < group.CountItems(); i++) {
			Area* area = group.ItemAt(i);
			if (area->Top() == tab || area->Bottom() == tab)
				return true;
		}
		return false;
	}

private:
			TabConnections*		fConnections;
};

// These functions should be/ have been members of InsertDetection but gcc2 don't
// likes template methods in classes (?). In gcc4 it works fine.
namespace InsertDetectionHelper {
	template<class SearchTab, typename SearchDirection>
	void _GetAllAreasFor(BALMLayout *layout, TabConnections* connections,
		SearchTab* tab,	BObjectList<Area>& areaList)
	{
		// sometime tabs are at the same position so collect all of them
		BObjectList<SearchTab> allTabs;
		SearchDirection search;
		for (int32 i = 0; i < search.CountTabs(layout); i++) {
			SearchTab* currentTab = search.TabAt(layout, i);
			if (fabs(tab->Value() - currentTab->Value()) < 0.1)
				allTabs.AddItem(currentTab);
		}
		for (int32 i = 0; i < allTabs.CountItems(); i++) {
			SearchTab* currentTab = allTabs.ItemAt(i);
			tab_links<SearchTab>& tabLinks
				= search.LinkMap(connections)[currentTab];
			BObjectList<Area>& tabAreas = search.AreasSearchDirection(tabLinks);
			areaList.AddList(&tabAreas);
		}
	}
	
	template<class SearchTab, typename SearchDirection, class OrthTab>
	void _FindInsertTabs(BALMLayout *layout, TabConnections* connections,
		SearchTab* tab, BRect targetFrame, OrthTab** tab1, OrthTab** tab2,
		BObjectList<Area>& tabAreas)
	{
		SearchDirection search;
		float middle = search.OrthSide1(targetFrame)
			+ search.OrthExtent(targetFrame) / 2;
	
		float tab1MinDist = DBL_MAX;
		float tab2MinDist = DBL_MAX;
	
		_GetAllAreasFor<SearchTab, SearchDirection>(layout, connections, tab,
			tabAreas);
		for (int32 i = 0; i < tabAreas.CountItems(); i++) {
			Area* area = tabAreas.ItemAt(i);
	
			OrthTab* currentTab1 = search.OrthTab1(area);
			float currentDist1 = fabs(search.OrthSide1(targetFrame)
				- currentTab1->Value());
			if (currentDist1 < tab1MinDist && currentTab1->Value() < middle) {
				tab1MinDist = currentDist1;
				*tab1 = currentTab1;
			}
	
			OrthTab* currentTab2 = search.OrthTab2(area);
			float currentDist2 = fabs(currentTab2->Value()
				- search.OrthSide2(targetFrame));
			if (currentDist2 < tab2MinDist  && currentTab2->Value() > middle) {
				tab2MinDist = currentDist2;
				*tab2 = currentTab2;
			}
		}
	
		if ((*tab1) == NULL || (*tab2) == NULL) {
			*tab1 = NULL;
			*tab2 = NULL;
			return;
		}
		if ((*tab1)->Value() > search.OrthSide2(targetFrame)
			|| (*tab2)->Value() < search.OrthSide1(targetFrame)) {
			*tab1 = NULL;
			*tab2 = NULL;
		}
	}
	
	template<class SearchTab, typename SearchDirection, class OrthTab>
	void _FillAreasBetween(SearchTab* tab, OrthTab* tab1, OrthTab* tab2,
		BArray<Area*>& areaList, const BObjectList<Area>& areaCandidates)
	{
		SearchDirection search;
		for (int32 i = 0; i < areaCandidates.CountItems(); i++) {
			Area* area = areaCandidates.ItemAt(i);
	
			OrthTab* currentTab1 = search.OrthTab1(area);
			OrthTab* currentTab2 = search.OrthTab2(area);
			if (tab1->Value() <= currentTab1->Value()
				&& currentTab2->Value() <= tab2->Value())
				areaList.AddItem(area);
		}
	}
			
	template<class SearchTab, class OrthTab, typename SearchDirection1,
		typename SearchDirection2>
	void GetInsertAreas(BALMLayout *layout, TabConnections* connections,
		SearchTab* tab, BRect targetFrame, BArray<Area*>& areaList,
		area_side& insertDirection)
	{
		areaList.MakeEmpty();
	
		OrthTab* tab11 = NULL;
		OrthTab* tab12 = NULL;
		BObjectList<Area> areaCandidates1;
		_FindInsertTabs<SearchTab, SearchDirection1, OrthTab>(layout,
			connections, tab, targetFrame, &tab11, &tab12, areaCandidates1);
		OrthTab* tab21 = NULL;
		OrthTab* tab22 = NULL;
		BObjectList<Area> areaCandidates2;
		_FindInsertTabs<SearchTab, SearchDirection2, OrthTab>(layout,
			connections, tab, targetFrame, &tab21, &tab22, areaCandidates2);
		if ((tab21 == NULL || tab22 == NULL)
			&& (tab11 == NULL || tab12 == NULL))
			return;
	
		float extent = SearchDirection1().Extent(targetFrame);
	
		float diff1 = DBL_MAX;
		if (tab11 != NULL && tab12 != NULL)
			diff1 = fabs((tab12->Value() - tab11->Value()) - extent);
		float diff2 = DBL_MAX;
		if (tab21 != NULL && tab22 != NULL)
			diff2 = fabs((tab22->Value() - tab21->Value()) - extent);
	
		if (diff1 < diff2) {
			_FillAreasBetween<SearchTab, SearchDirection1, OrthTab>(tab, tab11,
				tab12, areaList, areaCandidates1);
			insertDirection = SearchDirection1().SearchDirection();
		} else {
			_FillAreasBetween<SearchTab, SearchDirection2, OrthTab>(tab, tab21,
				tab22, areaList, areaCandidates2);
			insertDirection = SearchDirection2().SearchDirection();
		}
	}
}

class InsertDetection {
public:
	InsertDetection(BALMLayout* layout, TabConnections* connections)
		:
		fLayout(layout),
		fConnections(connections)
	{
	}

	void GetInsertAreas(XTab* tab, BRect targetFrame, BArray<Area*>& areaList,
		area_side& insertDirection)
	{
		InsertDetectionHelper::GetInsertAreas<XTab, YTab, left_search,
			right_search>(fLayout, fConnections, tab, targetFrame, areaList,
				insertDirection);
	}

	void GetInsertAreas(YTab* tab, BRect targetFrame, BArray<Area*>& areaList,
		area_side& insertDirection)
	{
		InsertDetectionHelper::GetInsertAreas<YTab, XTab, top_search,
			bottom_search>(fLayout, fConnections, tab, targetFrame, areaList,
				insertDirection);
	}

private:
			BALMLayout*			fLayout;
			TabConnections*		fConnections;
};


}	// namespace BALM


#endif	// GROUP_DETECTION_H

/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	OVERLAP_MANAGER_H
#define	OVERLAP_MANAGER_H

#include <map>
#include <vector>

#include <Debug.h>
#include <View.h>

#include <ALMLayout.h>

#include "LinearProgrammingTypes.h"


#if DEBUG
#include <stdio.h>
#endif


enum area_side {
	kNoSide = -100,
	kLeft = 0,
	kTop = 1,
	kRight = 2,
	kBottom = 3
};

/*
static area_side OppositeSide(area_side side)
{
	int32 opposite = (int32)side + 2;
	if (opposite >= 4)
		opposite -= 4;
	return (area_side)opposite;
}


static area_side NeighbourSide(Area* area, Area* neighbour)
{
	if (area->Left() == neighbour->Right())
		return kLeft;
	if (area->Right() == neighbour->Left())
		return kRight;
	if (area->Top() == neighbour->Bottom())
		return kTop;
	if (area->Bottom() == neighbour->Top())
		return kBottom;
	return kNoSide;
}
*/


namespace BALM {


template <class TYPE>
struct tab_links {
	BObjectList<TYPE> tabs1; // left or top
	BObjectList<TYPE> tabs2; // right or bottom
	BObjectList<Area> areas1;
	BObjectList<Area> areas2;
};

// This function should be/was a member of TabConnections but gcc2 don't likes
// template methods in classes (?). In gcc4 it works fine.
namespace TabConnectionsHelper {
	template <class TabType, class DirectionPolicy>
	void FillTabLinks(BALMLayout* layout, std::map<TabType*,
		tab_links<TabType> >& tabMap) {
		DirectionPolicy policy;
		for (int32 i = 0; i < policy.CountTabs(layout); i++) {
			TabType* tab = policy.TabAt(layout, i);
			tab_links<TabType>& links = tabMap[tab];
			policy.FillLinks(layout, tab, links);
		}
	};
}

class TabConnections {
public:
	void Fill(BALMLayout* layout)
	{
		Clear();
		TabConnectionsHelper::FillTabLinks<XTab, HorizontalPolicy>(layout,
			fXTabLinkMap);
		TabConnectionsHelper::FillTabLinks<YTab, VerticalPolicy>(layout,
			fYTabLinkMap);
	}


	void Clear()
	{
		fXTabLinkMap.clear();
		fYTabLinkMap.clear();
	}


	std::map<XTab*, tab_links<XTab> >& GetXTabLinkMap()
	{
		return fXTabLinkMap;
	}


	std::map<YTab*, tab_links<YTab> >& GetYTabLinkMap()
	{
		return fYTabLinkMap;
	}


	bool TabsAtTheLeft(XTab* tab) const
	{
		std::map<XTab*, tab_links<XTab> >::const_iterator it
			= fXTabLinkMap.find(tab);
		if (it == fXTabLinkMap.end())
			return false;
		return it->second.tabs1.CountItems() > 0;
	}

	bool TabsAtTheRight(XTab* tab) const
	{
		std::map<XTab*, tab_links<XTab> >::const_iterator it
			= fXTabLinkMap.find(tab);
		if (it == fXTabLinkMap.end())
			return false;
		return it->second.tabs2.CountItems() > 0;
	}

	bool TabsAtTheTop(YTab* tab) const
	{
		std::map<YTab*, tab_links<YTab> >::const_iterator it
			= fYTabLinkMap.find(tab);
		if (it == fYTabLinkMap.end())
			return false;
		return it->second.tabs1.CountItems() > 0;
	}

	bool TabsAtTheBottom(YTab* tab) const
	{
		std::map<YTab*, tab_links<YTab> >::const_iterator it
			= fYTabLinkMap.find(tab);
		if (it == fYTabLinkMap.end())
			return false;
		return it->second.tabs2.CountItems() > 0;
	}


	bool AreasAtTheLeft(XTab* tab) const
	{
		std::map<XTab*, tab_links<XTab> >::const_iterator it
			= fXTabLinkMap.find(tab);
		if (it == fXTabLinkMap.end())
			return false;
		return it->second.areas1.CountItems() > 0;
	}

	bool AreasAtTheRight(XTab* tab) const
	{
		std::map<XTab*, tab_links<XTab> >::const_iterator it
			= fXTabLinkMap.find(tab);
		if (it == fXTabLinkMap.end())
			return false;
		return it->second.areas2.CountItems() > 0;
	}

	bool AreasAtTheTop(YTab* tab) const
	{
		std::map<YTab*, tab_links<YTab> >::const_iterator it
			= fYTabLinkMap.find(tab);
		if (it == fYTabLinkMap.end())
			return false;
		return it->second.areas1.CountItems() > 0;
	}

	bool AreasAtTheBottom(YTab* tab) const
	{
		std::map<YTab*, tab_links<YTab> >::const_iterator it
			= fYTabLinkMap.find(tab);
		if (it == fYTabLinkMap.end())
			return false;
		return it->second.areas2.CountItems() > 0;
	}


	bool ConnectedToLeftBorder(XTab* left, BALMLayout* layout)
	{
		if (left == layout->Left())
			return true;

		const tab_links<XTab>& link = fXTabLinkMap[left];
		for (int32 i = 0; i < link.areas1.CountItems(); i++) {
			if (ConnectedToLeftBorder(link.areas1.ItemAt(i)->Left(), layout))
				return true;
		}
		return false;
	}


	bool ConnectedToTopBorder(YTab* top, BALMLayout* layout)
	{
		if (top == layout->Top())
			return true;

		const tab_links<YTab>& link = fYTabLinkMap[top];
		for (int32 i = 0; i < link.areas1.CountItems(); i++) {
			if (ConnectedToTopBorder(link.areas1.ItemAt(i)->Top(), layout))
				return true;
		}
		return false;
	}


	bool ConnectedToRightBorder(XTab* right, BALMLayout* layout)
	{
		if (right == layout->Right())
			return true;

		const tab_links<XTab>& link = fXTabLinkMap[right];
		for (int32 i = 0; i < link.areas2.CountItems(); i++) {
			if (ConnectedToRightBorder(link.areas2.ItemAt(i)->Right(), layout))
				return true;
		}
		return false;
	}


	bool ConnectedToBottomBorder(YTab* bottom, BALMLayout* layout)
	{
		if (bottom == layout->Bottom())
			return true;

		const tab_links<YTab>& link = fYTabLinkMap[bottom];
		for (int32 i = 0; i < link.areas2.CountItems(); i++) {
			if (ConnectedToBottomBorder(link.areas2.ItemAt(i)->Bottom(),
				layout))
				return true;
		}
		return false;
	}

	bool ConnectedTo(Area* area, area_side side)
	{
		if (side == kLeft) {
			XTab* tab = area->Left();
			const tab_links<XTab>& link = fXTabLinkMap[tab];
			if (link.areas2.CountItems() > 1 || link.areas1.CountItems() > 0)
				return true;
		} else if (side == kRight) {
			XTab* tab = area->Right();
			const tab_links<XTab>& link = fXTabLinkMap[tab];
			if (link.areas1.CountItems() > 1 || link.areas2.CountItems() > 0)
				return true;
		} else if (side == kTop) {
			YTab* tab = area->Top();
			const tab_links<YTab>& link = fYTabLinkMap[tab];
			if (link.areas2.CountItems() > 1 || link.areas1.CountItems() > 0)
				return true;
		} else if (side == kBottom) {
			YTab* tab = area->Bottom();
			const tab_links<YTab>& link = fYTabLinkMap[tab];
			if (link.areas1.CountItems() > 1 || link.areas2.CountItems() > 0)
				return true;
		}

		return false;
	}

	template<typename Type>
	void PrintTabConnections(std::map<Type*, tab_links<Type> > linkMap)
	{
		typename std::map<Type*, tab_links<Type> >::iterator it;
		for (it = linkMap.begin(); it != linkMap.end(); it++) {
			tab_links<Type>& links = it->second;
			Type* tab = it->first;
			printf("Tab (%p), pos %f\n", tab, tab->Value());
			for (int32 i = 0; i < links.tabs1.CountItems(); i++) {
				Type* child = links.tabs1.ItemAt(i);
				printf("- tabs1 (%p), pos %f\n", child, child->Value());
			}
			for (int32 i = 0; i < links.tabs2.CountItems(); i++) {
				Type* child = links.tabs2.ItemAt(i);
				printf("- tabs2 (%p), pos %f\n", child, child->Value());
			}
		}
	}

	void PrintXTabConnections()
	{
		PrintTabConnections<XTab>(fXTabLinkMap);
	}

	void PrintYTabConnections()
	{
		PrintTabConnections<YTab>(fYTabLinkMap);
	}

private:
	class HorizontalPolicy {
	public:
		int32 CountTabs(BALMLayout* layout)
		{
			return layout->CountXTabs();
		}

		XTab* TabAt(BALMLayout* layout, int32 index)
		{
			return layout->XTabAt(index);
		}

		void FillLinks(BALMLayout* layout, XTab* tab,
			tab_links<XTab>& links)
		{
			for (int32 i = 0; i < layout->CountAreas(); i++) {
				Area* area = layout->AreaAt(i);
				if (area->Left() == tab) {
					if (!links.tabs2.HasItem(area->Right()))
						links.tabs2.AddItem(area->Right());
					links.areas2.AddItem(area);
				} else if (area->Right() == tab) {
					if (!links.tabs1.HasItem(area->Left()))
						links.tabs1.AddItem(area->Left());
					links.areas1.AddItem(area);
				}
				#if DEBUG
				if ((links.tabs1.HasItem(area->Right())
					&& links.tabs2.HasItem(area->Right()))
					|| (links.tabs1.HasItem(area->Left())
					&& links.tabs2.HasItem(area->Left()))) {
					printf("Invalid map at x Tab %p (%f), Area: ", tab,
						tab->Value());
					area->Frame().PrintToStream();
					debugger("Invalid!");
				}
				#endif
			}
		}
	};

	class VerticalPolicy {
	public:
		int32 CountTabs(BALMLayout* layout)
		{
			return layout->CountYTabs();
		}

		YTab* TabAt(BALMLayout* layout, int32 index)
		{
			return layout->YTabAt(index);
		}

		void FillLinks(BALMLayout* layout, YTab* tab,
			tab_links<YTab>& links)
		{
			for (int32 i = 0; i < layout->CountAreas(); i++) {
				Area* area = layout->AreaAt(i);
				if (area->Top() == tab) {
					if (!links.tabs2.HasItem(area->Bottom()))
						links.tabs2.AddItem(area->Bottom());
					links.areas2.AddItem(area);
				} else if (area->Bottom() == tab) {
					if (!links.tabs1.HasItem(area->Top()))
						links.tabs1.AddItem(area->Top());
					links.areas1.AddItem(area);
				}
				#if DEBUG
				if ((links.tabs1.HasItem(area->Bottom())
					&& links.tabs2.HasItem(area->Bottom()))
					|| (links.tabs1.HasItem(area->Top())
					&& links.tabs2.HasItem(area->Top()))) {
					printf("Invalid map at y Tab %p (%f), Area: ", tab,
						tab->Value());
					area->Frame().PrintToStream();
					debugger("Invalid!");
				}
				#endif
			}	
		}
	};

private:
			std::map<XTab*, tab_links<XTab> > fXTabLinkMap;
			std::map<YTab*, tab_links<YTab> > fYTabLinkMap;
};


//! Interface class
class OverlapManagerEngine {
public:
	virtual						~OverlapManagerEngine() {}

	virtual void				DisconnectAreas() = 0;
	virtual void				ConnectAreas() = 0;

	virtual void				Draw(BView* view) = 0;
};


class OverlapManager {
public:
	OverlapManager(BALMLayout* layout);

	~OverlapManager()
	{
		DisconnectAreas();
		delete fOverlapEngine;
	}

	TabConnections*
	GetTabConnections()
	{
		return &fTabConnections;
	}

	void DisconnectAreas();
	void FillTabConnections();
	void ConnectAreas(bool fillTabConnections = true);
	void Draw(BView* view);

	void ReconnectAreas()
	{
		DisconnectAreas();
		ConnectAreas();
	}

	void AddArea(Area* area)
	{
		ReconnectAreas();
	}

	void RemoveArea(Area* area)
	{
		ReconnectAreas();
	}

	void AreaResized(Area* area, XTab* fromLeft, YTab* fromTop,
		XTab* fromRight, YTab* fromBottom)
	{
		ReconnectAreas();
	}

private:
			BALMLayout*			fALMLayout;
			OverlapManagerEngine* fOverlapEngine;
			TabConnections		fTabConnections;
};

// This function should be/was a member of SimpleOverlapEngine but gcc2 don't
// likes template methods in classes (?). In gcc4 it works fine.
namespace SimpleOverlapEngineHelper {
	template <class TYPE>
	TYPE* FindClosestConnectedTab(TYPE* start, TYPE* target,
		std::map<TYPE*, tab_links<TYPE> >& tabMap,
		bool leftOrUpDirection, float& minDist)
	{
		TYPE* foundTab = start;
		tab_links<TYPE>& links = tabMap[start];
		if (leftOrUpDirection) {
			minDist = start->Value() - target->Value();
			for (int32 i = 0; i < links.tabs1.CountItems(); i++) {
				TYPE* tab = links.tabs1.ItemAt(i);
				float dist = tab->Value() - target->Value();
				if (dist <= 0)
					continue;
				TYPE* nextTab = FindClosestConnectedTab(tab, target, tabMap,
					leftOrUpDirection, dist);
				if (dist < minDist) {
					foundTab = nextTab;
					minDist = dist;
				}
			}
		} else {
			minDist = target->Value() - start->Value();
			for (int32 i = 0; i < links.tabs2.CountItems(); i++) {
				TYPE* tab = links.tabs2.ItemAt(i);
				float dist = target->Value() - tab->Value();
				if (dist <= 0)
					continue;
				TYPE* nextTab = FindClosestConnectedTab(tab, target, tabMap,
					leftOrUpDirection, dist);
				if (dist < minDist) {
					foundTab = nextTab;
					minDist = dist;
				}
			}
		}
	
		return foundTab;
	}
}
	
class SimpleOverlapEngine : public OverlapManagerEngine {
public:
	SimpleOverlapEngine(BALMLayout* layout, OverlapManager* manager)
		:
		fALMLayout(layout),
		fTabConnections(manager->GetTabConnections()),
		fXTabLinkMap(fTabConnections->GetXTabLinkMap()),
		fYTabLinkMap(fTabConnections->GetYTabLinkMap())
	{
		
	}
	
	virtual void DisconnectAreas()
	{
		fDebugInfos.clear();

		for (int32 i = 0; i < fVConstraints.CountItems(); i++)
			fALMLayout->Solver()->RemoveConstraint(fVConstraints.ItemAt(i));
		for (int32 i = 0; i < fHConstraints.CountItems(); i++)
			fALMLayout->Solver()->RemoveConstraint(fHConstraints.ItemAt(i));

		fVConstraints.MakeEmpty();
		fHConstraints.MakeEmpty();
	}

	virtual void ConnectAreas()
	{
		BObjectList<Area> allAreas;
		for (int32 i = 0; i < fALMLayout->CountAreas(); i++) {
			Area* area = fALMLayout->AreaAt(i);
			allAreas.AddItem(area);
		}

		for (int32 i = 0; i < allAreas.CountItems(); i++) {
			Area* area = allAreas.ItemAt(i);
			BObjectList<Area> currentAreas(allAreas);
			currentAreas.RemoveItem(area);
			_RemoveLeftConnectedAreas(area->Left(),	currentAreas);
			_RemoveTopConnectedAreas(area->Top(), currentAreas);
			_RemoveRightConnectedAreas(area->Right(), currentAreas);
			_RemoveBottomConnectedAreas(area->Bottom(), currentAreas);

			while (currentAreas.CountItems() > 0) {
				Area* closestArea = _FindClosestArea(area, currentAreas);
				if (closestArea == NULL)
					break;
				currentAreas.RemoveItem(closestArea);
				float distLeft, distTop, distRight, distBottom;
				_GetDistances(area, closestArea, distLeft, distTop, distRight,
					distBottom);

				if (distLeft <= distTop && distLeft <= distRight
					&& distLeft <= distBottom) {
					float dist;
					XTab* closestTab = SimpleOverlapEngineHelper::FindClosestConnectedTab<XTab>(
						area->Left(), closestArea->Right(), fXTabLinkMap, true,
						dist);
					/* Check if the found tab is already insert at the other
					side. This can happen if both tabs at the same position. */
					tab_links<XTab>& links = fXTabLinkMap[closestArea->Right()];
					if (!links.tabs1.HasItem(closestTab)) {
						Constraint* constraint
							= fALMLayout->Solver()->AddConstraint(
								-1., closestArea->Right(), 1., closestTab,
								LinearProgramming::kGE, 0);
						fHConstraints.AddItem(constraint);
		
						tab_links<XTab>& links = fXTabLinkMap[closestTab];
						links.tabs1.AddItem(closestArea->Right());
						tab_links<XTab>& links2 = fXTabLinkMap[closestArea->Right()];
						links2.tabs2.AddItem(closestTab);
		
						_RemoveLeftConnectedAreas(closestTab, currentAreas);
		
						_AddDebugInfo(closestArea, area, closestArea->Right(),
							closestTab);
					}
				} else if (distTop <= distLeft && distTop <= distRight
					&& distTop <= distBottom) {
					float dist;
					YTab* closestTab = SimpleOverlapEngineHelper::FindClosestConnectedTab<YTab>(
						area->Top(), closestArea->Bottom(), fYTabLinkMap, true,
						dist);
					tab_links<YTab>& links = fYTabLinkMap[closestArea->Bottom()];
					if (!links.tabs1.HasItem(closestTab)) {
						Constraint* constraint
							= fALMLayout->Solver()->AddConstraint(
								-1., closestArea->Bottom(), 1., closestTab,
								LinearProgramming::kGE, 0);
						fVConstraints.AddItem(constraint);

						tab_links<YTab>& links = fYTabLinkMap[closestTab];
						links.tabs1.AddItem(closestArea->Bottom());
						tab_links<YTab>& links2 = fYTabLinkMap[closestArea->Bottom()];
						links2.tabs2.AddItem(closestTab);

						_RemoveTopConnectedAreas(closestTab, currentAreas);

						_AddDebugInfo(closestArea, area, closestArea->Bottom(),
							closestTab);
					}
				} else if (distRight <= distTop && distRight <= distLeft
					&& distRight <= distBottom) {
					float dist;
					XTab* closestTab = SimpleOverlapEngineHelper::FindClosestConnectedTab<XTab>(
						area->Right(), closestArea->Left(), fXTabLinkMap, false,
						dist);
					tab_links<XTab>& links = fXTabLinkMap[closestArea->Left()];
					if (!links.tabs2.HasItem(closestTab)) {
						Constraint* constraint
							= fALMLayout->Solver()->AddConstraint(
								-1., closestTab, 1., closestArea->Left(),
								LinearProgramming::kGE, 0);
						fHConstraints.AddItem(constraint);

						tab_links<XTab>& links = fXTabLinkMap[closestTab];
						links.tabs2.AddItem(closestArea->Left());
						tab_links<XTab>& links2 = fXTabLinkMap[closestArea->Left()];
						links2.tabs1.AddItem(closestTab);

						_RemoveRightConnectedAreas(closestTab, currentAreas);

						_AddDebugInfo(area, closestArea, closestTab,
							closestArea->Left());
					}
				} else if (distBottom <= distTop && distBottom <= distRight
					&& distBottom <= distLeft) {
					float dist;
					YTab* closestTab = SimpleOverlapEngineHelper::FindClosestConnectedTab<YTab>(
						area->Bottom(), closestArea->Top(), fYTabLinkMap, false,
						dist);

					tab_links<YTab>& links = fYTabLinkMap[closestArea->Top()];
					if (!links.tabs2.HasItem(closestTab)) {
						Constraint* constraint
							= fALMLayout->Solver()->AddConstraint(
								-1., closestTab, 1., closestArea->Top(),
								LinearProgramming::kGE, 0);
						fVConstraints.AddItem(constraint);

						tab_links<YTab>& links = fYTabLinkMap[closestTab];
						links.tabs2.AddItem(closestArea->Top());
						tab_links<YTab>& links2 = fYTabLinkMap[closestArea->Top()];
						links2.tabs1.AddItem(closestTab);

						_RemoveBottomConnectedAreas(closestTab, currentAreas);

						_AddDebugInfo(area, closestArea, closestTab,
							closestArea->Top());
					}
				} else
					debugger("upps");
			}

			// connect to the borders?
			tab_links<XTab>& leftLinks = fXTabLinkMap[area->Left()];
			tab_links<YTab>& topLinks = fYTabLinkMap[area->Top()];
			tab_links<XTab>& rightLinks = fXTabLinkMap[area->Right()];
			tab_links<YTab>& bottomLinks = fYTabLinkMap[area->Bottom()];
			if (area->Left() != fALMLayout->Left()
				&& leftLinks.tabs1.CountItems() == 0) {
				Constraint* constraint = fALMLayout->Solver()->AddConstraint(
					-1., fALMLayout->Left(), 1., area->Left(),
					LinearProgramming::kGE, 0);
				fHConstraints.AddItem(constraint);

				tab_links<XTab>& links = fXTabLinkMap[area->Left()];
				links.tabs1.AddItem(fALMLayout->Left());
				_AddDebugInfo(NULL, area, area->Left(), NULL);
			}
			if (area->Top() != fALMLayout->Top()
				&& topLinks.tabs1.CountItems() == 0) {
				Constraint* constraint = fALMLayout->Solver()->AddConstraint(
					-1., fALMLayout->Top(), 1., area->Top(),
					LinearProgramming::kGE, 0);
				fVConstraints.AddItem(constraint);

				tab_links<YTab>& links = fYTabLinkMap[area->Top()];
				links.tabs1.AddItem(fALMLayout->Top());

				_AddDebugInfo(NULL, area, area->Top(), NULL);
			}
			if (area->Right() != fALMLayout->Right()
				&& rightLinks.tabs2.CountItems() == 0) {
				Constraint* constraint = fALMLayout->Solver()->AddConstraint(
					-1., area->Right(), 1., fALMLayout->Right(),
					LinearProgramming::kGE, 0);
				fHConstraints.AddItem(constraint);

				tab_links<XTab>& links = fXTabLinkMap[area->Right()];
				links.tabs2.AddItem(fALMLayout->Right());

				_AddDebugInfo(area, NULL, area->Right(), NULL);
			}
			if (area->Bottom() != fALMLayout->Bottom()
				&& bottomLinks.tabs2.CountItems() == 0) {
				Constraint* constraint = fALMLayout->Solver()->AddConstraint(
					-1., area->Bottom(), 1., fALMLayout->Bottom(),
					LinearProgramming::kGE, 0);
				fVConstraints.AddItem(constraint);

				tab_links<YTab>& links = fYTabLinkMap[area->Bottom()];
				links.tabs2.AddItem(fALMLayout->Bottom());

				_AddDebugInfo(area, NULL, area->Bottom(), NULL);
			}			
		}
	}

	struct debug_info {
		Area*	area1;
		Area*	area2;
		XTab*	xTab1;
		YTab*	yTab1;
		XTab*	xTab2;
		YTab*	yTab2;
		
		bool	horizontalConnection;
	};

	virtual void Draw(BView* view)
	{
		const std::vector<debug_info>& infos
			= fDebugInfos;
		for (unsigned int i = 0; i < infos.size(); i++) {
			const debug_info& info = infos[i];
			BPoint start;
			BPoint end;
			if (info.area1 != NULL) {
				BRect rect1 = info.area1->Frame();
				if (info.horizontalConnection) {
					if (info.xTab1 != NULL)
						start.x = info.xTab1->Value();
					else
						start.x = rect1.right;
					start.y = rect1.top + rect1.Height() / 2;
				} else {
					start.x = rect1.left + rect1.Width() / 2;
					if (info.yTab1 != NULL)
						start.y = info.yTab1->Value();
					else
						start.y = rect1.bottom;
				}
			}
			if (info.area2 != NULL) {
				BRect rect2 = info.area2->Frame();
				if (info.horizontalConnection) {
					if (info.xTab2 != NULL)
						end.x = info.xTab2->Value();
					else
						end.x = rect2.left;
					end.y = rect2.top + rect2.Height() / 2;
				} else {
					end.x = rect2.left + rect2.Width() / 2;
					if (info.yTab2 != NULL)
						end.y = info.yTab2->Value();
					else
						end.y = rect2.top;
				}
			}
			if (info.area1 == NULL) {
				if (info.horizontalConnection) {
					start.x = 0;
					start.y = end.y;
				} else {
					start.x = end.x;
					start.y = 0;
				}
			}
			if (info.area2 == NULL) {
				BRect frame = view->Frame();
				if (info.horizontalConnection) {
					end.x = frame.Width();
					end.y = start.y;
				} else {
					end.x = start.x;
					end.y = frame.Height();
				}
			}
			
	
			view->SetPenSize(1);
			rgb_color colour = {0, 0, 255};
			view->SetHighColor(colour);
			view->StrokeLine(start, end);
		}
	}

	void
	DisableOverlapConstraints(bool disable)
	{
		debugger("you have good reasons to use this?");
		if (disable == true) {
			for (int32 i = 0; i < fVConstraints.CountItems(); i++) {
				fALMLayout->Solver()->RemoveConstraint(fVConstraints.ItemAt(i),
					false);
			}
			for (int32 i = 0; i < fHConstraints.CountItems(); i++) {
				fALMLayout->Solver()->RemoveConstraint(fHConstraints.ItemAt(i),
					false);
			}
			return;
		}
		for (int32 i = 0; i < fVConstraints.CountItems(); i++)
			fALMLayout->Solver()->AddConstraint(fVConstraints.ItemAt(i));
		for (int32 i = 0; i < fHConstraints.CountItems(); i++)
			fALMLayout->Solver()->AddConstraint(fHConstraints.ItemAt(i));
	}

private:
	void _AddDebugInfo(Area* area1, Area* area2, XTab* tab1, XTab* tab2)
	{
		debug_info info;
		info.area1 = area1;
		info.area2 = area2;
		info.xTab1 = tab1;
		info.xTab2 = tab2;
		info.horizontalConnection = true;

		fDebugInfos.push_back(info);
	}

	void _AddDebugInfo(Area* area1, Area* area2, YTab* tab1, YTab* tab2)
	{
		debug_info info;
		info.area1 = area1;
		info.area2 = area2;
		info.yTab1 = tab1;
		info.yTab2 = tab2;
		info.horizontalConnection = false;

		fDebugInfos.push_back(info);
	}

	void _GetDistances(Area* main, Area* other, float& distLeft, float& distTop,
		float& distRight, float& distBottom)
	{
		distLeft = main->Left()->Value() - other->Right()->Value();
		distTop = main->Top()->Value() - other->Bottom()->Value();
		distRight = other->Left()->Value() - main->Right()->Value();
		distBottom = other->Top()->Value() - main->Bottom()->Value();

		if (fabs(distLeft) < 0.00001)
			distLeft = 0;
		else if (distLeft < 0)
			distLeft = B_SIZE_UNLIMITED;

		if (fabs(distTop) < 0.00001)
			distTop = 0;
		else if (distTop < 0)
			distTop = B_SIZE_UNLIMITED;

		if (fabs(distRight) < 0.00001)
			distRight = 0;
		else if (distRight < 0)
			distRight = B_SIZE_UNLIMITED;

		if (fabs(distBottom) < 0.00001)
			distBottom = 0;
		else if (distBottom < 0)
			distBottom = B_SIZE_UNLIMITED;
	}

	Area* _FindClosestArea(Area* area, const BObjectList<Area>& areas)
	{
		float minDist = B_SIZE_UNLIMITED;
		Area* closestArea = NULL;
		for (int32 i = 0; i < areas.CountItems(); i++) {
			Area* currentArea = areas.ItemAt(i);

			float distLeft, distTop, distRight, distBottom;
			_GetDistances(area, currentArea, distLeft, distTop, distRight,
					distBottom);

			if (distLeft <= distTop && distLeft <= distRight
				&& distLeft <= distBottom) {
				if (distLeft < minDist) {
					minDist = distLeft;
					closestArea = currentArea;
				}
			} else if (distTop <= distLeft && distTop <= distRight
				&& distTop <= distBottom) {
				if (distTop < minDist) {
					minDist = distTop;
					closestArea = currentArea;
				}		
			} else if (distRight <= distTop && distRight <= distLeft
				&& distRight <= distBottom) {
				if (distRight < minDist) {
					minDist = distRight;
					closestArea = currentArea;
				}		
			} else if (distBottom <= distTop && distBottom <= distRight
				&& distBottom <= distLeft) {
				if (distBottom < minDist) {
					minDist = distBottom;
					closestArea = currentArea;
				}		
			}
		}

		ASSERT(minDist >= 0);
		return closestArea;
	}


	bool _RemoveLeftConnectedAreas(XTab* left, BObjectList<Area>& areas)
	{
		if (left == fALMLayout->Left())
			return true;

		const tab_links<XTab>& link = fXTabLinkMap[left];
		for (int32 i = 0; i < link.areas1.CountItems(); i++)
			areas.RemoveItem(link.areas1.ItemAt(i));
		for (int32 i = 0; i < link.tabs1.CountItems(); i++)
			_RemoveLeftConnectedAreas(link.tabs1.ItemAt(i), areas);

		return link.tabs1.CountItems() > 0;
	}


	bool _RemoveTopConnectedAreas(YTab* top, BObjectList<Area>& areas)
	{
		if (top == fALMLayout->Top())
			return true;

		const tab_links<YTab>& link = fYTabLinkMap[top];
		for (int32 i = 0; i < link.areas1.CountItems(); i++)
			areas.RemoveItem(link.areas1.ItemAt(i));
		for (int32 i = 0; i < link.tabs1.CountItems(); i++)
			_RemoveTopConnectedAreas(link.tabs1.ItemAt(i), areas);

		return link.tabs1.CountItems() > 0;
	}


	bool _RemoveRightConnectedAreas(XTab* right, BObjectList<Area>& areas)
	{
		if (right == fALMLayout->Right())
			return true;

		const tab_links<XTab>& link = fXTabLinkMap[right];
		for (int32 i = 0; i < link.areas2.CountItems(); i++)
			areas.RemoveItem(link.areas2.ItemAt(i));
		for (int32 i = 0; i < link.tabs2.CountItems(); i++)
			_RemoveRightConnectedAreas(link.tabs2.ItemAt(i), areas);

		return link.tabs2.CountItems() > 0;
	}
	
	
	bool _RemoveBottomConnectedAreas(YTab* bottom, BObjectList<Area>& areas)
	{
		if (bottom == fALMLayout->Bottom())
			return true;

		const tab_links<YTab>& link = fYTabLinkMap[bottom];
		for (int32 i = 0; i < link.areas2.CountItems(); i++)
			areas.RemoveItem(link.areas2.ItemAt(i));
		for (int32 i = 0; i < link.tabs2.CountItems(); i++)
			_RemoveBottomConnectedAreas(link.tabs2.ItemAt(i), areas);

		return link.tabs2.CountItems() > 0;
	}

private:
			BALMLayout*			fALMLayout;
			BObjectList<Constraint>	fVConstraints;
			BObjectList<Constraint>	fHConstraints;

			TabConnections*		fTabConnections;
			std::map<XTab*, tab_links<XTab> >& fXTabLinkMap;
			std::map<YTab*, tab_links<YTab> >& fYTabLinkMap;

			std::vector<debug_info>	fDebugInfos;
};


}	// namespace BALM


#endif	// OVERLAP_MANAGER_H

/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#include "AreaRemoval.h"


using namespace BALM;

void AreaRemovalHelper::_MoveGroupBy(const BObjectList<Area>& group,
	const BPoint& delta)
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


AreaRemoval::AreaRemoval(TabConnections* connections, BALMLayout* layout)
	:
	fTabConnections(connections),
	fLayout(layout)
{
}


void
AreaRemoval::FillEmptySpace(BALMLayout* layout, XTab* left, YTab* top,
	XTab* right, YTab* bottom)
{
	TabConnections connections;
	AreaRemoval areaRemoval(&connections, layout);
	areaRemoval.AreaRemoved(left, top, right, bottom);
}


void
AreaRemoval::AreaRemoved(XTab* left, YTab* top, XTab* right, YTab* bottom)
{
	// TODO maybe do this a bit more efficient somewhere else?
	fTabConnections->Fill(fLayout);

	std::map<XTab*, tab_links<XTab> >& xConnections
		= fTabConnections->GetXTabLinkMap();
	std::map<YTab*, tab_links<YTab> >& yConnections
		= fTabConnections->GetYTabLinkMap();

	if (!fTabConnections->ConnectedToLeftBorder(left, fLayout)
		&& !fTabConnections->ConnectedToRightBorder(left, fLayout)) {
		AreaRemovalHelper::MoveGroups<XTab, ar_right_search>(
			fLayout, fTabConnections, xConnections[left].areas1);
		AreaRemovalHelper::MoveGroups<XTab, ar_right_search>(
			fLayout, fTabConnections, xConnections[left].areas2);
	}

	if (!fTabConnections->ConnectedToLeftBorder(right, fLayout)
		&& !fTabConnections->ConnectedToRightBorder(right, fLayout)) {
		AreaRemovalHelper::MoveGroups<XTab, ar_left_search>(
			fLayout, fTabConnections, xConnections[right].areas2);
		AreaRemovalHelper::MoveGroups<XTab, ar_left_search>(
			fLayout, fTabConnections, xConnections[right].areas1);
	}

	if (!fTabConnections->ConnectedToTopBorder(top, fLayout)
		&& !fTabConnections->ConnectedToBottomBorder(top, fLayout)) {
		AreaRemovalHelper::MoveGroups<YTab, ar_bottom_search>(
			fLayout, fTabConnections, yConnections[top].areas1);
		AreaRemovalHelper::MoveGroups<YTab, ar_bottom_search>(
			fLayout, fTabConnections, yConnections[top].areas2);
	}

	if (!fTabConnections->ConnectedToTopBorder(bottom, fLayout)
		&& !fTabConnections->ConnectedToBottomBorder(bottom, fLayout)) {
		AreaRemovalHelper::MoveGroups<YTab, ar_top_search>(
			fLayout, fTabConnections, yConnections[bottom].areas2);
		AreaRemovalHelper::MoveGroups<YTab, ar_top_search>(
			fLayout, fTabConnections, yConnections[bottom].areas1);
	}
}


void
AreaRemoval::AreaDetachedX(Area* area, area_side side, XTab* oldTab)
{
	fTabConnections->Fill(fLayout);

	BObjectList<Area> areaList;
	areaList.AddItem(area);

	std::map<XTab*, tab_links<XTab> >& connections
		= fTabConnections->GetXTabLinkMap();
	if (connections.find(oldTab) == connections.end())
		oldTab = NULL;

	if (side == kLeft) {
		AreaRemovalHelper::MoveGroups<XTab, ar_right_search>(fLayout,
			fTabConnections, areaList);
		if (oldTab != NULL) {
			AreaRemovalHelper::MoveGroups<XTab, ar_left_search>(fLayout,
				fTabConnections, connections[oldTab].areas1);
			AreaRemovalHelper::MoveGroups<XTab, ar_right_search>(fLayout,
				fTabConnections, connections[oldTab].areas2);
		}
	} else if (side == kRight) {
		AreaRemovalHelper::MoveGroups<XTab, ar_left_search>(fLayout,
			fTabConnections, areaList);
		if (oldTab != NULL) {
			AreaRemovalHelper::MoveGroups<XTab, ar_left_search>(fLayout,
				fTabConnections, connections[oldTab].areas1);
			AreaRemovalHelper::MoveGroups<XTab, ar_right_search>(fLayout,
				fTabConnections, connections[oldTab].areas2);
		}
	}
}


void
AreaRemoval::AreaDetachedY(Area* area, area_side side, YTab* oldTab)
{
	fTabConnections->Fill(fLayout);

	BObjectList<Area> areaList;
	areaList.AddItem(area);

	std::map<YTab*, tab_links<YTab> >& connections
		= fTabConnections->GetYTabLinkMap();
	if (connections.find(oldTab) == connections.end())
		oldTab = NULL;

	if (side == kTop) {
		AreaRemovalHelper::MoveGroups<YTab, ar_bottom_search>(fLayout,
			fTabConnections, areaList);
		if (oldTab != NULL) {
			AreaRemovalHelper::MoveGroups<YTab, ar_top_search>(
				fLayout, fTabConnections, connections[oldTab].areas1);
			AreaRemovalHelper::MoveGroups<YTab, ar_bottom_search>(
				fLayout, fTabConnections, connections[oldTab].areas2);
		}
	} else if (side == kBottom) {
		AreaRemovalHelper::MoveGroups<YTab, ar_top_search>(fLayout,
			fTabConnections, areaList);
		if (oldTab != NULL) {
			AreaRemovalHelper::MoveGroups<YTab, ar_top_search>(
				fLayout, fTabConnections, connections[oldTab].areas1);
			AreaRemovalHelper::MoveGroups<YTab, ar_bottom_search>(
				fLayout, fTabConnections, connections[oldTab].areas2);
		}
	}
}

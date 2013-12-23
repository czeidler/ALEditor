/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "OverlapManager.h"


BALM::OverlapManager::OverlapManager(BALMLayout* layout)
	:
	fALMLayout(layout)
{
	fOverlapEngine = new SimpleOverlapEngine(layout, this);
}
	

void
BALM::OverlapManager::DisconnectAreas()
{
	fTabConnections.Clear();
	fOverlapEngine->DisconnectAreas();
}


void
BALM::OverlapManager::FillTabConnections()
{
	fTabConnections.Fill(fALMLayout);
}


void
BALM::OverlapManager::ConnectAreas(bool fillTabConnections)
{
	if (fillTabConnections)
		FillTabConnections();
	fOverlapEngine->ConnectAreas();
}


void
BALM::OverlapManager::Draw(BView* view)
{
	fOverlapEngine->Draw(view);
}

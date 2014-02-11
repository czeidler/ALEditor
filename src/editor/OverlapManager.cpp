/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "OverlapManager.h"

#include "cdt/quadedge.h"


BALM::OverlapManager::OverlapManager(BALMLayout* layout)
	:
	fALMLayout(layout)
{
	fOverlapEngine = new SimpleOverlapEngine(layout, this);
	//fOverlapEngine = new CDTEngine(fALMLayout);
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


BALM::CDTEngine::CDTEngine(BALMLayout* layout)
	:
	fALMLayout(layout),
	fMesh(NULL)
{
}


void
BALM::CDTEngine::DisconnectAreas()
{
	delete fMesh;
	fMesh = NULL;
}


void
BALM::CDTEngine::ConnectAreas()
{
	Point2d leftTop(fALMLayout->Left()->Value(),
		fALMLayout->Top()->Value());
	Point2d rightTop(fALMLayout->Right()->Value(),
		fALMLayout->Top()->Value());
	Point2d rightBottom(fALMLayout->Right()->Value(),
		fALMLayout->Bottom()->Value());
	Point2d leftBottom(fALMLayout->Left()->Value(),
		fALMLayout->Bottom()->Value());
	fMesh = new Mesh(leftTop, rightTop, rightBottom, leftBottom);
	for (int32 i = 0; i < fALMLayout->CountAreas(); i++) {
		Area* area = fALMLayout->AreaAt(i);
		BRect frame = area->Frame();
		fMesh->InsertEdge(Point2d(frame.left, frame.top),
			Point2d(frame.right, frame.top));
		fMesh->InsertEdge(Point2d(frame.right, frame.top),
			Point2d(frame.right, frame.bottom));
		fMesh->InsertEdge(Point2d(frame.right, frame.bottom),
			Point2d(frame.left, frame.bottom));
		fMesh->InsertEdge(Point2d(frame.left, frame.bottom),
			Point2d(frame.left, frame.top));
	}
}


static void draw_edge(void* cookie, void* edge, Boolean isConstrained)
{
	BView* view = (BView*)cookie;
	Edge* e = (Edge*)edge;
	Point2d a, b;
	a = e->Org2d();
	b = e->Dest2d();
	BPoint start(a[0], a[1]);
	BPoint end(b[0], b[1]);
	view->SetPenSize(1);
	if (isConstrained) {
		rgb_color colour = {255, 0, 0};
		view->SetHighColor(colour);
	} else {
		rgb_color colour = {0, 255, 0};
		view->SetHighColor(colour);
	}
	view->StrokeLine(start, end);
}


void
BALM::CDTEngine::Draw(BView* view)
{
	fMesh->ApplyEdges(draw_edge, view);
}


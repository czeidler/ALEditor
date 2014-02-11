/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include <LayoutArchive.h>

#include <Application.h>
#include <DataIO.h>
#include <File.h>
#include <fs_attr.h>
#include <Node.h>
#include <Roster.h>

#include <CustomizableRoster.h>
#include <CustomizableView.h>


using namespace BALM;


LayoutArchive::LayoutArchive(BALMLayout* layout)
	:
	fLayout(layout)
{
}


BView*
LayoutArchive::FindView(const char* identifier)
{
	for (int32 i = 0; i < fLayout->CountAreas(); i++) {
		Area* area = fLayout->AreaAt(i);

		BView* view = area->Item()->View();
		CustomizableView* customizable = dynamic_cast<CustomizableView*>(view);
		if (customizable == NULL)
			continue;
		if (customizable->Identifier() == identifier)
			return view;
	}
	return NULL;
}


BLayoutItem*
LayoutArchive::FindLayoutItem(const char* identifier)
{
	for (int32 i = 0; i < fLayout->CountAreas(); i++) {
		Area* area = fLayout->AreaAt(i);

		BLayoutItem* item = area->Item();
		CustomizableView* customizable = dynamic_cast<CustomizableView*>(item);
		if (customizable == NULL)
			continue;
		if (customizable->Identifier() == identifier)
			return item;
	}
	return NULL;
}


enum {
	kLeftBorderIndex = -2,
	kTopBorderIndex = -3,
	kRightBorderIndex = -4,
	kBottomBorderIndex = -5,
};


void
LayoutArchive::ClearLayout()
{
	while(true) {
		BLayoutItem* item = fLayout->RemoveItem(int32(0));
		if (item == NULL)
			break;
		BView* view = item->View();
		CustomizableView* customizable = dynamic_cast<CustomizableView*>(view);
		if (customizable == NULL)
			delete view;
		else {
			delete item;
			continue;
		}
		customizable = dynamic_cast<CustomizableView*>(item);
		if (customizable == NULL)
			delete item;
	}
}


status_t
LayoutArchive::SaveLayout(BMessage* archive, bool saveComponent) const
{
	archive->MakeEmpty();

	float left, top, right, bottom;
	fLayout->GetInsets(&left, &top, &right, &bottom);
	archive->AddFloat("leftInset", left);
	archive->AddFloat("topInset", top);
	archive->AddFloat("rightInset", right);
	archive->AddFloat("bottomInset", bottom);
	
	float hSpacing, vSpacing;
	fLayout->GetSpacing(&hSpacing, &vSpacing);
	archive->AddFloat("hSpacing", hSpacing);
	archive->AddFloat("vSpacing", vSpacing);

	archive->AddInt32("nXTabs", fLayout->CountXTabs());
	archive->AddInt32("nYTabs", fLayout->CountYTabs());

	XTabList xTabs = fLayout->GetXTabs();
	xTabs.RemoveItem(fLayout->Left());
	xTabs.RemoveItem(fLayout->Right());
	YTabList yTabs = fLayout->GetYTabs();
	yTabs.RemoveItem(fLayout->Top());
	yTabs.RemoveItem(fLayout->Bottom());
	
	int32 nAreas = fLayout->CountAreas();
	for (int32 i = 0; i < nAreas; i++) {
		Area* area = fLayout->AreaAt(i);
		if (saveComponent)
			_SaveComponent(area, archive);

		if (area->Left() == fLayout->Left())
			archive->AddInt32("left", kLeftBorderIndex);
		else
			archive->AddInt32("left", xTabs.IndexOf(area->Left()));
		if (area->Top() == fLayout->Top())
			archive->AddInt32("top", kTopBorderIndex);
		else
			archive->AddInt32("top", yTabs.IndexOf(area->Top()));
		if (area->Right() == fLayout->Right())
			archive->AddInt32("right", kRightBorderIndex);
		else
			archive->AddInt32("right", xTabs.IndexOf(area->Right()));
		if (area->Bottom() == fLayout->Bottom())
			archive->AddInt32("bottom", kBottomBorderIndex);
		else
			archive->AddInt32("bottom", yTabs.IndexOf(area->Bottom()));

		// store values
		archive->AddFloat("leftValue", area->Left()->Value());
		archive->AddFloat("rightValue", area->Right()->Value());
		archive->AddFloat("topValue", area->Top()->Value());
		archive->AddFloat("bottomValue", area->Bottom()->Value());
	}

	// save custom constraints
	for (int32 i = 0; i < fLayout->CountConstraints(); i++) {
		Constraint* constraint = fLayout->ConstraintAt(i);

		archive->AddString("label", constraint->Label());
		archive->AddInt32("operator", constraint->Op());
		archive->AddDouble("rightSide", constraint->RightSide());
		archive->AddDouble("penaltyNeg", constraint->PenaltyNeg());
		archive->AddDouble("penaltyPos", constraint->PenaltyPos());
		BMessage leftSide;
		SummandList* summands = constraint->LeftSide();
		for (int32 s = 0; s < summands->CountItems(); s++) {
			Summand* summand = summands->ItemAt(s);
			leftSide.AddDouble("coeff", summand->Coeff());
			bool isXTab = true;
			XTab* xTab = static_cast<XTab*>(summand->Var());
			int32 varIndex = -1;
			if (xTab == fLayout->Left())
				varIndex = kLeftBorderIndex;
			else if (xTab == fLayout->Right())
				varIndex = kRightBorderIndex;
			else
				varIndex = xTabs.IndexOf(xTab);
			if (varIndex == -1) {
				isXTab = false;
				YTab* yTab = static_cast<YTab*>(summand->Var());
				if (yTab == fLayout->Top())
					varIndex = kTopBorderIndex;
				else if (yTab == fLayout->Bottom())
					varIndex = kBottomBorderIndex;
				else
					varIndex = yTabs.IndexOf(yTab);
			}
			leftSide.AddInt32("var", varIndex);
			leftSide.AddBool("isXTab", isXTab);
		}
		archive->AddMessage("leftSide", &leftSide);
	}
	return B_OK;
}


status_t
LayoutArchive::RestoreLayout(const BMessage* archive, bool restoreComponents)
{
	if (restoreComponents)
		ClearLayout();

	float left, top, right, bottom;
	archive->FindFloat("leftInset", &left);
	archive->FindFloat("topInset", &top);
	archive->FindFloat("rightInset", &right);
	archive->FindFloat("bottomInset", &bottom);
	fLayout->SetInsets(left, top, right, bottom);

	float hSpacing, vSpacing;
	archive->FindFloat("hSpacing", &hSpacing);
	archive->FindFloat("vSpacing", &vSpacing);
	fLayout->SetSpacing(hSpacing, vSpacing);

	int32 neededXTabs;
	int32 neededYTabs;
	status_t status = B_OK;
	status = archive->FindInt32("nXTabs", &neededXTabs);
	if (status != B_OK)
		return status;
	status = archive->FindInt32("nYTabs", &neededYTabs);
	if (status != B_OK)
		return status;
	// First store a reference to all needed tabs otherwise they might get lost
	// while editing the layout
	std::vector<BReference<XTab> > newXTabs;
	std::vector<BReference<YTab> > newYTabs;
	int32 existingXTabs = fLayout->CountXTabs();
	for (int32 i = 0; i < neededXTabs; i++) {
		if (i < existingXTabs)
			newXTabs.push_back(BReference<XTab>(fLayout->XTabAt(i)));
		else
			newXTabs.push_back(fLayout->AddXTab());
	}
	int32 existingYTabs = fLayout->CountYTabs();
	for (int32 i = 0; i < neededYTabs; i++) {
		if (i < existingYTabs)
			newYTabs.push_back(BReference<YTab>(fLayout->YTabAt(i)));
		else
			newYTabs.push_back(fLayout->AddYTab());
	}

	XTabList xTabs = fLayout->GetXTabs();
	xTabs.RemoveItem(fLayout->Left());
	xTabs.RemoveItem(fLayout->Right());
	YTabList yTabs = fLayout->GetYTabs();
	yTabs.RemoveItem(fLayout->Top());
	yTabs.RemoveItem(fLayout->Bottom());

	if (restoreComponents) {
		int32 aIndex = -1;
		while (true) {
			aIndex++;
			BMessage componentArchive;
			if (archive->FindMessage("component", aIndex, &componentArchive)
				!= B_OK)
				break;
			Area* area = _CreateComponent(&componentArchive);
			if (area == NULL)
				continue;
			_RestoreArea(area, aIndex, archive, xTabs, yTabs);
		}
	} else {
		int32 nAreas = fLayout->CountAreas();
		for (int32 i = 0; i < nAreas; i++) {
			Area* area = fLayout->AreaAt(i);
			if (area == NULL)
				return B_ERROR;
	
			_RestoreArea(area, i, archive, xTabs, yTabs);
		}
	}

	// restore custom constraints
	// remove all constraints first
	while (true) {
		Constraint* constraint = fLayout->ConstraintAt(0);
		if (constraint == NULL)
			break;
		fLayout->RemoveConstraint(constraint, true);
	}

	int32 cIndex = -1;
	while (true) {
		cIndex++;

		LinearProgramming::OperatorType op;
		status_t status = archive->FindInt32("operator", cIndex, (int32*)&op);
		if (status != B_OK)
			break;

		Constraint* constraint = new Constraint;
		SummandList* summands = constraint->LeftSide();

		double rightSide = archive->FindDouble("rightSide", cIndex);
		double penaltyNeg = archive->FindDouble("penaltyNeg", cIndex);
		double penaltyPos = archive->FindDouble("penaltyPos", cIndex);
		BString label = archive->FindString("label", cIndex);
		constraint->SetLabel(label);
		constraint->SetOp(op);
		constraint->SetRightSide(rightSide);
		constraint->SetPenaltyNeg(penaltyNeg);
		constraint->SetPenaltyPos(penaltyPos);

		BMessage leftSideMsg;
		archive->FindMessage("leftSide", cIndex, &leftSideMsg);
		int32 vIndex = -1;
		while (true) {
			vIndex++;

			double coeff;
			status_t status = leftSideMsg.FindDouble("coeff", vIndex, &coeff);
			if (status != B_OK)
				break;
			bool isXTab = leftSideMsg.FindBool("isXTab", vIndex);
			int32 varIndex = leftSideMsg.FindInt32("var", vIndex);
			if (isXTab) {
				XTab* tab = NULL;
				if (varIndex == kLeftBorderIndex)
					tab = fLayout->Left();
				else if (varIndex == kRightBorderIndex)
					tab = fLayout->Right();
				else
					tab = xTabs.ItemAt(varIndex);
				Summand* summand = new Summand(coeff, tab);
				summands->AddItem(summand);
			} else {
				YTab* tab = NULL;
				if (varIndex == kTopBorderIndex)
					tab = fLayout->Top();
				else if (varIndex == kBottomBorderIndex)
					tab = fLayout->Bottom();
				else
					tab = yTabs.ItemAt(varIndex);
				Summand* summand = new Summand(coeff, tab);
				summands->AddItem(summand);
			}		
		}
		fLayout->AddConstraint(constraint);
	}
	return B_OK;
}


status_t
LayoutArchive::SaveToFile(BFile* file, const BMessage* message)
{
	return message->Flatten(file);
}


status_t
LayoutArchive::RestoreFromFile(BFile* file, bool restoreComponents)
{
	BMessage archive;
	status_t status = archive.Unflatten(file);
	if (status != B_OK)
		return status;

	return RestoreLayout(&archive, restoreComponents);
}


status_t
LayoutArchive::SaveToAppFile(const char* attribute, const BMessage* message)
{
	if (be_app == NULL)
		return B_ERROR;
	app_info appInfo;
	be_app->GetAppInfo(&appInfo);
	BNode node(&appInfo.ref);
	return SaveToAttribute(&node, attribute, message);
}


status_t
LayoutArchive::RestoreFromAppFile(const char* attribute, bool restoreComponents)
{
	if (be_app == NULL)
		return B_ERROR;
	app_info appInfo;
	be_app->GetAppInfo(&appInfo);
	BNode node(&appInfo.ref);
	return RestoreFromAttribute(&node, attribute, restoreComponents);
}


status_t
LayoutArchive::SaveToAttribute(BNode* node, const char* attribute,
	const BMessage* message)
{
	BMallocIO buffer;
	status_t status = message->Flatten(&buffer);
	if (status != B_OK)
		return status;

	ssize_t written = node->WriteAttr(attribute, B_RAW_TYPE, 0, buffer.Buffer(),
		buffer.BufferLength());
	if (written < 0)
		return B_ERROR;
	return B_OK;
}


status_t
LayoutArchive::RestoreFromAttribute(BNode* node, const char* attribute,
	bool restoreComponents)
{
	attr_info info;
	status_t status = node->GetAttrInfo(attribute, &info);
	if (status != B_OK)
		return status;
	if (info.type != B_RAW_TYPE)
		return B_ERROR;
	char* buffer = (char*)malloc(info.size);
	if (buffer == NULL)
		return B_NO_MEMORY;
	ssize_t read = node->ReadAttr(attribute, B_RAW_TYPE, 0, buffer, info.size);
	if (read < 0) {
		free(buffer);
		return B_ERROR;
	}
	BMessage archive;
	status = archive.Unflatten(buffer);
	free(buffer);
	if (status != B_OK)
		return status;

	return RestoreLayout(&archive, restoreComponents);
}


bool
LayoutArchive::_RestoreArea(Area* area, int32 i, const BMessage* archive,
	XTabList& xTabs, YTabList& yTabs)
{
	int32 left = -1;
	if (archive->FindInt32("left", i, &left) != B_OK)
		return false;
	int32 top = archive->FindInt32("top", i);
	int32 right = archive->FindInt32("right", i);
	int32 bottom = archive->FindInt32("bottom", i);

	XTab* leftTab = NULL;
	YTab* topTab = NULL;
	XTab* rightTab = NULL;
	YTab* bottomTab = NULL;

	if (left == kLeftBorderIndex)
		leftTab = fLayout->Left();
	else
		leftTab = xTabs.ItemAt(left);
	if (top == kTopBorderIndex)
		topTab = fLayout->Top();
	else
		topTab = yTabs.ItemAt(top);
	if (right == kRightBorderIndex)
		rightTab = fLayout->Right();
	else
		rightTab = xTabs.ItemAt(right);
	if (bottom == kBottomBorderIndex)
		bottomTab = fLayout->Bottom();
	else
		bottomTab = yTabs.ItemAt(bottom);
	if (leftTab == NULL || topTab == NULL || rightTab == NULL
		|| bottomTab == NULL)
		return false;

	area->SetLeft(leftTab);
	area->SetTop(topTab);
	area->SetRight(rightTab);
	area->SetBottom(bottomTab);

	// restore values
	leftTab->SetValue(archive->FindFloat("leftValue", i));
	rightTab->SetValue(archive->FindFloat("rightValue", i));
	topTab->SetValue(archive->FindFloat("topValue", i));
	bottomTab->SetValue(archive->FindFloat("bottomValue", i));

	return true;
}


bool	
LayoutArchive::_SaveComponent(Area* area, BMessage* archive) const
{
	BMessage componentData;

	BView* view = area->Item()->View();
	CustomizableView* customizable = dynamic_cast<CustomizableView*>(view);
	if (customizable == NULL) {
		BLayoutItem* item = area->Item();
		customizable = dynamic_cast<CustomizableView*>(item);
		if (customizable == NULL) {
			archive->AddMessage("component", &componentData);
			return false;
		}
	}

	BString objectName = customizable->ObjectName();
	componentData.AddString("objectName", objectName);
	BString identifier = customizable->Identifier();
	componentData.AddString("identifier", identifier);

	archive->AddMessage("component", &componentData);
	return true;
}


Area*
LayoutArchive::_CreateComponent(const BMessage* archive)
{
	BString name = archive->FindString("objectName");

	CustomizableRoster* roster = CustomizableRoster::DefaultRoster();
	BReference<Customizable> clone = roster->InstantiateCustomizable(name);

	CustomizableView* customizable
		= dynamic_cast<CustomizableView*>(clone.Get());
	if (customizable == NULL)
		return NULL;

	BString identifier = archive->FindString("identifier");
	customizable->SetIdentifier(identifier);

	Area* area = NULL;
	if (customizable->View().Get() != NULL) {
		area = fLayout->AddView(customizable->View(), fLayout->Left(),
			fLayout->Top());
	} else if (customizable->LayoutItem().Get() != NULL) {
		area = fLayout->AddItem(customizable->LayoutItem(), fLayout->Left(),
			fLayout->Top());
	}

	roster->AddToShelf(clone);

	return area;
}

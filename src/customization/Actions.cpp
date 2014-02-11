/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "Actions.h"


using namespace BALM;


ActionHistory::~ActionHistory()
{
	for (int32 i = 0; i < fActions.CountItems(); i++)
		delete fActions.ItemAt(i);
}


status_t
ActionHistory::PerformAction(CustomizationAction* action)
{
	if (fActions.AddItem(action) == false) {
		delete action;
		return B_NO_MEMORY;
	}
	status_t status = action->Perform();
	if (status != B_OK) {
		fActions.RemoveItem(action);
		delete action;
		return status;
	}
	return status;
}

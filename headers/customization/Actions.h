/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	ACTIONS_H
#define	ACTIONS_H


#include "CustomizableRoster.h"


namespace BALM {

class CustomizationAction {
public:
	virtual						~CustomizationAction() {}

	virtual	status_t			Perform() = 0;
	/*! Try undo the action, fails if there are some later dependencies on this
	aciton. */
	virtual	status_t			Undo() = 0;

};


class ActionHistory {
public:
								~ActionHistory();

			status_t			PerformAction(CustomizationAction* action);

private:
			BObjectList<CustomizationAction>	fActions;
};


} // end namespace BALM


using BALM::CustomizationAction;


#endif // ACTIONS_H

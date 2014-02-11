/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	CUSTOMIZABLE_NODE_FACTORY_H
#define	CUSTOMIZABLE_NODE_FACTORY_H


#include <ListView.h>
#include <Window.h>

#include <CustomizableRoster.h>


const uint32 kMsgCreateComponent = '_CCP';


class FactoryDragListView : public BListView {
public:
								FactoryDragListView(CustomizableRoster* roster);

	virtual	void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* message);
	virtual void				AttachedToWindow();
	virtual bool				InitiateDrag(BPoint point, int32 index,
									bool wasSelected);
private:
			BALM::CustomizableAddOnList	fInstalledComponents;
			CustomizableRoster*	fRoster;
			BPoint				fCurrentMousePosition;
};


class CustomizableFactoryWindow : public BWindow {
public:
								CustomizableFactoryWindow(
									CustomizableRoster* roster);
};


#endif // CUSTOMIZABLE_NODE_FACTORY_H

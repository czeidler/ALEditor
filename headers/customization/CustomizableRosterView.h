/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	CUSTOMIZABLE_ROSTER_VIEW_H
#define	CUSTOMIZABLE_ROSTER_VIEW_H


#include <Button.h>
#include <ListView.h>

#include "CustomizableRoster.h"


namespace BALM {


class InstalledCustomizableView : public BView {
public:
								InstalledCustomizableView();
			void				AttachedToWindow();
			void				MessageReceived(BMessage* message);
private:
			BListView*			fInstalledComponentsListView;
			BButton*			fCreateComponent;

			std::vector<BString> fInstalledComponents;
};


class CustomizableRosterView : public BView {
public:
								CustomizableRosterView();
								~CustomizableRosterView();

			void				AttachedToWindow();
			void				MessageReceived(BMessage* message);

private:
			void				_LoadCustomizableList();

			CustomizableRoster*	fRoster;

			BListView*			fCustomizableListView;
			BButton*			fAddComponent;
			BButton*			fRemoveComponent;
};


}	// namespace BALM


using BALM::InstalledCustomizableView;
using BALM::CustomizableRosterView;


#endif	// CUSTOMIZABLE_ROSTER_VIEW_H

/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	CUSTOMIZABLE_EDIT_VIEW_H
#define	CUSTOMIZABLE_EDIT_VIEW_H


#include <OutlineListView.h>

#include <Customizable.h>


class CustomizableEditView;


class ConnectionListView : public BOutlineListView {
public:
								ConnectionListView(
									CustomizableEditView* parent);

			CustomizableEditView*	Parent() { return fParent; }
			void				MouseDown(BPoint point);
private:
			CustomizableEditView*	fParent;
};


class CustomizableEditView : public BView {
public:
								CustomizableEditView();
								~CustomizableEditView();

			void				SetTo(Customizable* component);
			void				MessageReceived(BMessage* message);

private:
			void				_FillSocketConnections(
									Customizable::Socket* socket,
									BStringItem* socketItem);

			Customizable*		fCustomizable;

			BOutlineListView*	fConnectionListView;
};


#endif	// CUSTOMIZABLE_EDIT_VIEW_H


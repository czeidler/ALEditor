/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	CUSTOMIZABLE_VIEW_H
#define	CUSTOMIZABLE_VIEW_H


#include <Layout.h>
#include <LayoutItem.h>
#include <View.h>

#include <Customizable.h>
#include <CustomizableRoster.h>

#include <ArrayContainer.h>

#if HAS_PIGIN
#include "ViewContainer.h"
#else
#include "CustomTypes.h"

class IViewContainer : public Customizable {
public:
	virtual 					~IViewContainer() {}

	virtual BString				Identifier() = 0;
	virtual void				SetIdentifier(const BString& id) = 0;

	virtual BViewLocalPointer	View() = 0;
	virtual BViewLocalPointer	CreateConfigView() = 0;
	virtual BLayoutItemLocalPointer	LayoutItem() = 0;

	virtual void				RemoveSelf() = 0;
};
#endif

namespace BALM {

class CustomizableView : public IViewContainer {
public:
								CustomizableView(BView* parent,
									bool own = false);
								CustomizableView(BLayoutItem* item,
									bool own = false);
	virtual						~CustomizableView();

			BString				Identifier();
			void				SetIdentifier(const BString& id);

			BViewLocalPointer	View();
			BViewLocalPointer	CreateConfigView();
			BLayoutItemLocalPointer	LayoutItem();

			void				RemoveSelf();

			BSize				PreferredSize();
			BSize				MinSize();
			BRect				Frame();
			BLayout*			Layout();

protected:

private:
			BString				fName;

			BView*				fView;
			BLayoutItem*		fLayoutItem;
			bool				fOwnObject;
};


} // end namespace BALM


using BALM::CustomizableView;


#endif // CUSTOMIZABLE_VIEW_H

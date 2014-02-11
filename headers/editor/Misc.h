/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	MISC_H
#define	MISC_H


#include <Bitmap.h>
#include <Rect.h>


#include <CustomizableView.h>


namespace BALM {
	

class EditorHelper {
public:


static BRect GetDragFrame(const BSize prefSize, const BSize minSize)
{
	const float kDefaultWidth = 100;
	const float kDefaultHeight = 100;
	const float kToLarge = 250;

	BRect frame;
	frame.left = 0;
	frame.top = 0;
	// validate frame
	if (prefSize.width > kToLarge) {
		if (minSize.width > kDefaultWidth)
			frame.right = minSize.width;
		else
			frame.right = kDefaultWidth;
	} else
		frame.right = prefSize.width;
	if (prefSize.height > kToLarge) {
		if (minSize.height > kDefaultHeight)
			frame.bottom = minSize.height;
		else
			frame.bottom = kDefaultHeight;
	} else
		frame.bottom = prefSize.height;

// app server bug:
if (frame.right > 150)
	frame.right = 150;
if (frame.bottom > 40)
	frame.bottom = 40;

	return frame;
}


static BBitmap* CreateBitmap(BALM::CustomizableAddOn* addOn, BRect& frame,
	BMessage* message = NULL)
{
	BReference<Customizable> customizable = addOn->InstantiateCustomizable(
		message);
	CustomizableView* customizableView = dynamic_cast<CustomizableView*>(
		customizable.Get());
	if (customizableView == NULL) {
		frame = BRect(-1, -1, -1, -1);
		return NULL;
	}

	BSize prefSize = customizableView->PreferredSize();
	BSize minSize = customizableView->MinSize();

	frame = GetDragFrame(prefSize, minSize);
return NULL;
	BView* view = customizableView->View();
	if (view == NULL)
		return NULL;
	BBitmap* bitmap = new BBitmap(frame, B_RGBA32, true);
	if (!bitmap->Lock())
		return NULL;
	bitmap->AddChild(view);
	view->ResizeTo(frame.Width(), frame.Height());
	view->Draw(frame);
	view->Sync();
	view->RemoveSelf();
	bitmap->Unlock();

	BBitmap* dragBitmap = new BBitmap(frame, B_RGBA32);
	memcpy(dragBitmap->Bits(), bitmap->Bits(), bitmap->BitsLength());
	delete bitmap;

	return dragBitmap;
}


};


}


#endif	// MISC_H

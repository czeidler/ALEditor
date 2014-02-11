#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H

#include <PointerRef.h>

#include <Bitmap.h>
#include <LayoutItem.h>
#include <View.h>

//TODO let pidgen generate this declarations?

typedef BALM::PointerRef<BBitmap> BBitmapLocalPointer;

status_t BBitmapLocalPointerToMessage(const BBitmapLocalPointer* pointer,
	BMessage* message);
status_t BBitmapLocalPointerFromMessage(BBitmapLocalPointer* pointer,
	BMessage* message);


typedef BALM::PointerRef<BView> BViewLocalPointer;

status_t BViewLocalPointerToMessage(const BViewLocalPointer* pointer,
	BMessage* message);
status_t BViewLocalPointerFromMessage(BViewLocalPointer* pointer,
	BMessage* message);

typedef BALM::PointerRef<BLayoutItem> BLayoutItemLocalPointer;

status_t BLayoutItemLocalPointerToMessage(const BLayoutItemLocalPointer* pointer,
	BMessage* message);
status_t BLayoutItemLocalPointerFromMessage(BLayoutItemLocalPointer* pointer,
	BMessage* message);


#endif //CUSTOM_TYPES_H

#include "CustomTypes.h"


status_t
BBitmapLocalPointerToMessage(const BBitmapLocalPointer* pointer,
	BMessage* message)
{
	return pointer->ToMessage(message);
}


status_t
BBitmapLocalPointerFromMessage(BBitmapLocalPointer* pointer,
	BMessage* message)
{
	return pointer->FromMessage(message);
}


status_t
BViewLocalPointerToMessage(const BViewLocalPointer* pointer,
	BMessage* message)
{
	return pointer->ToMessage(message);
}


status_t
BViewLocalPointerFromMessage(BViewLocalPointer* pointer,
	BMessage* message)
{
	return pointer->FromMessage(message);
}


status_t
BLayoutItemLocalPointerToMessage(const BLayoutItemLocalPointer* pointer,
	BMessage* message)
{
	return pointer->ToMessage(message);
}


status_t
BLayoutItemLocalPointerFromMessage(BLayoutItemLocalPointer* pointer,
	BMessage* message)
{
	return pointer->FromMessage(message);
}

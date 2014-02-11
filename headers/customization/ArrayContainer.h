/*
 * Copyright 2009-2011, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2011-2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef _ARRAY_CONTAINER_H
#define _ARRAY_CONTAINER_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SupportDefs.h>


namespace BPrivate {


template<typename Element>
class BArray {
public:
	inline						BArray();
								BArray(const BArray<Element>& other);
	inline						~BArray();

	inline	int32				CountItems() const		{ return fSize; }
	inline	bool				IsEmpty() const		{ return fSize == 0; }
	inline	Element*			Elements() const	{ return fElements; }

	inline	bool				AddItem(const Element& element);
	inline	bool				AddUninitialized(int32 elementCount);
	inline	bool				AddItemAt(const Element& element, int32 index);
	inline	bool				AddUninitializedAt(int32 index, int32 count);
	inline	bool				RemoveItemAt(int32 index, int32 count = 1);
	inline	bool				RemoveItem(const Element& element);

	inline	int32				IndexOf(const Element& element);

	inline	void				MakeEmpty();

	inline	Element&			ItemAt(int32 index);
	inline	const Element&		ItemAt(int32 index) const;

	inline	Element&			operator[](int32 index);
	inline	const Element&		operator[](int32 index) const;

			BArray<Element>&	operator=(const BArray<Element>& other);

private:
	static	const int32			kMinCapacity = 8;

	inline	void				_Initialize(Element* target,
									const Element& source);
	inline	void				_Destroy(Element* element);
	inline	void				_MoveLeft(Element* from, Element* to,
									int32 count);
	inline	void				_MoveRight(Element* from, Element* to,
									int32 count);
	inline	void				_Move(Element* from, Element* to,
									int32 count);
			bool				_Resize(int32 index, int32 delta);

private:
			Element*			fElements;
			int32				fSize;
			int32				fCapacity;
};


template<typename Element>
BArray<Element>::BArray()
	:
	fElements(NULL),
	fSize(0),
	fCapacity(0)
{
}


template<typename Element>
BArray<Element>::BArray(const BArray<Element>& other)
	:
	fElements(NULL),
	fSize(0),
	fCapacity(0)
{
	*this = other;
}


template<typename Element>
BArray<Element>::~BArray()
{
	MakeEmpty();
}


template<typename Element>
bool
BArray<Element>::AddItem(const Element& element)
{
	if (!_Resize(fSize, 1))
		return false;

	_Initialize(fElements + fSize - 1, element);

	return true;
}


template<typename Element>
inline bool
BArray<Element>::AddUninitialized(int32 elementCount)
{
	return AddUninitializedAt(fSize, elementCount);
}


template<typename Element>
bool
BArray<Element>::AddItemAt(const Element& element, int32 index)
{
	if (index < 0 || index > fSize)
		index = fSize;

	if (!_Resize(index, 1))
		return false;

	_Initialize(fElements + index, element);

	return true;
}


template<typename Element>
bool
BArray<Element>::AddUninitializedAt(int32 index, int32 count)
{
	if (index < 0 || index > fSize || count < 0)
		return false;
	if (count == 0)
		return true;

	if (!_Resize(index, count))
		return false;

	return true;
}


template<typename Element>
bool
BArray<Element>::RemoveItemAt(int32 index, int32 count)
{
	if (index < 0 || count < 0 || index + count > fSize)
		return false;

	if (count == 0)
		return true;

	for (int32 i = 0; i < count; i++)
		_Destroy(fElements + index + i);

	_Resize(index, -count);

	return true;
}


template<typename Element>
bool
BArray<Element>::RemoveItem(const Element& element)
{
	return RemoveItemAt(IndexOf(element), 1);
}


template<typename Element>
void
BArray<Element>::MakeEmpty()
{
	if (fSize == 0)
		return;

	for (int32 i = 0; i < fSize; i++)
		_Destroy(fElements + i);

	free(fElements);

	fElements = NULL;
	fSize = 0;
	fCapacity = 0;
}


template<typename Element>
int32
BArray<Element>::IndexOf(const Element& element)
{
	for (int32 i = 0; i < CountItems(); i++) {
		if (ItemAt(i) == element)
			return i;
	}
	return -1;
}


template<typename Element>
Element&
BArray<Element>::ItemAt(int32 index)
{
	return fElements[index];
}


template<typename Element>
const Element&
BArray<Element>::ItemAt(int32 index) const
{
	return fElements[index];
}


template<typename Element>
Element&
BArray<Element>::operator[](int32 index)
{
	return fElements[index];
}


template<typename Element>
const Element&
BArray<Element>::operator[](int32 index) const
{
	return fElements[index];
}


template<typename Element>
BArray<Element>&
BArray<Element>::operator=(const BArray<Element>& other)
{
	MakeEmpty();

	if (other.fSize > 0 && _Resize(0, other.fSize)) {
		for (int32 i = 0; i < fSize; i++)
			_Initialize(fElements + i, other.ItemAt(i));
	}

	return *this;
}


template<typename Element>
void
BArray<Element>::_Initialize(Element* target, const Element& source)
{
	new(target) Element(source);
}


template<typename Element>
void
BArray<Element>::_Destroy(Element* element)
{
	element->~Element();
}


template<typename Element>
void
BArray<Element>::_MoveLeft(Element* from, Element* to, int32 count)
{
	while (count-- > 0) {
		new(to) Element(*from);
		from->~Element();
		to++;
		from++;
	}
}


template<typename Element>
void
BArray<Element>::_MoveRight(Element* from, Element* to, int32 count)
{
	to += count - 1;
	from += count -1;
	while (count-- > 0) {
		new(to) Element(*from);
		from->~Element();
		to--;
		from--;
	}
}


template<typename Element>
void
BArray<Element>::_Move(Element* from, Element* to, int32 count)
{
	while (count-- > 0) {
		new(to) Element(*from);
		from->~Element();
		to++;
		from++;
	}
}


template<typename Element>
bool
BArray<Element>::_Resize(int32 index, int32 delta)
{
	// determine new capacity
	int32 newSize = fSize + delta;
	int32 newCapacity = kMinCapacity;
	while (newCapacity < newSize)
		newCapacity *= 2;

	Element* target = fElements;
	if (newCapacity != fCapacity) {
		target = (Element*)malloc(newCapacity * sizeof(Element));
		if (target == NULL)
			return false;
		if (index > 0)
			_Move(fElements, target, index);
	}

	if (index < fSize) {
		if (delta > 0) {
			// leave a gap of delta elements
			_MoveRight(fElements + index, target + index + delta,
				fSize - index);
		} else {
			// drop -delta elements
			_MoveLeft(fElements + index - delta, target + index,
				fSize - index + delta);
		}
	}
	
	if (newCapacity != fCapacity) {
		free(fElements);
		fElements = target;
		fCapacity = newCapacity;
	}
	fSize = newSize;
	return true;
}


}	// namespace BPrivate


using BPrivate::BArray;


#endif	// _ARRAY_CONTAINER_H

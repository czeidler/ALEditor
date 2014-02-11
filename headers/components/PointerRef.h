/*
 * Copyright 2011, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef	POINTER_REF_H
#define	POINTER_REF_H


#include <Message.h>


namespace BALM {


template<typename Type>
class PointerRef {
public:
	PointerRef()
		:
		fObject(NULL)
	{
	}

	status_t ToMessage(BMessage* message) const
	{
		return message->AddPointer("p", fObject);
	}

	status_t FromMessage(const BMessage* message)
	{
		Type* pointer = NULL;
		status_t status = message->FindPointer("p", (void**)&pointer);
		if (status != B_OK)
			return status;
		fObject = pointer;
		return B_OK;
	}

	PointerRef(Type* object)
		:
		fObject(NULL)
	{
		SetTo(object);
	}

	PointerRef(const PointerRef<Type>& other)
		:
		fObject(NULL)
	{
		SetTo(other.fObject);
	}

	~PointerRef()
	{
		Unset();
	}

	void SetTo(Type* object)
	{
		fObject = object;
	}

	void Unset()
	{
		fObject = NULL;
	}

	Type* Get() const
	{
		return fObject;
	}

	Type& operator*() const
	{
		return *fObject;
	}

	Type* operator->() const
	{
		return fObject;
	}

	operator Type*() const
	{
		return fObject;
	}

	PointerRef& operator=(const PointerRef<Type>& other)
	{
		SetTo(other.fObject);
		return *this;
	}

	bool operator==(const PointerRef<Type>& other) const
	{
		return (fObject == other.fObject);
	}

	bool operator!=(const PointerRef<Type>& other) const
	{
		return (fObject != other.fObject);
	}

private:
	Type*	fObject;
};


}	// namespace BALM

using BALM::PointerRef;

#endif	// POINTER_REF_H


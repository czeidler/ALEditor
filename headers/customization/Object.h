/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	B_OBJECT_H
#define	B_OBJECT_H

#include <vector>

#include <PObject.h>
#include <WeakReferenceable.h>


class BObject;


enum {
	B_OBJECT_EVENT_CONNECTED = '_ECo',
	B_OBJECT_EVENT_DISCONNECTED = '_EDi'
};


//TODO hide these two structs?
struct connection_data {
	BReference<BObject>	target;
	PMethod*	method;
};


struct event_data {
	BString				event;
	PMethodInterface*	interface;
	std::vector<connection_data>	connections;
};


/*! When sending a event async strong an weak references could go away while the
BMessage is queueing. This struct holds all arguments refs during this time. */
struct async_refs : BReferenceable {
	std::vector<BReference<BObject> >	strongRefs;
	std::vector<BWeakReference<BObject> >	weakRefs;	
};


class BObject : public PObject, public BWeakReferenceable {
public:
								BObject();
	virtual						~BObject();

	virtual	void				MessageReceived(BMessage* message);
			//! A looper is nessecary to resceive async events.
			void				SetLooper(BLooper* looper);
	
 			int32				InterfaceIndex(const BString& interface);
			int32				MethodIndex(PMethod* method);

			status_t			AddEvent(const char* event,
									PMethodInterface* interface);
			int32				CountEvents();
	const	event_data*			EventAt(int32 i);
	virtual	status_t			ConnectEvent(const char* event,
									BObject* target, const char* method);
	virtual status_t			DisconnectEvent(const char* event,
									BObject* target);
	virtual	status_t			RunMethodAsync(PMethod*, PArgs &in,
									PArgs &out, async_refs* refs = NULL);
			status_t			FireEventAsync(const char* event, PArgs &in,
									PArgs &out, async_refs* refs = NULL);
			status_t			FireEventSync(const char* event, PArgs &in,
									PArgs &out);

			bool				ConnectedToEvent(BObject* source);
			bool				DisconnectedFromEvent(BObject* source);

private:
	class EventConnections;

			EventConnections*	_GetEventConnections();

			EventConnections*	fEventConnections;
};


#endif	// B_OBJECT_H

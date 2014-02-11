/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "Object.h"

#include <Autolock.h>
#include <Looper.h>


class BObject::EventConnections {
public:
	~EventConnections()
	{
		for (int32 i = 0; i < fConnectedToEvents.CountItems(); i++)
			delete fConnectedToEvents.ItemAt(i);
	}

	void Disconnect(BObject* source)
	{
		BAutolock _(fConnectionLock);

		for (unsigned int i = 0; i < fEvents.size(); i++) {
			for (unsigned int c = 0; c < fEvents[i].connections.size(); c++)
				fEvents[i].connections[c].target->DisconnectedFromEvent(source);
		}
	
		for (int32 i = 0; i < fConnectedToEvents.CountItems(); i++)
			fConnectedToEvents.ItemAt(i)->DisconnectEvent(NULL, source);
	}

	bool ConnectedToEvent(BObject* source)
	{
		BAutolock _(fConnectionLock);
		return fConnectedToEvents.AddItem(source);
	}

	bool DisconnectedFromEvent(BObject* source)
	{
		BAutolock _(fConnectionLock);
		return fConnectedToEvents.RemoveItem(source);
	}

	status_t AddEvent(const char* event, PMethodInterface* interface)
	{
		event_data* data = _FindEvent(event);
		if (data != NULL)
			return B_BAD_VALUE;

		event_data newData;
		fEvents.push_back(newData);
		data = &fEvents[fEvents.size() - 1];

		data->event = event;
		data->interface = interface;
		return B_OK;
	}

	status_t ConnectEvent(BObject* source, BObject* target,
		const char* event, const char* methodName)
	{
		BAutolock _(fConnectionLock);

		event_data* data = _FindEvent(event);
		if (data == NULL)
			return B_BAD_VALUE;
		PMethod* method = target->FindMethod(methodName);
		if (method == NULL)
			return B_BAD_VALUE;

		connection_data connection;
		connection.target = target;
		connection.method = method;
		
		data->connections.push_back(connection);
	
		if (target->ConnectedToEvent(source) == true)
			return B_OK;

		return B_BAD_VALUE;
	}

	status_t DisconnectEvent(const char* event, BObject* target)
	{
		BAutolock _(fConnectionLock);

		if (event == NULL) {
			for (unsigned int i = 0; i < fEvents.size(); i++)
				_DisconnectFromEvent(&fEvents[i], target);
			return B_OK;
		}
	
		event_data* data = _FindEvent(event);
		if (data == NULL)
			return B_BAD_VALUE;
		if (_DisconnectFromEvent(data, target) == true)
			return B_OK;

		return B_BAD_VALUE;
	}

	status_t FireEventAsync(const char* event, PArgs &in, PArgs &out,
		async_refs* refs)
	{
		BAutolock _(fConnectionLock);

		event_data* data = _FindEvent(event);
		if (data == NULL)
			return B_BAD_VALUE;
		for (unsigned int i = 0; i < data->connections.size(); i++) {
			connection_data& con = data->connections[i];
			con.target->RunMethodAsync(con.method, in, out, refs);
		}
		return B_OK;
	}

	status_t FireEventSync(const char* event, PArgs &in, PArgs &out)
	{
		BAutolock _(fConnectionLock);

		event_data* data = _FindEvent(event);
		if (data == NULL)
			return B_BAD_VALUE;
		for (unsigned int i = 0; i < data->connections.size(); i++) {
			connection_data& con = data->connections[i];
			con.method->Run(con.target, in, out);
		}
		return B_OK;
	}

	bool AsyncMethodSent(async_refs* refs)
	{
		BAutolock _(fConnectionLock);
		refs->AcquireReference();
		fAsyncArgumentRefs.AddItem(refs);
		return true;
	}

	void AsyncMethodFinished(async_refs* refs)
	{
		if (refs == NULL)
			return;
		BAutolock _(fConnectionLock);
		int32 count = refs->ReleaseReference();
		if (count == 1)
			fAsyncArgumentRefs.RemoveItem(refs);
	}

	int32
	CountEvents()
	{
		return fEvents.size();
	}

	const event_data*
	EventAt(int32 index)
	{
		if (index < 0 || index >= (int32)fEvents.size())
			return NULL;
		return &fEvents[index];
	}

private:
	bool _DisconnectFromEvent(event_data* data, BObject* target)
	{
		for (unsigned int i = 0; i < data->connections.size(); i++) {
			connection_data& con = data->connections[i];
			if (con.target.Get() == target) {
				data->connections.erase(data->connections.begin() + i);
				return true;
			}
		}
		return false;
	}

	event_data* _FindEvent(const char* event)
	{
		for (unsigned int i = 0; i < fEvents.size(); i++) {
			event_data& data = fEvents[i];
			if (data.event == event)
				return &data;
		}
		return NULL;
	}

private:
			BLocker				fConnectionLock;
			std::vector<event_data>	fEvents;
			BObjectList<BObject>	fConnectedToEvents;
			BObjectList<async_refs>	fAsyncArgumentRefs;
};


const int32 kMsgRunAsyncEvent = '_REv';


BObject::BObject()
	:
	fEventConnections(NULL)
{
}


BObject::~BObject()
{
	if (fEventConnections != NULL)
		fEventConnections->Disconnect(this);
	delete fEventConnections;
}


void
BObject::MessageReceived(BMessage* message)
{
	switch (message->what) {
	case kMsgRunAsyncEvent:
	{
		PArgs inArgs, outArgs;
		PMethod* method = NULL;
		BMessage in, out;
		async_refs* refs = NULL;
		message->FindPointer("refs", (void**)&refs);
		message->FindPointer("method", (void**)&method);
		message->FindMessage("in", &in);

		inArgs.SetBackend(in);
		method->Run(this, inArgs, outArgs);

		// release references
		if (fEventConnections != NULL)
			fEventConnections->AsyncMethodFinished(refs);
		break;
	}

	default:
		PObject::MessageReceived(message);	
	}	
}


void
BObject::SetLooper(BLooper* looper)
{
	BLooper* oldLooper = Looper();
	if (oldLooper == looper)
		return;
	if (oldLooper != NULL)
		oldLooper->RemoveHandler(this);
	looper->AddHandler(this);
}


int32
BObject::InterfaceIndex(const BString& interface)
{
	for (int32 i = 0; i < CountInterfaces(); i++) {
		if (InterfaceAt(i) == interface)
			return i;
	}
	return -1;
}


int32
BObject::MethodIndex(PMethod* method)
{
	for (int32 i = 0; i < CountMethods(); i++) {
		if (MethodAt(i) == method)
			return i;
	}
	return -1;
}


status_t
BObject::AddEvent(const char* event, PMethodInterface* interface)
{
	EventConnections* eventConnections = _GetEventConnections();
	if (eventConnections == NULL)
		return B_NO_MEMORY;
	return eventConnections->AddEvent(event, interface);
}


int32
BObject::CountEvents()
{
	if (fEventConnections == NULL)
		return 0;
	return fEventConnections->CountEvents();
}


const event_data*
BObject::EventAt(int32 i)
{
	if (fEventConnections == NULL)
		return NULL;
	return fEventConnections->EventAt(i);
}


status_t
BObject::ConnectEvent(const char* event, BObject* target,
	const char* methodName)
{
	if (fEventConnections == NULL)
		return B_ERROR;
	return fEventConnections->ConnectEvent(this, target, event,	methodName);
}


status_t
BObject::DisconnectEvent(const char* event, BObject* target)
{
	if (fEventConnections == NULL)
		return B_ERROR;

	return fEventConnections->DisconnectEvent(event, target);
}
							

status_t
BObject::RunMethodAsync(PMethod* method, PArgs &in, PArgs &out,
	async_refs* refs)
{
	BLooper* looper = Looper();
	if (looper == NULL)
		return B_ERROR;
	if (refs != NULL) {
		if (fEventConnections == NULL);
			_GetEventConnections();
		if (fEventConnections == NULL)
			return B_NO_MEMORY;
	}

	BMessage message(kMsgRunAsyncEvent);
	message.AddPointer("refs", refs);
	message.AddPointer("method", method);
	BMessage inMessage = in.GetBackend();
	message.AddMessage("in", &inMessage);
	status_t status = looper->PostMessage(&message, this);
	if (status != B_OK)
		return status;

	if (refs != NULL)
		fEventConnections->AsyncMethodSent(refs);
	return B_OK;
}


status_t
BObject::FireEventAsync(const char* event, PArgs &in, PArgs &out,
	async_refs* refs)
{
	if (fEventConnections == NULL)
		return B_ERROR;
	return fEventConnections->FireEventAsync(event, in, out, refs);
}


status_t
BObject::FireEventSync(const char* event, PArgs &in, PArgs &out)
{
	if (fEventConnections == NULL)
		return B_ERROR;
	return fEventConnections->FireEventSync(event, in,out);
}


bool
BObject::ConnectedToEvent(BObject* source)
{
	EventConnections* eventConnections = _GetEventConnections();
	if (eventConnections == NULL)
		return false;
	return eventConnections->ConnectedToEvent(source);
}


bool
BObject::DisconnectedFromEvent(BObject* source)
{
	if (fEventConnections == NULL)
		return false;
	return fEventConnections->DisconnectedFromEvent(source);
}


BObject::EventConnections*
BObject::_GetEventConnections()
{
	if (fEventConnections != NULL)
		return fEventConnections;

	EventConnections* eventConnections = new EventConnections;
	if (atomic_test_and_set(reinterpret_cast<vint32*>(&fEventConnections),
		reinterpret_cast<vint32>(eventConnections), 0) != 0)
		delete eventConnections;
	return fEventConnections;
}

/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "Customizable.h"

#include <CustomizableRoster.h>

#include <typeinfo>

// Copied from Archiveable.cpp
#include <string>
#include <stdlib.h>


Customizable::Socket::Socket(Customizable* parent, const char* name,
	const char* interface, int32 min, int32 max)
	:
	fParent(parent),
	fLocked(false),
	fName(name),
	fInterface(interface),
	fMinConnections(min),
	fMaxConnections(max)
{
}


Customizable*
Customizable::Socket::Parent() const
{
	return fParent;
}


const char*
Customizable::Socket::Name() const
{
	return fName;
}


const char*
Customizable::Socket::Interface() const
{
	return fInterface;
}


int32
Customizable::Socket::CountConnections() const
{
	return fConnections.CountItems();
}


Customizable*
Customizable::Socket::ConnectionAt(int32 i) const
{
	return fConnections.ItemAt(i);
}


int32
Customizable::Socket::MinConnections() const
{
	return fMinConnections;
}


int32
Customizable::Socket::MaxConnections() const
{
	return fMaxConnections;
}


bool
Customizable::Socket::HasEmptySlot() const
{
	if (fMaxConnections < 0 || CountConnections() < fMaxConnections)
		return true;
	return false;
}


status_t
Customizable::Socket::_Connect(Customizable* interface)
{
	if (fLocked == true)
		return B_BAD_INDEX;
	if (interface->UsesInterface(fInterface) == false)
		return B_ERROR;
	if (fMaxConnections >= 0 && fConnections.CountItems() >= fMaxConnections)
		return B_BAD_INDEX;
	if (fConnections.AddItem(interface) == false)
		return B_NO_MEMORY;

	return B_OK;
}


status_t
Customizable::Socket::_Disconnect(Customizable* interface)
{
	if (fLocked == true)
		return B_BAD_INDEX;
	if (fConnections.RemoveItem(interface) == false)
		return B_ERROR;

	return B_OK;
}


Customizable::Customizable(CustomizableRoster* roster, const char* name)
	:
	fRoster(roster),
	fObjectName(name)
{
	if (fRoster == NULL)
		fRoster = CustomizableRoster::DefaultRoster();
	fRoster->Register(this);
}


Customizable::~Customizable()
{
	fRoster->Unregister(this);
}


void
Customizable::Stop()
{
}


void
Customizable::Resume()
{
}


status_t
Customizable::SaveState(BMessage* archive) const
{
	return B_OK;
}


status_t
Customizable::RestoreState(const BMessage* archive)
{
	return B_OK;
}


void
Customizable::SetObjectName(const char* name)
{
	fObjectName = name;
}

	
BString
Customizable::ObjectName() const
{
	BString out;
	if (fObjectName != "")
		out = fObjectName;
	else {
		BString temp = typeid(*this).name();
		demangle_class_name(temp, out);
	}
	// add some interface info
	/*
	for (int32 i = 0; i < CountInterfaces(); i++) {
		if (i == 0)
			out += " (";
		else
			out += ", ";


		out += InterfaceAt(i);
	}
	if (CountInterfaces() > 0)
		out += ")";
	*/
	return out;
}


status_t
Customizable::Connect(const char* socket, Customizable* interface)
{
	Socket* info = FindSocket(socket);
	if (info == NULL)
		return B_ERROR;
	return Connect(info, interface);
}


status_t
Customizable::Connect(Socket* socket, Customizable* interface)
{
	status_t status = socket->_Connect(interface);
	if (status != B_OK)
		return status;
	interface->Connected(socket);
	return B_OK;
}


status_t
Customizable::Disconnect(const char* socket, Customizable* interface)
{
	Socket* info = FindSocket(socket);
	if (info == NULL)
		return B_ERROR;
	return Disconnect(info, interface);
}


status_t
Customizable::Disconnect(Socket* socket, Customizable* interface)
{
	status_t status = socket->_Disconnect(interface);
	if (status != B_OK)
		return status;
	interface->Disconnected(socket);
	return B_OK;
}
									

status_t
Customizable::StartCustomization()
{
	return B_OK;
}


status_t
Customizable::EndCustomization()
{
	return B_OK;
}


bool
Customizable::Exchangeable() const
{
	return fExchangeable;
}


status_t
Customizable::CopyState(Customizable* copyTo) const
{
	if (fExchangeable == false)
		return B_ERROR;
	return B_OK;	
}


int32
Customizable::CountSockets() const
{
	return fSockets.size();
}


Customizable::Socket*
Customizable::SocketAt(int32 i) const
{
	if (i < 0 || (unsigned int)i >= fSockets.size())
		return NULL;
	std::vector<Socket>& sockets = const_cast<std::vector<Socket>&>(fSockets);
	return &sockets[i];
}


Customizable::Socket*
Customizable::FindSocket(const char* name)
{
	for (unsigned int i = 0; i < fSockets.size(); i++) { 
		Socket* info = &fSockets[i];
		if (BString(info->Name()) == name)
			return info;
	}
	return NULL;
}


bool
Customizable::Orphan() const
{
	if (CountOwnConnections() > 0)
		return false;
	for (int32 i = 0; i < CountSockets(); i++) {
		if (SocketAt(i)->CountConnections() > 0)
			return false;
	}
	return true;
}


int32
Customizable::CountOwnConnections() const
{
	return fConnectedToList.CountItems();
}


Customizable::Socket*
Customizable::OwnConnectionAt(int32 i) const
{
	return fConnectedToList.ItemAt(i);
}


status_t
Customizable::ConnectEvent(const char* event, BObject* target,
	const char* method)
{
	status_t status = BObject::ConnectEvent(event, target, method);
	if (status != B_OK)
		return status;
	if (fRoster)
		fRoster->_NotifyWatchers(B_OBJECT_EVENT_CONNECTED);
	return status;
}


status_t
Customizable::DisconnectEvent(const char* event, BObject* target)
{
	status_t status = BObject::DisconnectEvent(event, target);
	if (status != B_OK)
		return status;
	if (fRoster)
		fRoster->_NotifyWatchers(B_OBJECT_EVENT_DISCONNECTED);
	return status;
}


void
Customizable::SetExchangeable(bool exchangeable)
{
	fExchangeable = exchangeable;
}


bool
Customizable::AddSocket(const char* socket, const char* interface,
	int32 minConnections, int32 maxConnections)
{
	Socket info(this, socket, interface, minConnections, maxConnections);
	fSockets.push_back(info);
	return true;
}


bool
Customizable::Connected(Socket* socket)
{
	if (fRoster)
		fRoster->_NotifyWatchers(B_SOCKET_CONNECTED);

	return fConnectedToList.AddItem(socket);
}


bool
Customizable::Disconnected(Socket* socket)
{
	if (fRoster)
		fRoster->_NotifyWatchers(B_SOCKET_DISCONNECTED);
	return fConnectedToList.RemoveItem(socket);
}

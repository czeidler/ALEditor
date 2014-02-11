/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	CUSTOMIZABLE_H
#define	CUSTOMIZABLE_H


#include <vector>

#include <Handler.h>
#include <String.h>

#include <Object.h>
#include <WeakReferenceable.h>


class EventConnections;


namespace BALM {


class Customizable;
class CustomizableRoster;

typedef BObjectList<Customizable> CustomizableList;


class Customizable : public BObject {
public:
	class Socket {
	public:
								Socket(Customizable* parent, const char* name,
									const char* interface, int32 min,
									int32 max);

			Customizable*		Parent() const;

			const char*			Name() const;
			const char*			Interface() const;

			int32				CountConnections() const;
			Customizable*		ConnectionAt(int32 i) const;

			int32				MinConnections() const;
			int32				MaxConnections() const;

			bool				HasEmptySlot() const;

	private:
		friend class Customizable;

			status_t			_Connect(Customizable* interface);
			status_t			_Disconnect(Customizable* interface);

			Customizable*		fParent;
			// connections can't be edit
			bool				fLocked;
			BString				fName;
			BString				fInterface;
			int32				fMinConnections;
			int32				fMaxConnections;
			CustomizableList	fConnections;			
	};

	typedef BObjectList<Socket> SocketList;

								Customizable(CustomizableRoster* roster = NULL,
									const char* name = NULL);
	virtual						~Customizable();

			CustomizableRoster*	Roster() { return fRoster; }

	virtual void				Stop();
	virtual void				Resume();

	virtual status_t			SaveState(BMessage* archive) const;
	virtual status_t			RestoreState(const BMessage* archive);

			void				SetObjectName(const char* name);
			BString				ObjectName() const;

 			status_t			Connect(const char* socket,
									Customizable* interface);
	virtual status_t			Connect(Socket* socket,
									Customizable* interface);
			status_t			Disconnect(const char* socket,
									Customizable* interface);
	virtual status_t			Disconnect(Socket* socket,
									Customizable* interface);
			/*! Check if all mandatory sockets are connected. */ 
			bool				Connected();

	virtual status_t			StartCustomization();
	virtual status_t			EndCustomization();

			/*! Indicate that the state can be propagated to another
			Customizable in CopyState.*/
			bool				Exchangeable() const;
			/*! Clone the state to another Customizable. To do so only the
			interface methods are should be used. */
	virtual	status_t			CopyState(Customizable* copyTo) const;

			/*! This or at least a similar object must stay. */
			bool				Removable();
			void				SetRemovable(bool removable = true);

			int32				CountSockets() const;
			Socket*				SocketAt(int32 i) const;
			Socket*				FindSocket(const char* name);

			bool				Orphan() const;

			int32				CountOwnConnections() const;
			Socket*				OwnConnectionAt(int32 i) const;

			// overwrite BObject methods to send notifications to the roster
	virtual	status_t			ConnectEvent(const char* event,
									BObject* target, const char* method);
	virtual status_t			DisconnectEvent(const char* event,
									BObject* target);
protected:
			void				SetExchangeable(bool exchangeable = true);
	virtual	bool				AddSocket(const char* socket, const char* interface,  
									int32 minConnections = 0,
									int32 maxConnections = 1);

			bool				Connected(Socket* socket);
			bool				Disconnected(Socket* socket);

private:
			CustomizableRoster* fRoster;
			BString				fObjectName;
			bool				fExchangeable;
			bool				fRemovable;
			std::vector<Socket>	fSockets;

			SocketList			fConnectedToList;
};


}	// namespace BALM


using BALM::Customizable;
using BALM::CustomizableList;


#endif	// CUSTOMIZABLE_H

/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "CustomizableRoster.h"

#include <AutoLocker.h>
#include <Messenger.h>


using BALM::Layer;


const char* B_CLASS_FIELD = "class";


static CustomizableRoster* gCustomizableRoster = NULL;


bool
BALM::CustomizableAddOn::IconPicture(BView* view, BPicture** picture,
	BRect& frame)
{
	return false;
}


Layer::Layer()
{
}


bool
Layer::Register(Customizable* customizable)
{
	bool result = fCustomizables.AddItem(customizable);
	if (result == false)
		return false;

	if (fCustomizableRefs.AddItem(customizable) == false) {
		fCustomizables.RemoveItem(customizable);
		return false;	
	}
	return true;
}


bool
Layer::Unregister(Customizable* customizable)
{
	int32 index = fCustomizables.IndexOf(customizable);
	if (index < 0)
		return false;
	fCustomizables.RemoveItemAt(index);

	return fCustomizableRefs.RemoveItemAt(index);
}


void
Layer::GetCustomizableList(BArray<BWeakReference<Customizable> >& list)
{
	list = fCustomizableRefs;
}
									

bool
Layer::AddToShelf(Customizable* customizable)
{
	return fShelf.AddItem(customizable);
}


bool
Layer::RemoveFromShelf(Customizable* customizable)
{
	return fShelf.RemoveItem(customizable);
}


void
Layer::GetShelfList(BArray<BReference<Customizable> >& list)
{
	list = fShelf;
}


CustomizableRoster::CustomizableRoster()
{
	fLayerList.AddItem(new Layer);
}


CustomizableRoster::~CustomizableRoster()
{
	for (int32 i = 0; i < fInstalledCustomizables.CountItems(); i++)
		delete fInstalledCustomizables.ItemAt(i);

	for (int32 i = 0; i < fLayerList.CountItems(); i++)
		delete fLayerList.ItemAt(i);
}


CustomizableRoster*
CustomizableRoster::DefaultRoster()
{
	if (gCustomizableRoster == NULL)
		gCustomizableRoster = new CustomizableRoster();
	return gCustomizableRoster;
}


BReference<Customizable>
CustomizableRoster::InstantiateCustomizable(const char* name,
	const BMessage* archive)
{
	BReference<Customizable> customizable;

	AutoLocker<BLocker> _(fListLocker);
	for (int32 i = 0; i < fInstalledCustomizables.CountItems(); i++) {
		CustomizableAddOn* addOn = fInstalledCustomizables.ItemAt(i);
		if (BString(addOn->Name()) == name) {
			return addOn->InstantiateCustomizable(archive);
		}
	}

	return NULL;
}


bool
CustomizableRoster::InstallCustomizable(CustomizableAddOn* addOn)
{
	AutoLocker<BLocker> _(fListLocker);
	fInstalledCustomizables.AddItem(addOn);
	return true;
}


void
CustomizableRoster::GetInstalledCustomizableList(CustomizableAddOnList& list)
{
	AutoLocker<BLocker> _(fListLocker);
	list.MakeEmpty();
	list.AddList(&fInstalledCustomizables);
}


void
CustomizableRoster::GetCustomizableList(
	BArray<BWeakReference<Customizable> >& list)
{
	AutoLocker<BLocker> _(fListLocker);
	Layer* layer = _Layer(NULL);
	if (layer != NULL)
		layer->GetCustomizableList(list);
}


bool
CustomizableRoster::StartWatching(BHandler* target)
{
	AutoLocker<BLocker> _(fListLocker);
	if (fWatcherList.HasItem(target) == true)
		return true;
	return fWatcherList.AddItem(target);
}


bool
CustomizableRoster::StopWatching(BHandler* target)
{
	AutoLocker<BLocker> _(fListLocker);
	return fWatcherList.RemoveItem(target);
}


void
CustomizableRoster::GetCompatibleConnections(Customizable::Socket* socket,
	CustomizableList& list)
{
	BArray<BWeakReference<Customizable> > allCustomizable;
	GetCustomizableList(allCustomizable);
	for (int32 i = 0; i < allCustomizable.CountItems(); i++) {
		BReference<Customizable> customizable
			= allCustomizable.ItemAt(i).GetReference();
		if (customizable == NULL)
			continue;
		if (customizable->UsesInterface(socket->Interface()) == true)
			list.AddItem(customizable);
	}
}


bool
CustomizableRoster::AddToShelf(Customizable* customizable)
{
	AutoLocker<BLocker> _(fListLocker);

	Layer* layer = _Layer(customizable);
	if (layer != NULL)
		return layer->AddToShelf(customizable);

	return false;
}


bool
CustomizableRoster::RemoveFromShelf(Customizable* customizable)
{
	AutoLocker<BLocker> _(fListLocker);

	Layer* layer = _Layer(customizable);
	if (layer != NULL)
		return layer->RemoveFromShelf(customizable);
	return false;
}


void
CustomizableRoster::GetShelfList(BArray<BReference<Customizable> >& list)
{
	AutoLocker<BLocker> _(fListLocker);

	Layer* layer = _Layer(NULL);
	if (layer != NULL)
		layer->GetShelfList(list);
}


bool
CustomizableRoster::Register(Customizable* customizable)
{
	AutoLocker<BLocker> _(fListLocker);
	Layer* layer = _Layer(customizable);
	bool status = false;
	if (layer != NULL) {
		status = layer->Register(customizable);
		if (status) {
			BMessage message(B_CUSTOMIZABLE_ADDED);
			message.AddPointer("customizable", customizable);
			_NotifyWatchers(&message);
		}
	}
	return status;
}


bool
CustomizableRoster::Unregister(Customizable* customizable)
{
	AutoLocker<BLocker> _(fListLocker);
	Layer* layer = _Layer(customizable);
	bool status = false;
	if (layer != NULL) {
		status = layer->Unregister(customizable);
		if (status)
			_NotifyWatchers(B_CUSTOMIZABLE_REMOVED);
	}
	return status;
}


Layer*
CustomizableRoster::_Layer(Customizable* customizable)
{
	return fLayerList.ItemAt(0);
}


void
CustomizableRoster::_NotifyWatchers(BMessage* message)
{
	for (int32 i = 0; i < fWatcherList.CountItems(); i++) {
		BMessenger messenger(fWatcherList.ItemAt(i));
		messenger.SendMessage(message);
	}
}


void
CustomizableRoster::_NotifyWatchers(int32 what)
{
	for (int32 i = 0; i < fWatcherList.CountItems(); i++) {
		BMessenger messenger(fWatcherList.ItemAt(i));
		messenger.SendMessage(what);
	}
}

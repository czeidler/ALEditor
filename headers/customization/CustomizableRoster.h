/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	CUSTOMIZABLE_ROSTER_H
#define	CUSTOMIZABLE_ROSTER_H


#include <Customizable.h>

#include <Bitmap.h>
#include <Handler.h>
#include <Locker.h>
#include <ObjectList.h>
#include <Picture.h>
#include <View.h>

#include <Mangle.h>

#include <ArrayContainer.h>


enum {
	B_CUSTOMIZABLE_ADDED = '_Cad',
	B_CUSTOMIZABLE_REMOVED = '_Crm',

	B_SOCKET_CONNECTED = '_SCo',
	B_SOCKET_DISCONNECTED = '_SDi',
};


namespace BALM {


class CustomizableAddOn {
public:
	virtual						~CustomizableAddOn() {}

	virtual	BReference<Customizable>	InstantiateCustomizable(
											const BMessage* message) = 0;
	virtual BString				Name() = 0;
	virtual bool				IconPicture(BView* view, BPicture** picture,
									BRect& frame);
};


typedef BObjectList<CustomizableAddOn> CustomizableAddOnList;


class Layer {
public:
								Layer();

			bool				Register(Customizable* customizable);
			bool				Unregister(Customizable* customizable);

			void				GetCustomizableList(
									BArray<BWeakReference<Customizable> >& list);

			/*! Hold a strong reference to the Customizable. */
			bool				AddToShelf(Customizable* customizable);
			bool				RemoveFromShelf(Customizable* customizable);

			void				GetShelfList(
									BArray<BReference<Customizable> >& list);
private:
			CustomizableList	fCustomizables;
			BArray<BWeakReference<Customizable> >	fCustomizableRefs;

			BArray<BReference<Customizable> >	fShelf;
};


class CustomizableRoster {
public:
								CustomizableRoster();
								~CustomizableRoster();

	static	CustomizableRoster* DefaultRoster();

			BReference<Customizable>	InstantiateCustomizable(const char* name,
											const BMessage* archive = NULL);

			bool				InstallCustomizable(CustomizableAddOn* addOn);
			void				GetInstalledCustomizableList(
									CustomizableAddOnList& list);

			void				GetCustomizableList(
									BArray<BWeakReference<Customizable> >& list);

			bool				StartWatching(BHandler* target);
			bool				StopWatching(BHandler* target);

			void				GetCompatibleConnections(
									Customizable::Socket* socket,
									CustomizableList& list);

			bool				AddToShelf(Customizable* customizable);
			bool				RemoveFromShelf(Customizable* customizable);
			void				GetShelfList(
									BArray<BReference<Customizable> >& list);

protected:
	friend class Customizable;
			bool				Register(Customizable* customizable);
			bool				Unregister(Customizable* customizable);

private:
	friend class Customizable::Socket;
	friend class BALM::Layer;

			Layer*				_Layer(Customizable* customizable);

			void				_NotifyWatchers(BMessage* message);
			void				_NotifyWatchers(int32 what);

			CustomizableAddOnList	fInstalledCustomizables;

			BObjectList<BHandler>	fWatcherList;

			BObjectList<Layer>	fLayerList;

			BLocker				fListLocker;
};


template<class Type>
class CustomizableInstaller {
public:
	CustomizableInstaller()
	{
		CustomizableRoster* roster = CustomizableRoster::DefaultRoster();
		if (roster == NULL)
			return;
		
		roster->InstallCustomizable(new AddOn<Type>);
	}

	CustomizableInstaller(const unsigned char* bits, BRect pictureFrame)
	{
		CustomizableRoster* roster = CustomizableRoster::DefaultRoster();
		if (roster == NULL)
			return;
		
		roster->InstallCustomizable(new AddOn<Type>(bits, pictureFrame));
	}

private:
	template<class T>
	class AddOn : public CustomizableAddOn {
	public:
		AddOn()
			:
			fBitmap(NULL)
		{	
		}

		AddOn(const unsigned char* bits, BRect pictureFrame)
			:
			fBitmap(bits),
			fPictureFrame(pictureFrame)
		{
		}

		BReference<Customizable>
		InstantiateCustomizable(const BMessage* message)
		{
			Customizable* customizable
				= T::InstantiateCustomizable(message);
			return BReference<Customizable>(customizable, true);
		}

		BString Name()
		{
			BString out;
			demangle_class_name(typeid(T).name(), out);
			return out;
		}

		bool IconPicture(BView* view, BPicture** picture, BRect& frame)
		{
			if (fBitmap == NULL)
				return false;
			BBitmap bitmap(fPictureFrame, B_RGBA32);
			memcpy(bitmap.Bits(), (void*)fBitmap, bitmap.BitsLength());
			view->BeginPicture(new BPicture);
			view->DrawBitmap(&bitmap);
			*picture = view->EndPicture();
			if (*picture == NULL)
				return false;
			frame = fPictureFrame;
			return true;
		}

	private:
		const unsigned char*	fBitmap;
		BRect		fPictureFrame;
	};
};


}	// namespace BALM


using BALM::CustomizableInstaller;
using BALM::CustomizableRoster;


#endif	// CUSTOMIZABLE_ROSTER_H

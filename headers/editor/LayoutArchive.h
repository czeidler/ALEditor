/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	LAYOUT_ARCHIVE_H
#define	LAYOUT_ARCHIVE_H


#include <ALMLayout.h>


class BFile;
class BNode;


namespace BALM {


/*! Stores and load a complete layout including widgets. */
class LayoutArchive {
public:
								LayoutArchive(BALMLayout* layout);

			void				ClearLayout();

			//! Ignores the EditView
			status_t			SaveLayout(BMessage* archive,
									bool saveComponents) const;
			status_t			RestoreLayout(const BMessage* archive,
									bool restoreComponents);

			status_t			SaveToFile(BFile* file,
									const BMessage* message);
			status_t			RestoreFromFile(BFile* file,
									bool restoreComponents = true);

			status_t			SaveToAppFile(const char* attribute,
									const BMessage* message);
			status_t			RestoreFromAppFile(const char* attribute,
									bool restoreComponents = true);

			status_t			SaveToAttribute(BNode* node,
									const char* attribute,
									const BMessage* message);
			status_t			RestoreFromAttribute(BNode* node,
									const char* attribute,
									bool restoreComponents = true);

	template <class Type>
	Type* FindView(const char* identifier)
	{
		return dynamic_cast<Type*>(FindView(identifier));
	}

	template <class Type>
	Type* FindLayoutItem(const char* identifier)
	{
		return dynamic_cast<Type*>(FindLayoutItem(identifier));
	}

protected:
			BView*				FindView(const char* identifier);
			BLayoutItem*		FindLayoutItem(const char* identifier);

private:
			bool				_RestoreArea(Area* area, int32 i,
									const BMessage* archive, XTabList& xTabs,
									YTabList& yTabs);
			bool				_SaveComponent(Area* area,
									BMessage* archive) const;
			Area*				_CreateComponent(const BMessage* archive);

			BALMLayout*			fLayout;
};
	
	
} 	// namespace BALM


using BALM::LayoutArchive;


#endif // LAYOUT_ARCHIVE_H

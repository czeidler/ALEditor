/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	INFO_SYSTEM_H
#define	INFO_SYSTEM_H


#include <View.h>


class Informant {
public:
	virtual						~Informant() {}

	virtual void				Info(const char* text) = 0;
	virtual void				Error(const char* text) = 0;

	virtual	void				Clear() = 0;
};


class ToolTipInformant : public Informant {
public:
								ToolTipInformant(BView* view);

	virtual void				Info(const char* text);
	virtual void				Error(const char* text);
	virtual	void				Clear();

private:
			BView*				fView;
};


#endif	// INFO_SYSTEM_H

/*
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#include "InfoSystem.h"


#include <ToolTip.h>


ToolTipInformant::ToolTipInformant(BView* view)
	:
	fView(view)
{
}


void
ToolTipInformant::Info(const char* text)
{
	fView->SetToolTip(text);
	fView->ShowToolTip(fView->ToolTip());
}


void
ToolTipInformant::Error(const char* text)
{
	Info(text);
}


void
ToolTipInformant::Clear()
{
	fView->SetToolTip((BToolTip*)NULL);
	fView->HideToolTip();
}


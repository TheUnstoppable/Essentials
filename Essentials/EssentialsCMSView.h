/*
	Essentials - Essential features for servers
	Copyright (C) 2022 Unstoppable

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
*/

#pragma once

#include "EssentialsEventClass.h"
#include "EssentialsCMSDefinition.h"

class EssentialsCMSDialogView;
class EssentialsCMSChatView;

class ESSENTIALS_API EssentialsCMSView {
	friend class EssentialsCMSDefinition;
	friend class EssentialsCMSDialogView;
	friend class EssentialsCMSChatView;

public:
	virtual EssentialsCMSDialogView* As_DialogContent() { return NULL; }
	virtual EssentialsCMSChatView* As_ChatContent() { return NULL; }

	virtual bool Perform(int playerId) = 0;

	const EssentialsCMSDefinition* Get_Definition() { return Definition; }

protected:
	EssentialsCMSDefinition* Definition;

private:
	EssentialsCMSView() : Definition(0) {}
	EssentialsCMSView(EssentialsCMSDefinition* Definition) : Definition(Definition) {}
};

class ESSENTIALS_API EssentialsCMSDialogView : public EssentialsCMSView {
	friend class EssentialsCMSDialogDefinition;
public:
	virtual EssentialsCMSDialogView* As_DialogContent() { return this; }

	virtual bool Perform(int playerId);

protected:
	EssentialsCMSDialogView() : EssentialsCMSView(0) {}
	EssentialsCMSDialogView(EssentialsCMSDefinition* Definition) : EssentialsCMSView(Definition) {}
};

class ESSENTIALS_API EssentialsCMSChatView : public EssentialsCMSView {
	friend class EssentialsCMSChatDefinition;
public:
	virtual EssentialsCMSChatView* As_ChatContent() { return this; }

	virtual bool Perform(int playerId);

protected:
	EssentialsCMSChatView() : EssentialsCMSView(0) {}
	EssentialsCMSChatView(EssentialsCMSDefinition* Definition) : EssentialsCMSView(Definition) {}
};
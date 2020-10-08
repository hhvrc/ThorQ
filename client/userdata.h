#ifndef USERDATA_H
#define USERDATA_H

#include <QString>

struct UserData
{
	QString name;
	bool isInSession;
	bool isInSteamVR;
	bool hasCollarConnected;
	bool hasRequestedControl;
};

#endif // USERDATA_H

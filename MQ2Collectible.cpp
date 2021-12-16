// MQ2Collectible.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup.

#include <mq/Plugin.h>

PreSetup("MQ2Collectible");
PLUGIN_VERSION(0.1);

void ShowHelp()
{
			WriteChatf("\ao[MQ2Collectible] \arUsage: \ag/collectible\ag collected|need|both|help log|bazaar expansion|collection name\aw");
}

void CollectibleCMD(SPAWNINFO* pChar, char* szLine)
{
	bool bCollected 		= false;
	bool bNeed      		= false;
	bool bLog				= false;
	bool bBazaar 			= false;
	bool bExpansion			= false;
	bool bCollection		= false;

	char* szName;

	int  id 				= 0;

	PCHARINFO pCharInfo = GetCharInfo();
	PcProfile* pCharInfo2 = GetPcProfile();

	// Begin Parameters

	char szArg[MAX_STRING] = { 0 };
	GetArg(szArg, szLine, 1);

	if (strlen(szLine)==0 || !_stricmp(szArg, "help")) {
		ShowHelp();
		return;
	}

	if (!_stricmp(szArg, "collected")) {
		bCollected = true;
	} else if (!_stricmp(szArg, "need")) {
		bNeed = true;
	} else if (!_stricmp(szArg, "both")) {
		bCollected = true;
		bNeed = true;
	} else {
		ShowHelp();
		return;
	}

	WriteChatf("Type: %s",szArg);

	GetArg(szArg, szLine, 2);

	if (!_stricmp(szArg, "log") || !_stricmp(szArg, "l")) {
		bLog = true;
		WriteChatf("log");
	} else if (!_stricmp(szArg, "bazaar") || !_stricmp(szArg, "baz") || !_stricmp(szArg, "b")) {
		bBazaar = true;
		WriteChatf("bazaar");
	} else {
		ShowHelp();
		return;
	}

	GetArg(szArg, szLine, 3);

	if (strlen(szArg)) {
		if (!_stricmp(szArg, "expansion") || !_stricmp(szArg, "exp") || !_stricmp(szArg, "e")) {
			bExpansion = true;
			WriteChatf("Expansion");
		} else if (!_stricmp(szArg, "collections") || !_stricmp(szArg, "collection") || !_stricmp(szArg, "coll") || !_stricmp(szArg, "c")) {
			bCollection = true;
			WriteChatf("Collection");
		} else {
			ShowHelp();
			return;
		}
	}

	if (bExpansion || bCollection) {
		szName = GetNextArg(szLine);

		if (strlen(szName)==0) {
			ShowHelp();
			return;
		}
		if (IsNumber(szName)) {
			id = atoi(szName);
			if (id<1) {
				ShowHelp();
				return;
			}
		}
		WriteChatf("Name: %s", szName);
	}

	// End Parameters

}

void CollectibleTLO(SPAWNINFO* pChar, char* szLine)
{
		
	PCHARINFO pCharInfo = GetCharInfo();
	PcProfile* pCharInfo2 = GetPcProfile();

}

PLUGIN_API void InitializePlugin()
{
	DebugSpewAlways("MQ2Collectible::Initializing version %f", MQ2Version);

	const std::string szAlias = GetPrivateProfileString("Aliases", "/collectible", "None", gPathMQini);
	if (szAlias != "None") {
		WriteChatf("\awMQ2Collectible: \arWarning! The alias /collectible already exists. Please delete it by entering \"\ay/alias /collectible delete\ar\" then try again.");
		EzCommand("/timed 10 /plugin MQ2Collectible unload");
	}

	AddCommand("/collectible", CollectibleCMD);
	//AddMQ2Data("collectible", CollectibleTLO);
}

PLUGIN_API void ShutdownPlugin()
{
	DebugSpewAlways("MQ2Collectible::Shutting down");

	RemoveCommand("/collectible");
	RemoveMQ2Data("collectible");
}


/**
 * @fn SetGameState
 *
 * This is called when the GameState changes.  It is also called once after the
 * plugin is initialized.
 *
 * For a list of known GameState values, see the constants that begin with
 * GAMESTATE_.  The most commonly used of these is GAMESTATE_INGAME.
 *
 * When zoning, this is called once after @ref OnBeginZone @ref OnRemoveSpawn
 * and @ref OnRemoveGroundItem are all done and then called once again after
 * @ref OnEndZone and @ref OnAddSpawn are done but prior to @ref OnAddGroundItem
 * and @ref OnZoned
 *
 * @param GameState int - The value of GameState at the time of the call
 */
PLUGIN_API void SetGameState(int GameState)
{
	// DebugSpewAlways("MQ2Collectible::SetGameState(%d)", GameState);
}


/**
 * @fn OnPulse
 *
 * This is called each time MQ2 goes through its heartbeat (pulse) function.
 *
 * Because this happens very frequently, it is recommended to have a timer or
 * counter at the start of this call to limit the amount of times the code in
 * this section is executed.
 */
PLUGIN_API void OnPulse()
{
/*
	static std::chrono::steady_clock::time_point PulseTimer = std::chrono::steady_clock::now();
	// Run only after timer is up
	if (std::chrono::steady_clock::now() > PulseTimer)
	{
		// Wait 5 seconds before running again
		PulseTimer = std::chrono::steady_clock::now() + std::chrono::seconds(5);
		DebugSpewAlways("MQ2Collectible::OnPulse()");
	}
*/
}

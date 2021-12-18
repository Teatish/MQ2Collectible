// MQ2Collectible.cpp
//
// by American Nero, December 16, 2021
//
// Appreciation to Kaen01 for his suggestion to add logfile output in Bazaar.mac format,
// and Knightly, Chatwiththisname, and Brainiac for stylistic and technical guidance.
//
// Provides command /collectible that produces logfiles and console output
// Provides TLO ${Collectible} that retrieves a collectibles status (true = collected, false = need)
//
// Repo with detailed usage at https://gitlab.com/redguides/plugins/mq2collectible/README.MD
//
// Please update README.MD prior to making substantial code changes so that the requirements are implicitly
// understood and usage is adequately described.

#include <mq/Plugin.h>

PreSetup("MQ2Collectible");
PLUGIN_VERSION(0.1);

// This plugin is dependent on MQ2Log, for now.

void CollectibleCMD(SPAWNINFO* pChar, char* szLine);
void LookupCollection(char* szCharName, char* szCollExpName, bool bCollected, bool bNeed, bool bLog, bool bBazaar, bool bConsole, bool bCollection, bool bExpansion);
void ShowCMDHelp();
void CheckLogDirectory();
char* CreateLogFileName(char* szCharName, bool bCollected, bool bNeed, bool bLog, bool bBazaar);
bool LogOutput(char* szLogFileName, char* szLogThis);

const char* COLLECTED = "[COLLECTED]";
const char* NEED      = "[NEED]";

// ----------------------------------------------
// Command section
// ----------------------------------------------

void CollectibleCMD(SPAWNINFO* pChar, char* szLine)
{

	bool bCollected 		= false;
	bool bNeed      		= false;
	bool bLog				= false;
	bool bBazaar 			= false;
	bool bConsole 			= false;
	bool bExpansion			= false;
	bool bCollection		= false;
	char* szCharName 		= pChar->Name; 
	char szCollExpName[256] = { 0 };
	char szArg[16] 			= { 0 };

	// Parse Parameters
	GetArg(szArg, szLine, 1);

	if (strlen(szLine)==0 || !_stricmp(szArg, "help") || !_stricmp(szArg, "-h")) {
		ShowCMDHelp();
		return;
	}

	if (!_stricmp(szArg, "collected") || !_stricmp(szArg, "-cd")) {
		bCollected = true;
	} else if (!_stricmp(szArg, "need") || !_stricmp(szArg, "-n")) {
		bNeed = true;
	} else if (!_stricmp(szArg, "both") || !_stricmp(szArg, "-b")) {
		bCollected = true;
		bNeed = true;
	} else {
		ShowCMDHelp();
		return;
	}

	GetArg(szArg, szLine, 2);

	if (!_stricmp(szArg, "log") || !_stricmp(szArg, "-l")) {
		bLog = true;
	} else if (!_stricmp(szArg, "bazaar") || !_stricmp(szArg, "-bz")) {
		bBazaar = true;
	} else if (!_stricmp(szArg, "console") || !_stricmp(szArg, "-cs")) {
		bConsole = true;
	} else {
		ShowCMDHelp();
		return;
	}

	GetArg(szArg, szLine, 3);

	if (strlen(szArg)) {
		if (!_stricmp(szArg, "expansion") || !_stricmp(szArg, "-e")) {
			bExpansion = true;
		} else if (!_stricmp(szArg, "collection") || !_stricmp(szArg, "-cn")) {
			bCollection = true;
		} else {
			ShowCMDHelp();
			return;
		}
	}

	if (bExpansion || bCollection) {
		GetArg(szCollExpName, szLine, 4);
		if (strlen(szCollExpName)==0) {
			ShowCMDHelp();
			return;
		}
		if (IsNumber(szCollExpName)) {
			ShowCMDHelp();
			return;
		}
	}

	// Look up by Collection or Expansion if specified, or all.
	LookupCollection(szCharName, szCollExpName, bCollected, bNeed, bLog, bBazaar, bConsole, bCollection, bExpansion);
}

void LookupCollection(char* szCharName, char* szCollExpName, bool bCollected, bool bNeed, bool bLog, bool bBazaar, bool bConsole, bool bCollection, bool bExpansion)
{

	// Find category "Collections", then search through the subcategories
	// until we find the needed Collection, then iterate through the
	// collectibles.

	AchievementManager& AchMgr = AchievementManager::Instance();

	int   AchID        = 0;
	int   AchParentID  = 0;
	int   AchIdx       = 0;
	int   AchCt        = 0;
	int   CompTypeCt   = 0;
	int   x            = 0;
	int   y            = 0;

	bool  bFoundCollection = false;
	bool  bFoundExpansion  = false;
	bool  bLogStarted      = false;
	bool  bCollectedStatus = false;

	char* szLogFile;
	char  szCharBuffer[MAX_STRING] = { 0 };

	for (const AchievementCategory& AchCat : AchMgr.categories) {

		if (!string_equals(AchCat.name, "Collections")) continue;

		// Secrets of Faydwer says Collections, but there aren't any. As of Terror of Luclin, Events and 10 expansions actually have real collections.
		AchCt = AchCat.GetAchievementCount();
		if (!AchCt) continue;

		// Obtain the Expansion name.
		AchParentID = AchCat.parentId;
		const AchievementCategory& AchParent = *AchMgr.GetAchievementCategoryById(AchParentID);

		if (bExpansion && !!_stricmp(AchParent.name.c_str(), szCollExpName)) continue;

		bFoundExpansion = true;

		for (x = 0; x < AchCt; ++x) {

			AchID       = AchCat.GetAchievementId(x);
			AchIdx      = AchMgr.GetAchievementIndexById(AchID);
			
			const SingleAchievementAndComponentsInfo* AchCompInfo = AchMgr.GetAchievementClientInfoByIndex(AchIdx);

			// Anything nested under the achievement?
			if (!AchCompInfo) continue;

			const Achievement* Ach = AchMgr.GetAchievementByIndex(AchIdx);

			// Is it the right collection?
			if (bCollection && !!_stricmp(Ach->name.c_str(), szCollExpName)) continue;

			bFoundCollection = true;

			CompTypeCt = Ach->componentsByType[AchievementComponentCompletion].GetCount();

			// Are we logging the output?
			if ((bLog || bBazaar) && !bLogStarted) {

				if (!IsPluginLoaded("MQ2Log")) {

					WriteChatf("\aw[MQ2Collectible] \ayMQ2Log must be loaded to log output. Trying to do it for you.");
					EzCommand("/timed 10 /plugin MQ2Log load");
					return;
				}
				szLogFile = CreateLogFileName(szCharName, bCollected, bNeed, bLog, bBazaar);

				WriteChatf("\n\aw[MQ2Collectible] \ayWriting to Logfile: %s", szLogFile);

				// Start logging with mq2log
				// Write a comment at top with the parameters used to make the log
				bLogStarted = true;
			}

			if (bConsole) {
				WriteChatf("\n\aw[MQ2Collectible] \ao%s, %s", Ach->name.c_str(), AchParent.description.c_str());
				WriteChatf("\ao---------------------------------------------------------------------------------------------------");
			}
			if (bLog) {
				sprintf_s(szCharBuffer, "\n[MQ2Collectible] %s, %s", Ach->name.c_str(), AchParent.description.c_str());
				sprintf_s(szCharBuffer, "%s\n---------------------------------------------------------------------------------------------------", szCharBuffer);
				if (!LogOutput(szLogFile, szCharBuffer)) return;
			}

			// List the collectibles
			for (int y = 0; y < CompTypeCt; y++) {

				const AchievementComponent& CompTypeCompletion = Ach->componentsByType[AchievementComponentCompletion][y];

				// Need first, since I am assuming more will NEED collectibles, or else they wouldn't use this plugin.
				if (!AchCompInfo->IsComponentComplete(AchievementComponentCompletion, y) && bNeed) {

					bCollectedStatus = false;

					if (bConsole) {
						WriteChatf("\ay%s      \ao%s",NEED,CompTypeCompletion.description.c_str());
						continue;
					}
					if (bLog) {
						sprintf_s(szCharBuffer,"%s      %s", NEED, CompTypeCompletion.description.c_str());
						if (!LogOutput(szLogFile, szCharBuffer)) return;
						continue;
					}
				}

				if (AchCompInfo->IsComponentComplete(AchievementComponentCompletion, y) && bCollected) {

					bCollectedStatus = true;

					if (bConsole) {
						WriteChatf("\ao%s \ao%s", COLLECTED, CompTypeCompletion.description.c_str());
						continue;
					}
					if (bLog) {
						sprintf_s(szCharBuffer, "%s %s", COLLECTED, CompTypeCompletion.description.c_str());
						if (!LogOutput(szLogFile, szCharBuffer)) return;
						continue;
					}
				}

				if (bBazaar && bLogStarted) {

					if (bNeed && !bCollectedStatus) {
					// Write INI entry for the collectible.
					// Use bCollectedStatus to set Collected
					// [CompTypeCompletion.description.c_str()]
					// Collected=0
					// Collection=Ach->name.c_str(), AchParent.description.c_str()
					// etc
					}
					if (bCollected && bCollectedStatus) {
						// Write INI entry for collected....
					}
				}
			}
			// Are we done?
			if (bCollection && bFoundCollection) break;
		}
		if (bExpansion && bFoundExpansion) break;
	}

	if (bCollection && !bFoundCollection) {
		WriteChatf("\n\aw[MQ2Collectible] \ayCould not find Collection %s", szCollExpName);
	}
	if (bExpansion && !bFoundExpansion) {
		WriteChatf("\n\aw[MQ2Collectible] \ayCould not find Expansion %s", szCollExpName);
	}
}

void CheckLogDirectory()
{
	char szLogDir[MAX_STRING];
	sprintf_s(szLogDir, "%s\\Collectible", gPathLogs);

	_mkdir(szLogDir);
}

char* CreateLogFileName(char* szCharName, bool bCollected, bool bNeed, bool bLog, bool bBazaar)
{
	CheckLogDirectory();

	char* szCollectStatus;
	char* szLogType;
	char szLogFileName[MAX_STRING] = { 0 };

	if (bCollected && bNeed) {
		szCollectStatus = "both";
	} else if (bCollected) {
		szCollectStatus = "coll";
	} else if (bNeed) {
		szCollectStatus = "need";
	}

	if (bLog) {
		szLogType = "log";
	} else if (bBazaar) {
		szLogType = "baz";
	}

	sprintf_s(szLogFileName, "%s\\Collectible\\%s_%s_%s.%s", gPathLogs, (char*)__ServerName, szCharName, szCollectStatus, szLogType);

	return (char*)szLogFileName;
}

bool LogOutput(char* szLogFileName, char* szLogThis)
{
	FILE* fOut = NULL;

	errno_t bError = fopen_s(&fOut, szLogFileName, "at");
	if (bError) {
		MacroError("\aw[MQ2Collectible] \ayCouldn't write to the logfile. Check permissions at directory Logs\\Collectible. Logfile: %s", szLogFileName);
		return false;
	}

	fprintf(fOut, "%s\n", szLogThis);
	fclose(fOut);

	return true;
}



void ShowCMDHelp()
{
			WriteChatf("\awMQ2Collectible \ayUsage: \ag/collectible collected|need|both|help log|bazaar|console (optional unless console then specify collection: expansion|collection \"name\")");
			WriteChatf("\awMQ2Collectible \ayAbbreviations: \ag-cd|collected, -nd|need, -b|both, -h|h|help, -l|log, -bz|bazaar, -cs|console, -e|expansion, -cn|collection\n");
			WriteChatf("\awMQ2Collectible \ayExample: \ag/collectible need log collection \"Dead Relics\"\aw Quotations required. Produces logfile with collectibles needed from Dead Relics collection.");
			WriteChatf("\awMQ2Collectible \ayExample: \ag/collectible -n -l -cn \"Dead Relics\"\aw Same as above with abbreviated parameters.");
			WriteChatf("\awMQ2Collectible \ayExample: \ag/collectible -b -bz -e \"Terror of Luclin\"\aw Produces Bazaar.mac compatible logfile with all collected and uncollected collectibles from Terror of Luclin expansion.");
			WriteChatf("\awMQ2Collectible \ayExample of logfile naming convention: \agCollectible_need_bazaar_Terror_of_Luclin.ini");
			WriteChatf("\awMQ2Collectible \ayExample console: \ag/collectible -b -cs -cn \"Flame-Licked Clothing\n\aw Outputs the status of collectibles from the collection.");
			WriteChatf("\awMQ2Collectible \ayFor more examples see \aghttps://gitlab.com/redguides/plugins/mq2collectible/README.MD");
			// WriteChatf("");
}

// ----------------------------------------------
// TLO Section
// ----------------------------------------------

void ShowTLOHelp();

void CollectibleTLO(SPAWNINFO* pChar, char* szLine)
{
		
	PCHARINFO pCharInfo = GetCharInfo();
	PcProfile* pCharInfo2 = GetPcProfile();

}

// ----------------------------------------------
// Plugin APIs
// ----------------------------------------------

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

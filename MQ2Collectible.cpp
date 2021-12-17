// MQ2Collectible.cpp
//
// by American Nero, December 16, 2021
//
// Appreciation to Kaen01 for his suggestion to add logfile output in Bazaar.mac format,
// and Knightly, Chatwiththisname, and Brainiac for technical guidance.
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

void LookupCollection(char* szCollectionName, bool bCollected, bool bNeed, bool bLog, bool bBazaar, bool bConsole);
void LookupCollectionsByExpansion(char* szExpansionName, bool bCollected, bool bNeed, bool bLog, bool bBazaar, bool bConsole);

void ConsoleOutput();
// void LookupCollectible(std::string szCollectibleName);

void CollectibleCMD(SPAWNINFO* pChar, char* szLine);
void ShowCMDHelp();

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

	char szName[256] 		= { 0 };

	int  iItemID			= 0;

	// Parse Parameters

	char szArg[16] = { 0 };
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
		GetArg(szName, szLine, 4);
		if (strlen(szName)==0) {
			ShowCMDHelp();
			return;
		}
		if (IsNumber(szName)) {
			int iItemID = atoi(szName);
			if (iItemID<1) {
				ShowCMDHelp();
				return;
			}
		}
	}

	// Look up by Collection or Expansion if specified, or all.
	if (bCollection) {
		LookupCollection(szName, bCollected, bNeed, bLog, bBazaar, bConsole);
	} else if (bExpansion) {
		LookupCollectionsByExpansion(szName, bCollected, bNeed, bLog, bBazaar, bConsole);
	}

	// PCHARINFO pCharInfo = GetCharInfo();
	// PcProfile* pCharInfo2 = GetPcProfile();
}

void LookupCollection(char* szCollectionName, bool bCollected, bool bNeed, bool bLog, bool bBazaar, bool bConsole)
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

	bool  bFound       = false;

	for (const AchievementCategory& AchCat : AchMgr.categories) {

		if (!string_equals(AchCat.name, "Collections")) continue;

		// Secrets of Faydwer says Collections, but there aren't any. As of Terror of Luclin, Events and 10 expansions actually have real collections.
		AchCt = AchCat.GetAchievementCount();
		if (!AchCt) continue;

		// Obtain the Expansion name.
		AchParentID = AchCat.parentId;
		const AchievementCategory& AchParent = *AchMgr.GetAchievementCategoryById(AchParentID);

		//WriteChatf("AchCat: %s, %s, AchCt: %d", AchCat.name.c_str(), AchParent.name.c_str(), AchCt);

		for (x = 0; x < AchCt; ++x) {

			AchID       = AchCat.GetAchievementId(x);
			AchIdx      = AchMgr.GetAchievementIndexById(AchID);
			
			const SingleAchievementAndComponentsInfo* AchCompInfo = AchMgr.GetAchievementClientInfoByIndex(AchIdx);

			// Anything nested under the achievement?
			if (!AchCompInfo) continue;

			const Achievement* Ach = AchMgr.GetAchievementByIndex(AchIdx);

			// Is it the right collection?
			if (!!_stricmp(Ach->name.c_str(), szCollectionName)) continue;

			bFound = true;

			CompTypeCt = Ach->componentsByType[AchievementComponentCompletion].GetCount();

			if (bConsole) {
				WriteChatf("\nMQ2Collectible: %s, %s", Ach->name.c_str(), AchParent.description.c_str());
				WriteChatf("-------------------------------------------------------------------------------");
			}

			// List the collectibles
			for (int y = 0; y < CompTypeCt; y++) {

				const AchievementComponent& CompTypeCompletion = Ach->componentsByType[AchievementComponentCompletion][y];

				// Need first, since I am assuming more will NEED collectibles, or else they wouldn't use this plugin.
				if (!AchCompInfo->IsComponentComplete(AchievementComponentCompletion, y) && bNeed) {
					if (bConsole) {
						WriteChatf("%s %s",CompTypeCompletion.description.c_str(),NEED);
					}
					continue;
				}

				if (AchCompInfo->IsComponentComplete(AchievementComponentCompletion, y) && bCollected) {
					if (bConsole) {
						WriteChatf("%s %s",CompTypeCompletion.description.c_str(),COLLECTED);
					}
					continue;
				}
			}
			// We are done.
			break;
		}
	}
	if (!bFound) {
		WriteChatf("\nMQ2Collectible: Could not find Collection %s", szCollectionName);
	}
}

void LookupCollectionsByExpansion(char* szExpansionName, bool bCollected, bool bNeed, bool bLog, bool bBazaar, bool bConsole)
{

	AchievementManager& achievemanager = AchievementManager::Instance();

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

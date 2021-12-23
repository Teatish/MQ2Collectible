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

void CollectibleCMD(SPAWNINFO* pChar, char* szLine);
void LookupCollection(char* szCharName, char* szCollExpName, bool bCollected, bool bNeed, bool bLog, bool bBazaar, bool bConsole, bool bCollection, bool bExpansion);
void ShowCMDHelp();
void CheckLogDirectory();
char* CreateLogFileName(char* szCharName, bool bCollected, bool bNeed, bool bLog, bool bBazaar);
bool LogOutput(char* szLogFileName, char* szLogThis);
std::string_view trimP(std::string_view tempStr);

// Bazaar.mac defaults - probly put this in a ini or read from their ini at some point.
const int SELLMIN   = 2000000;
const int SELLMAX   = 2000000;
const int BUYMIN    = 1;
const int BUYMAX    = 1;
const int MINBUYCT  = 1;

// ----------------------------------------------
// Command section
// ----------------------------------------------

// May want to place the vars into a struct instead of passing individually?
void CollectibleCMD(SPAWNINFO* pChar, char* szLine)
{
	char szArg[16] 			= { 0 };
	
	// Parse Parameters
	GetArg(szArg, szLine, 1);

	if (strlen(szLine)==0 || !_stricmp(szArg, "help") || !_stricmp(szArg, "-h"))
	{
		ShowCMDHelp();
		return;
	}

	bool bCollected 		= false;
	bool bNeed      		= false;

	if (!_stricmp(szArg, "collected") || !_stricmp(szArg, "-cd"))
	{
		bCollected = true;
	}
	else if (!_stricmp(szArg, "need") || !_stricmp(szArg, "-n"))
	{
		bNeed = true;
	}
	else if (!_stricmp(szArg, "both") || !_stricmp(szArg, "-b"))
	{
		bCollected = true;
		bNeed = true;
	}
	else
	{
		ShowCMDHelp();
		return;
	}

	GetArg(szArg, szLine, 2);

	bool bLog				= false;
	bool bBazaar 			= false;
	bool bConsole 			= false;

	if (!_stricmp(szArg, "log") || !_stricmp(szArg, "-l"))
	{
		bLog = true;
	}
	else if (!_stricmp(szArg, "bazaar") || !_stricmp(szArg, "-bz"))
	{
		bBazaar = true;
	}
	else if (!_stricmp(szArg, "console") || !_stricmp(szArg, "-cs"))
	{
		bConsole = true;
	}
	else
	{
		ShowCMDHelp();
		return;
	}

	GetArg(szArg, szLine, 3);

	bool bExpansion			= false;
	bool bCollection		= false;

	if (strlen(szArg))
	{
		if (!_stricmp(szArg, "expansion") || !_stricmp(szArg, "-e"))
		{
			bExpansion = true;
		}
		else if (!_stricmp(szArg, "collection") || !_stricmp(szArg, "-cn"))
		{
			bCollection = true;
		}
		else
		{
			ShowCMDHelp();
			return;
		}
	}

	char szCollExpName[256] = { 0 };

	if (bExpansion || bCollection)
	{
		GetArg(szCollExpName, szLine, 4);

		if (strlen(szCollExpName)==0)
		{
			ShowCMDHelp();
			return;
		}

		if (IsNumber(szCollExpName))
		{
			ShowCMDHelp();
			return;
		}
	}

	// Look up by Collection or Expansion if specified, or all.
	LookupCollection(pChar->Name, szCollExpName, bCollected, bNeed, bLog, bBazaar, bConsole, bCollection, bExpansion);
}

void LookupCollection(char* szCharName, char* szCollExpName, bool bCollected, bool bNeed, bool bLog, bool bBazaar, bool bConsole, bool bCollection, bool bExpansion)
{
	std::string_view trimCollExpName = trimP(szCollExpName);

	// Find category "Collections", then search through the subcategories
	// until we find the needed Collection, then iterate through the
	// collectibles.

	AchievementManager& AchMgr = AchievementManager::Instance();

	bool  bFoundExpansion          = false;
	bool  bFoundCollection         = false;
	bool  bLogStarted              = false;
	char* szLogFile;
	char  szCharBuffer[MAX_STRING] = { 0 };
	int   iMatchesFound            = 0;

	const char* COLLECTED	= "[COLLECTED]";
	const char* NEED		= "[NEED]";
	const char* INCOMPLETE	= "[INCOMPLETE]";
	const char* COMPLETED	= "[COMPLETED]";

	for (const AchievementCategory& AchCat : AchMgr.categories)
	{
		if (!string_equals(AchCat.name, "Collections")) continue;

		// Secrets of Faydwer says Collections, but there aren't any. As of Terror of Luclin, Events and 10 expansions actually have real collections.
		if (!AchCat.GetAchievementCount()) continue;

		// Obtain the Expansion name.
		const AchievementCategory& AchParent = *AchMgr.GetAchievementCategoryById(AchCat.parentId);

		//if (bExpansion && !!_stricmp(AchParent.name.c_str(), szCollExpName)) continue;
		//if (bExpansion && ci_find_substr(AchParent.name, szCollExpName)<0) continue;
		if (bExpansion && ci_find_substr(AchParent.name, trimCollExpName)==-1) continue;

		bFoundExpansion = true;

		for (int x = 0; x < AchCat.GetAchievementCount(); ++x)
		{
			int AchIdx = AchMgr.GetAchievementIndexById(AchCat.GetAchievementId(x));	
			const SingleAchievementAndComponentsInfo* AchCompInfo = AchMgr.GetAchievementClientInfoByIndex(AchIdx);

			// Anything nested under the achievement?
			if (!AchCompInfo) continue;

			const Achievement* Ach = AchMgr.GetAchievementByIndex(AchIdx);
			
			// Trimmed at parentheses, if present.
			std::string_view AchName = trimP(Ach->name.c_str());

			// Is it the right collection?
			if (bCollection && ci_find_substr(AchName, trimCollExpName)==-1) continue;

			// Is it a collection of collections? A collection is an achievement, and a collection of collections only has components which are achievements.
			// Given a correct name
			// Is it an achievement?
			// 

			// A meta achievement is a collection of achievements that are not based on collectibles.
			// We need to know that its from collectibles, not just a collection of achievement collections.
			// Terms like "Complete the", "upon completing" found in the description were tried but results were not satisfactory.
			//
			// New idea. Given a collection name, retrieve *all* of the collectibles that it encompasses.
			// Given a set, explore any subsets, and the members.
			//
			// Given a key, find its id and component id. If that component id is a member

			bFoundCollection = true;

			int CompTypeCt = Ach->componentsByType[AchievementComponentCompletion].GetCount();

			// Are we logging the output?
			if ((bLog || bBazaar) && !bLogStarted)
			{
				szLogFile = CreateLogFileName(szCharName, bCollected, bNeed, bLog, bBazaar);
				bLogStarted = true;
			}

			bool bCollectedStatus = false;
			bool bHeaderDone      = false;

			//std::string tempStr = fmt::format("Testing Search [{}] Trim [{}] AchName [{}]", szCollExpName, trimCollExpName, Ach->name.c_str());
			//WriteChatf("%s", tempStr);
			
			// List the collectibles
			for (int y = 0; y < CompTypeCt; y++)
			{
				const AchievementComponent& CompTypeCompletion = Ach->componentsByType[AchievementComponentCompletion][y];
				int iAchFound = 0;

				// If achievement has a component that is itself an achievement then we know it is a meta achievement.
				// A component that is not itself an achievement must be a collectible.
				
				/*
				  - Call of the Forsaken (cid: 2100)
					-- Collections (cid: 2109)
 
					  --- Hearing the Call (aid: 2100 0010, cid: NA) [Meta]
						---- The Source of the Ethernere (cid: 2100 0010)
						---- Dead Relics (cid: 2100 0011)
						---- ...
 
						--- The Source of the Ethernere (aid: 2107 7200, cid: 2107 7200)
							---- Scattered Civilization (aid: 2107 7210, ci: 2107 7210
							---- Rusty Water Can (aid: NA, cid: 2107 7210)
							---- Scorched Cookware (aid: NA, cid: 2107 7211)
							---- Bent Notched Blade (aid: NA, cid: 2107 7212)
							---- ...

						--- Geological Discoveries (id: 2107 7201, cid: 
				*/

				//std::string tempStr = fmt::format("Checking {} [{}]", y, CompTypeCompletion.description.c_str());
				//WriteChatf("%s", tempStr.c_str());

				//How many times was the achievement listed? If > 1 then its a meta achievement.

				// Is the component also an achievement?
					
				// This is a terrible, ugly thing. Fine if 2-3 nested levels of a single meta achievement.
				for (int z = 0; z < AchCat.GetAchievementCount(); ++z)
				{
					int AchsIdx = AchMgr.GetAchievementIndexById(AchCat.GetAchievementId(z));
					const SingleAchievementAndComponentsInfo* AchsCompInfo = AchMgr.GetAchievementClientInfoByIndex(AchsIdx);

					// Anything nested under the achievement?
					if (!AchsCompInfo) continue;

					const Achievement* AchsAch = AchMgr.GetAchievementByIndex(AchsIdx);

					//std::string tempStr = fmt::format("Checking: {} [{}] [{}] [{}]", AchsIdx, CompTypeCompletion.description.c_str(), AchsAch->name.c_str(), iAchFound);
					//WriteChatf("%s", tempStr.c_str());

					if (ci_find_substr(AchsAch->name, CompTypeCompletion.description)<0) continue;

					iAchFound++;

					//tempStr = fmt::format("Found: {} [{}] [{}] [{}]", AchsIdx, CompTypeCompletion.description.c_str(), AchsAch->name.c_str(), iAchFound);

					//WriteChatf("%s", tempStr.c_str());

					break;

					//for (int w = 0; w < AchsAch->componentsByType[AchievementComponentCompletion].GetCount(); w++)
					//{
					//	const AchievementComponent& AchsComps = AchsAch->componentsByType[AchievementComponentCompletion][w];

					//	std::string tempStr = fmt::format("Checking {} [{}]", w, AchsComps.description.c_str());
					//	WriteChatf("%s", tempStr.c_str());

					//	if (ci_find_substr(AchName, trimP(AchsComps.description.c_str()))<0) continue;

					//	iAchFound++;

					//	tempStr = fmt::format("Found: {} [{}] [{}]", AchsIdx, AchsComps.description.c_str(), iAchFound);

					//	WriteChatf("%s", tempStr.c_str());

					//	break;
					//}

				}

				if (bConsole && !bHeaderDone)
				{
					WriteChatf("\n\aw[MQ2Collectible] \ao%s, %s", Ach->name.c_str(), AchParent.description.c_str());
					//WriteChatf("\ao%s", Ach->description.c_str());
					WriteChatf("\ao-----------------------------------------------------------------------------------------");
				}

				if (bLog && !bHeaderDone)
				{
					sprintf_s(szCharBuffer, "\n[MQ2Collectible] %s, %s", Ach->name.c_str(), AchParent.description.c_str());
					//sprintf_s(szCharBuffer, "%s\n%s", szCharBuffer, Ach->description.c_str());
					sprintf_s(szCharBuffer, "%s\n-----------------------------------------------------------------------------------------", szCharBuffer);
					if (!LogOutput(szLogFile, szCharBuffer)) return;
				}

				bHeaderDone = true;

				// Need first, since I am assuming more will NEED collectibles, or else they wouldn't use this plugin.

				if (!AchCompInfo->IsComponentComplete(AchievementComponentCompletion, y) && bNeed)
				{
					iMatchesFound++;
					bCollectedStatus = false;

					if (bConsole)
					{
						if (iAchFound)
						{
							WriteChatf("\ay%s \ao%s", INCOMPLETE, CompTypeCompletion.description.c_str());
							continue;
						}
						else
						{
							WriteChatf("\ay%s       \ao%s", NEED, CompTypeCompletion.description.c_str());
							continue;
						}
					}

					if (bLog)
					{
						if (iAchFound)
						{
							sprintf_s(szCharBuffer, "%s %s", INCOMPLETE, CompTypeCompletion.description.c_str());
							if (!LogOutput(szLogFile, szCharBuffer)) return;
							continue;
						}
						else
						{
							sprintf_s(szCharBuffer, "%s      %s", NEED, CompTypeCompletion.description.c_str());
							if (!LogOutput(szLogFile, szCharBuffer)) return;
							continue;
						}
					}
				}

				// Collected
				if (AchCompInfo->IsComponentComplete(AchievementComponentCompletion, y) && bCollected)
				{
					iMatchesFound++;
					bCollectedStatus = true;

					if (bConsole)
					{
						if (iAchFound)
						{
							WriteChatf("\ao%s  \ao%s", COMPLETED, CompTypeCompletion.description.c_str());
							continue;
						}
						else
						{
							WriteChatf("\ao%s  \ao%s", COLLECTED, CompTypeCompletion.description.c_str());
							continue;
						}
					}

					if (bLog)
					{
						if (iAchFound)
						{
							sprintf_s(szCharBuffer, "%s  %s", COMPLETED, CompTypeCompletion.description.c_str());
							if (!LogOutput(szLogFile, szCharBuffer)) return;
							continue;
						}
						else
						{
							sprintf_s(szCharBuffer, "%s  %s", COLLECTED, CompTypeCompletion.description.c_str());
							if (!LogOutput(szLogFile, szCharBuffer)) return;
							continue;

						}
					}
				}

				if (bBazaar && bLogStarted)
				{
					if (!iAchFound)
					{
						sprintf_s(szCharBuffer, "[%s]\n", CompTypeCompletion.description.c_str());
						sprintf_s(szCharBuffer, "%sCollected=%d\nCollection=%s, %s", szCharBuffer, (int)bCollectedStatus, Ach->name.c_str(), AchParent.description.c_str());

						if ((bNeed && !bCollectedStatus) || (bNeed && bCollected))
						{
							sprintf_s(szCharBuffer, "%s\nBuyPriceMin=%d\nBuyPriceMax=%d\nMinBuyCount=%d", szCharBuffer, BUYMIN, BUYMAX, MINBUYCT);
						}

						if ((bCollected && bCollectedStatus) || (bCollected && bNeed))
						{
							sprintf_s(szCharBuffer, "%s\nSellPriceMin=%d\nSellPriceMax=%d", szCharBuffer, SELLMIN, SELLMAX);
						}

						if (!LogOutput(szLogFile, szCharBuffer)) return;
					}
				}
			}

			// Are we done finding a Collection?
			//if (bCollection && bFoundCollection) break;
		}

		// Are we done finding an Expansion?
		if (bExpansion && bFoundExpansion) break;
	}

	if (bCollection && !bFoundCollection)
	{
		WriteChatf("\n\aw[MQ2Collectible] \ayThis is not a Collection of Collectibles: %s", szCollExpName);
	}

	if (bExpansion && !bFoundExpansion)
	{
		WriteChatf("\n\aw[MQ2Collectible] \ayCould not find Expansion/Category: %s", szCollExpName);
	}

	if (iMatchesFound)
	{
		if (bConsole)
		{
			WriteChatf("\n\aw[MQ2Collectible] \ayFound %d matches", iMatchesFound);
		}

		if (bLog || bBazaar)
		{
			WriteChatf("\n\aw[MQ2Collectible] \ayFound %d matches: %s", iMatchesFound, szLogFile);
		}
	}
	else
	{
		WriteChatf("\aw[MQ2Collectible] \ayNo matches found.");
	}
}

// Trim the string at the first "(" encountered. If found, search for " (" and trim there instead.
std::string_view trimP(std::string_view tempStr)
{
	int iFoundSubstr = ci_find_substr(tempStr, "(");
	if (iFoundSubstr==-1)
	{
		return tempStr;
	}

	if (iFoundSubstr>0 && tempStr[iFoundSubstr - 1] == ' ')
	{
		return tempStr.substr(0, iFoundSubstr - 1);
	}

	return tempStr.substr(0, iFoundSubstr);
}

void CheckLogDirectory()
{
	char szLogDir[MAX_STRING];
	sprintf_s(szLogDir, "%s\\Collectible", gPathLogs);

	bool bDirStatus = _mkdir(szLogDir);
}

char* CreateLogFileName(char* szCharName, bool bCollected, bool bNeed, bool bLog, bool bBazaar)
{
	CheckLogDirectory();

	char* szCollectStatus;
	char* szLogType;
	char szLogFileName[MAX_STRING] = { 0 };

	if (bCollected && bNeed)
	{
		szCollectStatus = "both";
	}
	else if (bCollected)
	{
		szCollectStatus = "coll";
	}
	else if (bNeed)
	{
		szCollectStatus = "need";
	}

	if (bLog)
	{
		szLogType = "log";
	} 
	else if (bBazaar)
	{
		szLogType = "baz";
	}

	if (bBazaar)
	{
		sprintf_s(szLogFileName, "%s\\Collectible\\%s_%s_%s_%s.log", gPathLogs, (char*)__ServerName, szCharName, szCollectStatus, szLogType);
	}

	if (bLog)
	{
		sprintf_s(szLogFileName, "%s\\Collectible\\%s_%s_%s.log", gPathLogs, (char*)__ServerName, szCharName, szCollectStatus);
	}

	return szLogFileName;
}

bool LogOutput(char* szLogFileName, char* szLogThis)
{
	FILE* fOut = NULL;

	errno_t bError = fopen_s(&fOut, szLogFileName, "at");

	if (bError)
	{
		MacroError("\aw[MQ2Collectible] \ayCouldn't write to the logfile. Check permissions at directory\n\aw[MQ2Collectible] \atLogs\\Collectible. Logfile: %s", szLogFileName);
		return false;
	}

	fprintf(fOut, "%s\n", szLogThis);
	fclose(fOut);

	return true;
}

void ShowCMDHelp()
{
	WriteChatf("\n\aw[MQ2Collectible] \ayUsage: \at/collectible collected|need|both|help log|bazaar|console expansion|collection \"name\"");
	WriteChatf("\aw[MQ2Collectible] \ayOmitting expansion|collection will return all within the Expansion or within a Collection.");
	WriteChatf("\aw[MQ2Collectible] \ayExpansions (starting with Call of the Forsaken) and the Event category have collectibles.\n");
	WriteChatf("\aw[MQ2Collectible] \ayAbbrevs: \at-cd|collected, -nd|need, -b|both, -h|h|help, -l|log, -bz|bazaar,");
	WriteChatf("\aw[MQ2Collectible]          \at-cs|console, -e|expansion, -cn|collection\n");
	WriteChatf("\aw[MQ2Collectible] \ayExample: \at/collectible need log collection \"The Plaguebringer's Chosen\"\aw Note that quotes are required.\n");
	WriteChatf("\aw[MQ2Collectible] \ayExample: \at/collectible -n -l -cn \"The Plaguebringer's Chosen\"\aw Same as above with abbreviated parameters.\n");
	WriteChatf("\aw[MQ2Collectible] \ayExample: \at/collectible -b -bz -e \"Terror of Luclin\"\aw Produces Bazaar.mac compatible logfile");
	WriteChatf("\aw[MQ2Collectible]          \awwith all collected and uncollected collectibles. A big list!\n");
	WriteChatf("\aw[MQ2Collectible] \ayLogfile name example: \atLogs\\Collectible\\MyCharName_servername_need_baz.log\n");
	WriteChatf("\aw[MQ2Collectible] \ayExample to console: \at/collectible -b -cs -cn \"Flame-Licked Clothing\"");
	WriteChatf("\aw[MQ2Collectible]                     \awOutputs the status of collectibles from that collection.\n");
	WriteChatf("\aw[MQ2Collectible] \ayTLO: \at${Collectible[\"collectible name\"]} returns true|false \awfor its collection status.\n");
}

// ----------------------------------------------
// TLO Section
// ----------------------------------------------

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

	if (szAlias != "None")
	{
		WriteChatf("\awMQ2Collectible: \arWarning! The alias /collectible already exists. Please delete it by entering \"\ay/alias /collectible delete\ar\" then try again.");
		EzCommand("/timed 10 /plugin MQ2Collectible unload");
		// Probably not needed. But at least I won't wake up in the middle of the night wondering about it.
		return;
	}

	AddCommand("/collectible", CollectibleCMD);
	//AddMQ2Data("Collectible", CollectibleTLO);

	WriteChatf("\aw[MQ2Collectible] \at/collectible -h \aoby American Nero (https://gitlab.com/redguides/plugins/mq2collectible)");
}

PLUGIN_API void ShutdownPlugin()
{
	DebugSpewAlways("MQ2Collectible::Shutting down");

	RemoveCommand("/collectible");
	RemoveMQ2Data("Collectible");
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

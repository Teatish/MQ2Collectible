// MQ2Collectible.cpp
//
// by American Nero, December 16, 2021
//
// Appreciation to Kaen01 for his suggestion to add logfile output in Bazaar.mac format,
// and Brainiac, Knightly, and Chatwiththisname for stylistic and technical guidance.
//
// Provides command /collectible that produces logfiles and console output
// Provides TLO ${Collectible} that retrieves a collectibles status and other members
//
// Repo with detailed usage at https://gitlab.com/redguides/plugins/mq2collectible/README.MD
//
// Please update README.MD prior to making substantial code changes so that the requirements are implicitly
// understood and usage is adequately described.

#include <mq/Plugin.h>

PreSetup("MQ2Collectible");
PLUGIN_VERSION(0.3);

using namespace mq::datatypes;

void CollectibleCMD(SPAWNINFO* pChar, char* szLine);
std::string_view LookupCollectible(std::string_view Collectible);
void LookupCollection(char* szCharName, std::string_view CollExpName, bool bCollected, bool bNeed, bool bLog, bool bBazaar, bool bConsole, bool bCollection, bool bExpansion);
void ShowCMDHelp();
void CheckLogDirectory();
std::string CreateLogFileName(const char* szCharName, bool bCollected, bool bNeed, bool bLog, bool bBazaar);
bool LogOutput(const char* szLogFileName, const char* szLogThis);
std::string_view trimP(std::string_view tempStr);
int GetCollectibleState(std::string_view CollectibleName);

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

	if (strlen(szLine)==0 || !_stricmp(szArg, "help") || !_stricmp(szArg, "-h") || !_stricmp(szArg, "-help"))
	{
		ShowCMDHelp();
		return;
	}

	bool bCollected 		= false;
	bool bNeed      		= false;

	if (!_stricmp(szArg, "collected") || !_stricmp(szArg, "-cd") || !_stricmp(szArg, "-collected"))
	{
		bCollected = true;
	}
	else if (!_stricmp(szArg, "need") || !_stricmp(szArg, "-n") || !_stricmp(szArg, "-need"))
	{
		bNeed = true;
	}
	else if (!_stricmp(szArg, "all") || !_stricmp(szArg, "-a") || !_stricmp(szArg, "-all"))
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

	if (!_stricmp(szArg, "log") || !_stricmp(szArg, "-l") || !_stricmp(szArg, "-log"))
	{
		bLog = true;
	}
	else if (!_stricmp(szArg, "bazaar") || !_stricmp(szArg, "-bz") || !_stricmp(szArg, "-baz") || !_stricmp(szArg, "-bazaar"))
	{
		bBazaar = true;
	}
	else if (!_stricmp(szArg, "console") || !_stricmp(szArg, "-cs") || !_stricmp(szArg, "-con") || !_stricmp(szArg, "-console"))
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
	bool bCollectible		= false;

	if (strlen(szArg))
	{
		if (!_stricmp(szArg, "expansion") || !_stricmp(szArg, "-e") || !_stricmp(szArg, "-exp") || !_stricmp(szArg, "-expansion"))
		{
			bExpansion = true;
		}
		else if (!_stricmp(szArg, "collection") || !_stricmp(szArg, "-cn") || !_stricmp(szArg, "-collection"))
		{
			bCollection = true;
		}
		else if (!_stricmp(szArg, "collectible") || !_stricmp(szArg, "-cl") || !_stricmp(szArg, "-c") || !_stricmp(szArg, "-collectible"))
		{
			bCollectible = true;
		}
		else
		{
			ShowCMDHelp();
			return;
		}
	}

	char szSearchName[256] = { 0 };

	if (bExpansion || bCollection || bCollectible)
	{
		GetArg(szSearchName, szLine, 4);

		if (strlen(szSearchName)==0)
		{
			ShowCMDHelp();
			return;
		}

		if (IsNumber(szSearchName))
		{
			ShowCMDHelp();
			return;
		}
	}

	std::string_view AchSearch = trimP(szSearchName);

	if (bCollectible)
	{
		std::string_view strCollection = LookupCollectible(AchSearch);

		if (strCollection.empty()) return;
		{
			AchSearch = trimP(strCollection);
			bCollection = true;
		}
	}

	// Look up by Collection or Expansion if specified, or all.
	LookupCollection(pChar->Name, AchSearch, bCollected, bNeed, bLog, bBazaar, bConsole, bCollection, bExpansion);
}

void LookupCollection(char* szCharName, std::string_view CollExpName, bool bCollected, bool bNeed, bool bLog, bool bBazaar, bool bConsole, bool bCollection, bool bExpansion)
{
	// Find category "Collections", then search through the subcategories
	// until we find the needed Collection, then iterate through the
	// collectibles.

	AchievementManager& AchMgr = AchievementManager::Instance();

	bool  bFoundExpansion          = false;
	bool  bFoundCollection         = false;
	bool  bLogStarted              = false;
	char  szCharBuffer[MAX_STRING] = { 0 };
	int   iMatchesFound            = 0;

	std::string LogFile     = "";

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
		if (bExpansion && ci_find_substr(AchParent.name, CollExpName)==-1) continue;

		bFoundExpansion = true;

		for (int x = 0; x < AchCat.GetAchievementCount(); ++x)
		{
			int AchIdx = AchMgr.GetAchievementIndexById(AchCat.GetAchievementId(x));	
			const SingleAchievementAndComponentsInfo* AchCompInfo = AchMgr.GetAchievementClientInfoByIndex(AchIdx);

			// Anything nested under the achievement?
			if (!AchCompInfo) continue;

			const Achievement* Ach = AchMgr.GetAchievementByIndex(AchIdx);
			
			// Trimmed at parentheses, if present.
			std::string_view AchName = trimP(Ach->name);

			// Is it the right collection?
			if (bCollection && ci_find_substr(AchName, CollExpName)==-1) continue;

			// Is it a collection of collections? A collection is an achievement, and a collection of collections only has components which are achievements.
			// Is it an achievement?

			// A meta achievement is a collection of achievements that are not based on collectibles.
			// We need to know that its from collectibles, not just a collection of achievement collections.

			bFoundCollection = true;

			int CompTypeCt = Ach->componentsByType[AchievementComponentCompletion].GetCount();

			// Are we logging the output?
			if ((bLog || bBazaar) && !bLogStarted)
			{
				LogFile = CreateLogFileName(szCharName, bCollected, bNeed, bLog, bBazaar);
				bLogStarted = true;
			}

			bool bCollectedStatus = false;
			bool bHeaderDone      = false;
			
			// List the collectibles
			for (int y = 0; y < CompTypeCt; y++)
			{
				const AchievementComponent& CompTypeCompletion = Ach->componentsByType[AchievementComponentCompletion][y];
				int iAchFound = 0;

				// If achievement has a component that is itself an achievement then we know it is a meta achievement.
				// A component that is not itself an achievement must be a collectible.
				
				//How many times was the achievement listed? If > 0 then its a meta achievement.

				// Is the component also an achievement?

				// This is a terrible, ugly thing. Fine if 2-3 nested levels of a single meta achievement.
				for (int z = 0; z < AchCat.GetAchievementCount(); ++z)
				{
					int AchsIdx = AchMgr.GetAchievementIndexById(AchCat.GetAchievementId(z));
					const SingleAchievementAndComponentsInfo* AchsCompInfo = AchMgr.GetAchievementClientInfoByIndex(AchsIdx);

					// Anything nested under the achievement?
					if (!AchsCompInfo) continue;

					const Achievement* AchsAch = AchMgr.GetAchievementByIndex(AchsIdx);

					if (ci_find_substr(AchsAch->name, CompTypeCompletion.description)<0) continue;

					iAchFound++;

					break;
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
					if (!LogOutput(LogFile.c_str(), szCharBuffer)) return;
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
							if (!LogOutput(LogFile.c_str(), szCharBuffer)) return;
							continue;
						}
						else
						{
							sprintf_s(szCharBuffer, "%s      %s", NEED, CompTypeCompletion.description.c_str());
							if (!LogOutput(LogFile.c_str(), szCharBuffer)) return;
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
							sprintf_s(szCharBuffer, "%s %s", COMPLETED, CompTypeCompletion.description.c_str());
							if (!LogOutput(LogFile.c_str(), szCharBuffer)) return;
							continue;
						}
						else
						{
							sprintf_s(szCharBuffer, "%s  %s", COLLECTED, CompTypeCompletion.description.c_str());
							if (!LogOutput(LogFile.c_str(), szCharBuffer)) return;
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

						if (!LogOutput(LogFile.c_str(), szCharBuffer)) return;
					}
				}
			}

			// Are we done finding a Collection?
			//if (bCollection && bFoundCollection) break;
		}

		// Are we done finding an Expansion?
		if (bExpansion && bFoundExpansion) break;
	}

	if (!iMatchesFound)
	{
		WriteChatf("\aw[MQ2Collectible] \ayNo matches found.");
	}
}

// Given the name of a collectible, find the collection it belongs to.
std::string_view LookupCollectible(std::string_view CollectibleName)
{
	// Find the category "Collections", then search through the achievements
	// components. When found, check that the component is not itself an achievement
	// before returning the achievement the collectible was found in.

	AchievementManager& AchMgr = AchievementManager::Instance();

	bool   bMatchFound = 0;

	for (const AchievementCategory& AchCat : AchMgr.categories)
	{
		if (!string_equals(AchCat.name, "Collections")) continue;

		// Secrets of Faydwer says Collections, but there aren't any. As of Terror of Luclin, Events and 10 expansions actually have real collections.
		if (!AchCat.GetAchievementCount()) continue;

		for (int x = 0; x < AchCat.GetAchievementCount(); ++x)
		{
			int AchIdx = AchMgr.GetAchievementIndexById(AchCat.GetAchievementId(x));
			const SingleAchievementAndComponentsInfo* AchCompInfo = AchMgr.GetAchievementClientInfoByIndex(AchIdx);

			// Anything nested under the achievement?
			if (!AchCompInfo) continue;

			const Achievement* Ach = AchMgr.GetAchievementByIndex(AchIdx);

			// Trimmed at parentheses, if present.
			std::string_view AchName = trimP(Ach->name);

			int CompTypeCt = Ach->componentsByType[AchievementComponentCompletion].GetCount();

			//std::string tempStr = fmt::format("Testing Search [{}] Trim [{}] AchName [{}]", szCollExpName, trimCollExpName, Ach->name.c_str());
			//WriteChatf("%s", tempStr);

			// List the collectibles
			for (int y = 0; y < CompTypeCt; y++)
			{
				const AchievementComponent& CompTypeCompletion = Ach->componentsByType[AchievementComponentCompletion][y];

				if (ci_find_substr(CompTypeCompletion.description, CollectibleName) < 0) continue;

				return AchName;
			}
		}
	}

	if (!bMatchFound)
	{
		WriteChatf("\aw[MQ2Collectible] \ayNo matches found.");
	}

	return "";
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

std::string CreateLogFileName(const char* szCharName, bool bCollected, bool bNeed, bool bLog, bool bBazaar)
{
	CheckLogDirectory();

	const char* szCollectStatus = nullptr;
	const char* szLogType = nullptr;

	std::string LogFileName;

	if (bCollected && bNeed)
	{
		szCollectStatus = "all";
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
		LogFileName = fmt::format("{}\\Collectible\\{}_{}_{}_{}.log", gPathLogs, (char*)__ServerName, szCharName, szCollectStatus, szLogType);
	}

	if (bLog)
	{
		LogFileName = fmt::format("{}\\Collectible\\{}_{}_{}.log", gPathLogs, (char*)__ServerName, szCharName, szCollectStatus);
	}

	WriteChatf("\n\aw[MQ2Collectible] \ayLog appended to: \aw%s\n", LogFileName);

	return LogFileName;
}

bool LogOutput(const char* LogFileName, const char* szLogThis)
{
	FILE* fOut = NULL;

	errno_t bError = fopen_s(&fOut, LogFileName, "at");

	if (bError)
	{
		MacroError("\aw[MQ2Collectible] \ayCouldn't write to the logfile. Check permissions at directory\n\aw[MQ2Collectible] \atLogs\\Collectible. Logfile: %s", LogFileName);
		return false;
	}

	fprintf(fOut, "%s\n", szLogThis);
	fclose(fOut);

	return true;
}

void ShowCMDHelp()
{
	WriteChatf("\n\aw[MQ2Collectible] \ayUsage: \at/collectible collected|need|all|help log|bazaar|console expansion|collection|collectible \"name\"\n");
	WriteChatf("\aw[MQ2Collectible] \ayOmitting the \"name\" when searching for expansion|collection will return all results within. Accepts partial name.");
	WriteChatf("\aw[MQ2Collectible] \ayExpansions (starting with Call of the Forsaken), and the Event category, have collectibles.\n");
	WriteChatf("\aw[MQ2Collectible] \ayAbbrevs: \at-cd|collected, -nd|need, -a|all, -h|h|help, -l|log, -bz|bazaar,");
	WriteChatf("\aw[MQ2Collectible]          \at-cs|console, -e|expansion, -cn|collection -cl|collectible\n");
	WriteChatf("\aw[MQ2Collectible] \ayExample: \at/collectible need log collection \"The Plaguebringer's Chosen\"\aw Note that quotes are required.");
	WriteChatf("\aw[MQ2Collectible] \ayExample: \at/collectible -n -l -cn \"The Plaguebringer's Chosen\"\aw Same as above with abbreviated parameters.");
	WriteChatf("\aw[MQ2Collectible] \ayExample: \at/collectible -a -cs -cl \"frozen giant's toe\" ");
	WriteChatf("\aw[MQ2Collectible] \ayExample: \at/collectible -a -bz -e \"Terror of Luclin\"\aw Produces Bazaar.mac compatible logfile");
	WriteChatf("\aw[MQ2Collectible]          \awwith all collected and uncollected collectibles. A big list!");
	WriteChatf("\aw[MQ2Collectible] \ayLogfile name example: \atLogs\\Collectible\\MyCharName_servername_need_baz.log\n");
	WriteChatf("\aw[MQ2Collectible] \ayExample to console: \at/collectible -a -cs -cn \"Flame-Licked Clothing\"");
	WriteChatf("\aw[MQ2Collectible]                     \awOutputs the status of collectibles from that collection.\n");
	WriteChatf("\aw[MQ2Collectible] \ayTLO: \at${Collectible} Members: Collected|Status, ComponentID, Name, Expansion, Collection, FullCollection");
	WriteChatf("\aw[MQ2Collectible] \ayTLO: \atPartial collectible names are accepted. Case insensitive. The first match is returned. Use Name to see the full name.");
	WriteChatf("\aw[MQ2Collectible] \ayTLO: \at${Collectible[\"collectible name\"].Collected} returns -1 (not found), or 0|1 collected status.");
	WriteChatf("\aw[MQ2Collectible] \ayTLO: \at${Collectible[\"collectible name\"].Collection} Returns the collection name of the collectible or NULL.");
	WriteChatf("\aw[MQ2Collectible] \ayTLO: \at${Collectible[\"collectible name\"].ComponentID} Returns the component ID of the collectible.\n");
}

// ----------------------------------------------
// TLO Section
// ----------------------------------------------

// MQ2CollectibleType
class MQ2CollectibleType : public MQ2Type
{
	public:
		MQ2CollectibleType();

		void MQ2CollectibleType::GetCollectibleState(std::string_view CollectibleName);
		bool GetMember(MQVarPtr VarPtr, const char* Member, char* Index, MQTypeVar& Dest) override;
		static bool dataCollectible(const char* szIndex, MQTypeVar& Ret);
};
MQ2CollectibleType* pCollectibleType = nullptr;

struct COLLECTIBLEELEMENT
{
	std::string query;
	std::string previousQuery;
	int id;
	int status;
	std::string name;
	std::string expansion;
	std::string collection;
	std::string collectionExpansion;
};
COLLECTIBLEELEMENT* pCollectible = new COLLECTIBLEELEMENT;

enum class CollectibleMembers
{
	ComponentID = 1,
	Status,
	Collected,
	Name,
	Expansion,
	Collection,
	FullCollection,
};

MQ2CollectibleType::MQ2CollectibleType() : MQ2Type("collectible")
{
	ScopedTypeMember(CollectibleMembers, ComponentID);
	ScopedTypeMember(CollectibleMembers, Status);
	ScopedTypeMember(CollectibleMembers, Collected);
	ScopedTypeMember(CollectibleMembers, Name);
	ScopedTypeMember(CollectibleMembers, Expansion);
	ScopedTypeMember(CollectibleMembers, Collection);
	ScopedTypeMember(CollectibleMembers, FullCollection);
}

// Given the name of a collectible, find the collection it belongs to.
void MQ2CollectibleType::GetCollectibleState(std::string_view CollectibleName)
{
	pCollectible->previousQuery = pCollectible->query;

	// Find the category "Collections", then search through the achievements
	// components. When found, check that the component is not itself an achievement
	// before returning the achievement the collectible was found in.

	AchievementManager& AchMgr = AchievementManager::Instance();

	bool   bMatchFound = 0;

	for (const AchievementCategory& AchCat : AchMgr.categories)
	{
		if (!string_equals(AchCat.name, "Collections")) continue;

		// Secrets of Faydwer says Collections, but there aren't any. As of Terror of Luclin, Events and 10 expansions actually have real collections.
		if (!AchCat.GetAchievementCount()) continue;

		// Obtain the Expansion name.
		const AchievementCategory& AchParent = *AchMgr.GetAchievementCategoryById(AchCat.parentId);
		for (int x = 0; x < AchCat.GetAchievementCount(); ++x)
		{
			int AchIdx = AchMgr.GetAchievementIndexById(AchCat.GetAchievementId(x));
			const SingleAchievementAndComponentsInfo* AchCompInfo = AchMgr.GetAchievementClientInfoByIndex(AchIdx);

			// Anything nested under the achievement?
			if (!AchCompInfo) continue;

			const Achievement* Ach = AchMgr.GetAchievementByIndex(AchIdx);

			// Trimmed at parentheses, if present.
			std::string_view AchName = trimP(Ach->name);

			int CompTypeCt = Ach->componentsByType[AchievementComponentCompletion].GetCount();

			//std::string tempStr = fmt::format("Testing Search [{}] Trim [{}] AchName [{}]", szCollExpName, trimCollExpName, Ach->name.c_str());
			//WriteChatf("%s", tempStr);

			// List the collectibles
			for (int y = 0; y < CompTypeCt; y++)
			{
				const AchievementComponent& CompTypeCompletion = Ach->componentsByType[AchievementComponentCompletion][y];

				if (ci_find_substr(CompTypeCompletion.description, CollectibleName) < 0) continue;

				// We may have been given a partial collectible name; grab the name from the object.
				pCollectible->id = CompTypeCompletion.id;
				pCollectible->status = AchCompInfo->IsComponentComplete(AchievementComponentCompletion, y) ? 1 : 0;
				pCollectible->name = CompTypeCompletion.description;
				pCollectible->collection = AchName;
				pCollectible->expansion = AchParent.name;
				pCollectible->collectionExpansion = fmt::format("{}, {}", AchName, AchParent.name);

				return;
			}
		}
	}

	pCollectible->id = -1;
	pCollectible->status = -1;
	pCollectible->name = "-1";
	pCollectible->collection = "-1";
	pCollectible->expansion = "-1";
	pCollectible->collectionExpansion = "-1";

	return;
}

bool MQ2CollectibleType::GetMember(MQVarPtr VarPtr, const char* Member, char* Index, MQTypeVar& Dest)
{
	PcProfile* pProfile = GetPcProfile();
	if (!pProfile) return false;

	MQTypeMember* pMember = MQ2CollectibleType::FindMember(Member);
	if (!pMember) return false;

	switch (static_cast<CollectibleMembers>(pMember->ID))
	{
	case CollectibleMembers::ComponentID:
		if (pCollectible->query == pCollectible->previousQuery && pCollectible->status == 1)
		{
			Dest.DWord = pCollectible->id;
			Dest.Type = pIntType;

			return true;
		}

		GetCollectibleState(pCollectible->query);
		Dest.DWord = pCollectible->id;
		Dest.Type = pIntType;

		return true;

		// Alias of Collected
	case CollectibleMembers::Status:
		if (pCollectible->query == pCollectible->previousQuery && pCollectible->status == 1)
		{
			Dest.DWord = pCollectible->status;
			Dest.Type = pIntType;

			return true;
		}

		GetCollectibleState(pCollectible->query);
		Dest.DWord = pCollectible->status;
		Dest.Type = pIntType;

		return true;

	case CollectibleMembers::Collected:

		if (pCollectible->query == pCollectible->previousQuery && pCollectible->status == 1)
		{
			Dest.DWord = pCollectible->status;
			Dest.Type = pIntType;

			return true;
		}

		GetCollectibleState(pCollectible->query);
		Dest.DWord = pCollectible->status;
		Dest.Type = pIntType;

		return true;

	case CollectibleMembers::Name:

		if (pCollectible->query == pCollectible->previousQuery && pCollectible->status != -1)
		{
			strcpy_s(DataTypeTemp, pCollectible->name.c_str());
			Dest.Ptr = &DataTypeTemp[0];
			Dest.Type = pStringType;
			return true;
		}

		GetCollectibleState(pCollectible->query);
		strcpy_s(DataTypeTemp, pCollectible->name.c_str());
		Dest.Ptr = &DataTypeTemp[0];
		Dest.Type = pStringType;

		return true;

	case CollectibleMembers::Expansion:
		if (pCollectible->query == pCollectible->previousQuery && pCollectible->status != -1)
		{
			strcpy_s(DataTypeTemp, pCollectible->expansion.c_str());
			Dest.Ptr = &DataTypeTemp[0];
			Dest.Type = pStringType;
			return true;
		}

		GetCollectibleState(pCollectible->query);
		strcpy_s(DataTypeTemp, pCollectible->expansion.c_str());
		Dest.Ptr = &DataTypeTemp[0];
		Dest.Type = pStringType;

		return true;

	case CollectibleMembers::Collection:
		if (pCollectible->query == pCollectible->previousQuery && pCollectible->status != -1)
		{
			strcpy_s(DataTypeTemp, pCollectible->collection.c_str());
			Dest.Ptr = &DataTypeTemp[0];
			Dest.Type = pStringType;
			return true;
		}

		GetCollectibleState(pCollectible->query);
		strcpy_s(DataTypeTemp, pCollectible->collection.c_str());
		Dest.Ptr = &DataTypeTemp[0];
		Dest.Type = pStringType;

		return true;

	case CollectibleMembers::FullCollection:
		if (pCollectible->query == pCollectible->previousQuery && pCollectible->status != -1)
		{
			strcpy_s(DataTypeTemp, pCollectible->collectionExpansion.c_str());
			Dest.Ptr = &DataTypeTemp[0];
			Dest.Type = pStringType;
			return true;
		}

		GetCollectibleState(pCollectible->query);
		strcpy_s(DataTypeTemp, pCollectible->collectionExpansion.c_str());
		Dest.Ptr = &DataTypeTemp[0];
		Dest.Type = pStringType;

		return true;

	default:

		break;
	}

	return false;
}

bool MQ2CollectibleType::dataCollectible(const char* szIndex, MQTypeVar& Ret)
{
	pCollectible->query = trimP(szIndex);

	if (pCollectible->query.empty())
	{
		MacroError("[MQ2Collectible] Please provide a collectible name.");
		return false;
	}
	if (pCollectible->query.size() > 128)
	{
		MacroError("[MQ2Collectible] Collectible name must be no more than 128 characters.");
		return false;
	}

	Ret.DWord = 0;
	Ret.Type = pCollectibleType;

	return true;
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
	AddMQ2Data("Collectible", MQ2CollectibleType::dataCollectible);
	
	pCollectibleType = new MQ2CollectibleType;

	WriteChatf("\aw[MQ2Collectible] \at/collectible \aoby American Nero (https://gitlab.com/redguides/plugins/mq2collectible)");
}

PLUGIN_API void ShutdownPlugin()
{
	DebugSpewAlways("MQ2Collectible::Shutting down");

	RemoveCommand("/collectible");
	RemoveMQ2Data("Collectible");

	delete pCollectibleType;
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

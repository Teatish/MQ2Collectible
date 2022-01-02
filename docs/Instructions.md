MQ2Collectible



1) Download to your plugins folder

2) /plugin mq2collectible

3) /collectible for instructions



To unload, /plugin mq2collectible unload.





Usage



[CODE title="Basics"]

/collectible collected|need|all|help log|bazaar|console optional: expansion|collection|collectible "name"



${Collectible[collectible name].Collected|Status .Name .ComponentID .Expansion; .Collection .FullCollection}

[/CODE]



- /collectible parameters must be ordered as shown.

- The search "name" must be enclosed by quotes. Case insensitive. Partial match.

- The Achievement and its collectibles are returned when searching by Collectible.

- Logfiles are appended to.

- Quotes are not necessary with ${Collectible}



/collectible Parameters



- The first parameter defines whether you want to search for collected|completed, need|incomplete, or all. Abbreviations: -cd, -n, -a

- The second parameter directs the output to the console, log, or to a bazaar format logfile. Abbreviations: -cs, -l, -bz

- The third parameter sets the search to the collectible, collection, or expansion. Abbreviations: -cl, -cn, -e

- The "name" must be in quotes. It is not case sensitive, and it will perform partial search. The first match will be returned.

- Be prepared for a lot of output if you search by expansion (-e), or don't specify any search parameters which would then provide a complete dump of all achievements and collectibles.



/collectible Parameter Abbreviations



[CODE title="Abbrevs"]

-cd|collected

 -n|need

 -a|all

 -h|h|help|(no parameters)

 -l|log

-bz|bazaar

-cs|console

 -e|expansion

-cn|collection

-cl|collectible

[/CODE]



/collectible Bazaar Notes



When using the bazaar logfile option, please note:



- The bazaar logfile is provided for you to copy/paste to your Bazaar.ini. Results are appended, not replaced like an INI. Any additional parameters in the [collectible name] blocks are informational and have no bearing on Bazaar.mac.

- collected assumes you want to sell collectibles and provides default sell (2000000pp) parameters

- need assumes you want to buy collectibles and provides default buy (1pp) parameters

- all provides default buy (1pp) and sell (2000000pp) parameters

- Default buy and sell amounts are hardcoded.



${Collectible} Usage



 Returns 0|1 depending on whether it has been collected, or -1 if the collectible was not found. Partial name is accepted and returns the first collectible it matches. Case insensitive.



[CODE title="Basic usage"]${Collectible[collectible name].Collected}[/CODE]



Returns the name of the Expansion, Collection, or -1 if the collectible was not found.





[CODE title="More"]${Collectible[Broken Wrist Shackles].Status}         -> 0|1

${Collectible[Broken Wrist Shackles].Collected}      -> 0|1

${Collectible[Broken Wrist Shackles].ComponentID}    -> 290911301

${Collectible[broken wrist shack].Name}              -> Broken Wrist Shackles

${Collectible[Broken Wrist Shackles].Expansion}      -> Terror of Luclin

${Collectible[Broken Wrist Shackles].Collection}     -> Breaker of Chains

${Collectible[Broken Wrist Shackles].FullCollection} -> Breaker of Chains, Terror of Luclin

${Collectible[Brasse's Brassiere].Status}            -> -1

${Collectible[brasse's brassiere].Collection}        -> -1[/CODE]



Special



Enables you to view the status of collectibles when mousing over them. You may want to use something like boxhud.



[CODE title="MQ2Hud.ini"]Config\MQ2Hud.ini



[Elements]



CollectionItemMO = 19,100,100,255,255,255 ,${If[${EverQuest.LastMouseOver.MouseOver},${If[${FindItem[=${EverQuest.LastMouseOver.Tooltip}].Collectible},${If[${Collectible[${FindItem[=${EverQuest.LastMouseOver.Tooltip}]}].Collected},Collected,Need]},""]},""]}

CollectionItemMOB = 19,100,100,255,255,255 ,${If[${EverQuest.LastMouseOver.MouseOver},${If[${FindItem[=${EverQuest.LastMouseOver.Tooltip}].Collectible},${If[${Collectible[${FindItemBank[=${EverQuest.LastMouseOver.Tooltip}]}].Collected},Collected,Need]},""]},""]}[/CODE]
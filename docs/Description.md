MQ2Collectible



- Command /collectible searches Achievements and provides readable output on the status of Collections and Collectibles



- TLO ${Collectible[collectible name].Collected} provides Collectible status. Other properties available.





/collectible Examples



This is a drilldown to show the range of outputs and parameters.



Output a collection to the console:



1640398617146.png



This shows that "Hearing the Call" is an Achievement of collections. The output can be directed to a log:



1640399297128.png



[CODE]

[MQ2Collectible] Hearing the Call, Call of the Forsaken

-----------------------------------------------------------------------------------------

[INCOMPLETE] The Source of the Ethernere

[INCOMPLETE] Dead Relics

[INCOMPLETE] Bixie Hive Hodgepodge

[INCOMPLETE] The Darklight Palace

[/CODE]



Drilling down further:



1640399920667.png



We find another Achievement of collections. Continuing:



1640399989587.png



Finally a collection of collectibles. If you want to buy the collectibles you need, you can output to bazaar format, then copy and paste into your Bazaar.ini:



1640400275927.png



[CODE title="Baz Need"][Rusty Water Can]

Collected=0

Collection=Scattered Civilization (The Western Plains of Karana), Call of the Forsaken

BuyPriceMin=1

BuyPriceMax=1

MinBuyCount=1

[Scorched Cookware]

Collected=0

Collection=Scattered Civilization (The Western Plains of Karana), Call of the Forsaken

BuyPriceMin=1

BuyPriceMax=1

MinBuyCount=1



etc.

[/CODE]



You can output in bazaar format the collectibles you need, have collected, or just all:



1640401091554.png



[CODE title="All"][Rusty Water Can]

Collected=0

Collection=Scattered Civilization (The Western Plains of Karana), Call of the Forsaken

BuyPriceMin=1

BuyPriceMax=1

MinBuyCount=1

SellPriceMin=2000000

SellPriceMax=2000000

[Scorched Cookware]

Collected=1

Collection=Scattered Civilization (The Western Plains of Karana), Call of the Forsaken

BuyPriceMin=1

BuyPriceMax=1

MinBuyCount=1

SellPriceMin=2000000

SellPriceMax=2000000



etc.[/CODE]



Tip



Have a collectible, but can't remember what Achievement it is in?



1640401365243.png



Notice it was a search using a partial name. All collectibles with name containing "clutching" would have been output to the console.





${Collectible} Usage



The result of .Collected (or .Status) returns 0 (need) or 1 (collected). If not found, all members return -1.



[CODE]${Collectible[Broken Wrist Shackles].Status}         -> 0|1

${Collectible[Broken Wrist Shackles].Collected}      -> 0|1

${Collectible[Broken Wrist Shackles].ComponentID}    -> 290911301

${Collectible[broken wrist shack].Name}              -> Broken Wrist Shackles

${Collectible[Broken Wrist Shackles].Expansion}      -> Terror of Luclin

${Collectible[Broken Wrist Shackles].Collection}     -> Breaker of Chains

${Collectible[Broken Wrist Shackles].FullCollection} -> Breaker of Chains, Terror of Luclin

${Collectible[Brasse's Brassiere].Status}            -> -1

${Collectible[brasse's brassiere].Collection}        -> -1[/CODE]





Special



Enables you to view the status of collectibles when mousing over them.  You may want to use something like boxhud instead. This will show Collected/Need in the upper left corner.



[CODE title="MQ2Hud.ini"]Config\MQ2Hud.ini



[Elements]

CollectionItemMO = 19,100,100,255,255,255 ,${If[${EverQuest.LastMouseOver.MouseOver},${If[${FindItem[=${EverQuest.LastMouseOver.Tooltip}].Collectible},${If[${Collectible[${FindItem[=${EverQuest.LastMouseOver.Tooltip}]}].Collected},Collected,Need]},""]},""]}

CollectionItemMOB = 19,100,100,255,255,255 ,${If[${EverQuest.LastMouseOver.MouseOver},${If[${FindItem[=${EverQuest.LastMouseOver.Tooltip}].Collectible},${If[${Collectible[${FindItemBank[=${EverQuest.LastMouseOver.Tooltip}]}].Collected},Collected,Need]},""]},""]}

[/CODE]
# MQ2Collectible

- Command /collectible provides Collectible status and log output.
- TLO ${Collectible} provides Collectible status

## Usage

```
${Collectible[id|name].Collected(optional)} returns true/false

/collectible collected|need|both|help log|bazaar|console (optional: expansion|collection "name")
```

- Parameters must be ordered as shown.
- The "name" must be enclosed with quotations.
- Output to console requires the collection "name".
- Output to console not provided for expansion due to potential overflow and clogging in general.

### Alternate Parameters

```
-cd|collected
 -n|need
 -b|both
 -h|h|help|(no parameters)
 -l|log
-bz|bazaar
-cs|console
 -e|expansion
-cn|collection
```

## /collectible Examples

When using bazaar option, please note:

```
collected: assumes you want to sell collectibles
need     : assumes you want to buy collectibles
both     : makes no assumptions and populates with default buy (1pp) and sell (2000000pp) parameters
```

**/collectible collected log**

```
Outputs logfile of collectibles you have collected from all collections across all expansions:

Collectible_collected.ini

[Collectible Name]
ItemID=
Collected=1
Expansion=
Collection=
```

**/collectible need log**

```
Outputs logfile of collectibles you need from all collections across all expansions:

Collectible_need.ini

[Collectible Name]
ItemID=
Collected=0
Expansion=
Collection=
```

**/collectible need bazaar expansion Terror of Luclin**

```
Outputs Bazaar.mac compatible logfile with all the needed collectibles from the Terror of Luclin expansion:

Collectible_need_Terror_of_Luclin.ini

[Collectible Name]
ItemID=
Collected=0
Expansion=Terror of Luclin
Collection=
BuyPriceMin=1
BuyPriceMax=1
```

**/collectible collected bazaar collection Dead Relics**

```
Outputs Bazaar.mac compatible logfile with all the collected collectibles from the Dead Relics collection:

Collectible_collected_Dead_Relics.ini

[Collectible Name]
ItemID=
Collected=1
Expansion=Call of the Forsaken
Collection=Dead Relics
SellPriceMin=2000000
SellPriceMax=2000000
```

**/collectible both bazaar collection Headhunter (The Overthere)**

```
Outputs Bazaar.mac compatible logfile with all collectibles from the Headhunter (The Overthere) collection:

Collectible_both_Headhunter_(The_Overthere).ini

[Collectible Name]
id=
Collected=
Expansion=Ring of Scale
Collection=Headhunter (The Overthere)
BuyPriceMin=1
BuyPriceMax=1
SellPriceMin=2000000
SellPriceMax=2000000
```
## TLO Examples

```
/varset Coll ${Collectible[Brasse's Brassiere].Collected}
/echo ${Collectible[Clutching Foot]}
/echo ${Collectible[81723]}
```

```
MQ2Hud.ini

[Elements]
CollectionItemMOI = 7,30,0,255,255,255 ,${If[${EverQuest.LastMouseOver.MouseOver},${If[${FindItem[=${EverQuest.LastMouseOver.Tooltip}].Collectible},${If[${Collectible[${FindItem[=${EverQuest.LastMouseOver.Tooltip}]}]},Collected,Need]},""]},""]}
CollectionItemMOB = 7,30,0,255,255,255 ,${If[${EverQuest.LastMouseOver.MouseOver},${If[${FindItem[=${EverQuest.LastMouseOver.Tooltip}].Collectible},${If[${Collectible[${FindItemBank[=${EverQuest.LastMouseOver.Tooltip}]}]},Collected,Need]},""]},""]}
```

# MQ2Collectible

- Command /collectible provides Collectible status and log output.
- TLO ${Collectible} provides Collectible status

## Usage

```
/collectible collected|need|both|help log|bazaar|console (optional: expansion|collection "name")

${Collectible["name"].Collected(optional)} returns true/false
```

- Parameters must be ordered as shown.
- The "name" must be enclosed with quotations.
- Output to console requires the collection "name".
- Output to console not provided for expansion option due to potential overflow and clogging in general.

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

## Command Examples

When using the bazaar logfile option, please note:

- The bazaar logfile is provided for you to copy/paste to your Bazaar.ini. Any additional parameters in the [item] block are informational and have no bearing on Bazaar.mac. Let the maintainer know if the Bazaar.ini format changes.
- **collected** assumes you want to _sell_ collectibles and provides default sell (2000000pp) parameters
- **need** assumes you want to _buy_ collectibles and provides default buy (1pp) parameters
- **both** provides default _buy_ (1pp) and _sell_ (2000000pp) parameters

**/collectible collected console collection "Flame-Licked Clothing"**

```
Outputs to console:

Flame-Licked Clothing, Claws of Veeshan
---------------------------------------
Flame-Licked Trousers [collected]
...
...
...
Flame-Licked Belt [need]
```

**/collectible collected log**

```
Outputs logfile of collectibles you have collected from all collections across all expansions:

Collectible_server_charname_coll_log.ini

[Collectible Name]
Collected=1
Collection=
Expansion=
```

**/collectible need log**

```
Outputs logfile of collectibles you need from all collections across all expansions:

Collectible_server_charname_need_log.ini

[Collectible Name]
Collected=0
Collection=
Expansion=
```

**/collectible need bazaar expansion "Terror of Luclin"**

```
Outputs Bazaar.mac compatible logfile with all the needed collectibles from the Terror of Luclin expansion:

Collectible_server_charname_need_baz_Terror_of_Luclin.ini

[Collectible Name]
Collected=0
Collection=
Expansion=Terror of Luclin
BuyPriceMin=1
BuyPriceMax=1
```

**/collectible collected bazaar collection "Dead Relics"**

```
Outputs Bazaar.mac compatible logfile with all the collected collectibles from the Dead Relics collection:

Collectible_server_charname_collected_baz_Dead_Relics.ini

[Collectible Name]
Collected=1
Collection=Dead Relics
Expansion=Call of the Forsaken
SellPriceMin=2000000
SellPriceMax=2000000
```

**/collectible both bazaar collection "Headhunter (The Overthere)"**

```
Outputs Bazaar.mac compatible logfile with all collectibles from the Headhunter (The Overthere) collection:

Collectible_server_charname_both_baz_Headhunter_(The_Overthere).ini

[Collectible Name]
Collected=
Collection=Headhunter (The Overthere)
Expansion=Ring of Scale
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

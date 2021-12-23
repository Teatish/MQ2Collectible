# MQ2Collectible

- Command /collectible provides readable output on the status of Collectibles within Collections and by Expansion (or Events).
- TLO ${Collectible["collectible name"]} provides Collectible status

## Usage

```
/collectible collected|need|both|help log|bazaar|console (optional: expansion|collection "name")

${Collectible["collectible name"]}
```

- Parameters must be ordered as shown.
- The "name" must be enclosed by quotes.
- Logfiles are appended to.
- Use the entire collection name including parentheses, e.g. "Headhunter (The Overthere)"
- Collections of collections will slip into the results, e.g. "Dead Relics".

### Parameter Abbreviations

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

- The bazaar logfile is provided for you to copy/paste to your Bazaar.ini. Any additional parameters in the [collectible name] blocks are informational and have no bearing on Bazaar.mac.
- **collected** assumes you want to _sell_ collectibles and provides default sell (2000000pp) parameters
- **need** assumes you want to _buy_ collectibles and provides default buy (1pp) parameters
- **both** provides default _buy_ (1pp) and _sell_ (2000000pp) parameters
- Values are hardcoded. In the future, settings from Bazaar_settings.ini could be read.

**/collectible need console collection "Flame-Licked Clothing"**

**/collectible need log collection "Flame-Licked Clothing"**

Outputs to console, or to logfile:

Logs\Collectible\server_charname_need.ini
```
MQ2Collectible: Flame-Licked Clothing, Claws of Veeshan
-------------------------------------------------------------------------------
Flame-Licked Belt [NEED]
```

**/collectible -b -l -cn "Flame-Licked Clothing"**

Logfile: Logs\Collectible\server_charname_both.ini
```
[MQ2Collectible] Flame-Licked Clothing, Claws of Veeshan
-----------------------------------------------------------------------------------------
[COLLECTED] Flame-Licked Trousers
[COLLECTED] Flame-Licked Vest
[COLLECTED] Flame-Licked Cuffs
[NEED]      Flame-Licked Boots
[COLLECTED] Flame-Licked Undershirt
[COLLECTED] Flame-Licked Cap
[NEED]      Flame-Licked Pouch
[NEED]      Flame-Licked Belt
```

**/collectible -b -bz -e "Terror of Luclin"**

Outputs Bazaar.mac compatible logfile with _all_ the collectibles from the Terror of Luclin expansion, a very long list:

Logfile: Logs\Collectible\server_charname_need_baz.ini
```
[Strangely Worded Note about Hearth]
Collected=0
Collection=Found Paper, Terror of Luclin
BuyPriceMin=1
BuyPriceMax=1
MinBuyCount=1
SellPriceMin=2000000
SellPriceMax=2000000
[Broken Wrist Shackles]
Collected=1
Collection=Breaker of Chains, Terror of Luclin
BuyPriceMin=1
BuyPriceMax=1
MinBuyCount=1
SellPriceMin=2000000
SellPriceMax=2000000

etc.
```

## TLO Examples

 Returns -1 if the collectible is not found, otherwise 0|1 depending on whether it has been collected.
```
${Collectible["collectible name"]}
```

**MacroScript**

Oopsies... this will return -1
```
/varset Coll ${Collectible["Brasse's Brassiere"]}
```

Returns 0|1 depending on its status.
```
/echo ${Collectible["Clutching Foot"]}
```

**LUA**

Oopsies...typo... returns -1
```
mq = require('mq')
CollectibleStatus = mq.TLO.Collectible('Blame-Licked Belt')
```

Thumbs Up. Returns 0|1
```
mq = require('mq')
CollectibleStatus = mq.TLO.Collectible('Flame-Licked Belt')
```

**Config\MQ2Hud.ini**

You may want to use something like boxhud instead... or perhaps that functionality will be added here.

Enables you to view the status of collectibles when mousing over them.
```
[Elements]
CollectionItemMOI = 7,30,0,255,255,255 ,${If[${EverQuest.LastMouseOver.MouseOver},${If[${FindItem[=${EverQuest.LastMouseOver.Tooltip}].Collectible},${If[${Collectible[${FindItem[=${EverQuest.LastMouseOver.Tooltip}]}]},Collected,Need]},""]},""]}
CollectionItemMOB = 7,30,0,255,255,255 ,${If[${EverQuest.LastMouseOver.MouseOver},${If[${FindItem[=${EverQuest.LastMouseOver.Tooltip}].Collectible},${If[${Collectible[${FindItemBank[=${EverQuest.LastMouseOver.Tooltip}]}]},Collected,Need]},""]},""]}
```

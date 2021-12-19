# MQ2Collectible

- Command /collectible provides log output on the status of Collectibles within Collections and Expansions.
- TLO ${Collectible["collectible name"]} provides Collectible status

## Usage

```
/collectible collected|need|both|help log|bazaar|console (optional: expansion|collection "name")

${Collectible["collectible name"]} returns true/false
```

- Parameters must be ordered as shown.
- The "name" must be enclosed with quotes.
- Logfiles are appended to.
- Be sure to use the entire collection name, including parentheses, e.g. "Headhunter (The Overthere)"

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

Outputs to console (-cs), or to logfile (-l): Logs\Collectible\server_charname_need.ini
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

**/collectible need bazaar expansion "Terror of Luclin"**

Outputs Collectibles from _all_ collections across _all_ expansions. I am going to spare you the several thousand line dump. DIY!

Logfile: Logs\Collectible\server_charname_need_baz.log

**/collectible -n -bz -e "Terror of Luclin"**

Outputs Bazaar.mac compatible logfile with _all_ the needed collectibles from the Terror of Luclin expansion (actual results not shown):

Logfile: Logs\Collectible\server_charname_need_baz.ini
```
[Collectible Name]
Collected=0
Collection=collection name, Terror of Luclin
BuyPriceMin=1
BuyPriceMax=1
MinBuyCount=1
[Collectible Name]
Collected=0
Collection=collection name, Terror of Luclin
BuyPriceMin=1
BuyPriceMax=1
MinBuyCount=1

etc.
```

**/collectible collected bazaar collection "Dead Relics"**
Outputs Bazaar.mac compatible logfile with all the collectibles from the Dead Relics collection:

Logfile: Logs\Collectible\server_charname_coll_baz.ini
```
[The Plaguebringer's Chosen]
Collected=1
Collection=Dead Relics, Call of the Forsaken
BuyPriceMin=1
BuyPriceMax=1
MinBuyCount=1
SellPriceMin=2000000
SellPriceMax=2000000
[The Spared]
Collected=0
Collection=Dead Relics, Call of the Forsaken
BuyPriceMin=1
BuyPriceMax=1
MinBuyCount=1
SellPriceMin=2000000
SellPriceMax=2000000
[Kyle's Cadre]
Collected=0
Collection=Dead Relics, Call of the Forsaken
BuyPriceMin=1
BuyPriceMax=1
MinBuyCount=1
SellPriceMin=2000000
SellPriceMax=2000000
[Remnants of the Unlucky]
Collected=1
Collection=Dead Relics, Call of the Forsaken
BuyPriceMin=1
BuyPriceMax=1
MinBuyCount=1
SellPriceMin=2000000
SellPriceMax=2000000
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

Thumbs Up - returns 0|1 depending on its status.
```
/echo ${Collectible["Clutching Foot"]}
```

**LUA**

Oopsies...
```
mq = require('mq')
CollectibleStatus = mq.TLO.Collectible('Blame-Licked Belt')
```

Thumbs Up
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

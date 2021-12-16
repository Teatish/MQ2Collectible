# MQ2Collectible

- New TLO ${Collectible} provides Collectible status
- New command /collectible provides Collectible status and log output.

## Usage

```
${Collectible[id|name].Collected(optional, assumed if absent)} returns true/false

/collectible collected|need|both|help log|bazaar (optional: expansion|collection name)
```

Parameters must be ordered as shown.

### Convenience Abbreviations

```
collected|c|coll
need|n
both|b
help|h|(empty)
log|l
bazaar|baz|b
expansion|exp|e
collection|c|coll
```

## TLO Examples

```
${Collectible[...]} returns true/false
${Collectible[...].Collected} returns true/false
```

```
${Collectible[Brasse's Brassiere].Collected}
${Collectible[Clutching Foot]}
${Collectible[81723]}
```

```
MQ2Hud.ini

[Elements]
CollectionItemMO = 7,30,0,255,255,255 ,${If[${EverQuest.LastMouseOver.MouseOver},${If[${FindItem[=${EverQuest.LastMouseOver.Tooltip}].Collectible},${If[${Collectible[${FindItem[=${EverQuest.LastMouseOver.Tooltip}]}]},Collected,Need]},""]},""]}
```

### /collectible Examples

When using bazaar option, please note:

```
collected: assumes you want to sell collectibles
need     : assumes you want to buy collectibles
both     : makes no assumptions and populates with buy and sell parameters
```

/collectible collected log

Outputs logfile of collectibles you have collected from all achievements across all expansions:
Collectible_Collected.ini

[Collectible Name]
ItemID=
Collected=1
Expansion=
Collection=
```

```
/collectible need log

Outputs logfile of collectibles you need from all achievements across all expansions:
Collectible_Need.ini

[Collectible Name]
ItemID=
Collected=0
Expansion=
Collection=
```

```
/collectible need bazaar expansion Terror of Luclin

Outputs Bazaar.mac compatible logfile with all the needed collectibles from the Terror of Luclin expansion:
Collectible_Need_Terror_of_Luclin.ini

[Collectible Name]
Collected=0
BuyPriceMin=1
BuyPriceMax=1
```

```
/collectible collected bazaar collection Dead Relics

Outputs Bazaar.mac compatible logfile with all the collected collectibles from the Dead Relics achievement:
Collectible_Need_Dead_Relics.ini

[Collectible Name]
Collected=0
BuyPriceMin=1
BuyPriceMax=1
```


```
/collectible both bazaar collection Headhunter (The Overthere)

Outputs logfile Collectible_Both_Headhunter_(The_Overthere).ini

[Collectible Name]
Collected=0|1
BuyPriceMin=1
BuyPriceMax=1
SellPriceMin=2000000
SellPriceMax=2000000
```

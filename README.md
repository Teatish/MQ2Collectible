# MQ2Collectible

Provides Collectible status and log output.

## Usage

```
${Collectible[id|name].Collected(optional, assumed if absent)} returns true/false

/collectible need|collected log|bazaar (optional: expansion|achievement name)
```

### Example Usage TLO

```
${Collectible[]} returns true/false
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

### Example Usage Slash Command
```
/collectible collected log

Outputs logfile Collectible_Collected.ini

[Collectible Name]
ItemID=
Expansion=
Collection=
```

```
/collectible need log

Outputs logfile Collectible_Need.ini

[Collectible Name]
ItemID=
Expansion=
Collection=
```

```
/collectible need bazaar expansion Terror of Luclin

Outputs logfile Collectible_Need_Terror_of_Luclin.ini using Bazaar.mac format:

[Collectible Name]
BuyPriceMin=1
BuyPriceMax=1
```

```
/collectible collected bazaar collection Dead Relics

Outputs logfile Collectible_Need_Dead_Relics.ini

[Collectible Name]
SellPriceMin=2000000
BuyPriceMax=2000000
```

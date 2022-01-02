[CODE title="Commands"]/collectible



${Collectible}[/CODE]

[CODE]Config\MQ2Hud.ini



[Elements]



CollectionItemMO = 19,100,100,255,255,255 ,${If[${EverQuest.LastMouseOver.MouseOver},${If[${FindItem[=${EverQuest.LastMouseOver.Tooltip}].Collectible},${If[${Collectible[${FindItem[=${EverQuest.LastMouseOver.Tooltip}]}].Collected},Collected,Need]},""]},""]}

CollectionItemMOB = 19,100,100,255,255,255 ,${If[${EverQuest.LastMouseOver.MouseOver},${If[${FindItem[=${EverQuest.LastMouseOver.Tooltip}].Collectible},${If[${Collectible[${FindItemBank[=${EverQuest.LastMouseOver.Tooltip}]}].Collected},Collected,Need]},""]},""]}



[/CODE]



"/collectible" in the console provides help.
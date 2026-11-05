# Exception analysis

Unique facts merge the same fact printed in several forms (counts per setting below). Scores of `dist` and `freq` are on different scales, so they are compared by rank overlap only.

## nell_dist_o0

- queries: 70, with at least one exception: 64
- exceptions: 18762
- distinct triples as printed: 2961
- after resolving direction (the same fact reported from both ends of an edge, 'A X-> B' and 'B X<- A'): 2843
- after merging inverse relations (NELL's 'inverseof' prefix; no effect if the dataset has none): 2715
- unique facts used in the tables below (--merge inverse): 2715 (0.8553 of the exceptions merged into another one)
- types: LabelException 12842, LinkException 5114
- score: median 5221, p90 1.187e+05, p99 3.767e+05, max 2.672e+06

### Per rule

| rule | queries | with exc. | exceptions | unique | merged share | exc./query (median, max) | score median | p90 | p99 | max | Label share |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ρ0 | 10 | 10 | 980 | 873 | 0.1092 | 105, 108 | 140.4 | 3888 | 4.688e+04 | 1.08e+05 | 0.5501 |
| ρ1 | 10 | 10 | 2708 | 656 | 0.7578 | 117, 1572 | 3375 | 4030 | 3.049e+04 | 1.08e+05 | 0.9054 |
| ρ2 | 10 | 10 | 1360 | 379 | 0.7213 | 116, 236 | 1239 | 4.675e+04 | 1.333e+05 | 1.333e+05 | 0.6719 |
| ρ3 | 10 | 5 | 492 | 440 | 0.1057 | 36, 116 | 53.58 | 641.6 | 8540 | 1.117e+04 | 0.6682 |
| ρ4 | 10 | 10 | 2206 | 131 | 0.9406 | 140, 636 | 2278 | 3.991e+04 | 3.767e+05 | 2.672e+06 | 0.7733 |
| ρ5 | 10 | 9 | 1368 | 135 | 0.9013 | 112, 352 | 2.599e+04 | 3.767e+05 | 6.56e+05 | 1.153e+06 | 0.7284 |
| ρ6 | 10 | 10 | 9648 | 101 | 0.9895 | 632, 3420 | 2.622e+04 | 1.418e+05 | 1.418e+05 | 1.925e+05 | 0.6733 |

### Top 15 exceptions (unique facts)

| score | fact | type | query | context of the query |
|---|---|---|---|---|
| 2.672e+06 | sportsteam:louis_cardinals -[organizationacronymhasname]-> sportsteam:st__louis_cardinals | LinkException | p48 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * |
| 2.672e+06 | sportsteam:st__louis_cardinals -[organizationnamehasacronym]-> sportsteam:louis_cardinals | LinkException | p48 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * |
| 6.56e+05 | sportsteam:notre_dame -[teamplaysagainstteam]-> sportsteam:falcons | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 6.56e+05 | sportsteam:falcons -[teamplaysagainstteam]-> sportsteam:notre_dame | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.79e+05 | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.79e+05 | sport:baseball -[players]-> athlete:alfonso_soriano | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.79e+05 | coach:andy_green -[athleteplayssport]-> sport:basketball | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.79e+05 | sport:basketball -[players]-> coach:andy_green | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.484e+05 | sportsteam:rutgers -[teamplaysagainstteam]-> sportsteam:knicks | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.484e+05 | sportsteam:knicks -[teamplaysagainstteam]-> sportsteam:rutgers | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.484e+05 | sportsteam:notre_dame -[teamplaysagainstteam]-> sportsteam:boston_bruins | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.484e+05 | sportsteam:boston_bruins -[teamplaysagainstteam]-> sportsteam:notre_dame | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.282e+05 | sportsteam:chicago_bulls -[teamplaysagainstteam]-> sportsteam:rutgers | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.282e+05 | sportsteam:rutgers -[teamplaysagainstteam]-> sportsteam:chicago_bulls | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 3.767e+05 | sportsteamposition:center -[sportsteampositionathlete]-> personmexico:carlos_arroyo | LinkException | p43 | male -[fatherofperson]-> *; male -[*]-> * |

### Top 15 contexts, each with its strongest exception (one fact per context)

| # | score | context | query | strongest fact | type | tied | exceptions in context |
|---|---|---|---|---|---|---|---|
| 1 | 2.672e+06 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * | p48 (ρ4) | sportsteam:louis_cardinals -[organizationacronymhasname]-> sportsteam:st__louis_cardinals | LinkException | +1 | 100 |
| 2 | 1.153e+06 | * -[*]-> person; * -[*]-> personnorthamerica | p55 (ρ5) | sportsteam:louis_cardinals -[organizationacronymhasname]-> sportsteam:st__louis_cardinals | LinkException | +1 | 112 |
| 3 | 4.49e+05 | * -[*]-> person; * -[*]-> personeurope | p56 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 112 |
| 4 | 4.426e+05 | * -[*]-> person; * -[*]-> personasia | p59 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 112 |
| 5 | 4.34e+05 | * -[*]-> person; * -[*]-> personcanada | p57 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 112 |
| 6 | 4.217e+05 | * -[*]-> person; * -[*]-> personafrica | p58 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 112 |
| 7 | 3.767e+05 | male -[fatherofperson]-> *; male -[*]-> * | p43 (ρ4) | sportsteamposition:center -[sportsteampositionathlete]-> personmexico:carlos_arroyo | LinkException | +1 | 108 |
| 8 | 1.925e+05 | grain -[*]-> *; grain -[*]-> * | p67 (ρ6) | animal:lamb -[animaleatfood]-> beverage:milk | LinkException |  | 1128 |
| 9 | 1.333e+05 | language -[*]-> * | p25 (ρ2) | country:canadians_canada -[organizationacronymhasname]-> country:the_united_kingdom | LinkException | +7 | 104 |
| 10 | 1.258e+05 | animal -[*]-> * | p29 (ρ2) | food:oil -[agriculturalproductincludingagriculturalproduct]-> vegetable:pepper | LinkException |  | 236 |
| 11 | 1.08e+05 | emotion -[emotionassociatedwithdisease]-> disease | p09 (ρ0) | disease:side_effects -[sideeffectcausedbydrug]-> bacteria:side_effects | LabelException | +1 | 100 |
| 12 | 1.08e+05 | emotion -[emotionassociatedwithdisease]-> * | p19 (ρ1) | disease:side_effects -[sideeffectcausedbydrug]-> bacteria:side_effects | LabelException | +1 | 102 |
| 13 | 7.488e+04 | musicfestival -[*]-> * | p22 (ρ2) | year:n1998 -[synonymfor]-> country:tanzania | LinkException | +1 | 180 |
| 14 | 7.069e+04 | drug -[drugworkedonbyagent]-> biotechcompany | p08 (ρ0) | drug:lipitor -[producedby]-> biotechcompany:pfizer | LinkException/UniqueEntityException | +5 | 108 |
| 15 | 7.069e+04 | drug -[drugworkedonbyagent]-> * | p18 (ρ1) | drug:lipitor -[producedby]-> biotechcompany:pfizer | LinkException/UniqueEntityException | +5 | 134 |

### Most frequent links (unique facts)

| link | facts | share | mean score | queries |
|---|---|---|---|---|
| agentcollaborateswithagent | 136 | 0.05009 | 3262 | 19 |
| dateof | 128 | 0.04715 | 2045 | 7 |
| atdate | 128 | 0.04715 | 2045 | 7 |
| agentcompeteswithagent | 96 | 0.03536 | 2196 | 11 |
| instrumentplayedbymusician | 63 | 0.0232 | 1296 | 5 |
| musicianplaysinstrument | 63 | 0.0232 | 1296 | 5 |
| agentinvolvedwithitem | 63 | 0.0232 | 2621 | 7 |
| iteminvolvedwithagent | 63 | 0.0232 | 2621 | 7 |
| synonymfor | 62 | 0.02284 | 1.476e+04 | 12 |
| physiologicalconditionpossiblytreatedbydrug | 46 | 0.01694 | 2428 | 2 |
| drugpossiblytreatsphysiologicalcondition | 46 | 0.01694 | 2428 | 2 |
| politicianusendorsespoliticianus | 41 | 0.0151 | 48.39 | 3 |
| politicianusendorsedbypoliticianus | 41 | 0.0151 | 48.39 | 3 |
| agentcontrols | 40 | 0.01473 | 2169 | 13 |
| controlledbyagent | 40 | 0.01473 | 2169 | 13 |

### Most frequent head and tail labels

| head label | facts | tail label | facts |
|---|---|---|---|
| country | 183 | country | 187 |
| musician | 144 | musician | 149 |
| city | 134 | disease | 138 |
| disease | 133 | city | 134 |
| politicianus | 122 | politicianus | 122 |
| musicinstrument | 119 | musicinstrument | 119 |
| athlete | 103 | dateliteral | 92 |
| dateliteral | 92 | athlete | 89 |
| mammal | 83 | mammal | 84 |
| drug | 74 | drug | 84 |
| politicsblog | 70 | politicsblog | 70 |
| website | 61 | website | 61 |
| journalist | 57 | journalist | 57 |
| company | 53 | biotechcompany | 53 |
| biotechcompany | 53 | company | 52 |

### Entities in exceptions of the most queries

| entity | queries | max score |
|---|---|---|
| america | 8 | 1.333e+05 |
| texas | 8 | 7.069e+04 |
| n2005 | 8 | 4.56e+04 |
| alfonso_soriano | 7 | 4.79e+05 |
| andy_green | 7 | 4.79e+05 |
| mlb | 7 | 2.079e+05 |
| baseball | 6 | 4.79e+05 |
| center | 6 | 3.767e+05 |
| carlos_arroyo | 6 | 3.767e+05 |
| n2006 | 6 | 7.069e+04 |
| world | 6 | 5.544e+04 |
| elvis_presley | 6 | 5.544e+04 |
| rosanne_cash | 6 | 1.748e+04 |
| n1998 | 5 | 7.517e+04 |
| state_university | 5 | 3.991e+04 |

### Queries with the most and the strongest exceptions

| query | rule | context | matching heads | exceptions | max score | time [s] |
|---|---|---|---|---|---|---|
| p64 | ρ6 | item -[*]-> *; item -[*]-> * | - | 3420 | 2.622e+04 | 330.6 |
| p66 | ρ6 | parlourgame -[*]-> *; parlourgame -[*]-> * | - | 2064 | 1.296e+04 | 1.198 |
| p16 | ρ1 | item -[instrumentplayedbymusician]-> * | - | 1572 | 2.622e+04 | 0.153 |
| p67 | ρ6 | grain -[*]-> *; grain -[*]-> * | - | 1128 | 1.925e+05 | 199.5 |
| p62 | ρ6 | wine -[*]-> *; wine -[*]-> * | - | 904 | 6.732e+04 | 31.1 |
| p68 | ρ6 | landscapefeatures -[*]-> *; landscapefeatures -[*]-> * | - | 816 | 2.47e+04 | 38.31 |
| p42 | ρ4 | journalist -[personleadsgeopoliticalorganization]-> *; journalist -[*]-> * | - | 636 | 1 | 26.11 |
| p45 | ρ4 | professor -[agentbelongstoorganization]-> *; professor -[*]-> * | - | 466 | 2.351e+04 | 167.5 |
| p65 | ρ6 | traditionalgame -[*]-> *; traditionalgame -[*]-> * | - | 448 | 1.296e+04 | 1.163 |
| p69 | ρ6 | hallwayitem -[*]-> *; hallwayitem -[*]-> * | - | 416 | 22.5 | 1.15 |
| p54 | ρ5 | * -[*]-> musician; * -[*]-> musicartist | - | 352 | 2.622e+04 | 3.144 |
| p29 | ρ2 | animal -[*]-> * | - | 236 | 1.258e+05 | 21.88 |
| p17 | ρ1 | item -[iteminvolvedwithagent]-> * | - | 228 | 3375 | 0.053 |
| p40 | ρ4 | videogame -[producedby]-> *; videogame -[*]-> * | - | 224 | 1 | 0.22 |
| p52 | ρ5 | * -[*]-> musician; * -[*]-> musicalbum | - | 208 | 25.11 | 0.461 |

## nell_dist_o1

- queries: 70, with at least one exception: 64
- exceptions: 18762
- distinct triples as printed: 2961
- after resolving direction (the same fact reported from both ends of an edge, 'A X-> B' and 'B X<- A'): 2843
- after merging inverse relations (NELL's 'inverseof' prefix; no effect if the dataset has none): 2715
- unique facts used in the tables below (--merge inverse): 2715 (0.8553 of the exceptions merged into another one)
- types: LabelException 12842, LinkException 5114
- score: median 5221, p90 1.187e+05, p99 3.767e+05, max 2.672e+06

### Per rule

| rule | queries | with exc. | exceptions | unique | merged share | exc./query (median, max) | score median | p90 | p99 | max | Label share |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ρ0 | 10 | 10 | 980 | 873 | 0.1092 | 105, 108 | 140.4 | 3888 | 4.688e+04 | 1.08e+05 | 0.5501 |
| ρ1 | 10 | 10 | 2708 | 656 | 0.7578 | 117, 1572 | 3375 | 4030 | 3.049e+04 | 1.08e+05 | 0.9054 |
| ρ2 | 10 | 10 | 1360 | 379 | 0.7213 | 116, 236 | 1239 | 4.675e+04 | 1.333e+05 | 1.333e+05 | 0.6719 |
| ρ3 | 10 | 5 | 492 | 440 | 0.1057 | 36, 116 | 53.58 | 641.6 | 8540 | 1.117e+04 | 0.6682 |
| ρ4 | 10 | 10 | 2206 | 131 | 0.9406 | 140, 636 | 2278 | 3.991e+04 | 3.767e+05 | 2.672e+06 | 0.7733 |
| ρ5 | 10 | 9 | 1368 | 135 | 0.9013 | 112, 352 | 2.599e+04 | 3.767e+05 | 6.56e+05 | 1.153e+06 | 0.7284 |
| ρ6 | 10 | 10 | 9648 | 101 | 0.9895 | 632, 3420 | 2.622e+04 | 1.418e+05 | 1.418e+05 | 1.925e+05 | 0.6733 |

### Top 15 exceptions (unique facts)

| score | fact | type | query | context of the query |
|---|---|---|---|---|
| 2.672e+06 | sportsteam:louis_cardinals -[organizationacronymhasname]-> sportsteam:st__louis_cardinals | LinkException | p48 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * |
| 2.672e+06 | sportsteam:st__louis_cardinals -[organizationnamehasacronym]-> sportsteam:louis_cardinals | LinkException | p48 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * |
| 6.56e+05 | sportsteam:notre_dame -[teamplaysagainstteam]-> sportsteam:falcons | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 6.56e+05 | sportsteam:falcons -[teamplaysagainstteam]-> sportsteam:notre_dame | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.79e+05 | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.79e+05 | sport:baseball -[players]-> athlete:alfonso_soriano | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.79e+05 | coach:andy_green -[athleteplayssport]-> sport:basketball | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.79e+05 | sport:basketball -[players]-> coach:andy_green | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.484e+05 | sportsteam:rutgers -[teamplaysagainstteam]-> sportsteam:knicks | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.484e+05 | sportsteam:knicks -[teamplaysagainstteam]-> sportsteam:rutgers | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.484e+05 | sportsteam:notre_dame -[teamplaysagainstteam]-> sportsteam:boston_bruins | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.484e+05 | sportsteam:boston_bruins -[teamplaysagainstteam]-> sportsteam:notre_dame | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.282e+05 | sportsteam:chicago_bulls -[teamplaysagainstteam]-> sportsteam:rutgers | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 4.282e+05 | sportsteam:rutgers -[teamplaysagainstteam]-> sportsteam:chicago_bulls | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 3.767e+05 | sportsteamposition:center -[sportsteampositionathlete]-> personmexico:carlos_arroyo | LinkException | p43 | male -[fatherofperson]-> *; male -[*]-> * |

### Top 15 contexts, each with its strongest exception (one fact per context)

| # | score | context | query | strongest fact | type | tied | exceptions in context |
|---|---|---|---|---|---|---|---|
| 1 | 2.672e+06 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * | p48 (ρ4) | sportsteam:louis_cardinals -[organizationacronymhasname]-> sportsteam:st__louis_cardinals | LinkException | +1 | 100 |
| 2 | 1.153e+06 | * -[*]-> person; * -[*]-> personnorthamerica | p55 (ρ5) | sportsteam:louis_cardinals -[organizationacronymhasname]-> sportsteam:st__louis_cardinals | LinkException | +1 | 112 |
| 3 | 4.49e+05 | * -[*]-> person; * -[*]-> personeurope | p56 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 112 |
| 4 | 4.426e+05 | * -[*]-> person; * -[*]-> personasia | p59 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 112 |
| 5 | 4.34e+05 | * -[*]-> person; * -[*]-> personcanada | p57 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 112 |
| 6 | 4.217e+05 | * -[*]-> person; * -[*]-> personafrica | p58 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 112 |
| 7 | 3.767e+05 | male -[fatherofperson]-> *; male -[*]-> * | p43 (ρ4) | sportsteamposition:center -[sportsteampositionathlete]-> personmexico:carlos_arroyo | LinkException | +1 | 108 |
| 8 | 1.925e+05 | grain -[*]-> *; grain -[*]-> * | p67 (ρ6) | animal:lamb -[animaleatfood]-> beverage:milk | LinkException |  | 1128 |
| 9 | 1.333e+05 | language -[*]-> * | p25 (ρ2) | country:canadians_canada -[organizationacronymhasname]-> country:the_united_kingdom | LinkException | +7 | 104 |
| 10 | 1.258e+05 | animal -[*]-> * | p29 (ρ2) | food:oil -[agriculturalproductincludingagriculturalproduct]-> vegetable:pepper | LinkException |  | 236 |
| 11 | 1.08e+05 | emotion -[emotionassociatedwithdisease]-> disease | p09 (ρ0) | disease:side_effects -[sideeffectcausedbydrug]-> bacteria:side_effects | LabelException | +1 | 100 |
| 12 | 1.08e+05 | emotion -[emotionassociatedwithdisease]-> * | p19 (ρ1) | disease:side_effects -[sideeffectcausedbydrug]-> bacteria:side_effects | LabelException | +1 | 102 |
| 13 | 7.488e+04 | musicfestival -[*]-> * | p22 (ρ2) | year:n1998 -[synonymfor]-> country:tanzania | LinkException | +1 | 180 |
| 14 | 7.069e+04 | drug -[drugworkedonbyagent]-> biotechcompany | p08 (ρ0) | drug:lipitor -[producedby]-> biotechcompany:pfizer | LinkException/UniqueEntityException | +5 | 108 |
| 15 | 7.069e+04 | drug -[drugworkedonbyagent]-> * | p18 (ρ1) | drug:lipitor -[producedby]-> biotechcompany:pfizer | LinkException/UniqueEntityException | +5 | 134 |

### Most frequent links (unique facts)

| link | facts | share | mean score | queries |
|---|---|---|---|---|
| agentcollaborateswithagent | 136 | 0.05009 | 3262 | 19 |
| dateof | 128 | 0.04715 | 2045 | 7 |
| atdate | 128 | 0.04715 | 2045 | 7 |
| agentcompeteswithagent | 96 | 0.03536 | 2196 | 11 |
| instrumentplayedbymusician | 63 | 0.0232 | 1296 | 5 |
| musicianplaysinstrument | 63 | 0.0232 | 1296 | 5 |
| agentinvolvedwithitem | 63 | 0.0232 | 2621 | 7 |
| iteminvolvedwithagent | 63 | 0.0232 | 2621 | 7 |
| synonymfor | 62 | 0.02284 | 1.476e+04 | 12 |
| physiologicalconditionpossiblytreatedbydrug | 46 | 0.01694 | 2428 | 2 |
| drugpossiblytreatsphysiologicalcondition | 46 | 0.01694 | 2428 | 2 |
| politicianusendorsespoliticianus | 41 | 0.0151 | 48.39 | 3 |
| politicianusendorsedbypoliticianus | 41 | 0.0151 | 48.39 | 3 |
| agentcontrols | 40 | 0.01473 | 2169 | 13 |
| controlledbyagent | 40 | 0.01473 | 2169 | 13 |

### Most frequent head and tail labels

| head label | facts | tail label | facts |
|---|---|---|---|
| country | 183 | country | 187 |
| musician | 144 | musician | 149 |
| city | 134 | disease | 138 |
| disease | 133 | city | 134 |
| politicianus | 122 | politicianus | 122 |
| musicinstrument | 119 | musicinstrument | 119 |
| athlete | 103 | dateliteral | 92 |
| dateliteral | 92 | athlete | 89 |
| mammal | 83 | mammal | 84 |
| drug | 74 | drug | 84 |
| politicsblog | 70 | politicsblog | 70 |
| website | 61 | website | 61 |
| journalist | 57 | journalist | 57 |
| company | 53 | biotechcompany | 53 |
| biotechcompany | 53 | company | 52 |

### Entities in exceptions of the most queries

| entity | queries | max score |
|---|---|---|
| america | 8 | 1.333e+05 |
| texas | 8 | 7.069e+04 |
| n2005 | 8 | 4.56e+04 |
| alfonso_soriano | 7 | 4.79e+05 |
| andy_green | 7 | 4.79e+05 |
| mlb | 7 | 2.079e+05 |
| baseball | 6 | 4.79e+05 |
| center | 6 | 3.767e+05 |
| carlos_arroyo | 6 | 3.767e+05 |
| n2006 | 6 | 7.069e+04 |
| world | 6 | 5.544e+04 |
| elvis_presley | 6 | 5.544e+04 |
| rosanne_cash | 6 | 1.748e+04 |
| n1998 | 5 | 7.517e+04 |
| state_university | 5 | 3.991e+04 |

### Queries with the most and the strongest exceptions

| query | rule | context | matching heads | exceptions | max score | time [s] |
|---|---|---|---|---|---|---|
| p64 | ρ6 | item -[*]-> *; item -[*]-> * | - | 3420 | 2.622e+04 | 449.7 |
| p66 | ρ6 | parlourgame -[*]-> *; parlourgame -[*]-> * | - | 2064 | 1.296e+04 | 1.763 |
| p16 | ρ1 | item -[instrumentplayedbymusician]-> * | - | 1572 | 2.622e+04 | 0.311 |
| p67 | ρ6 | grain -[*]-> *; grain -[*]-> * | - | 1128 | 1.925e+05 | 597.6 |
| p62 | ρ6 | wine -[*]-> *; wine -[*]-> * | - | 904 | 6.732e+04 | 46.18 |
| p68 | ρ6 | landscapefeatures -[*]-> *; landscapefeatures -[*]-> * | - | 816 | 2.47e+04 | 162.7 |
| p42 | ρ4 | journalist -[personleadsgeopoliticalorganization]-> *; journalist -[*]-> * | - | 636 | 1 | 24.38 |
| p45 | ρ4 | professor -[agentbelongstoorganization]-> *; professor -[*]-> * | - | 466 | 2.351e+04 | 328.8 |
| p65 | ρ6 | traditionalgame -[*]-> *; traditionalgame -[*]-> * | - | 448 | 1.296e+04 | 1.372 |
| p69 | ρ6 | hallwayitem -[*]-> *; hallwayitem -[*]-> * | - | 416 | 22.5 | 1.154 |
| p54 | ρ5 | * -[*]-> musician; * -[*]-> musicartist | - | 352 | 2.622e+04 | 2.075 |
| p29 | ρ2 | animal -[*]-> * | - | 236 | 1.258e+05 | 29.17 |
| p17 | ρ1 | item -[iteminvolvedwithagent]-> * | - | 228 | 3375 | 0.067 |
| p40 | ρ4 | videogame -[producedby]-> *; videogame -[*]-> * | - | 224 | 1 | 0.211 |
| p52 | ρ5 | * -[*]-> musician; * -[*]-> musicalbum | - | 208 | 25.11 | 0.113 |

## nell_freq_o0

- queries: 70, with at least one exception: 64
- exceptions: 221772
- distinct triples as printed: 37958
- after resolving direction (the same fact reported from both ends of an edge, 'A X-> B' and 'B X<- A'): 37867
- after merging inverse relations (NELL's 'inverseof' prefix; no effect if the dataset has none): 37376
- unique facts used in the tables below (--merge inverse): 37376 (0.8315 of the exceptions merged into another one)
- types: LabelException 1920, LinkException 1872
- score: median 0.9996, p90 0.9996, p99 0.9996, max 0.9999

### Per rule

| rule | queries | with exc. | exceptions | unique | merged share | exc./query (median, max) | score median | p90 | p99 | max | Label share |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ρ0 | 10 | 10 | 3864 | 3802 | 0.01605 | 174, 1526 | 0.9976 | 0.999 | 0.999 | 0.9994 | 0.2303 |
| ρ1 | 10 | 10 | 5208 | 4633 | 0.1104 | 419, 1526 | 0.9957 | 0.999 | 0.999 | 0.9995 | 0.5333 |
| ρ2 | 10 | 10 | 2410 | 1638 | 0.3203 | 138, 840 | 0.9988 | 0.9993 | 0.9996 | 0.9998 | 0.5586 |
| ρ3 | 10 | 5 | 1144 | 1094 | 0.04371 | 36, 600 | 0.9966 | 0.9966 | 0.998 | 0.999 | 0.2688 |
| ρ4 | 10 | 10 | 3550 | 2673 | 0.247 | 183, 1478 | 0.9978 | 0.9986 | 0.9998 | 0.9999 | 0.3478 |
| ρ5 | 10 | 9 | 66316 | 18470 | 0.7215 | 4546, 14056 | 0.9994 | 0.9994 | 0.9994 | 0.9998 | 0.5978 |
| ρ6 | 10 | 10 | 139280 | 5066 | 0.9636 | 624, 134600 | 0.9996 | 0.9996 | 0.9996 | 0.9997 | 0.836 |

### Top 15 exceptions (unique facts)

| score | fact | type | query | context of the query |
|---|---|---|---|---|
| 0.9999 | sportsteam:louis_cardinals -[organizationacronymhasname]-> sportsteam:st__louis_cardinals | LinkException | p48 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * |
| 0.9999 | sportsteam:st__louis_cardinals -[organizationnamehasacronym]-> sportsteam:louis_cardinals | LinkException | p48 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * |
| 0.9998 | athlete:rich_rodriguez -[personterminatedbyorganization]-> sportsteam:rams | LinkException | p48 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * |
| 0.9998 | sportsteam:rams -[organizationterminatedperson]-> athlete:rich_rodriguez | LinkException | p48 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * |
| 0.9998 | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | p56 | * -[*]-> person; * -[*]-> personeurope |
| 0.9998 | sport:baseball -[players]-> athlete:alfonso_soriano | LabelException | p56 | * -[*]-> person; * -[*]-> personeurope |
| 0.9998 | coach:andy_green -[athleteplayssport]-> sport:basketball | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 0.9998 | sport:basketball -[players]-> coach:andy_green | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 0.9998 | athlete:carlos_carrasco -[athleteplayssport]-> sport:baseball | LabelException | p58 | * -[*]-> person; * -[*]-> personafrica |
| 0.9998 | sport:baseball -[players]-> athlete:carlos_carrasco | LabelException | p58 | * -[*]-> person; * -[*]-> personafrica |
| 0.9998 | athlete:george_brett -[athleteplayssport]-> sport:baseball | LabelException | p58 | * -[*]-> person; * -[*]-> personafrica |
| 0.9998 | sport:baseball -[players]-> athlete:george_brett | LabelException | p58 | * -[*]-> person; * -[*]-> personafrica |
| 0.9998 | country:canadians_canada -[organizationacronymhasname]-> country:the_united_kingdom | LinkException | p25 | language -[*]-> * |
| 0.9998 | country:the_united_kingdom -[organizationnamehasacronym]-> country:canadians_canada | LinkException | p25 | language -[*]-> * |
| 0.9998 | country:__france -[agentcompeteswithagent]-> sportsteam:brazil | LinkException | p25 | language -[*]-> * |

### Top 15 contexts, each with its strongest exception (one fact per context)

| # | score | context | query | strongest fact | type | tied | exceptions in context |
|---|---|---|---|---|---|---|---|
| 1 | 0.9999 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * | p48 (ρ4) | sportsteam:louis_cardinals -[organizationacronymhasname]-> sportsteam:st__louis_cardinals | LinkException | +1 | 140 |
| 2 | 0.9998 | * -[*]-> person; * -[*]-> personeurope | p56 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 14056 |
| 3 | 0.9998 | * -[*]-> person; * -[*]-> personasia | p59 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 13874 |
| 4 | 0.9998 | * -[*]-> person; * -[*]-> personnorthamerica | p55 (ρ5) | coach:andy_green -[athleteplayssport]-> sport:basketball | LabelException | +3 | 13648 |
| 5 | 0.9998 | * -[*]-> person; * -[*]-> personcanada | p57 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 6948 |
| 6 | 0.9998 | * -[*]-> person; * -[*]-> personafrica | p58 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 13520 |
| 7 | 0.9998 | language -[*]-> * | p25 (ρ2) | country:canadians_canada -[organizationacronymhasname]-> country:the_united_kingdom | LinkException | +4 | 364 |
| 8 | 0.9997 | animal -[*]-> * | p29 (ρ2) | insect:caterpillars -[invertebrateturnintoinsect]-> insect:butterfly | LinkException |  | 100 |
| 9 | 0.9997 | programminglanguage -[*]-> *; programminglanguage -[*]-> * | p61 (ρ6) | dateliteral:n2005 -[dateof]-> weapon:car_bomb | LabelException | +39 | 726 |
| 10 | 0.9996 | item -[*]-> *; item -[*]-> * | p64 (ρ6) | clothing:coats -[clothingtogowithclothing]-> clothing:fur_hats | UniqueEntityException | +1345 | 134600 |
| 11 | 0.9996 | male -[fatherofperson]-> *; male -[*]-> * | p43 (ρ4) | sportsleague:mlb -[leagueplayers]-> athlete:alfonso_soriano | LabelException | +5 | 1478 |
| 12 | 0.9996 | currency -[*]-> * | p24 (ρ2) | country:canadians_canada -[organizationacronymhasname]-> country:the_united_kingdom | LinkException | +3 | 266 |
| 13 | 0.9996 | musicinstrument -[*]-> *; musicinstrument -[*]-> * | p60 (ρ6) | dateliteral:n2005 -[dateof]-> weapon:car_bomb | LabelException | +37 | 566 |
| 14 | 0.9995 | grain -[*]-> *; grain -[*]-> * | p67 (ρ6) | vegetable:pepper -[agriculturalproductcookedwithagricultureproduct]-> coffeedrink:coffee | LabelException | +1 | 586 |
| 15 | 0.9995 | personeurope -[personbelongstoorganization]-> * | p13 (ρ1) | company:viacom -[superpartof]-> website:cbs_corporation | LinkException | +5 | 840 |

### Most frequent links (unique facts)

| link | facts | share | mean score | queries |
|---|---|---|---|---|
| athleteplayssport | 8596 | 0.23 | 0.9994 | 5 |
| players | 8596 | 0.23 | 0.9994 | 5 |
| dateof | 1619 | 0.04332 | 0.9982 | 9 |
| atdate | 1619 | 0.04332 | 0.9982 | 9 |
| sideeffectcausedbydrug | 1570 | 0.04201 | 0.9989 | 4 |
| drughassideeffect | 1570 | 0.04201 | 0.9989 | 4 |
| clothingtogowithclothing | 1346 | 0.03601 | 0.9996 | 1 |
| instrumentplayedbymusician | 1179 | 0.03154 | 0.9958 | 6 |
| musicianplaysinstrument | 1179 | 0.03154 | 0.9958 | 6 |
| agentcollaborateswithagent | 864 | 0.02312 | 0.995 | 12 |
| leagueplayers | 733 | 0.01961 | 0.9986 | 1 |
| athleteplaysinleague | 733 | 0.01961 | 0.9986 | 1 |
| countryalsoknownas | 554 | 0.01482 | 0.999 | 3 |
| animaleatfood | 463 | 0.01239 | 0.9981 | 2 |
| agentcompeteswithagent | 371 | 0.009926 | 0.9938 | 7 |

### Most frequent head and tail labels

| head label | facts | tail label | facts |
|---|---|---|---|
| sport | 8599 | sport | 8599 |
| athlete | 8490 | athlete | 8482 |
| drug | 1690 | drug | 1690 |
| disease | 1602 | disease | 1602 |
| country | 1447 | country | 1450 |
| dateliteral | 1309 | dateliteral | 1309 |
| musicinstrument | 1181 | musicinstrument | 1181 |
| musician | 1162 | musician | 1163 |
| clothing | 1149 | clothing | 1149 |
| sportsleague | 733 | sportsleague | 733 |
| city | 709 | city | 710 |
| animal | 548 | coach | 512 |
| coach | 515 | mammal | 488 |
| mammal | 485 | website | 444 |
| website | 444 | personmexico | 429 |

### Entities in exceptions of the most queries

| entity | queries | max score |
|---|---|---|
| countries | 9 | 0.9995 |
| united_states | 9 | 0.999 |
| austin_jackson | 8 | 0.9994 |
| bobby_bowden | 8 | 0.9994 |
| andre_ethier | 8 | 0.9994 |
| jim_rice | 8 | 0.9994 |
| rafael_betancourt | 8 | 0.9994 |
| eric_johnson | 8 | 0.9994 |
| india | 8 | 0.9993 |
| rich_rodriguez | 7 | 0.9998 |
| alfonso_soriano | 7 | 0.9998 |
| canadians_canada | 7 | 0.9998 |
| hamilton | 7 | 0.9995 |
| laos | 7 | 0.9995 |
| serbia | 7 | 0.9995 |

### Queries with the most and the strongest exceptions

| query | rule | context | matching heads | exceptions | max score | time [s] |
|---|---|---|---|---|---|---|
| p64 | ρ6 | item -[*]-> *; item -[*]-> * | - | 134600 | 0.9996 | 235.3 |
| p56 | ρ5 | * -[*]-> person; * -[*]-> personeurope | - | 14056 | 0.9998 | 64.68 |
| p59 | ρ5 | * -[*]-> person; * -[*]-> personasia | - | 13874 | 0.9998 | 60.8 |
| p55 | ρ5 | * -[*]-> person; * -[*]-> personnorthamerica | - | 13648 | 0.9998 | 133.8 |
| p58 | ρ5 | * -[*]-> person; * -[*]-> personafrica | - | 13520 | 0.9998 | 57.54 |
| p57 | ρ5 | * -[*]-> person; * -[*]-> personcanada | - | 6948 | 0.9998 | 69.22 |
| p50 | ρ5 | * -[*]-> musician; * -[*]-> musicgenre | - | 2144 | 0.9982 | 0.781 |
| p53 | ρ5 | * -[*]-> musician; * -[*]-> musicinstrument | - | 1886 | 0.9971 | 0.748 |
| p09 | ρ0 | emotion -[emotionassociatedwithdisease]-> disease | - | 1526 | 0.9993 | 0.062 |
| p19 | ρ1 | emotion -[emotionassociatedwithdisease]-> * | - | 1526 | 0.9993 | 0.089 |
| p43 | ρ4 | male -[fatherofperson]-> *; male -[*]-> * | - | 1478 | 0.9996 | 215 |
| p63 | ρ6 | fungus -[*]-> *; fungus -[*]-> * | - | 924 | 0.9981 | 0.588 |
| p12 | ρ1 | politicsblog -[agentcompeteswithagent]-> * | - | 888 | 0.998 | 0.021 |
| p13 | ρ1 | personeurope -[personbelongstoorganization]-> * | - | 840 | 0.9995 | 0.217 |
| p22 | ρ2 | musicfestival -[*]-> * | - | 840 | 0.9993 | 0.401 |

## nell_freq_o1

- queries: 70, with at least one exception: 64
- exceptions: 203334
- distinct triples as printed: 14978
- after resolving direction (the same fact reported from both ends of an edge, 'A X-> B' and 'B X<- A'): 14888
- after merging inverse relations (NELL's 'inverseof' prefix; no effect if the dataset has none): 13919
- unique facts used in the tables below (--merge inverse): 13919 (0.9315 of the exceptions merged into another one)
- types: LabelException 8294, LinkException 4506
- score: median 0.9996, p90 0.9996, p99 0.9997, max 0.9999

### Per rule

| rule | queries | with exc. | exceptions | unique | merged share | exc./query (median, max) | score median | p90 | p99 | max | Label share |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ρ0 | 10 | 10 | 3864 | 3802 | 0.01605 | 174, 1526 | 0.9976 | 0.999 | 0.999 | 0.9994 | 0.2303 |
| ρ1 | 10 | 10 | 5814 | 2994 | 0.485 | 211, 1526 | 0.9968 | 0.999 | 0.999 | 0.9995 | 0.8752 |
| ρ2 | 10 | 10 | 2632 | 1114 | 0.5767 | 130, 1464 | 0.9995 | 0.9996 | 0.9998 | 0.9998 | 0.5602 |
| ρ3 | 10 | 5 | 1144 | 1094 | 0.04371 | 36, 600 | 0.9966 | 0.9966 | 0.998 | 0.999 | 0.2688 |
| ρ4 | 10 | 10 | 22352 | 1669 | 0.9253 | 160, 20776 | 0.9992 | 0.9992 | 0.9997 | 0.9999 | 0.2563 |
| ρ5 | 10 | 9 | 8016 | 1243 | 0.8449 | 148, 4824 | 0.9973 | 0.9982 | 0.9998 | 0.9999 | 0.766 |
| ρ6 | 10 | 10 | 159512 | 2003 | 0.9874 | 972, 134600 | 0.9996 | 0.9996 | 0.9997 | 0.9997 | 0.6965 |

### Top 15 exceptions (unique facts)

| score | fact | type | query | context of the query |
|---|---|---|---|---|
| 0.9999 | sportsteam:louis_cardinals -[organizationacronymhasname]-> sportsteam:st__louis_cardinals | LinkException | p48 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * |
| 0.9999 | sportsteam:st__louis_cardinals -[organizationnamehasacronym]-> sportsteam:louis_cardinals | LinkException | p48 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * |
| 0.9998 | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 0.9998 | sport:baseball -[players]-> athlete:alfonso_soriano | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 0.9998 | coach:andy_green -[athleteplayssport]-> sport:basketball | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 0.9998 | sport:basketball -[players]-> coach:andy_green | LabelException | p55 | * -[*]-> person; * -[*]-> personnorthamerica |
| 0.9998 | athlete:rich_rodriguez -[personterminatedbyorganization]-> sportsteam:rams | LinkException | p48 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * |
| 0.9998 | sportsteam:rams -[organizationterminatedperson]-> athlete:rich_rodriguez | LinkException | p48 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * |
| 0.9998 | insect:caterpillars -[invertebrateturnintoinsect]-> insect:butterfly | LinkException | p29 | animal -[*]-> * |
| 0.9998 | insect:fly -[synonymfor]-> insect:mosquito | LinkException | p29 | animal -[*]-> * |
| 0.9998 | insect:mosquito -[synonymfor]-> insect:fly | LinkException | p29 | animal -[*]-> * |
| 0.9998 | athlete:carlos_carrasco -[athleteplayssport]-> sport:baseball | LabelException | p43 | male -[fatherofperson]-> *; male -[*]-> * |
| 0.9998 | athlete:george_brett -[athleteplayssport]-> sport:baseball | LabelException | p43 | male -[fatherofperson]-> *; male -[*]-> * |
| 0.9998 | sport:baseball -[players]-> athlete:george_brett | LabelException | p43 | male -[fatherofperson]-> *; male -[*]-> * |
| 0.9998 | sport:baseball -[players]-> athlete:carlos_carrasco | LabelException | p43 | male -[fatherofperson]-> *; male -[*]-> * |

### Top 15 contexts, each with its strongest exception (one fact per context)

| # | score | context | query | strongest fact | type | tied | exceptions in context |
|---|---|---|---|---|---|---|---|
| 1 | 0.9999 | sportsgame -[eventatlocation]-> *; sportsgame -[*]-> * | p48 (ρ4) | sportsteam:louis_cardinals -[organizationacronymhasname]-> sportsteam:st__louis_cardinals | LinkException | +1 | 100 |
| 2 | 0.9999 | * -[*]-> person; * -[*]-> personnorthamerica | p55 (ρ5) | sportsteam:louis_cardinals -[organizationacronymhasname]-> sportsteam:st__louis_cardinals | LinkException | +1 | 120 |
| 3 | 0.9998 | * -[*]-> person; * -[*]-> personeurope | p56 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 104 |
| 4 | 0.9998 | * -[*]-> person; * -[*]-> personasia | p59 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 176 |
| 5 | 0.9998 | * -[*]-> person; * -[*]-> personcanada | p57 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 144 |
| 6 | 0.9998 | * -[*]-> person; * -[*]-> personafrica | p58 (ρ5) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +1 | 144 |
| 7 | 0.9998 | animal -[*]-> * | p29 (ρ2) | insect:caterpillars -[invertebrateturnintoinsect]-> insect:butterfly | LinkException |  | 100 |
| 8 | 0.9998 | male -[fatherofperson]-> *; male -[*]-> * | p43 (ρ4) | athlete:alfonso_soriano -[athleteplayssport]-> sport:baseball | LabelException | +5 | 108 |
| 9 | 0.9998 | language -[*]-> * | p25 (ρ2) | country:canadians_canada -[organizationacronymhasname]-> country:the_united_kingdom | LinkException | +4 | 112 |
| 10 | 0.9997 | grain -[*]-> *; grain -[*]-> * | p67 (ρ6) | animal:lamb -[animaleatfood]-> beverage:milk | LinkException |  | 1128 |
| 11 | 0.9997 | currency -[*]-> * | p24 (ρ2) | country:america -[countryhascompanyoffice]-> geopoliticallocation:gulf_coast | LinkException | +5 | 140 |
| 12 | 0.9997 | musicfestival -[*]-> * | p22 (ρ2) | dateliteral:n2008 -[dateof]-> crimeorcharge:theft | LabelException | +33 | 1464 |
| 13 | 0.9997 | geopolitcallocation -[cityofpersondeath]-> *; geopolitcallocation -[*]-> * | p46 (ρ4) | dateliteral:n2006 -[dateof]-> militaryeventtype:bombings | LabelException | +39 | 152 |
| 14 | 0.9997 | programminglanguage -[*]-> *; programminglanguage -[*]-> * | p61 (ρ6) | dateliteral:n2010 -[dateof]-> researchproject:pilot_project | LabelException | +39 | 1760 |
| 15 | 0.9996 | item -[*]-> *; item -[*]-> * | p64 (ρ6) | clothing:coats -[clothingtogowithclothing]-> clothing:fur_hats | UniqueEntityException | +1345 | 134600 |

### Most frequent links (unique facts)

| link | facts | share | mean score | queries |
|---|---|---|---|---|
| sideeffectcausedbydrug | 1570 | 0.1128 | 0.9989 | 4 |
| drughassideeffect | 1570 | 0.1128 | 0.9989 | 4 |
| clothingtogowithclothing | 1346 | 0.0967 | 0.9996 | 1 |
| animaleatfood | 927 | 0.0666 | 0.998 | 4 |
| instrumentplayedbymusician | 775 | 0.05568 | 0.9963 | 6 |
| musicianplaysinstrument | 775 | 0.05568 | 0.9963 | 6 |
| universityhasacademicprogram | 742 | 0.05331 | 0.9992 | 1 |
| academicprogramatuniversity | 742 | 0.05331 | 0.9992 | 1 |
| dateof | 578 | 0.04153 | 0.9991 | 10 |
| atdate | 578 | 0.04153 | 0.9991 | 10 |
| teammate | 320 | 0.02299 | 0.9909 | 2 |
| animaltypehasanimal | 312 | 0.02242 | 0.9976 | 1 |
| animalistypeofanimal | 312 | 0.02242 | 0.9976 | 1 |
| geopoliticallocationcontainscountry | 284 | 0.0204 | 0.9966 | 3 |
| countrylocatedingeopoliticallocation | 284 | 0.0204 | 0.9966 | 3 |

### Most frequent head and tail labels

| head label | facts | tail label | facts |
|---|---|---|---|
| drug | 1690 | drug | 1690 |
| disease | 1566 | disease | 1566 |
| clothing | 1152 | clothing | 1152 |
| animal | 998 | musicinstrument | 778 |
| musicinstrument | 778 | university | 742 |
| university | 742 | academicfield | 740 |
| academicfield | 740 | musician | 722 |
| musician | 722 | country | 718 |
| country | 715 | dateliteral | 521 |
| dateliteral | 521 | mammal | 479 |
| mammal | 477 | athlete | 301 |
| athlete | 309 | politicsblog | 277 |
| politicsblog | 277 | musicartist | 273 |
| musicartist | 273 | musicgenre | 267 |
| musicgenre | 267 | visualizableobject | 262 |

### Entities in exceptions of the most queries

| entity | queries | max score |
|---|---|---|
| america | 8 | 0.9998 |
| alfonso_soriano | 7 | 0.9998 |
| andy_green | 7 | 0.9998 |
| wolves | 7 | 0.9997 |
| n2005 | 7 | 0.9997 |
| car_bomb | 7 | 0.9997 |
| operation_iraqi_freedom | 7 | 0.9997 |
| belt | 7 | 0.9997 |
| baseball | 6 | 0.9998 |
| n2010 | 6 | 0.9997 |
| strategic_planning_process | 6 | 0.9997 |
| shooting | 6 | 0.9997 |
| president_obama | 6 | 0.9997 |
| bombings | 6 | 0.9997 |
| pilot_project | 6 | 0.9997 |

### Queries with the most and the strongest exceptions

| query | rule | context | matching heads | exceptions | max score | time [s] |
|---|---|---|---|---|---|---|
| p64 | ρ6 | item -[*]-> *; item -[*]-> * | - | 134600 | 0.9996 | 448.9 |
| p41 | ρ4 | female -[motherofperson]-> *; female -[*]-> * | - | 20776 | 0.9992 | 173.9 |
| p63 | ρ6 | fungus -[*]-> *; fungus -[*]-> * | - | 18488 | 0.9994 | 0.907 |
| p53 | ρ5 | * -[*]-> musician; * -[*]-> musicinstrument | - | 4824 | 0.9979 | 0.668 |
| p50 | ρ5 | * -[*]-> musician; * -[*]-> musicgenre | - | 2144 | 0.9982 | 1.464 |
| p61 | ρ6 | programminglanguage -[*]-> *; programminglanguage -[*]-> * | - | 1760 | 0.9997 | 10.43 |
| p09 | ρ0 | emotion -[emotionassociatedwithdisease]-> disease | - | 1526 | 0.9993 | 0.062 |
| p19 | ρ1 | emotion -[emotionassociatedwithdisease]-> * | - | 1526 | 0.9993 | 0.095 |
| p16 | ρ1 | item -[instrumentplayedbymusician]-> * | - | 1494 | 0.9968 | 0.322 |
| p22 | ρ2 | musicfestival -[*]-> * | - | 1464 | 0.9997 | 0.415 |
| p60 | ρ6 | musicinstrument -[*]-> *; musicinstrument -[*]-> * | - | 1216 | 0.9996 | 501 |
| p67 | ρ6 | grain -[*]-> *; grain -[*]-> * | - | 1128 | 0.9997 | 593.7 |
| p15 | ρ1 | plant -[inverseofanimaleatfood]-> * | - | 966 | 0.9995 | 0.097 |
| p12 | ρ1 | politicsblog -[agentcompeteswithagent]-> * | - | 904 | 0.998 | 0.022 |
| p68 | ρ6 | landscapefeatures -[*]-> *; landscapefeatures -[*]-> * | - | 816 | 0.9991 | 162.1 |

## Comparison between settings

Jaccard overlap of the top-k unique facts per query (ties at the cut-off included); 'same top-1' = the best exceptions of both runs share a fact.

### dist: o=0 vs o=1

| k | rule | queries | Jaccard median | Jaccard mean | same top-1 |
|---|---|---|---|---|---|
| 10 | ρ0 | 10 | 1 | 1 | 1 |
| 10 | ρ1 | 10 | 1 | 1 | 1 |
| 10 | ρ2 | 10 | 1 | 1 | 1 |
| 10 | ρ3 | 5 | 1 | 1 | 1 |
| 10 | ρ4 | 10 | 1 | 1 | 1 |
| 10 | ρ5 | 9 | 1 | 1 | 1 |
| 10 | ρ6 | 10 | 1 | 1 | 1 |
| 100 | ρ0 | 10 | 1 | 1 | - |
| 100 | ρ1 | 10 | 1 | 1 | - |
| 100 | ρ2 | 10 | 1 | 1 | - |
| 100 | ρ3 | 5 | 1 | 1 | - |
| 100 | ρ4 | 10 | 1 | 1 | - |
| 100 | ρ5 | 9 | 1 | 1 | - |
| 100 | ρ6 | 10 | 1 | 1 | - |

### o=0: dist vs freq

| k | rule | queries | Jaccard median | Jaccard mean | same top-1 |
|---|---|---|---|---|---|
| 10 | ρ0 | 10 | 0.1296 | 0.1421 | 0.7 |
| 10 | ρ1 | 10 | 0.06786 | 0.07827 | 0.4 |
| 10 | ρ2 | 10 | 0.009901 | 0.1313 | 0.2 |
| 10 | ρ3 | 5 | 0.4 | 0.3744 | 0.4 |
| 10 | ρ4 | 10 | 0.08013 | 0.2723 | 0.3 |
| 10 | ρ5 | 9 | 0.01277 | 0.181 | 0.6667 |
| 10 | ρ6 | 10 | 0 | 0.1048 | 0.1 |
| 100 | ρ0 | 10 | 0.1042 | 0.3248 | - |
| 100 | ρ1 | 10 | 0.04793 | 0.1545 | - |
| 100 | ρ2 | 10 | 0.07545 | 0.09629 | - |
| 100 | ρ3 | 5 | 0.2255 | 0.3772 | - |
| 100 | ρ4 | 10 | 0.01777 | 0.1893 | - |
| 100 | ρ5 | 9 | 0.001751 | 0.03038 | - |
| 100 | ρ6 | 10 | 0 | 0.0148 | - |

### o=1: dist vs freq

| k | rule | queries | Jaccard median | Jaccard mean | same top-1 |
|---|---|---|---|---|---|
| 10 | ρ0 | 10 | 0.1296 | 0.1421 | 0.7 |
| 10 | ρ1 | 10 | 0.127 | 0.254 | 0.4 |
| 10 | ρ2 | 10 | 0.303 | 0.3298 | 0.2 |
| 10 | ρ3 | 5 | 0.4 | 0.3744 | 0.4 |
| 10 | ρ4 | 10 | 0.8333 | 0.6192 | 0.7 |
| 10 | ρ5 | 9 | 0.3333 | 0.3435 | 0.7778 |
| 10 | ρ6 | 10 | 0.002137 | 0.3969 | 0.4 |
| 100 | ρ0 | 10 | 0.1042 | 0.3248 | - |
| 100 | ρ1 | 10 | 0.09465 | 0.3182 | - |
| 100 | ρ2 | 10 | 0.1845 | 0.2907 | - |
| 100 | ρ3 | 5 | 0.2255 | 0.3772 | - |
| 100 | ρ4 | 10 | 0.75 | 0.5635 | - |
| 100 | ρ5 | 9 | 0.2727 | 0.356 | - |
| 100 | ρ6 | 10 | 0.002137 | 0.3969 | - |

### freq: o=0 vs o=1

| k | rule | queries | Jaccard median | Jaccard mean | same top-1 |
|---|---|---|---|---|---|
| 10 | ρ0 | 10 | 1 | 1 | 1 |
| 10 | ρ1 | 10 | 0.5357 | 0.4932 | 0.9 |
| 10 | ρ2 | 10 | 0.3318 | 0.4598 | 0.6 |
| 10 | ρ3 | 5 | 1 | 1 | 1 |
| 10 | ρ4 | 10 | 0.07143 | 0.3476 | 0.3 |
| 10 | ρ5 | 9 | 0.3 | 0.4009 | 0.7778 |
| 10 | ρ6 | 10 | 0.9978 | 0.6579 | 0.7 |
| 100 | ρ0 | 10 | 1 | 1 | - |
| 100 | ρ1 | 10 | 0.6393 | 0.5369 | - |
| 100 | ρ2 | 10 | 0.06434 | 0.3256 | - |
| 100 | ρ3 | 5 | 1 | 1 | - |
| 100 | ρ4 | 10 | 0.04066 | 0.2392 | - |
| 100 | ρ5 | 9 | 0.001773 | 0.2202 | - |
| 100 | ρ6 | 10 | 0.07435 | 0.2848 | - |

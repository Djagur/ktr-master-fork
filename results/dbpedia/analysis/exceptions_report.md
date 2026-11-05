# Exception analysis

Unique facts merge the same fact printed in several forms (counts per setting below). Scores of `dist` and `freq` are on different scales, so they are compared by rank overlap only.

## dbpedia_dist_o0

- queries: 70, with at least one exception: 57
- exceptions: 7311
- distinct triples as printed: 2592
- after resolving direction (the same fact reported from both ends of an edge, 'A X-> B' and 'B X<- A'): 2482
- after merging inverse relations (NELL's 'inverseof' prefix; no effect if the dataset has none): 2482
- unique facts used in the tables below (--merge inverse): 2482 (0.6605 of the exceptions merged into another one)
- types: LabelException 1692, LinkException 4453
- score: median 8.273e+04, p90 2.845e+07, p99 1.172e+09, max 1.051e+10

### Per rule

| rule | queries | with exc. | exceptions | unique | merged share | exc./query (median, max) | score median | p90 | p99 | max | Label share |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ρ0 | 10 | 10 | 937 | 864 | 0.07791 | 102, 191 | 9311 | 1.696e+06 | 1.754e+07 | 2.843e+07 | 0.5482 |
| ρ1 | 10 | 10 | 1071 | 437 | 0.592 | 108, 112 | 8.861e+05 | 1.172e+09 | 2.917e+09 | 1.051e+10 | 0.331 |
| ρ2 | 10 | 10 | 2109 | 322 | 0.8473 | 101, 1346 | 8.273e+04 | 1.834e+07 | 6.146e+07 | 1.299e+08 | 0.09402 |
| ρ3 | 10 | 6 | 552 | 444 | 0.1957 | 56, 129 | 965 | 5.677e+05 | 5.404e+06 | 5.404e+06 | 0.298 |
| ρ4 | 10 | 10 | 1348 | 177 | 0.8687 | 102, 320 | 9.296e+06 | 3.175e+07 | 1.553e+08 | 1.783e+08 | 0.2077 |
| ρ5 | 10 | 10 | 1186 | 224 | 0.8111 | 104, 213 | 1.191e+04 | 2.938e+07 | 4.405e+08 | 4.405e+08 | 0.5496 |
| ρ6 | 10 | 1 | 108 | 14 | 0.8704 | 0, 108 | 1647 | 4451 | 7210 | 7210 | 0.5278 |

### Top 15 exceptions (unique facts)

| score | fact | type | query | context of the query |
|---|---|---|---|---|
| 1.051e+10 | British_Columbia_Intercollegiate_Hockey_League -[team]-> University_of_Victoria | LabelException | p12 | SoccerPlayer -[team]-> * |
| 4.24e+09 | Maccabi_Haifa -[keyPerson]-> Yochanan_Vollach | LinkException | p12 | SoccerPlayer -[team]-> * |
| 3.467e+09 | Matt_Bahr -[team]-> Colorado_Caribous | LabelException | p12 | SoccerPlayer -[team]-> * |
| 3.467e+09 | Martti_J%C3%A4rventie -[team]-> HIFK | LabelException | p12 | SoccerPlayer -[team]-> * |
| 3.467e+09 | Kimmo_Kuhta -[team]-> HIFK | LabelException | p12 | SoccerPlayer -[team]-> * |
| 3.467e+09 | Chris_Bahr -[team]-> Philadelphia_Atoms | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | Tr%C3%A9sor_Luntala -[team]-> Grasshopper | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | Frank_Jonke -[team]-> Notre_Dame_College | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | Nenad_Begovi%C4%87 -[team]-> Romorantin | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | Dan_Pelc -[team]-> New_Hampshire_Wildcats | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | John_Benson_%28footballer_born_1991%29 -[team]-> ASPIRE_Academy_for_Sports_Excellence | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | Santiago_Solari -[team]-> Renato_Cesarini | LabelException/UniqueEntityException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | Georgios_Gougoulias -[team]-> Pavlos_Melas | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.859e+09 | Pogo%C5%84_Lw%C3%B3w -[ground]-> Edward_Rydz-%C5%9Amig%C5%82y | LinkException | p12 | SoccerPlayer -[team]-> * |
| 2.859e+09 | Jez_Moxey -[board]-> Wolverhampton_Wanderers_F.C. | LinkException | p12 | SoccerPlayer -[team]-> * |

### Top 15 contexts, each with its strongest exception (one fact per context)

| # | score | context | query | strongest fact | type | tied | exceptions in context |
|---|---|---|---|---|---|---|---|
| 1 | 1.051e+10 | SoccerPlayer -[team]-> * | p12 (ρ1) | British_Columbia_Intercollegiate_Hockey_League -[team]-> University_of_Victoria | LabelException |  | 110 |
| 2 | 4.405e+08 | * -[*]-> Work; * -[*]-> Language | p50 (ρ5) | IPBA_%28TV_series%29 -[developer]-> Philippine_Basketball_Association | LinkException | +32 | 100 |
| 3 | 1.783e+08 | MilitaryConflict -[place]-> *; MilitaryConflict -[*]-> * | p46 (ρ4) | Loek_van_Mil -[award]-> Netherlands | LinkException |  | 104 |
| 4 | 1.299e+08 | MusicFestival -[*]-> * | p25 (ρ2) | Mutant_Chronicles -[genre]-> Techno | LabelException | +1 | 102 |
| 5 | 1.102e+08 | MusicalArtist -[associatedBand]-> * | p16 (ρ1) | NanoTek_Warrior -[artist]-> Don_Williams | LabelException |  | 106 |
| 6 | 5.12e+07 | FormulaOneRacer -[*]-> * | p27 (ρ2) | Loek_van_Mil -[award]-> Netherlands | LinkException |  | 100 |
| 7 | 4.576e+07 | Writer -[occupation]-> *; Writer -[*]-> * | p49 (ρ4) | Sixty_Six_Steps -[recordPlace]-> Rock_music | LinkException |  | 142 |
| 8 | 3.924e+07 | University -[head]-> *; University -[*]-> * | p48 (ρ4) | The_Autobiography_of_Miss_Jane_Pittman -[coverArtist]-> United_States | LinkException |  | 100 |
| 9 | 3.633e+07 | Currency -[*]-> * | p28 (ρ2) | Loek_van_Mil -[award]-> Netherlands | LinkException | +1 | 100 |
| 10 | 3.633e+07 | * -[*]-> Country; * -[*]-> University | p58 (ρ5) | Loek_van_Mil -[award]-> Netherlands | LinkException | +1 | 138 |
| 11 | 3.186e+07 | Weapon -[wars]-> *; Weapon -[*]-> * | p47 (ρ4) | The_Evil_Empire:_101_Ways_That_England_Ruined_the_World -[subject]-> United_Kingdom | LinkException | +2 | 102 |
| 12 | 3.062e+07 | PrimeMinister -[birthPlace]-> * | p13 (ρ1) | Sharpe%27s_Prey -[language]-> United_Kingdom | LinkException | +1 | 100 |
| 13 | 2.933e+07 | Philosopher -[influencedBy]-> *; Philosopher -[*]-> * | p41 (ρ4) | The_Autobiography_of_Miss_Jane_Pittman -[coverArtist]-> United_States | LinkException |  | 101 |
| 14 | 2.845e+07 | Bridge -[region]-> *; Bridge -[*]-> * | p44 (ρ4) | The_Autobiography_of_Miss_Jane_Pittman -[coverArtist]-> United_States | LinkException |  | 176 |
| 15 | 2.843e+07 | Bird -[class]-> Animal | p06 (ρ0) | Ruby-throated_Hummingbird -[class]-> Bird | LabelException |  | 142 |

### Most frequent links (unique facts)

| link | facts | share | mean score | queries |
|---|---|---|---|---|
| birthPlace | 185 | 0.07454 | 2.237e+07 | 27 |
| genus | 164 | 0.06608 | 4.999e+05 | 7 |
| species | 107 | 0.04311 | 4.909e+05 | 5 |
| recordLabel | 80 | 0.03223 | 3.054e+06 | 17 |
| language | 73 | 0.02941 | 1.881e+06 | 7 |
| binomialAuthority | 57 | 0.02297 | 1.563e+06 | 9 |
| almaMater | 54 | 0.02176 | 9.208e+05 | 6 |
| country | 52 | 0.02095 | 2.634e+07 | 15 |
| location | 46 | 0.01853 | 1.986e+07 | 7 |
| order | 45 | 0.01813 | 1.278e+06 | 7 |
| producer | 43 | 0.01732 | 2.953e+07 | 10 |
| knownFor | 43 | 0.01732 | 1.234e+06 | 13 |
| publisher | 41 | 0.01652 | 4.17e+07 | 10 |
| class | 41 | 0.01652 | 3.938e+06 | 6 |
| deathPlace | 35 | 0.0141 | 1.325e+07 | 11 |

### Most frequent head and tail labels

| head label | facts | tail label | facts |
|---|---|---|---|
| (none) | 2424 | (none) | 2478 |
| The_Legend_of_Zelda | 7 | The_Legend_of_Zelda | 2 |
| Shipwrecked | 5 | Metroid_II | 1 |
| Neil_Young | 2 | Army_of_Two | 1 |

### Entities in exceptions of the most queries

| entity | queries | max score |
|---|---|---|
| United_States | 15 | 1.553e+08 |
| United_Kingdom | 11 | 1.553e+08 |
| The_Autobiography_of_Miss_Jane_Pittman | 11 | 3.924e+07 |
| France | 9 | 7.698e+07 |
| Italy | 9 | 7.698e+07 |
| Jimmy_Robinson_%28recording_engineer%29 | 9 | 6.256e+07 |
| Jarom%C3%ADr_J%C3%A1gr | 9 | 5.06e+07 |
| Richard_McKeon | 8 | 1.553e+08 |
| Paris | 8 | 6.256e+07 |
| Association_of_Volleyball_Professionals | 8 | 2.726e+07 |
| Steven_M._Berman | 8 | 2.726e+07 |
| Rome | 7 | 1.553e+08 |
| William_B._McKinley | 7 | 1.358e+08 |
| Horace_Holley_%28Bah%C3%A1%27%C3%AD%29 | 7 | 7.698e+07 |
| Walter_Booth | 7 | 6.256e+07 |

### Queries with the most and the strongest exceptions

| query | rule | context | matching heads | exceptions | max score | time [s] |
|---|---|---|---|---|---|---|
| p29 | ρ2 | Bacteria -[*]-> * | - | 1346 | 5.222e+05 | 3.525 |
| p45 | ρ4 | BodyOfWater -[originalEndPoint]-> *; BodyOfWater -[*]-> * | - | 320 | 1.86e+04 | 9262 |
| p52 | ρ5 | * -[*]-> Work; * -[*]-> Film | - | 213 | 2.5 | - |
| p07 | ρ0 | Reptile -[class]-> Animal | - | 191 | 2.492e+07 | 0.296 |
| p44 | ρ4 | Bridge -[region]-> *; Bridge -[*]-> * | - | 176 | 2.845e+07 | 129.2 |
| p06 | ρ0 | Bird -[class]-> Animal | - | 142 | 2.843e+07 | 1.349 |
| p49 | ρ4 | Writer -[occupation]-> *; Writer -[*]-> * | - | 142 | 4.576e+07 | 5564 |
| p59 | ρ5 | * -[*]-> Country; * -[*]-> Lake | - | 142 | 613 | - |
| p05 | ρ0 | Mammal -[class]-> Animal | - | 141 | 1.681e+07 | 1.622 |
| p58 | ρ5 | * -[*]-> Country; * -[*]-> University | - | 138 | 3.633e+07 | - |
| p30 | ρ3 | VideoGame -[producer]-> Person; VideoGame -[publisher]-> Company | - | 129 | 1.11e+06 | 0.519 |
| p26 | ρ2 | SiteOfSpecialScientificInterest -[*]-> * | - | 114 | 1.178e+05 | 0.444 |
| p11 | ρ1 | MusicalWork -[language]-> * | - | 112 | 2.269e+07 | 10.38 |
| p53 | ρ5 | * -[*]-> Work; * -[*]-> Organisation | - | 112 | 2.357e+06 | - |
| p01 | ρ0 | Scientist -[knownFor]-> Disease | - | 111 | 5000 | 0.068 |

## dbpedia_dist_o1

- queries: 70, with at least one exception: 36
- exceptions: 4669
- distinct triples as printed: 2168
- after resolving direction (the same fact reported from both ends of an edge, 'A X-> B' and 'B X<- A'): 2067
- after merging inverse relations (NELL's 'inverseof' prefix; no effect if the dataset has none): 2067
- unique facts used in the tables below (--merge inverse): 2067 (0.5573 of the exceptions merged into another one)
- types: LabelException 818, LinkException 2894
- score: median 8.273e+04, p90 1.872e+07, p99 2.54e+09, max 1.051e+10

### Per rule

| rule | queries | with exc. | exceptions | unique | merged share | exc./query (median, max) | score median | p90 | p99 | max | Label share |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ρ0 | 10 | 10 | 937 | 864 | 0.07791 | 102, 191 | 9311 | 1.696e+06 | 1.754e+07 | 2.843e+07 | 0.5482 |
| ρ1 | 10 | 10 | 1071 | 437 | 0.592 | 108, 112 | 8.861e+05 | 1.172e+09 | 2.917e+09 | 1.051e+10 | 0.331 |
| ρ2 | 10 | 10 | 2109 | 322 | 0.8473 | 101, 1346 | 8.273e+04 | 1.834e+07 | 6.146e+07 | 1.299e+08 | 0.09402 |
| ρ3 | 10 | 6 | 552 | 444 | 0.1957 | 56, 129 | 965 | 5.677e+05 | 5.404e+06 | 5.404e+06 | 0.298 |
| ρ4 | 10 | 0 | 0 | 0 | - | 0, 0 | - | - | - | - | - |
| ρ5 | 10 | 0 | 0 | 0 | - | 0, 0 | - | - | - | - | - |
| ρ6 | 10 | 0 | 0 | 0 | - | 0, 0 | - | - | - | - | - |

### Top 15 exceptions (unique facts)

| score | fact | type | query | context of the query |
|---|---|---|---|---|
| 1.051e+10 | British_Columbia_Intercollegiate_Hockey_League -[team]-> University_of_Victoria | LabelException | p12 | SoccerPlayer -[team]-> * |
| 4.24e+09 | Maccabi_Haifa -[keyPerson]-> Yochanan_Vollach | LinkException | p12 | SoccerPlayer -[team]-> * |
| 3.467e+09 | Matt_Bahr -[team]-> Colorado_Caribous | LabelException | p12 | SoccerPlayer -[team]-> * |
| 3.467e+09 | Martti_J%C3%A4rventie -[team]-> HIFK | LabelException | p12 | SoccerPlayer -[team]-> * |
| 3.467e+09 | Kimmo_Kuhta -[team]-> HIFK | LabelException | p12 | SoccerPlayer -[team]-> * |
| 3.467e+09 | Chris_Bahr -[team]-> Philadelphia_Atoms | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | Tr%C3%A9sor_Luntala -[team]-> Grasshopper | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | Frank_Jonke -[team]-> Notre_Dame_College | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | Nenad_Begovi%C4%87 -[team]-> Romorantin | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | Dan_Pelc -[team]-> New_Hampshire_Wildcats | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | John_Benson_%28footballer_born_1991%29 -[team]-> ASPIRE_Academy_for_Sports_Excellence | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | Santiago_Solari -[team]-> Renato_Cesarini | LabelException/UniqueEntityException | p12 | SoccerPlayer -[team]-> * |
| 2.917e+09 | Georgios_Gougoulias -[team]-> Pavlos_Melas | LabelException | p12 | SoccerPlayer -[team]-> * |
| 2.859e+09 | Pogo%C5%84_Lw%C3%B3w -[ground]-> Edward_Rydz-%C5%9Amig%C5%82y | LinkException | p12 | SoccerPlayer -[team]-> * |
| 2.859e+09 | Jez_Moxey -[board]-> Wolverhampton_Wanderers_F.C. | LinkException | p12 | SoccerPlayer -[team]-> * |

### Top 15 contexts, each with its strongest exception (one fact per context)

| # | score | context | query | strongest fact | type | tied | exceptions in context |
|---|---|---|---|---|---|---|---|
| 1 | 1.051e+10 | SoccerPlayer -[team]-> * | p12 (ρ1) | British_Columbia_Intercollegiate_Hockey_League -[team]-> University_of_Victoria | LabelException |  | 110 |
| 2 | 1.299e+08 | MusicFestival -[*]-> * | p25 (ρ2) | Mutant_Chronicles -[genre]-> Techno | LabelException | +1 | 102 |
| 3 | 1.102e+08 | MusicalArtist -[associatedBand]-> * | p16 (ρ1) | NanoTek_Warrior -[artist]-> Don_Williams | LabelException |  | 106 |
| 4 | 5.12e+07 | FormulaOneRacer -[*]-> * | p27 (ρ2) | Loek_van_Mil -[award]-> Netherlands | LinkException |  | 100 |
| 5 | 3.633e+07 | Currency -[*]-> * | p28 (ρ2) | Loek_van_Mil -[award]-> Netherlands | LinkException | +1 | 100 |
| 6 | 3.062e+07 | PrimeMinister -[birthPlace]-> * | p13 (ρ1) | Sharpe%27s_Prey -[language]-> United_Kingdom | LinkException | +1 | 100 |
| 7 | 2.843e+07 | Bird -[class]-> Animal | p06 (ρ0) | Ruby-throated_Hummingbird -[class]-> Bird | LabelException |  | 142 |
| 8 | 2.598e+07 | Convention -[*]-> * | p22 (ρ2) | The_Autobiography_of_Miss_Jane_Pittman -[coverArtist]-> United_States | LinkException | +1 | 101 |
| 9 | 2.492e+07 | Reptile -[class]-> Animal | p07 (ρ0) | Fancy_rat -[class]-> Tetrapod | LabelException | +1 | 191 |
| 10 | 2.269e+07 | MusicalWork -[language]-> * | p11 (ρ1) | The_Path_%28video_game%29 -[distributor]-> Japanese_language | LinkException | +1 | 112 |
| 11 | 1.681e+07 | Mammal -[class]-> Animal | p05 (ρ0) | Dwarf_killer_whale -[class]-> Elasmobranchii | LabelException |  | 141 |
| 12 | 1.288e+07 | WrestlingEvent -[*]-> * | p21 (ρ2) | Dayton%2C_Ohio -[leader]-> Gary_Leitzell | LinkException | +9 | 101 |
| 13 | 6.894e+06 | Film -[narrator]-> * | p10 (ρ1) | Fran%C3%A7ois_P%C3%A9rier -[baftaAward]-> Gervaise | LinkException | +2 | 111 |
| 14 | 5.763e+06 | Reptile -[family]-> * | p18 (ρ1) | Virginia_striatula -[genus]-> Virginia | LinkException | +6 | 110 |
| 15 | 5.404e+06 | Saint -[beatifiedBy]-> Cleric; Saint -[deathPlace]-> Place | p33 (ρ3) | Peter_Butler_%28surgeon%29 -[employer]-> London | LinkException | +9 | 100 |

### Most frequent links (unique facts)

| link | facts | share | mean score | queries |
|---|---|---|---|---|
| genus | 164 | 0.07934 | 4.999e+05 | 7 |
| birthPlace | 141 | 0.06821 | 2.12e+07 | 19 |
| species | 107 | 0.05177 | 4.909e+05 | 5 |
| recordLabel | 76 | 0.03677 | 2.991e+06 | 13 |
| binomialAuthority | 57 | 0.02758 | 1.563e+06 | 9 |
| almaMater | 47 | 0.02274 | 5.405e+05 | 4 |
| order | 44 | 0.02129 | 1.307e+06 | 6 |
| location | 44 | 0.02129 | 7.432e+05 | 6 |
| producer | 42 | 0.02032 | 2.969e+07 | 9 |
| class | 41 | 0.01984 | 3.938e+06 | 6 |
| knownFor | 38 | 0.01838 | 1.252e+06 | 8 |
| country | 35 | 0.01693 | 1.052e+07 | 10 |
| publisher | 34 | 0.01645 | 3.724e+07 | 7 |
| deathPlace | 34 | 0.01645 | 6.872e+05 | 10 |
| family | 33 | 0.01597 | 6.029e+05 | 6 |

### Most frequent head and tail labels

| head label | facts | tail label | facts |
|---|---|---|---|
| (none) | 2024 | (none) | 2063 |
| The_Legend_of_Zelda | 7 | The_Legend_of_Zelda | 2 |
| Shipwrecked | 3 | Metroid_II | 1 |
| Neil_Young | 2 | Army_of_Two | 1 |

### Entities in exceptions of the most queries

| entity | queries | max score |
|---|---|---|
| United_States | 8 | 6.771e+07 |
| United_Kingdom | 6 | 6.256e+07 |
| Jimmy_Robinson_%28recording_engineer%29 | 6 | 6.256e+07 |
| Paris | 5 | 6.256e+07 |
| Richard_McKeon | 5 | 6.256e+07 |
| Walter_Booth | 5 | 6.256e+07 |
| Jarom%C3%ADr_J%C3%A1gr | 5 | 5.06e+07 |
| Italy | 5 | 5.06e+07 |
| The_Autobiography_of_Miss_Jane_Pittman | 5 | 3.654e+07 |
| France | 5 | 3.408e+07 |
| Peter_Butler_%28surgeon%29 | 5 | 2.79e+07 |
| London | 5 | 2.79e+07 |
| Association_of_Volleyball_Professionals | 5 | 2.726e+07 |
| Steven_M._Berman | 5 | 2.726e+07 |
| Jon_Paszt | 4 | 6.256e+07 |

### Queries with the most and the strongest exceptions

| query | rule | context | matching heads | exceptions | max score | time [s] |
|---|---|---|---|---|---|---|
| p29 | ρ2 | Bacteria -[*]-> * | - | 1346 | 5.222e+05 | 3.475 |
| p07 | ρ0 | Reptile -[class]-> Animal | - | 191 | 2.492e+07 | 0.287 |
| p06 | ρ0 | Bird -[class]-> Animal | - | 142 | 2.843e+07 | 1.297 |
| p05 | ρ0 | Mammal -[class]-> Animal | - | 141 | 1.681e+07 | 1.559 |
| p30 | ρ3 | VideoGame -[producer]-> Person; VideoGame -[publisher]-> Company | - | 129 | 1.11e+06 | 0.282 |
| p26 | ρ2 | SiteOfSpecialScientificInterest -[*]-> * | - | 114 | 1.178e+05 | 0.423 |
| p11 | ρ1 | MusicalWork -[language]-> * | - | 112 | 2.269e+07 | 10.39 |
| p01 | ρ0 | Scientist -[knownFor]-> Disease | - | 111 | 5000 | 0.025 |
| p10 | ρ1 | Film -[narrator]-> * | - | 111 | 6.894e+06 | 18.54 |
| p19 | ρ1 | Stream -[river]-> * | - | 111 | 30.33 | 0.137 |
| p12 | ρ1 | SoccerPlayer -[team]-> * | - | 110 | 1.051e+10 | 566.8 |
| p18 | ρ1 | Reptile -[family]-> * | - | 110 | 5.763e+06 | 3.631 |
| p20 | ρ2 | AutoRacingLeageu -[*]-> * | - | 110 | 22.33 | 0.285 |
| p17 | ρ1 | Non-ProfitOrganisation -[keyPerson]-> * | - | 107 | 9.395e+05 | 1.506 |
| p39 | ρ3 | Planet -[discoverer]-> Person; Planet -[discoverer]-> Scientist | - | 107 | 1.107e+04 | 0.209 |

## dbpedia_freq_o0

- queries: 70, with at least one exception: 26
- exceptions: 84856
- distinct triples as printed: 84330
- after resolving direction (the same fact reported from both ends of an edge, 'A X-> B' and 'B X<- A'): 84283
- after merging inverse relations (NELL's 'inverseof' prefix; no effect if the dataset has none): 84283
- unique facts used in the tables below (--merge inverse): 84283 (0.006753 of the exceptions merged into another one)
- types: LabelException 401, LinkException 228
- score: median 0.9999, p90 1, p99 1, max 1

### Per rule

| rule | queries | with exc. | exceptions | unique | merged share | exc./query (median, max) | score median | p90 | p99 | max | Label share |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ρ0 | 10 | 10 | 16467 | 16443 | 0.001457 | 136, 6447 | 0.9999 | 0.9999 | 0.9999 | 1 | 0.8355 |
| ρ1 | 10 | 10 | 52369 | 52215 | 0.002941 | 1724, 16327 | 0.9999 | 1 | 1 | 1 | 0.6186 |
| ρ2 | 10 | 6 | 16020 | 15625 | 0.02466 | 55, 12488 | 0.9999 | 0.9999 | 0.9999 | 1 | 0.382 |
| ρ3 | 10 | 0 | 0 | 0 | - | 0, 0 | - | - | - | - | - |
| ρ4 | 10 | 0 | 0 | 0 | - | 0, 0 | - | - | - | - | - |
| ρ5 | 10 | 0 | 0 | 0 | - | 0, 0 | - | - | - | - | - |
| ρ6 | 10 | 0 | 0 | 0 | - | 0, 0 | - | - | - | - | - |

### Top 15 exceptions (unique facts)

| score | fact | type | query | context of the query |
|---|---|---|---|---|
| 1 | British_Columbia_Intercollegiate_Hockey_League -[team]-> University_of_Victoria | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Tr%C3%A9sor_Luntala -[team]-> Grasshopper | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Frank_Jonke -[team]-> Notre_Dame_College | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Nenad_Begovi%C4%87 -[team]-> Romorantin | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Dan_Pelc -[team]-> New_Hampshire_Wildcats | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | John_Benson_%28footballer_born_1991%29 -[team]-> ASPIRE_Academy_for_Sports_Excellence | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Santiago_Solari -[team]-> Renato_Cesarini | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Georgios_Gougoulias -[team]-> Pavlos_Melas | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Paul_Foster_%28footballer%29 -[team]-> Avala | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Walter_Paz -[team]-> Gil_Vicente | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | William_Maldonado -[team]-> Promesas | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Mandla_Zwane -[team]-> Gil_Vicente | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Craig_Foster_%28footballer%29 -[team]-> Avala | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Cristian_Rolando_Ledesma -[team]-> Vencedor | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Douglas_Hyde -[profession]-> University_College_Dublin | LinkException | p12 | SoccerPlayer -[team]-> * |

### Top 15 contexts, each with its strongest exception (one fact per context)

| # | score | context | query | strongest fact | type | tied | exceptions in context |
|---|---|---|---|---|---|---|---|
| 1 | 1 | SoccerPlayer -[team]-> * | p12 (ρ1) | British_Columbia_Intercollegiate_Hockey_League -[team]-> University_of_Victoria | LabelException | +7 | 11857 |
| 2 | 1 | MusicalArtist -[associatedBand]-> * | p16 (ρ1) | Foe_Life -[album]-> Mack_10 | LinkException | +9 | 16327 |
| 3 | 1 | Bird -[class]-> Animal | p06 (ρ0) | Ribbed_elimia -[binomialAuthority]-> Jay | LinkException |  | 4190 |
| 4 | 1 | Mammal -[class]-> Animal | p05 (ρ0) | Domesticated_silver_fox -[binomial]-> Red_Fox | LinkException | +1 | 5011 |
| 5 | 1 | Reptile -[class]-> Animal | p07 (ρ0) | Fancy_rat -[class]-> Tetrapod | LabelException | +1 | 6447 |
| 6 | 1 | LunarCrater -[*]-> * | p23 (ρ2) | Peter_Butler_%28surgeon%29 -[employer]-> London | LinkException | +9 | 12488 |
| 7 | 0.9999 | MusicalWork -[language]-> * | p11 (ρ1) | Latin_America -[language]-> Spanish_language | LabelException | +1 | 13458 |
| 8 | 0.9999 | Film -[narrator]-> * | p10 (ρ1) | Hope_for_Haiti_Now:_A_Global_Benefit_for_Earthquake_Relief -[developer]-> George_Clooney | LinkException | +6 | 6791 |
| 9 | 0.9999 | Reptile -[family]-> * | p18 (ρ1) | Vesicular_exanthema_of_swine_virus -[family]-> Caliciviridae | LabelException | +3 | 1789 |
| 10 | 0.9998 | PrimeMinister -[birthPlace]-> * | p13 (ρ1) | Meri_Jeevan_Yatra -[country]-> Allahabad | LabelException |  | 1659 |
| 11 | 0.9997 | AutoRacingLeageu -[*]-> * | p20 (ρ2) | Nintendo_Power -[publisher]-> Nintendo | LabelException | +1 | 2561 |
| 12 | 0.9996 | Non-ProfitOrganisation -[keyPerson]-> * | p17 (ρ1) | Siskiyou_Mountains -[state]-> California | LabelException | +7 | 126 |
| 13 | 0.9996 | Convention -[*]-> * | p22 (ρ2) | Sviatopolk_I_of_Kiev -[spouse]-> Boles%C5%82aw_I_Chrobry | LinkException |  | 142 |
| 14 | 0.998 | BaseballLeague -[team]-> City | p09 (ρ0) | S._David_Griggs -[birthPlace]-> Portland%2C_Oregon | LabelException | +6 | 402 |
| 15 | 0.9973 | VideoGame -[producer]-> Person | p00 (ρ0) | Apogee_of_Fear -[cinematography]-> Richard_Garriott | LinkException |  | 146 |

### Most frequent links (unique facts)

| link | facts | share | mean score | queries |
|---|---|---|---|---|
| artist | 16300 | 0.1934 | 0.9999 | 1 |
| birthPlace | 14500 | 0.172 | 0.9997 | 8 |
| language | 13459 | 0.1597 | 0.9999 | 2 |
| team | 11842 | 0.1405 | 0.9999 | 2 |
| order | 9214 | 0.1093 | 0.9994 | 5 |
| starring | 6782 | 0.08047 | 0.9999 | 1 |
| class | 6470 | 0.07677 | 0.9991 | 5 |
| publisher | 2560 | 0.03037 | 0.9996 | 2 |
| genus | 1786 | 0.02119 | 0.9992 | 4 |
| discoverer | 356 | 0.004224 | 0.9973 | 1 |
| location | 212 | 0.002515 | 0.9875 | 4 |
| designer | 147 | 0.001744 | 0.9924 | 3 |
| deathCause | 125 | 0.001483 | 0.9945 | 2 |
| commander | 103 | 0.001222 | 0.9982 | 2 |
| binomialAuthority | 61 | 0.0007238 | 0.9821 | 3 |

### Most frequent head and tail labels

| head label | facts | tail label | facts |
|---|---|---|---|
| (none) | 82217 | (none) | 84282 |
| The_Legend_of_Zelda | 13 | Midnight | 1 |

### Entities in exceptions of the most queries

| entity | queries | max score |
|---|---|---|
| Robert_Mercier | 3 | 1 |
| Siga_Tandia | 3 | 1 |
| Philippe_Troussier | 3 | 1 |
| Lassana_Diarra | 3 | 1 |
| Serigne_Diop | 3 | 1 |
| Pierre_Chayrigu%C3%A8s | 3 | 1 |
| Jean-Marc_Pilorget | 3 | 1 |
| Filipe_Teixeira | 3 | 1 |
| S%C3%A9bastien_Bassong | 3 | 1 |
| Franck_Queudrue | 3 | 1 |
| Jonathan_Biabiany | 3 | 1 |
| Arnaud_Souquet | 3 | 1 |
| Armand_Traor%C3%A9 | 3 | 1 |
| Hamza_Bencherif | 3 | 1 |
| Jonathan_T%C3%A9hou%C3%A9 | 3 | 1 |

### Queries with the most and the strongest exceptions

| query | rule | context | matching heads | exceptions | max score | time [s] |
|---|---|---|---|---|---|---|
| p16 | ρ1 | MusicalArtist -[associatedBand]-> * | - | 16327 | 1 | 248 |
| p11 | ρ1 | MusicalWork -[language]-> * | - | 13458 | 0.9999 | 10.96 |
| p23 | ρ2 | LunarCrater -[*]-> * | - | 12488 | 1 | - |
| p12 | ρ1 | SoccerPlayer -[team]-> * | - | 11857 | 1 | 552.1 |
| p10 | ρ1 | Film -[narrator]-> * | - | 6791 | 0.9999 | 19.64 |
| p07 | ρ0 | Reptile -[class]-> Animal | - | 6447 | 1 | 0.308 |
| p05 | ρ0 | Mammal -[class]-> Animal | - | 5011 | 1 | 1.594 |
| p06 | ρ0 | Bird -[class]-> Animal | - | 4190 | 1 | 1.344 |
| p20 | ρ2 | AutoRacingLeageu -[*]-> * | - | 2561 | 0.9997 | - |
| p18 | ρ1 | Reptile -[family]-> * | - | 1789 | 0.9999 | 3.771 |
| p13 | ρ1 | PrimeMinister -[birthPlace]-> * | - | 1659 | 0.9998 | 37.02 |
| p29 | ρ2 | Bacteria -[*]-> * | - | 719 | 0.9973 | - |
| p09 | ρ0 | BaseballLeague -[team]-> City | - | 402 | 0.998 | 0.038 |
| p00 | ρ0 | VideoGame -[producer]-> Person | - | 146 | 0.9973 | 0.206 |
| p22 | ρ2 | Convention -[*]-> * | - | 142 | 0.9996 | - |

## dbpedia_freq_o1

- queries: 70, with at least one exception: 26
- exceptions: 143733
- distinct triples as printed: 139928
- after resolving direction (the same fact reported from both ends of an edge, 'A X-> B' and 'B X<- A'): 139881
- after merging inverse relations (NELL's 'inverseof' prefix; no effect if the dataset has none): 139881
- unique facts used in the tables below (--merge inverse): 139881 (0.0268 of the exceptions merged into another one)
- types: LabelException 418, LinkException 326
- score: median 1, p90 1, p99 1, max 1

### Per rule

| rule | queries | with exc. | exceptions | unique | merged share | exc./query (median, max) | score median | p90 | p99 | max | Label share |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ρ0 | 10 | 10 | 16467 | 16443 | 0.001457 | 136, 6447 | 0.9999 | 0.9999 | 0.9999 | 1 | 0.8355 |
| ρ1 | 10 | 10 | 111246 | 107813 | 0.03086 | 6382, 43851 | 1 | 1 | 1 | 1 | 0.5109 |
| ρ2 | 10 | 6 | 16020 | 15625 | 0.02466 | 55, 12488 | 0.9999 | 0.9999 | 0.9999 | 1 | 0.382 |
| ρ3 | 10 | 0 | 0 | 0 | - | 0, 0 | - | - | - | - | - |
| ρ4 | 10 | 0 | 0 | 0 | - | 0, 0 | - | - | - | - | - |
| ρ5 | 10 | 0 | 0 | 0 | - | 0, 0 | - | - | - | - | - |
| ρ6 | 10 | 0 | 0 | 0 | - | 0, 0 | - | - | - | - | - |

### Top 15 exceptions (unique facts)

| score | fact | type | query | context of the query |
|---|---|---|---|---|
| 1 | British_Columbia_Intercollegiate_Hockey_League -[team]-> University_of_Victoria | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Tr%C3%A9sor_Luntala -[team]-> Grasshopper | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Frank_Jonke -[team]-> Notre_Dame_College | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Nenad_Begovi%C4%87 -[team]-> Romorantin | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Dan_Pelc -[team]-> New_Hampshire_Wildcats | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | John_Benson_%28footballer_born_1991%29 -[team]-> ASPIRE_Academy_for_Sports_Excellence | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Santiago_Solari -[team]-> Renato_Cesarini | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Georgios_Gougoulias -[team]-> Pavlos_Melas | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Matt_Bahr -[team]-> Colorado_Caribous | LabelException/UniqueEntityException | p12 | SoccerPlayer -[team]-> * |
| 1 | Martti_J%C3%A4rventie -[team]-> HIFK | LabelException/UniqueEntityException | p12 | SoccerPlayer -[team]-> * |
| 1 | Kimmo_Kuhta -[team]-> HIFK | LabelException/UniqueEntityException | p12 | SoccerPlayer -[team]-> * |
| 1 | Chris_Bahr -[team]-> Philadelphia_Atoms | LabelException/UniqueEntityException | p12 | SoccerPlayer -[team]-> * |
| 1 | Paul_Foster_%28footballer%29 -[team]-> Avala | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | Walter_Paz -[team]-> Gil_Vicente | LabelException | p12 | SoccerPlayer -[team]-> * |
| 1 | William_Maldonado -[team]-> Promesas | LabelException | p12 | SoccerPlayer -[team]-> * |

### Top 15 contexts, each with its strongest exception (one fact per context)

| # | score | context | query | strongest fact | type | tied | exceptions in context |
|---|---|---|---|---|---|---|---|
| 1 | 1 | SoccerPlayer -[team]-> * | p12 (ρ1) | British_Columbia_Intercollegiate_Hockey_League -[team]-> University_of_Victoria | LabelException | +7 | 11911 |
| 2 | 1 | PrimeMinister -[birthPlace]-> * | p13 (ρ1) | Association_of_Volleyball_Professionals -[country]-> United_States | LabelException | +2 | 43851 |
| 3 | 1 | MusicalArtist -[associatedBand]-> * | p16 (ρ1) | Capital_News_Service_%28Michigan%29 -[owner]-> Michigan_State_University | LinkException | +3 | 26890 |
| 4 | 1 | Bird -[class]-> Animal | p06 (ρ0) | Ribbed_elimia -[binomialAuthority]-> Jay | LinkException |  | 4190 |
| 5 | 1 | Mammal -[class]-> Animal | p05 (ρ0) | Domesticated_silver_fox -[binomial]-> Red_Fox | LinkException | +1 | 5011 |
| 6 | 1 | MusicalWork -[language]-> * | p11 (ρ1) | Robert_Beaven -[birthPlace]-> England | LabelException | +1 | 13494 |
| 7 | 1 | Reptile -[class]-> Animal | p07 (ρ0) | Fancy_rat -[class]-> Tetrapod | LabelException | +1 | 6447 |
| 8 | 1 | LunarCrater -[*]-> * | p23 (ρ2) | Peter_Butler_%28surgeon%29 -[employer]-> London | LinkException | +9 | 12488 |
| 9 | 0.9999 | Film -[narrator]-> * | p10 (ρ1) | Hope_for_Haiti_Now:_A_Global_Benefit_for_Earthquake_Relief -[developer]-> George_Clooney | LinkException | +6 | 6882 |
| 10 | 0.9999 | Reptile -[family]-> * | p18 (ρ1) | Vesicular_exanthema_of_swine_virus -[family]-> Caliciviridae | LabelException | +3 | 1808 |
| 11 | 0.9997 | AutoRacingLeageu -[*]-> * | p20 (ρ2) | Nintendo_Power -[publisher]-> Nintendo | LabelException | +1 | 2561 |
| 12 | 0.9997 | Non-ProfitOrganisation -[keyPerson]-> * | p17 (ρ1) | EIKI -[division]-> California | LinkException | +5 | 5882 |
| 13 | 0.9996 | Convention -[*]-> * | p22 (ρ2) | Sviatopolk_I_of_Kiev -[spouse]-> Boles%C5%82aw_I_Chrobry | LinkException |  | 142 |
| 14 | 0.998 | BaseballLeague -[team]-> City | p09 (ρ0) | S._David_Griggs -[birthPlace]-> Portland%2C_Oregon | LabelException | +6 | 402 |
| 15 | 0.9973 | VideoGame -[producer]-> Person | p00 (ρ0) | Apogee_of_Fear -[cinematography]-> Richard_Garriott | LinkException |  | 146 |

### Most frequent links (unique facts)

| link | facts | share | mean score | queries |
|---|---|---|---|---|
| country | 43816 | 0.3132 | 0.9999 | 4 |
| artist | 26822 | 0.1917 | 1 | 1 |
| language | 13459 | 0.09622 | 0.9999 | 2 |
| birthPlace | 12821 | 0.09166 | 0.9998 | 8 |
| team | 11842 | 0.08466 | 0.9999 | 2 |
| order | 9214 | 0.06587 | 0.9994 | 5 |
| starring | 6854 | 0.049 | 0.9999 | 1 |
| class | 6470 | 0.04625 | 0.9991 | 5 |
| state | 2938 | 0.021 | 0.9993 | 3 |
| publisher | 2562 | 0.01832 | 0.9996 | 2 |
| genus | 1787 | 0.01278 | 0.9992 | 4 |
| discoverer | 356 | 0.002545 | 0.9973 | 1 |
| location | 213 | 0.001523 | 0.9875 | 5 |
| designer | 147 | 0.001051 | 0.9924 | 3 |
| deathCause | 125 | 0.0008936 | 0.9945 | 2 |

### Most frequent head and tail labels

| head label | facts | tail label | facts |
|---|---|---|---|
| (none) | 136186 | (none) | 139879 |
| The_Year%27s_Best_Science_Fiction | 19 | Sixx | 2 |

### Entities in exceptions of the most queries

| entity | queries | max score |
|---|---|---|
| Michigan_State_University | 3 | 1 |
| Sonoma_State_University | 3 | 1 |
| California_State_University%2C_Sacramento | 3 | 1 |
| University_of_California%2C_Los_Angeles | 3 | 1 |
| Westmont_College | 3 | 1 |
| University_of_Akron | 3 | 1 |
| Irvine_Valley_College | 3 | 1 |
| Brown_University | 3 | 1 |
| St._Louis_Community_College | 3 | 1 |
| Stanford_University | 3 | 1 |
| College_of_Mount_St._Joseph | 3 | 1 |
| United_States | 3 | 1 |
| _Ascension | 3 | 1 |
| _Legacy | 3 | 1 |
| Days_of_Thunder | 3 | 1 |

### Queries with the most and the strongest exceptions

| query | rule | context | matching heads | exceptions | max score | time [s] |
|---|---|---|---|---|---|---|
| p13 | ρ1 | PrimeMinister -[birthPlace]-> * | - | 43851 | 1 | 38.58 |
| p16 | ρ1 | MusicalArtist -[associatedBand]-> * | - | 26890 | 1 | 252.7 |
| p11 | ρ1 | MusicalWork -[language]-> * | - | 13494 | 1 | 10.64 |
| p23 | ρ2 | LunarCrater -[*]-> * | - | 12488 | 1 | - |
| p12 | ρ1 | SoccerPlayer -[team]-> * | - | 11911 | 1 | 570.1 |
| p10 | ρ1 | Film -[narrator]-> * | - | 6882 | 0.9999 | 19.07 |
| p07 | ρ0 | Reptile -[class]-> Animal | - | 6447 | 1 | 0.3 |
| p17 | ρ1 | Non-ProfitOrganisation -[keyPerson]-> * | - | 5882 | 0.9997 | 1.537 |
| p05 | ρ0 | Mammal -[class]-> Animal | - | 5011 | 1 | 1.608 |
| p06 | ρ0 | Bird -[class]-> Animal | - | 4190 | 1 | 1.351 |
| p20 | ρ2 | AutoRacingLeageu -[*]-> * | - | 2561 | 0.9997 | - |
| p18 | ρ1 | Reptile -[family]-> * | - | 1808 | 0.9999 | 3.897 |
| p29 | ρ2 | Bacteria -[*]-> * | - | 719 | 0.9973 | - |
| p09 | ρ0 | BaseballLeague -[team]-> City | - | 402 | 0.998 | 0.033 |
| p15 | ρ1 | Town -[twinCity]-> * | - | 296 | 0.9967 | 0.255 |

## Comparison between settings

Jaccard overlap of the top-k unique facts per query (ties at the cut-off included); 'same top-1' = the best exceptions of both runs share a fact.

### dist: o=0 vs o=1

| k | rule | queries | Jaccard median | Jaccard mean | same top-1 |
|---|---|---|---|---|---|
| 10 | ρ0 | 10 | 1 | 1 | 1 |
| 10 | ρ1 | 10 | 1 | 1 | 1 |
| 10 | ρ2 | 10 | 1 | 1 | 1 |
| 10 | ρ3 | 6 | 1 | 1 | 1 |
| 100 | ρ0 | 10 | 1 | 1 | - |
| 100 | ρ1 | 10 | 1 | 1 | - |
| 100 | ρ2 | 10 | 1 | 1 | - |
| 100 | ρ3 | 6 | 1 | 1 | - |

### o=0: dist vs freq

| k | rule | queries | Jaccard median | Jaccard mean | same top-1 |
|---|---|---|---|---|---|
| 10 | ρ0 | 10 | 0.1447 | 0.34 | 0.8 |
| 10 | ρ1 | 10 | 0.08894 | 0.182 | 0.3 |
| 10 | ρ2 | 6 | 0 | 0 | 0 |
| 100 | ρ0 | 10 | 0.06742 | 0.3946 | - |
| 100 | ρ1 | 10 | 0.004227 | 0.1367 | - |
| 100 | ρ2 | 6 | 0 | 1.334e-05 | - |

### o=1: dist vs freq

| k | rule | queries | Jaccard median | Jaccard mean | same top-1 |
|---|---|---|---|---|---|
| 10 | ρ0 | 10 | 0.1447 | 0.34 | 0.8 |
| 10 | ρ1 | 10 | 0.3745 | 0.3647 | 0.4 |
| 10 | ρ2 | 6 | 0 | 0 | 0 |
| 100 | ρ0 | 10 | 0.06742 | 0.3946 | - |
| 100 | ρ1 | 10 | 0.003768 | 0.1236 | - |
| 100 | ρ2 | 6 | 0 | 1.334e-05 | - |

### freq: o=0 vs o=1

| k | rule | queries | Jaccard median | Jaccard mean | same top-1 |
|---|---|---|---|---|---|
| 10 | ρ0 | 10 | 1 | 1 | 1 |
| 10 | ρ1 | 10 | 0.5821 | 0.5059 | 0.6 |
| 10 | ρ2 | 6 | 1 | 1 | 1 |
| 100 | ρ0 | 10 | 1 | 1 | - |
| 100 | ρ1 | 10 | 0.9993 | 0.7 | - |
| 100 | ρ2 | 6 | 1 | 1 | - |

# Plan du projet — à jour (tag `jalon-1` posé)

Ordre à respecter. Une étape à la fois, un commit Git par étape réussie.
Les étapes **[TOI]** se font à la main dans l'éditeur. Les étapes **[AGENT]** sont faites par l'agent de code.

---

## Jalon 1 : multijoueur coopératif de base — FAIT (tag `jalon-1`)

T0 à T6 réalisés, avec ces écarts réels (à connaître) :
- Le projet est le template **Third Person Blueprint** + module C++ `Salamander` ajouté à la main
  (Source/Salamander + Source/*.Target.cs + .uproject Modules). Le pawn est un **C++** (`ASalamanderCharacter`),
  le PlayerController reste le BP du template (non modifié).
- `IA_Sprint` / `IA_Crouch` créées par **script Python commandlet** (`Scripts/Editor/CreateInputActions.py`) ;
  le mapping clavier (Maj gauche / Ctrl gauche) est ajouté **en C++** dans `PossessedBy` (MapKey/UnmapKey).
- Map `Maps/Monde_Test` (sol vert, ciel, lumières) + `M_PlayerColor` (paramètre `BodyColor`) + `M_SolGreen`
  créés par script (`Scripts/Editor/CreateWorldAssets.py`). `GameDefaultMap=/Game/Maps/Menu` (clients),
  le serveur démarre sur `Monde_Test?listen` (argument).
- En CLI (`-game`), le moteur écrase l'option `?Name=` : on lit **`?Pseudo=`** en priorité (voir `GetPseudoOption`).
- Écran de saisie du pseudo/couleur : **Slate 100% C++** (`SLoginWidget`), map `Menu`, bouton → `open <ip>?Pseudo=..&Color=..`.
- Sauvegarde : `ISaveBackend` + `UFileSaveBackend` (USaveGame, `Saved/SaveGames/<pseudo>.sav`), chargé au login,
  sauvé au logout + toutes les 60 s. Clé = pseudo, `TODO(identity)`.
- Tests CLI automatisés côté serveur via paramètres : `-TestAutoDamage=<n>`, `-TestAutoLevel=<n>`,
  `-ConnectTo= -ConnectPseudo= -ConnectColor=`, `-TestAutoMove/Sprint/Crouch=1` (voir AGENTS.md « Leçons »).

Tests validés en CLI (serveur dédié `-server -game` + clients `-game -d3d11`, logs dans `Saved/Logs/NetTest/`) :
- 2 clients se voient ; `Health` répliquée (100→70→40 vu des 2 côtés) ; couleur par joueur (OnRep_Color) ;
  pseudo doublon refusé (« Pseudo deja pris »).
- Sauvegarde persistante : Level 9 + couleur retrouvés après **redémarrage du serveur**.
- `net.PktLag=100` + sprint + crouch + déplacement : **0 correction serveur** côté client (prédiction OK).
- Mort à 0 PV → respawn au point de départ après 3 s (voir jalon 2 étape 1).
- Menu Slate : client sans IP → écran → connexion → `Login Name=Bob` → monde + profil restauré.

Reste à valider **[TOI]** (éditeur, à la main) :
- T0 bis : Play, Number of Players = 2, Net Mode = Play As Client → voir les 2 personnages.
- Test visuel : sol vert, ciel bleu, couleurs des persos, sprint/accroupi visibles chez l'autre joueur.
- `Scripts/start_server.bat` / `start_client.bat` en conditions réelles (2 fenêtres client).
- Installer le workload VS 2026 « Game Development with C++ » **hors période de compilation**.
- Connexion depuis un autre PC : IP locale + port UDP 7777 (même réseau local) ; depuis l'extérieur :
  redirection UDP 7777 sur la box ou VPN type Tailscale (non vérifié ici).

---

## Jalon 2 : un combat qui a du sens — voir TASKS_JALON2.md (plan detaille, GAS)

Ordre : T7 migrer les stats vers GAS (AttributeSet Health/MaxHealth, ASC sur PlayerState, DebugDamage via GameplayEffect)
-> T8 mort/reapparition via GameplayTag + DA_DeathRules (PROVISOIRE) -> T9 attaque epee 3 coups (GA_MeleeAttack,
traces serveur, DA_MeleeAttackStats) -> T10 ennemi simple (machine a etats C++, attaque annoncee) -> T11 boss +
recompense sauvegardee (migration SaveVersion) -> T12 test en famille.

Note : une mort/reapparition simple (sans GAS) a deja ete faite (commit 13ac3f0) ; elle sera remplacee par T8.
Regles GAS dans AGENTS.md (section "Regles GAS (jalon 2)").

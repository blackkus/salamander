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

## Jalon 2 : un combat qui a du sens (en cours)

Si le combat n'est pas plaisant, le reste ne sauvera pas le jeu. Ordre :\n
1. **Mort et réapparition** — FAIT (`13ac3f0`) : à 0 PV, mort côté serveur, respawn au point de départ
   après 3 s, PV pleins, pas de pénalité (décision en attendant : pénalités de mort = décision ouverte).
2. **Ennemi simple** [AGENT] : se déplace, poursuit le joueur, attaque avec **annonce visuelle avant de frapper**
   (ex: flash/pose telegraphiée), dégâts calculés côté serveur. Test CLI : spawn auto de l'ennemi, poursuite,
   attaque annoncée puis dégâts sur le joueur (logs).
3. **Attaque à l'épée du joueur** [AGENT] : enchaînement de coups (combo), détection des touches **serveur**,
   dégâts **serveur**. Test CLI : `-TestAutoAttack=1` simule les coups ; l'ennemi perd des PV (logs 2 clients).
4. **Boss** [AGENT] : 2-3 attaques lisibles (telegraphs), barre de vie, récompense enregistrée dans la sauvegarde
   du joueur (champ `BossesVaincus` dans `UPlayerProfile` par ex.). Test : tuer le boss → sauvegarde → reconnexion → récompense retrouvée.
5. **Test en famille** [TOI] : vrai essai avec les enfants, noter ce qui est frustrant ou flou → itérer.

Règles qui tiennent pour tout le jalon 2 (AGENTS.md) : serveur autoritaire (dégâts/hit detection/PV côté serveur),
RPC `Server_...` pour les intentions, contenu en données (Data Tables/Data Assets), pas de Blueprint modifié,
un fichier par classe, compile après chaque étape, un commit par étape.

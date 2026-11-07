# Contexte du projet pour l'agent de code

> Nom du fichier : à adapter si l'agent attend un autre nom (voir sa documentation).
> Sinon, donne-lui ce fichier à lire au début de chaque session.

## Projet
- Nom de code : `Salamander` (module C++ du même nom).
- Moteur : Unreal Engine 5.8.4, C++. Windows.
- Action-RPG en ligne, coopératif, serveur dédié autoritaire, 10 joueurs maximum par instance au départ.
- Développeur : une seule personne, expérimentée (C#, Java). Le C++ Unreal est récent pour lui : explique brièvement tes choix.
- Le jeu sera vendu plus tard. N'utilise dans le code, les noms de classes, les assets ou les textes aucun nom, personnage ou univers d'œuvres existantes.

## Objectif du jalon 1
Plusieurs joueurs se connectent à un serveur et se voient dans un monde simple :
- sol vert, ciel bleu ;
- chaque joueur choisit un pseudo et une couleur à la connexion ;
- chaque joueur : niveau 1, 100 PV, pas de mana ;
- déplacements de base : marcher, courir, sauter, sprinter, s'accroupir ;
- pseudo, couleur et niveau sont sauvegardés **côté serveur** et retrouvés à la reconnexion.

## Règles non négociables
1. **Le serveur est l'autorité.** Santé, niveau, XP, inventaire, dégâts et loot sont calculés et modifiés uniquement sur le serveur. Les clients envoient des intentions (RPC `Server_...`), jamais des résultats.
2. **Sauvegarde derrière une interface** (`ISaveBackend`), avec un **numéro de version** dans chaque sauvegarde. Aujourd'hui : fichiers sur le disque du serveur.
3. **Le contenu est en données, pas en code** : valeurs d'équilibrage et listes dans des Data Tables (CSV/JSON) ou des Data Assets, pas en dur dans le C++.
4. **Ne modifie jamais** les fichiers binaires (`.uasset`, `.umap`) ni les Blueprints. Si une étape les exige, écris la liste exacte des étapes à faire à la main dans l'éditeur, et arrête-toi.
5. Ne touche pas aux fichiers du moteur Unreal.
6. Code réseau : toute variable répliquée est déclarée avec `UPROPERTY(Replicated)` ou `ReplicatedUsing`, avec `GetLifetimeReplicatedProps`. Les actions de gameplay passent par des RPC serveur validés.
7. Conventions Unreal : préfixes `A`, `U`, `F`, `E`, `I`, `T`. Un fichier par classe.
8. Petits changements. Compile après chaque modification. Ne passe pas à l'étape suivante si ça ne compile pas.

## Hors périmètre pour l'instant (ne pas implémenter)
Gravité sphérique, terrain modifiable, PvP, armes, combat, quêtes, plusieurs planètes ou instances, comptes et authentification, base de données externe, interface de jeu complète.

## Compiler
Commande à adapter aux chemins de la machine (à remplir une fois) :
```
"C:\Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" SalamanderEditor Win64 Development -Project="C:\Users\Blackkus\Documents\Unreal Projects\Salamander\Salamander.uproject" -WaitMutex
```
Une tâche est terminée quand : le projet compile sans erreur ni avertissement nouveau, et le test décrit dans la tâche est réalisable.

## Identité des joueurs (provisoire)
Pour le jalon 1, la clé de sauvegarde d'un joueur est son pseudo. C'est volontairement simple et **non sécurisé** (n'importe qui peut prendre un pseudo). À remplacer par un identifiant unique de compte avant toute ouverture au public. Marque ce point par un commentaire `TODO(identity)`.

## Décisions encore ouvertes (ne pas trancher sans demander)
Pénalités de mort, qui héberge les serveurs en production, PvP, effet exact des scénarios uniques, équilibrage.

## Leçons du jalon 1 (ne pas répéter)

- **Projet C++ ajouté à un projet Blueprint** : il faut `Source/<Projet>.Target.cs` + `Source/<Projet>Editor.Target.cs` (sinon UBT dit « has no code » et crée des Target.cs temporaires qui mènent à un CS0101). Un résidu `Intermediate/Source/*.Build.cs` provoque aussi CS0101 : vider `Intermediate/Source` avant de compiler.
- **Ne jamais compiler pendant qu'un process Unreal tourne** sur le projet (DLL verrouillé → LNK1104). Tuer les process Unreal avant chaque Build.bat.
- **API Python UE 5.8** : `EditorAssetLibrary.create_asset` n'existe plus → `AssetToolsHelpers.get_asset_tools().create_asset(nom_simple, package, class, factory)` ; `save_asset` prend un **path string**, pas l'objet ; factory = `unreal.InputAction_Factory` (underscore) ; `InputAction.value_type` est read-only ; `unreal.Keys` n'existe plus et `unreal.Key('X')` ne marche pas → mapper les touches en **C++** (`UInputMappingContext::UnmapKey` puis `MapKey` + `Triggers.Add(NewObject<UInputTriggerPressed>())`, idempotent) ; `LevelEditorSubsystem.new_level/load_level/save_current_level` remplace `EditorLevelLibrary` (déprécié) ; `MaterialExpressionVectorParameter.ParameterName` non exposé en Python → helper C++ (`USalamanderMaterialLib`) ; `connect_material_property(node, '', MP_BASE_COLOR)` prend 3 args.
- **FParse::Value tronque la valeur au '?'** → passer les options CLI en paramètres séparés (`-ConnectTo=`, `-ConnectPseudo=`, jamais `-ConnectTo=ip?a=b`).
- **En CLI (-game), le moteur écrase l'option `?Name=`** par le surnom local → lire `?Pseudo=` en priorité, `?Name=` en fallback (éditeur).
- **Menu client** : `GEngine->SetClientTravel(world, url, TRAVEL_Absolute)` (pas `ClientTravel` sur le PC, qui ne part pas en standalone sans pawn) ; widget Slate via `GEngine->GameViewport->AddViewportWidgetContent(...)`.
- **Clients en CLI sur la même machine que le serveur** : crash RHI en DX12 → lancer les clients avec `-d3d11` ; `-unattended` contre les boîtes de dialogue bloquantes ; `-ABSLOG=<fichier>` pour des logs séparés par process.
- **Prédiction custom (FSavedMove)** : renseigner le membre `ClientPredictionData` de la classe mère via `const_cast` dans `GetPredictionData_Client()` — le moteur y accède directement ensuite, sinon crash null côté client.
- **Composant répliqué créé dynamiquement côté serveur** : utiliser `AddInstanceComponent` (pas seulement `RegisterComponent`), sinon il n'est pas répliqué.
- **USaveGame** : non copiable par `operator=` → copier champ par champ. `FColor::ToLinearColor()` n'existe plus en 5.8 → `FLinearColor(FColor)`.
- **La console du serveur (-server -log) n'est PAS pilotable via un pipe stdin** → tests CLI auto via paramètres (`-TestAutoDamage=`, `-TestAutoLevel=`, `-ConnectTo=`, `-TestAutoMove/Sprint/Crouch=`).
- Toujours lancer les commandlets éditeur Python avec `-unattended -nullrhi -nosound -log`.

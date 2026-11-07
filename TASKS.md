# Plan du jalon 1 : tâches

Ordre à respecter. Une tâche à la fois, un commit Git par tâche réussie.
Les étapes **[TOI]** se font à la main dans l'éditeur. Les étapes **[AGENT]** sont à donner à l'agent de code (copie le bloc « Prompt »).

---

## T0. Mise en place [TOI] (environ 1 h)

1. Visual Studio avec les composants C++ pour jeux (vérifie dans la doc Epic la version et les composants exigés par UE 5.8).
2. Dépôt GitHub : crée-le avec le modèle `.gitignore` « Unreal Engine » proposé par GitHub.
3. Git LFS pour les assets binaires. Ajoute un fichier `.gitattributes` :
   ```
   *.uasset filter=lfs diff=lfs merge=lfs -text
   *.umap   filter=lfs diff=lfs merge=lfs -text
   *.png    filter=lfs diff=lfs merge=lfs -text
   *.fbx    filter=lfs diff=lfs merge=lfs -text
   ```
   puis `git lfs install`.
4. Crée le projet Unreal : modèle **Third Person**, **C++** (pas Blueprint), sans Starter Content, nom sans espace ni accent. Le nom devient le nom du module C++ et est pénible à changer ensuite. Le titre affiché au joueur est un réglage séparé et peut changer librement.
5. Copie `AGENTS.md` et `TASKS.md` à la racine du dépôt. Remplace `<NomProjet>`, `<CheminUE>` et `<CheminProjet>`.
6. Ouvre le projet, vérifie qu'il compile, lance le jeu une fois. Premier commit et push.

## T0 bis. Test réseau sans rien coder [TOI] (15 min)

Dans l'éditeur : Play, **Number of Players = 2**, **Net Mode = Play As Client**. Unreal lance alors un serveur dédié en mémoire et deux clients. Tu dois voir les deux personnages, et chacun voit l'autre bouger. Si ça marche, ta base réseau fonctionne.

---

## T1. Statistiques du joueur [AGENT]

**Prompt :**
> Lis AGENTS.md. Crée un composant `UStatsComponent` attaché au personnage, avec `Health` (répliqué, `ReplicatedUsing`, valeur de départ 100) et `MaxHealth` = 100. Le niveau est stocké dans le `PlayerState` (`Level` répliqué, départ 1). Aucune notion de mana. Seul le serveur peut modifier ces valeurs (fonctions `ServerOnly`). Ajoute une commande console `DebugDamage <n>` exécutée sur le serveur pour tester. Ne touche à aucun Blueprint.

**Test :** à 2 clients, la commande de dégâts sur le serveur change la santé, et les deux clients voient la nouvelle valeur (log).

## T2. Mouvement : courir, sauter, sprinter, s'accroupir [AGENT + TOI]

[TOI] avant : dans l'éditeur, crée les actions d'entrée `IA_Sprint` et `IA_Crouch` et ajoute-les au contexte de mapping du template (touches : Maj gauche, Ctrl gauche).

**Prompt :**
> Lis AGENTS.md. Ajoute le sprint et l'accroupissement au personnage. Marcher, courir et sauter existent déjà dans le modèle. Accroupi : utilise le système natif du `CharacterMovementComponent`. Sprint : implémente-le dans une sous-classe du `CharacterMovementComponent` avec un `FSavedMove_Character` personnalisé, pour que la vitesse soit prédite côté client sans corrections visibles (pas de rubberbanding). Les actions d'entrée sont des propriétés `UPROPERTY` exposées, que je renseignerai dans l'éditeur. Si l'approche complète te semble trop risquée, propose-moi la solution simple avec ses défauts avant de coder.

**Test :** à 2 clients avec 60 à 100 ms de latence simulée (`Net PktLag=60` en console), sprint et accroupi sont visibles chez l'autre joueur, sans à-coups répétés.

C'est la tâche la plus risquée de la soirée. Si elle bloque, passe à T3 et reviens-y.

## T3. Monde simple [TOI] (15 min)

- Sol : un matériau de couleur verte sur le sol du niveau.
- Ciel : le modèle contient déjà un ciel. S'il est bleu, laisse-le. Sinon, ajuste le Sky Atmosphere.
- Crée une carte propre au projet (`Maps/Monde_Test`) et définis-la comme carte par défaut.
- Crée un matériau `M_PlayerColor` avec un paramètre vectoriel nommé `BodyColor`, et applique-le au mesh du personnage.

## T4. Pseudo et couleur à la connexion [AGENT]

**Prompt :**
> Lis AGENTS.md. Le joueur donne son pseudo et sa couleur au moment de se connecter via les options d'URL : `open <ip>?Name=Bob?Color=FF8800`. Le `GameMode` lit ces options à la connexion et les stocke dans le `PlayerState` (`PlayerName` et `ColorRgb`, répliqués). À la réplication de la couleur (`OnRep`), crée une instance de matériau dynamique à partir de `M_PlayerColor` (paramètre `BodyColor`) et applique-la au mesh du personnage. Refuse la connexion si le pseudo est vide, trop long (plus de 16 caractères) ou déjà utilisé par un joueur connecté. Pas d'interface graphique à ce stade.

**Test :** deux clients avec des pseudos et couleurs différents se voient dans la bonne couleur. Un pseudo déjà pris est refusé.

## T5. Sauvegarde côté serveur [AGENT]

**Prompt :**
> Lis AGENTS.md. Crée une interface `ISaveBackend` (Load / Save d'un profil joueur) et une implémentation `FileSaveBackend` basée sur `USaveGame`, un fichier par joueur sur le disque du serveur. Le profil `UPlayerProfile` contient : `SaveVersion` (entier), `Name`, `ColorRgb`, `Level`. La clé est le pseudo, avec un `TODO(identity)`. Charge le profil à la connexion (sur le serveur seulement), sauvegarde à la déconnexion et toutes les 60 secondes. Si le fichier a une version plus ancienne, prévois une fonction de migration (vide pour l'instant). Aucune sauvegarde n'est écrite par un client.

**Test :** un joueur se connecte, change de niveau via une commande de debug serveur, se déconnecte, se reconnecte : son niveau et sa couleur sont retrouvés. Arrête et relance le serveur : toujours retrouvés.

## T6. Lancer un serveur et s'y connecter [AGENT + TOI]

**Prompt :**
> Lis AGENTS.md. Écris deux scripts Windows dans `Scripts/` : `start_server.bat` (lance le serveur dédié avec l'exécutable de l'éditeur, en mode `-server -game -log`, carte `Monde_Test`, port UDP 7777) et `start_client.bat` (demande un pseudo et une couleur puis se connecte à l'adresse donnée en argument, par défaut 127.0.0.1). Les chemins viennent d'une variable en tête de script. Documente en commentaire ce qui est à valider.

**Test :** serveur lancé dans une fenêtre, deux clients lancés dans deux autres fenêtres avec deux pseudos différents.

[TOI] Connexion depuis un autre PC :
- **Même réseau local :** l'adresse IP locale de ton PC, port UDP 7777.
- **Depuis l'extérieur :** redirige le port UDP 7777 sur ta box, ou utilise un réseau privé virtuel de type Tailscale. Je n'ai pas vérifié le détail de ces deux options dans ce contexte.

À ma connaissance, l'exécutable de serveur dédié *packagé* nécessite un moteur compilé depuis les sources. Ne te lance pas là-dedans ce soir : la méthode ci-dessus suffit pour jouer avec tes enfants.

---

## Interface de saisie du pseudo (après)

Le champ de saisie à l'écran est à part, parce que les interfaces de menu se construisent normalement avec des Blueprints. Deux voies, à décider demain : un écran fait à la main dans l'éditeur, ou une interface écrite en C++ (Slate) par l'agent.

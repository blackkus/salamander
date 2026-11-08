# Jalon 2 : un combat qui a du sens (avec GAS)

Même méthode que le jalon 1 : une tâche à la fois, un commit par tâche réussie.
**[TOI]** = à faire à la main dans l'éditeur. **[AGENT]** = à donner à l'agent de code.

## Décisions prises
- Système de compétences : **Gameplay Ability System (GAS)**.

## Décisions pas encore prises (ne pas les figer dans le code)
- Pénalités de mort, délai de réapparition, mort en boss.
- Source des animations et des modèles 3D.
- Effet exact d'un scénario unique accompli.

Règle : tout ce qui dépend de ces décisions est une **valeur dans un Data Asset ou une Data Table**, avec une valeur provisoire marquée `PROVISOIRE`. Changer d'avis ne doit pas demander de toucher au code.

## À ajouter dans AGENTS.md avant de commencer

```
## Règles GAS (jalon 2)
- L'AbilitySystemComponent des joueurs est sur le PlayerState (il survit à la mort et à la réapparition). Pour les ennemis et boss, il est sur le personnage.
- Mode de réplication : Mixed pour les joueurs, Minimal pour les ennemis (à vérifier dans la documentation).
- Les attributs sont dans un AttributeSet (Health, MaxHealth). Il n'y a pas de mana.
- Toute modification de santé passe par un GameplayEffect appliqué sur le serveur. Jamais d'écriture directe.
- Les compétences sont écrites en C++ (classes dérivées de UGameplayAbility), pas en Blueprint.
- Les GameplayTags sont déclarés en C++ ou dans le fichier de configuration texte (DefaultGameplayTags.ini).
- La détection des touches se fait sur le serveur, par traces de collision pendant une fenêtre de temps définie dans la compétence. Elle ne dépend pas d'une animation (les animations ne sont pas encore choisies).
- Les ennemis sont pilotés par du code C++ (machine à états simple), pas par des Behavior Trees (binaires).
- Valeurs de mort et d'équilibrage : dans un Data Asset, jamais en dur. Marque PROVISOIRE ce qui n'est pas décidé.
```

---

## T7. Installer GAS et migrer les statistiques [AGENT] — FAIT (cc70fb6)

Les statistiques du jalon 1 (`UStatsComponent`) doivent passer dans GAS, sinon il y aura deux systèmes de santé.

**Prompt :**
> Lis AGENTS.md. Active le plugin `GameplayAbilities` dans le fichier `.uproject` et ajoute les modules `GameplayAbilities`, `GameplayTags` et `GameplayTasks` au fichier `Build.cs`. Place l'`AbilitySystemComponent` sur le `PlayerState`. Crée un `AttributeSet` avec `Health` et `MaxHealth` (100 chacun, valeur de départ via un GameplayEffect d'initialisation). Initialise le composant côté serveur à la possession du personnage et côté client à la réplication du `PlayerState`. Remplace `UStatsComponent` par ces attributs, en gardant `Level` dans le `PlayerState`. Pas de mana. Conserve la commande de debug `DebugDamage <n>` en la faisant passer par un GameplayEffect appliqué sur le serveur.

**Test :** à 2 clients, `DebugDamage 30` sur le serveur ramène la santé de 100 à 70, et chaque client voit la valeur (log). Les joueurs se connectent toujours et retrouvent leur sauvegarde.

## T8. Mort et réapparition (provisoire) [AGENT] — FAIT (tag State.Dead, DA_DeathRules PROVISOIRE 5s, entrees bloquees)

**Prompt :**
> Lis AGENTS.md. À 0 PV, le serveur marque le joueur comme mort (GameplayTag), bloque ses entrées, attend un délai puis le réapparaît au point de départ avec la santé pleine. Délai et lieu viennent d'un Data Asset `DA_DeathRules`. Valeurs provisoires : 5 secondes, point de départ du niveau, aucune perte, marquées `PROVISOIRE`. Prévois dans ce Data Asset un champ `RespawnLocationType` (point de départ ou lieu enregistré), sans implémenter le lieu enregistré pour l'instant.

**Test :** un joueur à 0 PV meurt, réapparaît après le délai, et les autres clients voient la mort et la réapparition.

## T9. Attaque à l'épée en trois coups [AGENT + TOI]

[TOI] avant : crée une action d'entrée `IA_Attack` (clic gauche) et ajoute-la au contexte de mapping.

**Prompt :**
> Lis AGENTS.md. Crée une compétence `GA_MeleeAttack`, déclenchée par `IA_Attack`, exécutée en prédiction locale (le client la lance tout de suite, le serveur valide). Elle enchaîne trois coups si le joueur reclique dans une fenêtre de temps. Pour chaque coup, le serveur fait des traces de collision (forme généreuse devant le joueur) pendant une fenêtre de temps définie dans un Data Asset `DA_MeleeAttackStats` (durée, portée, dégâts). Les dégâts passent par un GameplayEffect appliqué sur le serveur. Un même coup ne touche une cible qu'une seule fois. Pas d'animation pour l'instant : un effet visuel de debug (ligne ou forme) suffit. Le serveur refuse l'attaque si le joueur est mort ou si le délai de recharge n'est pas écoulé.

**Test :** deux joueurs, l'un frappe l'autre (qui perd de la santé côté serveur). Avec 100 ms de latence simulée, les coups restent cohérents.

## T10. Un ennemi simple [AGENT + TOI]

[TOI] avant : place un `NavMeshBoundsVolume` couvrant la zone de test. Réutilise le personnage du modèle (teinté) comme corps provisoire.

**Prompt :**
> Lis AGENTS.md. Crée un ennemi dérivé de `ACharacter` avec son propre `AbilitySystemComponent` (mode Minimal) et les mêmes attributs. Une machine à états en C++ : attente, poursuite du joueur le plus proche, attaque. Avant d'infliger des dégâts, l'ennemi marque l'attaque par un signal visible (changement de couleur ou forme de debug) pendant une durée définie dans un Data Asset. Les dégâts passent par le même GameplayEffect que les joueurs. À 0 PV l'ennemi disparaît. Pas de Behavior Tree.

**Test :** l'ennemi poursuit, prévient, frappe, et peut être tué à l'épée. Deux joueurs le voient identiquement.

## T11. Un boss [AGENT] — FAIT (commit b01ceab, testé en CLI)

Réalisé : `ABossCharacter` (attaque légère + slam AoE télégraphié en jaune), `DA_BossStats`, récompense à la mort via `DT_BossReward` (XP + objet) pour tous les joueurs, `SaveVersion=2` + migration v1→v2 prouvée (`-TestMigrateV1=` : « migrated profile MigTest from v1 to v2 »), persistance XP prouvée après restart serveur (Bob Level=9 XP=100 rechargé). Bonus : fix « mort au login » (Health/MaxHealth défaut 100).

## T11. Un boss [AGENT]

**Prompt :**
> Lis AGENTS.md. Crée un boss à partir de l'ennemi simple, avec beaucoup plus de santé et deux ou trois attaques distinctes (chacune une compétence GAS avec son signal d'avertissement). Toutes les valeurs sont dans un Data Asset `DA_BossStats`. À la mort du boss, le serveur donne une récompense à chaque joueur présent (XP, plus un objet fictif) via une Data Table. La récompense est inscrite dans le profil sauvegardé : augmente `SaveVersion`, écris la fonction de migration de l'ancienne version, et prouve par un test que l'ancien profil se charge toujours.

**Test :** deux joueurs battent le boss, reçoivent la récompense, se déconnectent, se reconnectent : la récompense est toujours là. Un ancien fichier de sauvegarde du jalon 1 se charge sans erreur.

## T12. Test en famille

Fais jouer tes enfants contre le boss. Note, sans corriger sur le moment :
- ce qui est confus (ne voit-on pas venir une attaque ?) ;
- ce qui est frustrant ;
- ce qui amuse.

Les notes décident du jalon 3 plus sûrement que ma liste.

---

## Pour plus tard : les animations

Non décidé, donc le plan n'en dépend pas. Les options à comparer avant d'y toucher :
- animations gratuites ou payantes de la boutique Fab d'Epic : vérifie la **licence d'usage commercial**, puisque tu vends ;
- création ou commande à un animateur ;
- rester sur des animations génériques le temps de valider le plaisir de jeu.

Quand tu choisiras, les animations se brancheront sur les compétences par une étape à la main (les montages sont des fichiers binaires), sans changer la logique GAS.

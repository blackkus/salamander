# T12 — Test en famille (guide)

Objectif : faire jouer les enfants contre l'ennemi et le boss, et noter ce qui
est confus, frustrant ou amusant. **Ne rien corriger pendant le test** : on note,
on corrigera après.

## 1. Préparation (à faire une fois dans l'éditeur, ~10 min)

1. Ouvrir la map `Monde_Test` (Content/Maps/Monde_Test).
2. **NavMesh obligatoire** : Place Actors > Volumes > `Nav Mesh Bounds Volume`,
   l'étirer pour couvrir toute la zone de jeu (sol + environs). Sans ça, les
   ennemis ne peuvent pas se déplacer. Appuyer sur **P** pour voir le navmesh
   vert recouvrir le sol.
3. Vérifier que le sol est vert (`M_SolGreen` appliqué) et que chaque personnage
   apparaît dans la couleur choisie à la connexion.
4. Fermer l'éditeur (les tests se lancent avec les scripts, pas en Play in Editor,
   pour être au plus proche du vrai jeu en réseau).

## 2. Lancer une partie

### Option A — tout sur le même PC (le plus simple)

1. Double-clic sur `Scripts/start_server.bat` → fenêtre serveur noire avec logs.
2. Double-clic sur `Scripts/start_client.bat` → taper un pseudo (max 16 caractères)
   et une couleur hexa (ex `FF8800` orange, `00FF00` vert, `0088FF` bleu).
   Répéter pour chaque joueur (2 fenêtres client = 2 joueurs).

### Option B — un PC par joueur (même réseau local)

- Sur le PC serveur : `start_server.bat`, puis noter l'IP locale du PC
  (`ipconfig` dans un terminal, adresse IPv4, ex `192.168.1.20`).
- Sur chaque PC client : `start_client.bat 192.168.1.20` puis pseudo + couleur.
- Depuis l'extérieur du réseau local : rediriger le port UDP 7777 sur la box,
  ou utiliser un VPN type Tailscale.

### Option C — test rapide dans l'éditeur

Play > `Number of Players = 2`, `Net Mode = Play As Listen Server`.
Utile pour vérifier un truc rapidement, mais ce n'est pas le vrai mode réseau.

## 3. Scénario à faire jouer (~20 min)

| # | Action | Touches |
|---|--------|---------|
| 1 | Se déplacer, tourner la caméra | ZQSD / WASD + souris |
| 2 | Sprinter | Maj gauche (maintenir) |
| 3 | S'accroupir | Ctrl gauche |
| 4 | Trouver l'ennemi et l'attaquer | clic gauche près de lui |
| 5 | Observer l'ennemi avant qu'il frappe (changement de couleur) | — |
| 6 | Mourir (laisser l'ennemi tuer) → réapparaître | — |
| 7 | Trouver le boss (gros, violet) et le tuer à plusieurs | clic gauche, esquiver le cercle jaune au sol |
| 8 | Vérifier la récompense après le boss (XP) | — |

Notes : le combo d'épée = cliquer plusieurs fois rapidement (fenêtre ~0,6 s entre
deux coups). Le boss a deux attaques : une frappe simple et un slam de zone
(cercle jaune au sol → sortir du cercle avant la frappe).

## 4. Grille de notes (à remplir pendant/après)

Pour chaque joueur, noter : confus / frustrant / amusant.

- L'ennemi prévient-il assez avant de frapper ? (trop tôt / juste / trop tard)
- Le cercle jaune du boss est-il lisible ? (on voit où fuir ?)
- Le sprint et l'accroupi répondent-ils bien chez les autres joueurs ?
- Mourir est-il frustrant ? (trop punitif / pas assez / bien)
- Le combo à 3 coups est-il facile à enchaîner ?
- La couleur du joueur s'affiche-t-elle bien chez les autres ?
- Qu'est-ce qui a fait rire / rage / réagir ?

## 5. Après le test

- Noter la config essayée (Option A/B/C, combien de joueurs).
- Copier les notes dans `TASKS_JALON2.md` (section T12) ou un nouveau fichier,
  puis commit. Ces notes décident du jalon 3.
- Les logs serveur/clients sont dans `Saved/Logs/` si besoin de rejouer un moment.

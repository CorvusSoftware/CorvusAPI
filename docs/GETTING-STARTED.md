# Guide de démarrage

CorvusAPI vise un modèle simple : **un en-tête C**, **un module natif par cœur**, **un frontend hôte**. Aucune installation d'un émulateur particulier n'est nécessaire pour compiler le contrat.

## 1. Construire un cœur

Inclure `<corvusapi.h>` et exporter une fonction C nommée `corvus_get_api`. Pour la version expérimentale actuelle, le cœur doit renseigner `corvus_core_api` après avoir vérifié la version demandée et la taille du buffer annoncé par le frontend.

Les entrées obligatoires du contrat actuel sont :

- `get_info` : identité, nom, version, système, extensions de fichiers, nombre maximal de joueurs ;
- `create` / `destroy` : durée de vie d'une instance opaque ;
- `load_game` / `unload_game` : gestion du contenu et de sa mémoire ;
- `get_av_info` : géométrie, fréquence d'images et fréquence audio ;
- `set_button` : entrées par joueur et bouton ;
- `run_frame` : progression d'une image et callbacks synchrones.

Voir [`examples/minimal_core.c`](../examples/minimal_core.c) : ce module retourne une image 2×2, de l'audio stéréo silencieux à 48 kHz et réagit au bouton A. Il démontre le contrat sans inclure de code d'émulation ni de fichier commercial.

```sh
cmake -S . -B build -DCORVUSAPI_BUILD_VERIFIER=ON \
  -DCORVUSAPI_BUILD_EXAMPLES=ON -DCORVUSAPI_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Les bibliothèques natives résultantes ont une extension propre au système (`.so`, `.dylib`, `.dll`). Un seul nom de fichier n'est pas imposé par CorvusAPI : le frontend découvre les modules selon sa politique de distribution.

## 2. Charger depuis un frontend

1. Choisir un module **de confiance**, puis le charger par l'API système (`dlopen` ou `LoadLibrary`).
2. Résoudre `CORVUS_CORE_ENTRYPOINT` et convertir le symbole vers `corvus_get_api_fn`.
3. Initialiser à zéro `corvus_core_api`, remplir `struct_size`, puis négocier `CORVUS_API_VERSION`.
4. Vérifier le résultat, la version et les pointeurs obligatoires avant d'appeler des fonctions.
5. Fournir `corvus_host` avec `video_frame` obligatoire et les callbacks audio/journal optionnels.
6. Créer l'instance, charger le contenu, lire les informations AV, renseigner les commandes, exécuter les frames.
7. Enregistrer la batterie si le groupe facultatif est entièrement disponible ; acquitter la génération **seulement après écriture durable**.
8. Décharger le contenu, détruire l'instance, puis décharger la bibliothèque dynamique en dernier.

Le programme [`tools/corvusapi_verify.c`](../tools/corvusapi_verify.c) démontre la découverte dynamique et vérifie une partie de ce contrat. Il ne certifie pas la fidélité d'émulation et n'exécute pas de ROM.

## 3. Règles de durée de vie

- Tous les appels sur une même instance sont sérialisés par le frontend.
- `video_frame` et `audio_frames` sont invoqués synchroniquement depuis `run_frame` ; les buffers ne survivent pas à l'appel.
- La mémoire ROM et batterie fournie à `load_game` appartient au frontend. Le cœur copie toute donnée qu'il doit conserver.
- La structure `corvus_host` contient un `userdata` opaque restitué aux callbacks.
- Les fonctions et leurs pointeurs ne restent valides que pendant le chargement du module.
- Les extensions sont **optionnelles** et négociées avec leurs propres symboles.

Pour les détails contractuels, consulter [ABI.md](ABI.md) et [COMPATIBILITY.md](COMPATIBILITY.md).

## 4. Intégration dans un autre projet

```cmake
find_package(CorvusAPI 0.1.0 EXACT CONFIG REQUIRED)
target_link_libraries(mon_frontend PRIVATE CorvusAPI::CorvusAPI)
```

Le paquet CMake fournit uniquement les en-têtes et la cible d'interface, sans imposer une implémentation de cœur.

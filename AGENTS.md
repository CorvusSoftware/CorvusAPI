# Consignes de développement pour les agents — CorvusAPI

Ces règles concernent **tout le dépôt CorvusAPI**. Lire `README.md`, le contrat dans `docs/ABI.md` et les documents de la fonctionnalité touchée avant de modifier une interface publique. Les sources présentes dans `main` et `include/corvusapi.h` font foi en cas de divergence avec des notes historiques.

## Mission et architecture

CorvusAPI définit le **contrat ABI C public** entre les frontends et les cœurs d'émulation de l'écosystème CorvusSoftware, y compris les implémentations tierces. Ce dépôt fournit des en-têtes, des exemples, un vérificateur et des tests. Il **ne contient ni émulateur ni frontend**, et ne doit pas dépendre de CorvusCore, CorvusRetro, Android, JNI, RetroArch ou d'un moteur particulier.

### Organisation actuelle

- `include/corvusapi.h` : **en-tête canonique unique**. Il contient l'ABI de base et les déclarations de l'extension mémoire facultative.
- `include/achievements.h` : simple raccourci qui inclut `<corvusapi.h>`. Il ne doit ni dupliquer les types ni définir une seconde API.
- **Aucun répertoire `include/corvus/`** : les anciens chemins `<corvus/api.h>` et `<corvus/achievements.h>` ont été retirés. Ne pas les réintroduire.
- `examples/minimal_core.c` : module C11 synthétique pour démontrer le contrat, sans émulation commerciale.
- `tools/corvusapi_verify.c` : outil de vérification des modules natifs ; il charge et exécute du code du module.
- `tests/` : tests du cycle de vie du module et consommateur du paquet CMake installé.
- `cmake/` et `CMakeLists.txt` : paquet **INTERFACE** `CorvusAPI::CorvusAPI`, installation des deux en-têtes et fichiers de configuration `find_package`.
- `docs/` : spécifications, guide de démarrage, compatibilité, migration des anciens en-têtes et préparation de l'ABI 1.0.
- `.github/workflows/ci.yml` : compilations des en-têtes C11/C++20 et tests Linux, Windows, macOS.

## Règles impératives pour le contrat

1. Garder les en-têtes en **C compatible C11 et C++20**, sans dépendance native requise autre que les en-têtes standards du langage C. Ne pas exposer de types C++, Java, Android ni de structures privées d'un moteur.
2. La **version du paquet CMake est 0.1.0**, distincte de l'**ABI de base `CORVUS_API_VERSION == 0`**, qui est encore expérimentale. La version 1.0 ne doit pas être annoncée avant les validations de `docs/RELEASE-CHECKLIST.md`.
3. Préserver `CORVUS_CORE_ENTRYPOINT` (`corvus_get_api`) et les points d'entrée des extensions. Toute évolution incompatible des tailles de structures, champs, types, conventions d'appel ou sémantiques exige une décision explicite sur le versionnage et les migrations des consommateurs.
4. Respecter `struct_size` lors de la négociation de chaque interface : aucune écriture hors des limites fournies par l'hôte. Initialiser les structures comme le prévoit le contrat et vérifier version, tailles et fonctions obligatoires avant utilisation.
5. Respecter les durées de vie : les données de ROM et de batterie fournies par l'hôte sont empruntées pendant `load_game` ; le cœur copie ce qu'il doit conserver. Les callbacks audio/vidéo sont synchrones et leurs buffers ne survivent pas à l'appel.
6. Les fonctions de sauvegarde d'état et de batterie sont des **groupes optionnels complets**. N'acquitter une génération de batterie qu'après persistance durable confirmée par l'hôte.
7. L'extension de lecture mémoire pour RetroAchievements est **optionnelle**, négociée séparément avec `CORVUS_ACHIEVEMENTS_API_VERSION == 1` et `corvus_get_achievements_api`. Son absence ne doit pas empêcher le chargement d'un cœur. Les lectures doivent rester sans effet de bord, sur une instance valide, et retourner le préfixe contigu réellement disponible.
8. Pour les futures capacités (capteurs, vibration, câble link, triche, BIOS, etc.), privilégier des **extensions publiques négociées** plutôt que des accès internes à CorvusCore ou des règles propres à un frontend.
9. Ne pas ajouter de ROM commerciale, BIOS propriétaire, secret, fichier utilisateur ou code tiers non autorisé. Respecter la licence **MIT** et les mentions de copyright des fichiers existants.

## Compilation et validation

Depuis la racine :

```sh
cmake -S . -B build -DCORVUSAPI_BUILD_VERIFIER=ON \
  -DCORVUSAPI_BUILD_EXAMPLES=ON -DCORVUSAPI_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Pour tout changement d'interface ou de packaging :

- Compiler séparément `#include <corvusapi.h>` et `#include <achievements.h>` en **C11 et C++20**, ainsi que les deux ordres d'inclusion.
- Vérifier que `cmake --install` installe **les deux fichiers directement dans `include/`**, sans créer `include/corvus/`.
- Vérifier qu'un projet extérieur peut utiliser `find_package(CorvusAPI 0.1.0 EXACT CONFIG REQUIRED)` et `CorvusAPI::CorvusAPI`.
- Exécuter le vérificateur ABI sur le module d'exemple **de confiance** et les tests de cycle de vie. Un succès prouve la conformité du scénario testé, pas la fidélité d'un émulateur ni la sécurité du code natif.
- Consulter les jobs GitHub Actions pour Linux, Windows et macOS. Ne jamais annoncer un test comme réussi si son exécution n'a pas été confirmée.

## Méthode de contribution

1. Analyser l'impact sur les frontends, les cœurs indépendants et les sous-modules Git épinglés (notamment CorvusCore, puis CorvusRetro).
2. Limiter la modification à ce dépôt si elle ne nécessite pas un changement des consommateurs. Pour tout changement de chemin d'inclusion ou d'ABI, prévoir la migration coordonnée des consommateurs avant leur mise à jour.
3. Mettre à jour `docs/ABI.md`, `docs/ACHIEVEMENTS.md`, `docs/COMPATIBILITY.md`, `docs/MIGRATION-HEADER.md` ou `docs/GETTING-STARTED.md` selon les éléments concernés.
4. Adapter les exemples, le vérificateur, les tests et la CI avant d'affirmer la compatibilité.
5. Résumer les fichiers modifiés, les éventuelles ruptures **au niveau source ou binaire**, les vérifications exécutées et les limites restantes.
6. Travailler normalement par branche et pull request pour les changements techniques ; un changement direct de `main` n'est autorisé que lorsqu'il est **explicitement demandé** et doit rester ciblé et vérifiable.

Pour les règles complémentaires, consulter `CONTRIBUTING.md`, `SECURITY.md`, `LICENSE` et `docs/RELEASE-CHECKLIST.md`. `CLAUDE.md` importe ce fichier via `@AGENTS.md` et ne doit pas en dupliquer le contenu.

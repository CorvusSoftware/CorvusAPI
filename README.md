# CorvusAPI

**CorvusAPI** est l'interface C publique, légère et indépendante des plateformes de [CorvusSoftware](https://github.com/CorvusSoftware). Elle permet à un **frontend** de charger un **cœur** natif, sans dépendre d'un moteur d'émulation particulier.

CorvusAPI est un protocole d'interopérabilité ; **ce dépôt ne contient ni émulateur ni frontend**. [CorvusCore](https://github.com/CorvusSoftware/CorvusCore) fournit les moteurs officiels GB/GBC/GBA et [CorvusRetro](https://github.com/CorvusSoftware/CorvusRetro) est un frontend Android, mais les implémentations indépendantes sont les bienvenues.

## Statut et compatibilité

**ABI de base 0 — expérimentale.** CorvusAPI n'annonce pas encore la stabilité binaire 1.0. Les évolutions incompatibles sont susceptibles de nécessiter une nouvelle version négociée. L'extension facultative de lecture mémoire des succès est versionnée séparément (`CORVUS_ACHIEVEMENTS_API_VERSION = 1`).

- **En-tête canonique unique :** [`include/corvusapi.h`](include/corvusapi.h), C11 et C++20.
- **En-tête unique :** utilisez uniquement `<corvusapi.h>` pour l'API de base et les extensions.
- **En-têtes à la racine de `include/` :** `<corvusapi.h>` (contrat complet) et `<achievements.h>` (raccourci facultatif pour l'extension mémoire). L'ancien `<corvus/api.h>` a été supprimé : migrez les projets qui l'utilisaient vers `<corvusapi.h>`.
- **Entrée obligatoire des modules :** `corvus_get_api` (interface C, sans symboles C++ exposés).
- **Extension facultative :** `corvus_get_achievements_api` pour lire l'espace mémoire des succès.
- **Licence :** MIT pour cette API ; les cœurs et frontends conservent leurs propres licences.

## Démarrage rapide

Les utilisateurs peuvent inclure **un seul en-tête** :

```c
#include <corvusapi.h>

corvus_core_api api = {0};
api.struct_size = sizeof(api);
/* Un frontend obtient le symbole corvus_get_api depuis le module chargé.
   corvus_get_api_fn entry = ... ; */
int ok = entry(CORVUS_API_VERSION, &api);
```

Le frontend résout le symbole dans la bibliothèque native avant de l'appeler. Le type `corvus_get_api_fn` facilite ce chargement ; aucune bibliothèque d'exécution CorvusAPI n'est à lier.

Pour construire **un vrai exemple de module** en C11 et lancer les tests :

```sh
cmake -S . -B build -DCORVUSAPI_BUILD_VERIFIER=ON \
  -DCORVUSAPI_BUILD_EXAMPLES=ON -DCORVUSAPI_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

L'exemple synthétique [`examples/minimal_core.c`](examples/minimal_core.c) implémente les entrées obligatoires, le chargement de données, la vidéo ARGB8888, l'audio PCM stéréo et les boutons. Il **n'émule aucune console**.

Le binaire `corvusapi-verify` inspecte un module natif fourni en argument :

```sh
./build/corvusapi-verify chemin/vers/module.so
```

**Sécurité :** le vérificateur charge et exécute du code natif. N'utilisez que des modules de confiance ; il ne constitue pas un bac à sable.

## Installation CMake pour les intégrateurs

```sh
cmake -S . -B build
cmake --install build --prefix /chemin/installation
```

Dans un projet CMake externe, avec `CMAKE_PREFIX_PATH` pointant vers cette installation :

```cmake
find_package(CorvusAPI 0.1.0 EXACT CONFIG REQUIRED)
target_link_libraries(mon_coeur PRIVATE CorvusAPI::CorvusAPI)
```

Cette cible **INTERFACE** donne accès aux en-têtes sans bibliothèque à lier. La version du paquet CMake est stricte tant que l'ABI 0 est expérimentale.

## Documentation

- [Démarrer avec un cœur et un frontend](docs/GETTING-STARTED.md)
- [Contrat ABI, appels et durée de vie](docs/ABI.md)
- [Politique de compatibilité et extensions](docs/COMPATIBILITY.md)
- [Extension mémoire de succès](docs/ACHIEVEMENTS.md)
- [Critères de stabilisation 1.0](docs/RELEASE-CHECKLIST.md)
- [Contribuer](CONTRIBUTING.md) · [Signaler une vulnérabilité](SECURITY.md)

## Conception

CorvusAPI sépare le **moteur** (émulation, chargement, états et batterie) du **frontend** (interface utilisateur, fichiers, affichage, audio, contrôleurs et distribution). Elle ne nécessite ni Android, ni JNI, ni réseau, ni code de CorvusCore.

Copyright © 2026 CorvusSoftware — [Licence MIT](LICENSE).

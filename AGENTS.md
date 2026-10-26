# Instructions pour les agents — CorvusAPI

Ce dépôt définit **uniquement** l'interface de communication entre frontends et cœurs CorvusSoftware. Il ne contient pas d'émulateur et ne doit pas dépendre de CorvusCore, CorvusRetro, d'Android ou d'une interface tierce.

## Source de vérité

- `include/corvusapi.h` est **l'en-tête public canonique** de l'ABI C.
- `include/corvus/api.h` a été supprimé pour éviter la duplication des points d'entrée documentés. Ne pas le recréer ; utiliser exclusivement `<corvusapi.h>` pour le contrat de base.
- `include/corvus/achievements.h` reste une enveloppe de compatibilité optionnelle pour l'extension mémoire.
- `docs/ABI.md` décrit la base ; `docs/ACHIEVEMENTS.md` l'extension mémoire ; `docs/COMPATIBILITY.md` la politique de versionnage.
- `examples/minimal_core.c` donne un exemple synthétique indépendant ; `tools/corvusapi_verify.c` est un vérificateur, pas un environnement d'exécution sécurisé.
- Le paquet CMake doit exporter `CorvusAPI::CorvusAPI` aussi bien dans l'arbre source qu'après installation.

## Règles

1. Garder le contrat en **C11**, compatible C++20 et sans dépendances natives obligatoires.
2. L'ABI de base reste **version 0 expérimentale** tant que ses critères de stabilisation ne sont pas validés. Ne pas annoncer la version 1.0 par anticipation.
3. Ne pas changer l'ordre, la taille ou la signification d'un champ public ; ne pas modifier les symboles exportés, durées de vie, contrats de buffers, groupes optionnels ou conventions d'appel sans révision explicite du contrat et de ses tests.
4. Pour toute nouvelle capacité propre à un système, préférer une extension publique facultative négociée par version et symbole, jamais des pointeurs internes propres à un moteur.
5. Conserver les fichiers MIT et leurs avis de copyright. Ne pas importer de code propriétaire ni de ROMs commerciales.
6. Ne pas confondre vérification ABI et sécurité : charger un module natif exécute du code de ce module.
7. Maintenir une documentation précise des rôles frontend/cœur, des sauvegardes durables, de l'audio, de la vidéo et des erreurs.
8. Préférer les changements ciblés et testés aux gros refactorings ou aux copies d'autres projets.

## Vérifications

```sh
cmake -S . -B build -DCORVUSAPI_BUILD_VERIFIER=ON -DCORVUSAPI_BUILD_EXAMPLES=ON -DCORVUSAPI_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Valider les inclusions indépendantes `corvusapi.h` et `corvus/achievements.h` sous C11 et C++20, ainsi que l'installation CMake et un consommateur via `find_package`. Ne jamais affirmer qu'un test ou qu'une plateforme a réussi sans avoir consulté son résultat.

Toute modification fonctionnelle de l'ABI doit mettre à jour les docs, les tests et la checklist de release. Proposer les changements sur une branche/PR, sans fusion directe non sollicitée.

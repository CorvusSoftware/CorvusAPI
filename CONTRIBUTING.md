# Contribuer à CorvusAPI

Merci de contribuer au contrat ouvert de CorvusSoftware. CorvusAPI est une **API de bas niveau** : une modification qui semble mineure peut rendre des modules natifs incompatibles.

## Avant d'ouvrir une PR

- Ouvrir une issue de proposition pour toute modification de l'ABI, des callbacks, des versions ou des extensions.
- Expliquer le cas d'usage pour les cœurs **et** les frontends, ainsi que le comportement si la fonctionnalité est absente.
- Préserver les chemins d'inclusion historiques et le rôle de l'en-tête public `include/corvusapi.h`.
- Ne pas introduire de dépendance à CorvusCore, CorvusRetro, Android, une bibliothèque externe ou une plateforme spécifique.
- Maintenir la licence MIT et les attributions des contributions.

## Validation locale

```sh
cmake -S . -B build -DCORVUSAPI_BUILD_VERIFIER=ON -DCORVUSAPI_BUILD_EXAMPLES=ON -DCORVUSAPI_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Mettre à jour l'exemple C, les tests C11/C++20 et la documentation lorsqu'une signature ou un comportement change. Tester également un consommateur CMake installé si l'installation ou les chemins d'inclusion sont concernés.

## Présentation de la PR

Décrire le **contrat avant / après**, les conséquences sur la compatibilité, les tests lancés et les tests non effectués. Ne pas présenter la vérification du module synthétique comme une preuve de conformité universelle des cœurs, ni annoncer la stabilité de l'ABI 0.

## Sécurité

Les ROMs et les modules natifs peuvent être hostiles. Ne pas joindre de ROMs commerciales, BIOS, secrets ou données privées. Pour signaler une faille, voir [SECURITY.md](SECURITY.md).

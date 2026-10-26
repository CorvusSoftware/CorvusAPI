<!-- CorvusAPI - CorvusSoftware -->
## Objectif et comportement

Décrivez le besoin côté frontend et côté cœur.

## Contrat public et compatibilité

- [ ] Les types, symboles, versions et durées de vie publiques restent compatibles ; sinon, j'ai documenté la rupture et sa négociation.
- [ ] Tous les nouveaux consommateurs incluent `corvusapi.h` ; l'ancien `corvus/api.h` n'est pas réintroduit.
- [ ] Le raccourci optionnel `achievements.h` reste utilisable.
- [ ] Les exemples et la documentation ont été actualisés si nécessaire.
- [ ] Aucune dépendance à un frontend ou à un moteur particulier n'a été ajoutée.

## Validation

- [ ] Compilation C11/C++20
- [ ] `ctest --test-dir build -C Release --output-on-failure`
- [ ] Paquet installé et `find_package(CorvusAPI ...)` vérifiés

**Tests non exécutés / risques connus :**


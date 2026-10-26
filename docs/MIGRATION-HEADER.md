# Migration vers l'en-tête public unique

Depuis cette évolution, le **seul en-tête nécessaire pour le contrat de base et l'extension mémoire** est `<corvusapi.h>`.

## Modification à effectuer

Avant :

```c
#include <corvus/api.h>
```

Après :

```c
#include <corvusapi.h>
```

Les définitions `corvus_core_api`, `corvus_host`, `corvus_get_api_fn`, les valeurs de version et les symboles natifs sont **inchangés**. La modification concerne uniquement le chemin d'inclusion au moment de la compilation.

L'ancien `<corvus/achievements.h>` est lui aussi déplacé : utiliser `<achievements.h>` ou simplement `<corvusapi.h>` (qui contient déjà toutes les déclarations). Ce changement du chemin d'inclusion est une rupture de compatibilité source, pas ABI.

## Dépôts consommateurs

Les versions des sous-modules Git sont épinglées : les applications qui utilisent une ancienne révision de CorvusAPI continuent de fonctionner avec leurs anciens fichiers jusqu'à la mise à jour. Lors de l'actualisation d'un sous-module CorvusAPI, migrer **dans le même commit** les sources du consommateur vers `<corvusapi.h>` et les tests associés.

La CI du dépôt vérifie l'absence du fichier supprimé dans l'installation et compile les chemins qui restent supportés sous C11/C++20.

## Compatibilité

L'ABI native 0 reste expérimentale. Aucun symbole exporté, structure, callback ou protocole binaire n'a été modifié dans ce retrait. Il s'agit néanmoins d'une **rupture de compatibilité au niveau du code source** pour les projets qui incluaient l'ancien fichier.

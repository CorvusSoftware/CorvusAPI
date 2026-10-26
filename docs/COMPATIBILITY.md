# Compatibilité et évolution du contrat

## État actuel

- **Paquet de distribution :** version 0.1.0 ; une version de paquet n'est pas le numéro de l'ABI.
- **ABI de base :** `CORVUS_API_VERSION == 0`, **expérimentale**.
- **Extension de lecture mémoire :** version 1, optionnelle et négociée par `corvus_get_achievements_api`.
- **En-tête canonique :** `<corvusapi.h>`, sans dépendance externe au-delà de la bibliothèque standard C.
- L'ancien chemin `<corvus/api.h>` n'est plus distribué. Tous les consommateurs de l'API de base doivent utiliser `<corvusapi.h>`.
- `<achievements.h>` est désormais installé directement à côté de `<corvusapi.h>` ; il inclut ce dernier et n'ajoute aucun second contrat. L'ancien `<corvus/achievements.h>` est supprimé. Pour tout utiliser, `<corvusapi.h>` suffit.

CorvusAPI ne garantit pas encore une compatibilité binaire entre toutes les révisions du paquet 0.x. Le paquet CMake annonce délibérément une compatibilité de version **exacte** pour éviter qu'un consommateur reçoive implicitement une interface non validée.

## Principes pour modifier l'ABI

1. **Documenter d'abord** l'objectif, les nouveaux cas d'usage et les conséquences pour les frontends et les cœurs.
2. Ne jamais déplacer ou réordonner des champs publics, modifier la largeur de types, la convention d'appel, la propriété des buffers ou la sémantique d'un pointeur de fonction sans traiter cette modification comme une rupture.
3. Mettre à jour les numéros de version et la négociation lorsqu'une rupture est nécessaire ; elle ne doit pas être dissimulée derrière un simple changement de documentation.
4. Préférer une **extension optionnelle négociée** pour les capacités spécialisées. Un frontend ne doit pas supposer qu'une extension existe.
5. Ne jamais faire traverser cette frontière à un handle privé de moteur, un type C++, un objet Android, un allocateur privé ou une dépendance à un frontend spécifique.
6. Maintenir la documentation, les exemples, les tests C11/C++20 et les consommateurs historiques avec toute évolution.

Les capacités telles que link cable, triche, capteurs ou vibration ne sont **pas** promises par l'ABI de base actuelle. Elles nécessiteront des extensions définies dans le contrat public avant d'être annoncées comme universelles.

## Rôles et licences

Les **cœurs** fournissent le comportement d'émulation et implémentent les fonctions C demandées. Les **frontends** gèrent l'interface, les contrôleurs, les données utilisateur et l'installation des modules. CorvusAPI ne fait ni l'un ni l'autre : elle décrit l'échange.

La spécification et les en-têtes sont sous **MIT**. La licence des logiciels intégrant cette API dépend de leurs auteurs et de leurs autres dépendances.

## Avant de déclarer la version 1.0 stable

Terminer les critères documentés dans [RELEASE-CHECKLIST.md](RELEASE-CHECKLIST.md), notamment les tests sur plusieurs cœurs indépendants, les erreurs, les sauvegardes et les plateformes cibles. L'exemple synthétique et le vérificateur ABI couvrent la forme du contrat, **pas** la compatibilité de tous les émulateurs.

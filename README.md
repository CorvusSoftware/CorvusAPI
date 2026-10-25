# CorvusAPI

**CorvusAPI** est une interface binaire (ABI) C ouverte, developpee par
[CorvusSoftware](https://github.com/CorvusSoftware), pour connecter les
frontends d'emulation aux coeurs independants.

CorvusAPI ne contient aucun moteur d'emulation. Le frontend Android officiel
est [CorvusRetro](https://github.com/CorvusSoftware/CorvusRetro).
[CorvusCore](https://github.com/CorvusSoftware/CorvusCore) fournit les
moteurs officiels, sans etre une dependance obligatoire de cette API.

## Etat

**ABI version 0 : experimentale.** La stabilite binaire n'est pas encore
promise. Les changements incompatibles seront signales et versionnes avant
une version 1.0.

## Contrat public

- Header : [`include/corvus/api.h`](include/corvus/api.h)
- Specification du contrat : [`docs/ABI.md`](docs/ABI.md)
- Langages : ABI C, utilisable depuis le C et le C++
- Plateformes : pas de dependance Android, Java ou CorvusCore
- Distribution des coeurs : decision du frontend et de sa plateforme

La premiere revision definit la decouverte d'un coeur, la creation d'une
instance, le chargement d'un contenu, l'execution d'une frame, les boutons,
la video, l'audio et des fonctions optionnelles de sauvegarde.

Ce depot ne contient ni coeur d'exemple ni implementation de moteur.

## Utilisation du SDK

Un frontend ou un coeur peut inclure `corvus/api.h` directement et utiliser
la cible CMake d'interface `CorvusAPI::CorvusAPI`. Aucune bibliotheque
native CorvusAPI n'a besoin d'etre liee : les fonctions sont implementees
par le module du coeur.

## Licence et copyright

**MIT**, Copyright (C) 2026 CorvusSoftware.

Consulter [LICENSE](LICENSE). Les fichiers source originaux portent une
mention `CorvusAPI - CorvusSoftware` et un identifiant
`SPDX-License-Identifier: MIT`.

La licence d'un coeur tiers reste choisie par ses auteurs ; chaque
distribution doit respecter egalement les licences de ses dependances
et celles du frontend utilise.

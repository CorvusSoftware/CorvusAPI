# CorvusAPI 1.0 - stabilisation de l'ABI

Copyright (C) 2026 CorvusSoftware. SPDX-License-Identifier: MIT.

Le format public de CorvusAPI est inspire du principe d'interoperabilite
entre frontend et moteurs de libretro, mais son contrat et son implementation
sont independants de libretro et de CorvusCore.

## Prerequis d'une version 1.0

- [ ] Figement documente des structures publiques et des regles de versionnage,
      y compris la negotiation par `struct_size`.
- [ ] Clarification des comportements obligatoires et des codes d'erreur
      de chaque fonction publique.
- [ ] Tests de conformite du handshake, des metadonnees et des fonctions
      optionnelles via `corvusapi-verify`.
- [ ] Tests reels ROM, rendu ARGB, PCM stereo, controleurs et cycle
      charger/executer/decharger, verifies dans les repositories des moteurs.
- [ ] Tests de sauvegarde, restauration et reprise batterie apres une
      interruption et une erreur d'ecriture.
- [ ] Contrat d'identification durable des cœurs communautaires et
      des contenus dans les frontends.
- [ ] Tests des architectures cibles Android ARM64/x86_64 et des
      frontends hote, avec ASan/UBSan.
- [ ] Strategie d'extensions optionnelles non bloquantes pour fonctions
      telles que capteurs, vibration, triche, BIOS et link cable.
- [ ] Publication de la politique de compatibilite et du premier tag
      1.0.0 seulement apres validation des cœurs communautaires.

## Verificateur ABI

Le binaire `corvusapi-verify`, active avec
`CORVUSAPI_BUILD_VERIFIER=ON`, prend le chemin d'une bibliotheque
native deja construite. Il valide le contrat public sans necessiter
une ROM et sans inclure de coeur de demonstration.

Il ne prouve pas la conformite de l'emulation ni la securite du
code natif. N'executer que des bibliotheques approuvees, car elles
sont chargees dans le processus du verificateur.

## Tests automatises deja presents

- Compilations des trois chemins d'en-tete independamment et dans differents ordres, en C11 et C++20.
- Module d'exemple C11 autonome, lance par le verificateur ABI sur les plateformes CI.
- Test du cycle de vie synthétique : chargement, rendu ARGB8888, audio PCM stereo, commandes et dechargement.
- Installation de l'en-tete et import du paquet CMake par un consommateur externe, avec version exacte.

Ces validations sont des tests de contrat, **pas** une promesse de compatibilite binaire 1.0 ni une validation sur ROMs reelles.


## Licences

Le contrat public reste MIT avec le copyright CorvusSoftware.
Les moteurs CorvusCore et le frontend CorvusRetro conservent
leurs licences distinctes et leur propre attribution.

**Etat actuel : ABI version 0, non stabilisee.**

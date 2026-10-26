# Contrat de CorvusAPI

Copyright (C) 2026 CorvusSoftware. SPDX-License-Identifier: MIT.

## Objet

CorvusAPI definit un contrat **entre un frontend (hote) et un coeur**.
Elle n'exige ni Android, ni CorvusCore, ni un langage d'implementation
particulier derriere l'ABI C.

La revision actuelle est experimentale : `CORVUS_API_VERSION == 0`.
Aucune compatibilite binaire entre revisions n'est encore garantie.

## En-tête autonome

`#include <corvusapi.h>` fournit toutes les déclarations publiques dans un seul fichier C compatible C11 et C++20. Le chemin historique `#include <corvus/api.h>` est retiré : remplacez-le par `#include <corvusapi.h>`. Le raccourci `#include <corvus/achievements.h>` reste disponible pour l'extension mémoire. Aucun type, symbole ou numéro de version de l'ABI n'est changé : l'extension mémoire demeure facultative et négociée séparément.

## Decouverte et compatibilite

Chaque module expose le symbole C `corvus_get_api`, qui correspond au type
`corvus_get_api_fn`. Le frontend obtient ce symbole avec le mecanisme de
chargement pris en charge par sa plateforme ; la presence du symbole n'implique
pas que les modules puissent etre telecharges depuis l'application Android.

Le frontend initialise `corvus_core_api` a zero et renseigne
`struct_size = sizeof(corvus_core_api)` avant l'appel. Il demande
`CORVUS_API_VERSION`. Le coeur refuse une version incompatible avec 0,
retourne 1 uniquement si le contrat est pris en charge, et ne depasse jamais
la taille de structure annoncee par le frontend. Le coeur remplit
`api_version` et les fonctions supportees.

Les autres structures de sortie sont initialisees de la meme maniere :
zero pour les champs, `struct_size` pour la taille disponible. Les valeurs
de `struct_size` sont comptees en octets. Un champ de pointeur optionnel
absent vaut `NULL`.

## Vie d'une instance

Le frontend lit les informations de coeur via `get_info`, puis appelle
`create` avec un `corvus_host` valide. Le coeur doit copier la structure
hote ou conserver un pointeur dont la duree de vie est garantie jusqu'a
`destroy`. Le frontend fournit les callbacks obligatoires
`video_frame` et facultatifs `audio_frames` et `log`.

Une instance gere au plus un contenu a la fois. `load_game` fournit les
octets ROM et, si disponibles, les donnees batterie. Le coeur copie ces
donnees s'il les conserve. `get_av_info` est valable apres un chargement
reussi ; `run_frame` avance une frame emulee et transmet synchronement
la video et, eventuellement, l'audio. `unload_game` ferme le contenu et
`destroy` libere l'instance.

Les fonctions obligatoires sont listees dans le header. `reset` et
`last_error` sont optionnelles. `create` retourne `NULL` en cas d'echec.
Les fonctions a retour `int` retournent 1 en cas de succes et 0 en cas
d'echec, sauf stipulation contraire documentee par une revision ulterieure.
`unload_game`, `set_button` et `destroy` ne retournent rien.

## Threading et callbacks

Les appels a une meme instance sont **serialises par le frontend**.
Le coeur n'appelle les callbacks video/audio que depuis `run_frame`,
sur le thread de cet appel. Il ne conserve pas les buffers fournis aux
callbacks apres leur retour.

`video_frame` recoit une image d'entiers `uint32_t` codes en
`0xAARRGGBB`, dans l'ordre natif des entiers sur la plateforme.
`stride_pixels` est le nombre d'elements `uint32_t` entre deux lignes,
au minimum `width`. `audio_frames` recoit des paires
gauche/droite `int16_t` entrelacees ; son argument compte des **frames
stereo**, pas des echantillons individuels. La frequence audio et la
cadence video sont annoncees par `get_av_info`.

Les boutons sont identifies par `enum corvus_button`; `player` est
indexe a partir de zero. Le frontend fournit l'etat des boutons au coeur,
sans mecanisme de mapping de plateforme impose par CorvusAPI.

## Etats et batterie

Les trois pointeurs `serialize_size`, `serialize` et `unserialize`
sont soit tous presents, soit tous `NULL`. Un coeur peut annoncer une
taille d'etat variable, que le frontend doit interroger avant chaque
serialisation. Le frontend est proprietaire des buffers.

Les quatre pointeurs `battery_size`, `battery_dirty`,
`battery_copy` et `battery_acknowledge` forment aussi un groupe
optionnel tout-ou-rien. `battery_copy` fournit un instantane et sa
generation ; le frontend ne confirme cette generation **qu'apres
l'enregistrement durable**. Les modifications posterieures a la copie
restent alors marquees comme non enregistrees.

## Evolution

Les champs `struct_size` permettent d'etendre les structures.
Toute modification incompatible necessite une nouvelle version
de l'ABI et une documentation explicite. Les identifiants de boutons,
les formats de pixels et l'interpretation des tailles font partie
du contrat public ; ils ne doivent pas etre modifies silencieusement.

## Licence

L'interface et les documents de ce depot sont sous MIT. La MIT
n'impose pas aux auteurs des coeurs tiers de choisir la MIT. En
revanche, l'integration et la redistribution avec un frontend ou
des bibliotheques sous d'autres licences exigent de respecter
l'ensemble des obligations applicables.

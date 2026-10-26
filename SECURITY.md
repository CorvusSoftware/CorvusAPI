# Politique de sécurité — CorvusAPI

CorvusAPI est un contrat ABI expérimental, sans processus d'isolation intégré. **Un module natif chargé peut exécuter du code arbitraire avec les privilèges du processus hôte.** Le programme `corvusapi-verify` n'est pas un bac à sable : il charge et appelle réellement la bibliothèque cible.

## Signaler une vulnérabilité

Utiliser si possible l'onglet **Security → Report a vulnerability** du dépôt GitHub pour un signalement privé. Si cette option n'est pas disponible, ouvrir une issue ne contenant **aucun détail exploitable, secret, fichier utilisateur ni ROM commerciale** pour demander un canal de signalement sécurisé.

Indiquer le commit concerné, le système et l'architecture, les étapes minimales de reproduction, ainsi que l'effet observé. Ne pas divulguer publiquement une exploitation active avant qu'une correction soit étudiée.

## Périmètre

Les défauts de taille de structure, d'accès mémoire, de chargement de module, de durée de vie des callbacks et de sérialisation sont prioritaires. Les émulateurs et frontends consommateurs conservent leurs propres politiques de sécurité.

L'ABI 0 est expérimentale. Aucune promesse de support à long terme ou de délai de correction n'est annoncée.

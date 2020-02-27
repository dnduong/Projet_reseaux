# Projet_reseaux
Projet de Tchat réseau en pair-à-pair utilisant UDP
Exécution : Il suffit de taper la commande make et puis exécuter le programme main

Comment cela marche ? (Il n’y a pas d’interface graphique, on communique via le terminal)

- Tout d’abord, il faut remplir le nickname pour commencer le chat et puis
communiquer comme un chat normal (Attention, vous ne pouvez pas communiquer
pendant les 30 premières secondes parce que mon programme envoie les hello long
chaque 30 secondes, du coup vous ne pouvez pas recevoir les messages dans les
30 premières secondes mais vous pouvez toujours envoyer des messages, les
messages seront transmis à partir de la 31ème secondes grâce à l’inondation). Pour
quitter le programme, il suffit de taper “quit” dans le terminal.
Programmation :

- main.c : contient les fonctions principales du projets, notamment le parseur des
messages reçus, deux threads principaux : un pour l’envoie des datas en tapant sur
le clavier, l’autre le reste du programme (envoi des hellos, envoi des neighbours, la
gestion des voisins, etc..).
- data.c : contient une structure data, permet de réaliser l'inondation, ce fichier contient
principalement toutes les fonctions nécessaire pour l’inondation fiable. Cette
structure data contient spécialement une liste de Voisin à inonder, de la forme ip,
port, le nombre de fois que le message a été inondé à ce voisin (le champ ct), un
tableau de int de taille 6 contient les temps à inonder générés aléatoirement lors de
la création.
- voisin.c : contient deux structure Voisin et Potentiel, la structure Voisin est utilisé pour
représenter un voisin, la structure Potentiel est utilisé pour représenter soit un voisin
potentiel, soit un voisin symétrique, ce fichier contient par conséquence des fonctions
pour manipuler ces deux structures.
- message.c : contient des fonctions utilisées pour créer des messages à envoyer,
notamment les messages de type : hello court, hello long, neighbour, data, go away
et ack.
- ll.c : contient une structure node qui est en réalité une représentation des linked list
générique, il contient ainsi des fonctions pour manipuler les linked list.

Limite :
- Il pourra arriver que le programme s’arrête ou entre dans un boucle infini.

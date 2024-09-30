# Load Balancer

## Preparation

Avant de pouvoir exécuter le programme, il faut faire quelques configurations pour mettre en place le réseau de machines.

Tout d'abord, il faut générer une paire de clef asymétrique sur chaque machine qui constituera votre réseau. Pour cela, connectez vous à la machine cible, via ssh ou directement sur la machine, et taper la commande: ***ssh-keygen -n [nom_clef] -t rsa***\
Il vous demandera si vous voulez ajouter une passphrase à la clef, cela n'est pas nécessaire donc appuyer sur entrée pour passer.
Cela vous générera deux fichiers dans le répertoire ***$HOME/.ssh***, on vous conseille d'y créer un répertoire et d'y placer l'ensemble des clés qui seront générés.

Ecrivez l'ensemble des clés publics, fichier avec ***.pub***, des différentes machines dans le fichier ***authorized_keys***, s'il n'existe pas créer le, avec la commande suivante:\
***cat [nom_clef].pub > $HOME/.ssh/authorized_keys***. 

Maintenant, il faut que vous vous connectiez au moins une fois, si cela n'est pas déjà fait, à chaque machine via ssh pour enregistrer la clef dans le fichier ***known_hosts***.

De plus, pour permettre un déploiement d'OpenMpi plus optimisé, transmis de noeud en noeud, écrivez les lignes suivantes dans le fichier ***$HOME/.ssh/config***:

```bash
Host*
    ForwardAgent yes
    StrictHostKeyChecking no
```
\
Créer un fichier appelé ***hostfile*** et mettez y le nom de domaine de chaque machine participant au réseau. Vous pouvez regarder le fichier ***/exec/hostfile*** pour l'exemple.

Il reste encore une dernière étape avant de pouvoir lancer le programme. Les étapes citées ci-dessus ne sont a réalisé qu'une seule fois alors que l'étape qui va suivre devra être réalisé à chaque fois qu'on redémarre la machine sur laquelle on exécute le programme.

Pour éviter de taper le mot de passe à chaque connexion ssh des machines du réseau, il faut charger les clés privées de celle-ci, le fichier ne contenant pas l'extension ***.pub***, en mémoire via un agent.

Pour cela, commencez par lancer l'agent sur le terminal avec la commande suivante:
***eval 'ssh-agent' ou eval "$(ssh-agent)"*** selon le terminal.

Puis, il faut ajouter toutes les clés dans l'agent via cette commande:
***ssh-add $HOME/.ssh/[nom_clef]***, qu'il faut répéter jusqu'à avoir charger toutes les clés dont vous avez besoin pour l'exécution.

Faire un test en vous connectant à une seule machine via ssh, pour voir si cela vous demande votre mot de passe. Si ce n'est pas le cas, alors vous êtes prêt à exécuter le programme.


## Execution


### smt_loadbalancer

***smt_loadbalancer*** est la version automatisée de notre programme. Il enverra 10 tâches a exécuté à l'ensemble des machines du réseau puis testera différents scénarios comme tuer un processus, insérer une machine dans le réseau, ou supprimer une machine du réseau. A la fin de chaque traitement, il affichera les informations globales du réseau. Le programme se terminera tout seul.

Normalement, vous devriez avoir sur votre console un affichage comme dans le fichier ***/exec/trace.example.txt***


### loadbalancer

***loadbalancer*** est la version manuelle de notre programme où vous pouvez effectuer vos propre test. Il répond à 6 commandes:

***start ../progs/loop***, envoi une nouvelle tâche a executé sur une machine. Le code de la fonction start se trouve dans ***/implem/work.c***

***gps [-l]***,  affiche les informations des machines et des processus sur le réseau. Avec l'option ***-l***, cela affiche également pour chaque processus son utilisation CPU et mémoire.

***gkill -[numero_signal] [gpid]***, envoi du signal approprié, selon le numéro, au processus ayant le gpid indiqué. La commande gps affichera les différents gpid des processus sur le réseau

***insert [rank]***, insère une machine dans le réseau. La commande gps affichera les différents rank des machines sur le réseau.

***remove [rank]***, retire une machine du réseau

***quit***, termine proprement le programme


### Lancement

Pour lancer les exécutables, il faut lancer le script ***/exec/run.sh*** en tapant la commande suivante:

***./run.sh [nom_executable]***, soit smt_loadbalancer, soit loadbalancer

Ce script ouvrira un fichier ***/exec/log.txt*** contenant les informations relatives à l'exécution, notamment lorsqu'une machine est en surcharge ou en souscharge, compilera le code avec un make, puis l'exécutera et enfin supprimera tout les fichiers .o et les exécutables, crées lors de la compilation, à la fin de l'exécution du programme.


### Problèmes

Si l'exécution ne lance pas, faites des tests avec des connexions ssh vers les différentes machines contenue dans le fichier hostfile pour voir si elles répondent toujours, sinon enlevez les du hostfile et rajoutez en d'autres en veillant à toujours ajouter la clef privée correspondante dans l'agent. Sinon, quelques fois il peut y avoir des problèmes avec l'agent, arrêtez le, relancer le et chargez de nouveau les clés en mémoire.\
Il peut également y avoir des machines qui poseront problème, lors de l'exécution, ralentissant le programme, provoquant des erreurs, pendant l'utilisation de certaines commandes, ou encore engendrant une incohérence du réseau. Identifiez ces machines et remplacez ou supprimez les du fichiers hostfile.   

Parfois, il peut arriver qu'une erreur réseau se produise arrêtant le programme. Cela n'est rien de grave, il suffit juste de le relancer.


### Avant de quitter

Quand vous avez fini de faire tout vos test, n'oubliez pas de décharger les clés en mémoire en tuant l'agent avec la commande:
***ssh-agent -k***
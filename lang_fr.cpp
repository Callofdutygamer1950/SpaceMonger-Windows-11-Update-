
#include "stdafx.h"
#include "SpaceMonger.h"
#include "lang.h"

static const wchar_t *densitynames[] = {
	L"Petit Peu de Fichiers", L"Peu de Fichiers", L"Normal", L"Beaucoup de Fichiers",
	L"Très Beaucoup de Fichiers", L"Trop de Fichiers",
};

static const wchar_t *uscolornames[] = {
	L"Arc-en-Ceil", L"Colours de Windows", L"Blanc", L"Gris Clair", L"Gris Foncé",
	L"Rouge", L"Orange", L"Jaune", L"Vert", L"Aqua", L"Bleu", L"Violette",
	L"Par Type de Fichier",
};

static const wchar_t *monthnames[] = {
	L"Jan", L"Fev", L"Mars", L"Avril", L"Mai", L"Juin",
	L"Juil", L"Août", L"Sep", L"Oct", L"Nov", L"Dec",
};

static const wchar_t *attribnames[] = {
	L"Arch", L"Compress", L"Dossier", L"Encrypt", L"Caché", L"Pas-au-ligne",
	L"Lire-Seulement", L"Reparse-Pt", L"Clairsemé", L"Système", L"Temp",
};

static const wchar_t *frtoolbarnames[] = {
	L"Ouvrir", L"Rouvrir", L"Voir Tout", L"Vrombir", L"Démarrer",
	L"Espace Libre", L"Exécuter", L"Corbeille", L"Config", L"De SM", L"Plus Gros",
};

static const wchar_t *frthemenames[] = {
	L"Paramètre de Windows", L"Clair", L"Sombre",
};

static const wchar_t *frtypenames[] = {
	L"Vidéos", L"Images", L"Musique", L"Documents",
	L"Archives et images disque", L"Programmes et jeux", L"Système", L"Autres",
};

static const LangExtra ExtraFr = {
	L"Plus Gros Fichiers",
	L"Les 50 plus gros fichiers de %s. Les fichiers uniquement dans le cloud ne sont pas listés, car ils n'utilisent pas d'espace sur ce disque.",
	L"Ces fichiers sont vos exceptions. Ils ne figurent pas dans la liste des plus gros fichiers.",
	L"Nom", L"Taille", L"Dossier", L"Modifié",
	L"&Aller à", L"&Ajouter aux exceptions", L"&Retirer des exceptions", L"Afficher les e&xceptions", L"Fermer",
	L"Aucun fichier à afficher.",
	L"<Espace invisible à l'analyse: %d,%d%%>",
	L"Zones système protégées et fichiers illisibles",
	L"Relancer en administrateur pour en voir plus",
	L"Relancer en &Administrateur",
	L"E&xporter vers un tableur...",
	L"Exporter vers un tableur",
	L"Ceci enregistrera %s lignes. Excel ne peut ouvrir que les 1.048.576 premières.\n\nEnregistrer quand même?",
	L"SpaceMonger n'a pas pu enregistrer le fichier.",
	L"%s lignes enregistrées dans:\n%s",
	L"Rechercher (nom ou .ext)",
	L"%s résultats, %s",
	L"%s résultat, %s",
	L"Aucun résultat",
	frtypenames,
	L"SpaceMonger n'a pas pu ouvrir ce dossier. Il n'existe peut-être pas, ou vous n'avez pas la permission de le lire.\n\n%s",
};

Lang LangFr = {
	IDR_TOOLBAR_FR,
	L"De SpaceMonger",
	L"CE PROGRAMME EST UN LOGICIEL LIBRE ET GRATUIT.",
	L"Vous pouvez le partager et le modifier selon la Licence Publique"
		L" Générale GNU, version 3 (voir LICENSE.txt).  Les versions que vous"
		L" partagez doivent rester libres sous la même licence.  Ce programme"
		L" est fourni SANS AUCUNE GARANTIE, expresse ou implicite.",
	L"Vous avez des questions?  Trouvez un «bug»?  Voudriez quelque-chose?  "
		L"Envoyez e-mail à «JoeysItSupport@gmail.com».",
	L"D'accord", L"Annul",
	L"octets", L"Ko", L"Mo", L"Go",
	L"<l'Espace Libre: %d,%d%%>",
	L"&Vrombir", L"&Démarrer", L"Voir &Tout", L"&Exécuter", L"Mettre à la &Corbeille",
	L"&Ouvrir un Disque...", L"&Rouvrir", L"&Montrer l'Espace Libre",
	L"%lu,%01lu%%", L"%lu,%01lu %s",
	L"Total", L"Libre", L"Fichiers Totals:", L"Dossiers Totals:", L"Fichiers Trouvés:", L"Dossiers Trouvés:",
	L',', L'.',
	densitynames, uscolornames, monthnames, attribnames,
	L"Effacer...",
	L"Choisir un Disque",
	L"Scruter le Disque...",
	L"Configuration de SpaceMonger",
	L"Orientation des Fichiers", L"Densité:", L"Orient:", L"Horz", L"Égale", L"Vert",
	L"Colours", L"Fichiers", L"Dossiers",
	L"«ToolTips»", L"Montrer Nom «ToolTips»", L"Montrer Info «ToolTips»", L"Délai:", L"msec",
	L"Sentier", L"Nom-fichier", L"Petit Icon", L"Icon", L"Date/Heure", L"Octets", L"Attributs",
	L"Autres Options",
	L"Auto-Rouvrir à Effacer", L"Pas de Command «Effacer»",
	L"Vrombir Animé", L"Souvenir Position du Window",
	L"Montrer les Boites «Rollovers»",
	L"Propriétés...",
	frtoolbarnames,
	L"To",
	L"Afficher dans l'&Explorateur",
	L"Ouvrir un &Dossier...",
	L"&Dossier...",
	L"Choisir un Dossier",
	L"Déplacer ceci vers la Corbeille?\n\n%s",
	L"Windows n'a pas pu déplacer ceci vers la Corbeille.",
	L"Analyse",
	L"Ignorer les fichiers stockés uniquement dans le cloud (OneDrive, etc.)",
	L"Thème:",
	frthemenames,
	L"Scruter le Dossier...",
	&ExtraFr,
};

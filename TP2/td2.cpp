////////////////////////////////////////////////////////////////////////////////
/// \file   td2.cpp
/// \author Hubert Godin et Youssef Ktari
///
/// Ce fichier contient la solution du TD1. On 
////////////////////////////////////////////////////////////////////////////////






#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "structures.hpp"      // Structures de données pour la collection de films en mémoire.

#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>

#include "cppitertools/range.hpp"
#include "gsl/span"

#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.
#include "debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).

using namespace std;
using namespace iter;
using namespace gsl;

#pragma endregion//}

typedef uint8_t UInt8;
typedef uint16_t UInt16;

#pragma region "Fonctions de base pour lire le fichier binaire"//{

UInt8 lireUint8(istream& fichier)
{
	UInt8 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
UInt16 lireUint16(istream& fichier)
{
	UInt16 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUint16(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}

#pragma endregion//}

//TODO: Une fonction pour ajouter un Film à une ListeFilms, le film existant déjà; on veut uniquement ajouter le pointeur vers le film existant.  Cette fonction doit doubler la taille du tableau alloué, avec au minimum un élément, dans le cas où la capacité est insuffisante pour ajouter l'élément.  Il faut alors allouer un nouveau tableau plus grand, copier ce qu'il y avait dans l'ancien, et éliminer l'ancien trop petit.  Cette fonction ne doit copier aucun Film ni Acteur, elle doit copier uniquement des pointeurs.
const int MIN_CAPACITY = 1;

void addFilmToList(Film** listFilms, int& capacity, int& size, Film* film) {
    if (size == capacity) {
        int newCapacity = max(capacity * 2, MIN_CAPACITY);
        Film** newListFilms = new Film*[newCapacity];
        for (int i = 0; i < size; i++) {
            newListFilms[i] = listFilms[i];
        }
        delete[] listFilms;
        listFilms = newListFilms;
        capacity = newCapacity;
    }
    listFilms[size++] = film;
}
//TODO: Une fonction pour enlever un Film d'une ListeFilms (enlever le pointeur) sans effacer le film; la fonction prenant en paramètre un pointeur vers le film à enlever.  L'ordre des films dans la liste n'a pas à être conservé.
void enleverFilm(Film** listeFilms, int &tailleListe, Film* filmAEnlever) {
    for (int i = 0; i < tailleListe; i++) {
        if (listeFilms[i] == filmAEnlever) {
            listeFilms[i] = listeFilms[tailleListe - 1];
            tailleListe--;
            break;
        }
    }
}
//TODO: Une fonction pour trouver un Acteur par son nom dans une ListeFilms, qui retourne un pointeur vers l'acteur, ou nullptr si l'acteur n'est pas trouvé.  Devrait utiliser span.
Acteur* trouverActeur(span<Film> ListeFilms, const std::string& nomActeur) {
  for (const auto&film : ListeFilms) {
    for (const auto& acteur : film->acteurs.elements) {
      if (acteur.nom == nomActeur) {
		  return &acteur;
      }
    }
  }
  return nullptr;
}

//TODO: Compléter les fonctions pour lire le fichier et créer/allouer une ListeFilms.  La ListeFilms devra être passée entre les fonctions, pour vérifier l'existence d'un Acteur avant de l'allouer à nouveau (cherché par nom en utilisant la fonction ci-dessus).
Acteur* lireActeur(istream& fichier)
{
	Acteur acteur = {};
	acteur.nom = lireString(fichier);
	acteur.anneeNaissance = lireUint16(fichier);
	acteur.sexe = lireUint8(fichier);



}
	Film* lireFilm(istream & fichier)
{
	Film film = {};
	film.titre       = lireString(fichier);
	film.realisateur = lireString(fichier);
	film.anneeSortie = lireUint16 (fichier);
	film.recette     = lireUint16 (fichier);
	film.acteurs.nElements = lireUint8 (fichier);
	film.acteurs.elements = new Acteur[film.acteurs.nElements];
    
	for (int i = 0; i < film.acteurs.nElements; i++) {
		Acteur& acteur = lireActeur(fichier);
        film.acteurs.elements[i] = acteur;
        acteur.films.push_back(film);
    }
    
    return film;
}
	

ListeFilms creerListe(string nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);
	
	int nElements = lireUint16(fichier);
	
	ListeFilms liste;
	for (int i = 0; i < nElements; i++) {
		Film film = lireFilm(fichier);
		liste.push_back(film);
	}
	
	return liste;
}

//TODO: Une fonction pour détruire un film (relâcher toute la mémoire associée à ce film, et les acteurs qui ne jouent plus dans aucun films de la collection).  Noter qu'il faut enleve le film détruit des films dans lesquels jouent les acteurs.  Pour fins de débogage, affichez les noms des acteurs lors de leur destruction.
void detruireFilm(Film* film)
{
	
	
	for (int i = 0; i < film->acteurs.nElements; i++)
	{
		enleverFilm(film->acteurs.elements[i]->joueDans, film->acteurs.nElements, film);
		if (film->acteurs.elements[i]->joueDans.nElements == 0)
		{
			cout << "On détruit "<< film->acteurs.elements[i].nom << endl;
			//acteur->joueDans.~ListeFilms();
			delete[] film->acteurs.elements[i]->joueDans.elements;
			film->acteurs.elements[i]->joueDans.elements=nullptr;
			delete film->acteurs.elements[i];
			film->acteurs.elements[i] = nullptr;
			film->acteurs.nElements--;
		}


			
	}
	cout <<"On détruit " << film->titre << endl;
	
	delete[] film->acteurs.elements;
	film->acteurs.elements = nullptr;
	
	delete film;
	film = nullptr;
}
//TODO: Une fonction pour détruire une ListeFilms et tous les films qu'elle contient.
void detruireListeFilms(Film** listeFilms)
{
	
	for (int i = 0; i < listeFilms.nElements; i++)
	{
		detruireFilm(listeFilms.elements[i]);
	}
	delete[] listeFilms.elements;
	listeFilms.elements = nullptr;
	delete listeFilms;
	listeFilms = nullptr;
}
void afficherActeur(const Acteur& acteur)
{
	cout << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
}

//TODO: Une fonction pour afficher un film avec tous ces acteurs (en utilisant la fonction afficherActeur ci-dessus).

void afficherListeFilms(const ListeFilms& listeFilms)
{
	//TODO: Utiliser des caractères Unicode pour définir la ligne de séparation (différente des autres lignes de séparations dans ce progamme).
	static const string ligneDeSeparation = {};
	cout << ligneDeSeparation;
	//TODO: Changer le for pour utiliser un span.
	for (int i = 0; i < listeFilms.nElements; i++) {
		//TODO: Afficher le film.
		cout << ligneDeSeparation;
	}
}

void afficherFilmographieActeur(const ListeFilms& listeFilms, const string& nomActeur)
{
	//TODO: Utiliser votre fonction pour trouver l'acteur (au lieu de le mettre à nullptr).
	const Acteur* acteur = nullptr;
	if (acteur == nullptr)
		cout << "Aucun acteur de ce nom" << endl;
	else
		afficherListeFilms(acteur->joueDans);
}

int main()
{
	bibliotheque_cours::activerCouleursAnsi();  // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.

	int* fuite = new int; //TODO: Enlever cette ligne après avoir vérifié qu'il y a bien un "Fuite detectee" de "4 octets" affiché à la fin de l'exécution, qui réfère à cette ligne du programme.

	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";

	//TODO: Chaque TODO dans cette fonction devrait se faire en 1 ou 2 lignes, en appelant les fonctions écrites.

	//TODO: La ligne suivante devrait lire le fichier binaire en allouant la mémoire nécessaire.  Devrait afficher les noms de 20 acteurs sans doublons (par l'affichage pour fins de débogage dans votre fonction lireActeur).
	ListeFilms listeFilms = creerListe("films.bin");
	
	cout << ligneDeSeparation << "Le premier film de la liste est:" << endl;
	//TODO: Afficher le premier film de la liste.  Devrait être Alien.
	cout << listeFilms.elements[0] << endl;
		//TODO: Afficher la liste des films.  Il devrait y en avoir 7.
	for (int i = 0; i < listeFilms.nElements; i++)
	{
		cout << listeFilms.elements[i] << endl;
	}
	
	//TODO: Modifier l'année de naissance de Benedict Cumberbatch pour être 1976 (elle était 0 dans les données lues du fichier).  Vous ne pouvez pas supposer l'ordre des films et des acteurs dans les listes, il faut y aller par son nom.
	Acteur Benedict_Cumberbatch = trouverActeur(listeFilms, 'Benedict Cumberbatch');
	Benedict_Cumberbatch.anneeNaissance = 1976;
	//TODO: Afficher la liste des films où Benedict Cumberbatch joue.  Il devrait y avoir Le Hobbit et Le jeu de l'imitation.
	for (int i = 0; i < Benedict_Cumberbatch.joueDans.nElements; i++)
	{
		cout << ligneDeSeparation << "Liste des films où Benedict Cumberbatch joue sont:" << Benedict_Cumberbatch.joueDans[i]<< endl;
	}
	
	
	//TODO: Détruire et enlever le premier film de la liste (Alien).  Ceci devrait "automatiquement" (par ce que font vos fonctions) détruire les acteurs Tom Skerritt et John Hurt, mais pas Sigourney Weaver puisqu'elle joue aussi dans Avatar.
	detruireFilm(listeFilms.elements[0]);
	enleverFilm(listeFilms.elements, listeFilms.nElements, listeFilms.elements[0]);
	cout << ligneDeSeparation << "Les films sont maintenant:" << endl;
	//TODO: Afficher la liste des films. 
	for (int i = 0; i < listeFilms.nElements; i++)
	{
		cout << listeFilms.elements[i] << endl;
	}
	//TODO: Faire les appels qui manquent pour avoir 0% de lignes non exécutées dans le programme (aucune ligne rouge dans la couverture de code; c'est normal que les lignes de "new" et "delete" soient jaunes).  Vous avez aussi le droit d'effacer les lignes du programmes qui ne sont pas exécutée, si finalement vous pensez qu'elle ne sont pas utiles.
	
	//TODO: Détruire tout avant de terminer le programme.  La bibliothèque de verification_allocation devrait afficher "Aucune fuite detectee." a la sortie du programme; il affichera "Fuite detectee:" avec la liste des blocs, s'il manque des delete.
	detruireListeFilms(listeFilms);
}

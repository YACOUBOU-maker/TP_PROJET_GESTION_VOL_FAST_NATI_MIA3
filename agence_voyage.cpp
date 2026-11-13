// Projet : Gestion d'une agence de voyage (Cotonou)
// Application console C++ - Programmation Orientee Objet
// Compilation : g++ -std=c++11 agence_voyage.cpp -o agence

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <cctype>
using namespace std;

// ---------------------------------------------------------------
// Statuts
// ---------------------------------------------------------------
enum class StatutReservation { EN_ATTENTE, VALIDEE, ANNULEE };
enum class StatutPaiement { PAYE, REMBOURSE };

string texte(StatutReservation s) {
    if (s == StatutReservation::EN_ATTENTE) return "EN_ATTENTE";
    if (s == StatutReservation::VALIDEE) return "VALIDEE";
    return "ANNULEE";
}

string texte(StatutPaiement s) {
    return (s == StatutPaiement::PAYE) ? "PAYE" : "REMBOURSE";
}

// ---------------------------------------------------------------
// Classe Passager
// ---------------------------------------------------------------
class Passager {
private:
    int id;
    string nom, prenom, telephone;

public:
    Passager(int id, const string& nom, const string& prenom, const string& telephone)
        : id(id), nom(nom), prenom(prenom), telephone(telephone) {}

    int getId() const { return id; }
    string getNom() const { return nom; }
    string getPrenom() const { return prenom; }

    void afficher() const {
        cout << "Passager #" << id << " : " << prenom << " " << nom
             << " (tel : " << telephone << ")" << endl;
    }
};

// ---------------------------------------------------------------
// Classe Vol
// ---------------------------------------------------------------
class Vol {
private:
    string numero, depart, destination, dateHeure;
    int capacite;
    int placesReservees;

public:
    Vol(const string& numero, const string& depart, const string& destination,
        const string& dateHeure, int capacite)
        : numero(numero), depart(depart), destination(destination),
          dateHeure(dateHeure), capacite(capacite), placesReservees(0) {
        if (capacite <= 0)
            throw invalid_argument("La capacite d'un vol doit etre positive.");
    }

    string getNumero() const { return numero; }
    string getDestination() const { return destination; }
    int placesRestantes() const { return capacite - placesReservees; }
    bool estComplet() const { return placesReservees >= capacite; }

    void reserverPlace() {
        if (estComplet())
            throw runtime_error("Le vol " + numero + " est complet.");
        placesReservees++;
    }

    void libererPlace() {
        if (placesReservees > 0) placesReservees--;
    }

    void afficher() const {
        cout << "Vol " << numero << " : " << depart << " -> " << destination
             << " | " << dateHeure << " | places restantes : "
             << placesRestantes() << "/" << capacite << endl;
    }
};

// ---------------------------------------------------------------
// Classe Reservation (associee a UN passager et UN vol)
// ---------------------------------------------------------------
class Reservation {
private:
    int id;
    Passager* passager;
    Vol* vol;
    StatutReservation statut;

public:
    Reservation(int id, Passager* passager, Vol* vol)
        : id(id), passager(passager), vol(vol), statut(StatutReservation::EN_ATTENTE) {}

    int getId() const { return id; }
    Passager* getPassager() const { return passager; }
    Vol* getVol() const { return vol; }
    StatutReservation getStatut() const { return statut; }
    bool estValidee() const { return statut == StatutReservation::VALIDEE; }

    // La place n'est reservee definitivement qu'apres validation
    void valider() {
        if (statut == StatutReservation::ANNULEE)
            throw runtime_error("Reservation annulee : validation impossible.");
        if (statut == StatutReservation::VALIDEE)
            throw runtime_error("Reservation deja validee.");
        vol->reserverPlace();
        statut = StatutReservation::VALIDEE;
    }

    void annuler() {
        if (statut == StatutReservation::ANNULEE)
            throw runtime_error("Reservation deja annulee.");
        if (statut == StatutReservation::VALIDEE)
            vol->libererPlace();
        statut = StatutReservation::ANNULEE;
    }

    void afficher() const {
        cout << "Reservation #" << id << " | " << passager->getPrenom() << " "
             << passager->getNom() << " | vol " << vol->getNumero()
             << " | statut : " << texte(statut) << endl;
    }
};

// ---------------------------------------------------------------
// Classe Paiement
// ---------------------------------------------------------------
class Paiement {
private:
    int id;
    Reservation* reservation;
    double montant;
    StatutPaiement statut;

public:
    Paiement(int id, Reservation* reservation, double montant)
        : id(id), reservation(reservation), montant(montant), statut(StatutPaiement::PAYE) {
        if (montant <= 0)
            throw invalid_argument("Le montant doit etre positif.");
    }

    int getId() const { return id; }
    Reservation* getReservation() const { return reservation; }
    StatutPaiement getStatut() const { return statut; }

    void rembourser() {
        if (statut == StatutPaiement::REMBOURSE)
            throw runtime_error("Ce paiement est deja rembourse.");
        statut = StatutPaiement::REMBOURSE;
    }

    void afficher() const {
        cout << "Paiement #" << id << " : " << montant << " FCFA | statut : "
             << texte(statut) << endl;
    }
};

// ---------------------------------------------------------------
// Classe Agence : contient et gere tous les objets
// ---------------------------------------------------------------
class Agence {
private:
    vector<Passager*> passagers;
    vector<Vol*> vols;
    vector<Reservation*> reservations;
    vector<Paiement*> paiements;
    int prochainPassager, prochaineReservation, prochainPaiement;

    static string minuscules(string s) {
        for (size_t i = 0; i < s.size(); i++)
            s[i] = (char)tolower((unsigned char)s[i]);
        return s;
    }

public:
    Agence() : prochainPassager(1), prochaineReservation(1), prochainPaiement(1) {}

    ~Agence() {
        for (size_t i = 0; i < passagers.size(); i++) delete passagers[i];
        for (size_t i = 0; i < vols.size(); i++) delete vols[i];
        for (size_t i = 0; i < reservations.size(); i++) delete reservations[i];
        for (size_t i = 0; i < paiements.size(); i++) delete paiements[i];
    }

    Agence(const Agence&) = delete;
    Agence& operator=(const Agence&) = delete;

    // ----- Recherches -----
    Passager* trouverPassager(int id) const {
        for (size_t i = 0; i < passagers.size(); i++)
            if (passagers[i]->getId() == id) return passagers[i];
        return nullptr;
    }

    Vol* trouverVol(const string& numero) const {
        for (size_t i = 0; i < vols.size(); i++)
            if (minuscules(vols[i]->getNumero()) == minuscules(numero)) return vols[i];
        return nullptr;
    }

    Reservation* trouverReservation(int id) const {
        for (size_t i = 0; i < reservations.size(); i++)
            if (reservations[i]->getId() == id) return reservations[i];
        return nullptr;
    }

    Paiement* trouverPaiementDeReservation(int idReservation) const {
        for (size_t i = 0; i < paiements.size(); i++)
            if (paiements[i]->getReservation()->getId() == idReservation) return paiements[i];
        return nullptr;
    }

    // ----- 1. Enregistrer un passager -----
    Passager* enregistrerPassager(const string& nom, const string& prenom, const string& tel) {
        Passager* p = new Passager(prochainPassager++, nom, prenom, tel);
        passagers.push_back(p);
        return p;
    }

    // ----- 2. Ajouter un vol -----
    Vol* ajouterVol(const string& numero, const string& depart, const string& destination,
                    const string& dateHeure, int capacite) {
        if (trouverVol(numero) != nullptr)
            throw runtime_error("Un vol avec ce numero existe deja.");
        Vol* v = new Vol(numero, depart, destination, dateHeure, capacite);
        vols.push_back(v);
        return v;
    }

    // ----- 3. Afficher les vols disponibles -----
    void afficherVolsDisponibles() const {
        bool trouve = false;
        for (size_t i = 0; i < vols.size(); i++) {
            if (!vols[i]->estComplet()) {
                vols[i]->afficher();
                trouve = true;
            }
        }
        if (!trouve) cout << "Aucun vol disponible." << endl;
    }

    // ----- 4. Rechercher un vol (par numero ou destination) -----
    void rechercherVol(const string& mot) const {
        string m = minuscules(mot);
        bool trouve = false;
        for (size_t i = 0; i < vols.size(); i++) {
            string num = minuscules(vols[i]->getNumero());
            string dest = minuscules(vols[i]->getDestination());
            if (num == m || dest.find(m) != string::npos) {
                vols[i]->afficher();
                trouve = true;
            }
        }
        if (!trouve) cout << "Aucun vol trouve." << endl;
    }

    // ----- 5. Effectuer une reservation -----
    Reservation* effectuerReservation(int idPassager, const string& numeroVol) {
        Passager* p = trouverPassager(idPassager);
        if (p == nullptr)
            throw runtime_error("Passager non enregistre : il faut l'enregistrer avant de reserver.");
        Vol* v = trouverVol(numeroVol);
        if (v == nullptr)
            throw runtime_error("Vol introuvable.");
        if (v->estComplet())
            throw runtime_error("Le vol est complet.");
        for (size_t i = 0; i < reservations.size(); i++) {
            Reservation* r = reservations[i];
            if (r->getPassager() == p && r->getVol() == v
                && r->getStatut() != StatutReservation::ANNULEE)
                throw runtime_error("Ce passager est deja enregistre sur ce vol.");
        }
        Reservation* r = new Reservation(prochaineReservation++, p, v);
        reservations.push_back(r);
        return r;
    }

    // ----- 6. Enregistrer un paiement (valide la reservation) -----
    Paiement* enregistrerPaiement(int idReservation, double montant) {
        Reservation* r = trouverReservation(idReservation);
        if (r == nullptr)
            throw runtime_error("Reservation introuvable.");
        if (r->getStatut() == StatutReservation::ANNULEE)
            throw runtime_error("Reservation annulee : paiement impossible.");
        if (trouverPaiementDeReservation(idReservation) != nullptr)
            throw runtime_error("Cette reservation est deja payee.");
        if (montant <= 0)
            throw invalid_argument("Le montant doit etre positif.");
        r->valider();  // la place est reservee definitivement
        Paiement* p = new Paiement(prochainPaiement++, r, montant);
        paiements.push_back(p);
        return p;
    }

    // ----- 7. Annuler un paiement / une reservation -----
    void annulerPaiement(int idReservation) {
        Reservation* r = trouverReservation(idReservation);
        if (r == nullptr)
            throw runtime_error("Reservation introuvable.");
        Paiement* p = trouverPaiementDeReservation(idReservation);
        if (p != nullptr) {
            p->rembourser();   // reservation payee puis annulee : remboursement
            cout << "Paiement rembourse." << endl;
        }
        r->annuler();          // libere la place si elle etait validee
        cout << "Reservation annulee." << endl;
    }

    // ----- 8. Consulter les reservations -----
    void consulterReservations() const {
        if (reservations.empty()) { cout << "Aucune reservation." << endl; return; }
        for (size_t i = 0; i < reservations.size(); i++)
            reservations[i]->afficher();
    }

    // ----- 9. Afficher les informations d'une reservation -----
    void afficherInfosReservation(int id) const {
        Reservation* r = trouverReservation(id);
        if (r == nullptr)
            throw runtime_error("Reservation introuvable.");
        cout << "----- Reservation #" << id << " -----" << endl;
        r->getPassager()->afficher();
        r->getVol()->afficher();
        cout << "Statut : " << texte(r->getStatut()) << endl;
        Paiement* p = trouverPaiementDeReservation(id);
        if (p != nullptr) p->afficher();
        else cout << "Aucun paiement." << endl;
    }
};

// ---------------------------------------------------------------
// Fonctions de saisie securisee
// ---------------------------------------------------------------
string lireTexte(const string& invite) {
    string s;
    while (true) {
        cout << invite;
        if (!getline(cin, s)) exit(0);
        if (!s.empty()) return s;
        cout << "Saisie vide, recommencez." << endl;
    }
}

int lireEntier(const string& invite) {
    while (true) {
        string s = lireTexte(invite);
        try {
            size_t pos;
            int n = stoi(s, &pos);
            if (pos == s.size()) return n;
        } catch (...) {}
        cout << "Entrez un nombre entier valide." << endl;
    }
}

double lireReel(const string& invite) {
    while (true) {
        string s = lireTexte(invite);
        try {
            size_t pos;
            double x = stod(s, &pos);
            if (pos == s.size()) return x;
        } catch (...) {}
        cout << "Entrez un nombre valide." << endl;
    }
}

void afficherMenu() {
    cout << "\n===== AGENCE DE VOYAGE =====" << endl;
    cout << " 1. Enregistrer un passager" << endl;
    cout << " 2. Ajouter un vol" << endl;
    cout << " 3. Afficher les vols disponibles" << endl;
    cout << " 4. Rechercher un vol" << endl;
    cout << " 5. Effectuer une reservation" << endl;
    cout << " 6. Enregistrer un paiement" << endl;
    cout << " 7. Annuler un paiement" << endl;
    cout << " 8. Consulter les reservations" << endl;
    cout << " 9. Afficher les informations d'une reservation" << endl;
    cout << "10. Quitter" << endl;
}

// ---------------------------------------------------------------
// Programme principal
// ---------------------------------------------------------------
int main() {
    Agence agence;
    int choix = 0;

    do {
        afficherMenu();
        choix = lireEntier("Votre choix : ");

        try {
            switch (choix) {
            case 1: {
                string nom = lireTexte("Nom : ");
                string prenom = lireTexte("Prenom : ");
                string tel = lireTexte("Telephone : ");
                Passager* p = agence.enregistrerPassager(nom, prenom, tel);
                cout << "Passager enregistre avec l'id " << p->getId() << endl;
                break;
            }
            case 2: {
                string num = lireTexte("Numero du vol : ");
                string dep = lireTexte("Ville de depart : ");
                string dest = lireTexte("Destination : ");
                string date = lireTexte("Date et heure : ");
                int cap = lireEntier("Capacite : ");
                agence.ajouterVol(num, dep, dest, date, cap);
                cout << "Vol ajoute." << endl;
                break;
            }
            case 3:
                agence.afficherVolsDisponibles();
                break;
            case 4:
                agence.rechercherVol(lireTexte("Numero ou destination : "));
                break;
            case 5: {
                int idP = lireEntier("Id du passager : ");
                string num = lireTexte("Numero du vol : ");
                Reservation* r = agence.effectuerReservation(idP, num);
                cout << "Reservation creee (id " << r->getId()
                     << ", EN_ATTENTE). Elle sera validee apres paiement." << endl;
                break;
            }
            case 6: {
                int idR = lireEntier("Id de la reservation : ");
                double montant = lireReel("Montant (FCFA) : ");
                agence.enregistrerPaiement(idR, montant);
                cout << "Paiement enregistre, reservation validee." << endl;
                break;
            }
            case 7:
                agence.annulerPaiement(lireEntier("Id de la reservation a annuler : "));
                break;
            case 8:
                agence.consulterReservations();
                break;
            case 9:
                agence.afficherInfosReservation(lireEntier("Id de la reservation : "));
                break;
            case 10:
                cout << "Au revoir !" << endl;
                break;
            default:
                cout << "Choix invalide (1 a 10)." << endl;
            }
        } catch (const exception& e) {
            cout << "Erreur : " << e.what() << endl;
        }
    } while (choix != 10);

    return 0;
}

/*  _____   _____ ____    ___   ___ ___  ____
 * |  __ \ / ____/ __ \  |__ \ / _ \__ \|___ \
 * | |__) | |   | |  | |    ) | | | | ) | __) |
 * |  ___/| |   | |  | |   / /| | | |/ / |__ <
 * | |    | |___| |__| |  / /_| |_| / /_ ___) |
 * |_|     \_____\____/  |____|\___/____|____/
 */


#ifndef SYNCHRO_H
#define SYNCHRO_H

#include <QDebug>

#include <pcosynchro/pcosemaphore.h>

#include "locomotive.h"
#include "ctrain_handler.h"
#include "synchrointerface.h"
#include <thread>



/**
 * @brief La classe Synchro implémente l'interface SynchroInterface qui
 * propose les méthodes liées à la section partagée.
 */
class Synchro final : public SynchroInterface
{
public:

    /**
     * @brief Synchro Constructeur de la classe qui représente la section partagée.
     * Initialisez vos éventuels attributs ici, sémaphores etc.
     */
    Synchro() {
        // TODO

    }

    /**
     * @brief access Méthode à appeler pour accéder à la section partagée
     *
     * Elle doit arrêter la locomotive et mettre son thread en attente si nécessaire.
     *
     * @param loco La locomotive qui essaie accéder à la section partagée
     */
    void access(Locomotive &loco) override {
        // TODO
        loco.arreter();

        if(loco.numero() == 20){
            prioriteLocoA.acquire();
            sectionPartagee.acquire();

            diriger_aiguillage(1,  TOUT_DROIT, 0);
            diriger_aiguillage(22, TOUT_DROIT, 0);

        } else {
            prioriteLocoB.acquire();
            sectionPartagee.acquire();

            diriger_aiguillage(1,  DEVIE, 0);
            diriger_aiguillage(22, DEVIE, 0);

        }
        loco.demarrer();

        // Exemple de message dans la console globale
        afficher_message(qPrintable(QString("The engine no. %1 accesses the shared section.").arg(loco.numero())));
    }

    /**
     * @brief leave Méthode à appeler pour indiquer que la locomotive est sortie de la section partagée
     *
     * Réveille les threads des locomotives potentiellement en attente.
     *
     * @param loco La locomotive qui quitte la section partagée
     */
    void leave(Locomotive& loco) override {
        // TODO
        if(loco.numero() == 20){
            prioriteLocoB.release();
        } else {
            prioriteLocoA.release();
        }

        sectionPartagee.release();

        // Exemple de message dans la console globale
        afficher_message(qPrintable(QString("The engine no. %1 leaves the shared section.").arg(loco.numero())));
    }

    /**
     * @brief stopAtStation Méthode à appeler quand la locomotive doit attendre à la gare
     *
     * Implémentez toute la logique que vous avez besoin pour que les locomotives
     * s'attendent correctement.
     *
     * @param loco La locomotive qui doit attendre à la gare
     */
    void stopAtStation(Locomotive& loco) override {
        // TODO
        loco.arreter();
        afficher_message(qPrintable(QString("The engine no. %1 arrives at the station.").arg(loco.numero())));
        mutexGare.acquire();
        nbTrainGare ++;


        if(loco.numero() == 20){
            if(nbTrainGare == 2){ // Si premier déjà passé
                prioriteLocoB.acquire();
            } else {
                prioriteLocoB.release();
            }
            mutexGare.release();
        } else {
            if(nbTrainGare == 2){
                prioriteLocoA.acquire();
            } else {
                prioriteLocoA.release();
            }
            mutexGare.release();
        }

        waitAtStation();
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        loco.demarrer();
        afficher_message(qPrintable(QString("The engine no. %1 leaves the station.").arg(loco.numero())));
    }

    /* A vous d'ajouter ce qu'il vous faut */

private:
    // Méthodes privées ...
    // Attribut privés ...
    PcoSemaphore sectionPartagee{1};
    PcoSemaphore attendreGare{0};
    PcoSemaphore mutexGare{1};
    int nbTrainGare = 0;
    PcoSemaphore prioriteLocoA {1};
    PcoSemaphore prioriteLocoB {1};

    void waitAtStation() {
        mutexGare.acquire();
        if(nbTrainGare == 2){
            nbTrainGare = 0;
            mutexGare.release();
            attendreGare.release();

        } else {
            mutexGare.release();
            attendreGare.acquire();

        }

    }
};


#endif // SYNCHRO_H

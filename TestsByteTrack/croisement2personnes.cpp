#include <iostream>
#include <vector>
#include <cmath>
#include <string>

#include "ByteTrack/BYTETracker.h"

int main()
{
    printf("------\nCe test réalise le track de deux personnes qui se croisent.\n------\n\n");
    byte_track::BYTETracker tracker(
        30,     // FPS
        30,     // track buffer
        0.5f,   // track threshold
        0.6f,   // high threshold
        0.8f    // match threshold
    );

    for (int frame = 0; frame < 20; frame++)
    {
        std::vector<byte_track::Object> objects;

        // Positions réelles simulées des deux personnes.
        //
        // A va de gauche vers droite.
        // B va de droite vers gauche.
        float xA = 100.0f + frame * 10.0f;
        float xB = 400.0f - frame * 10.0f;

        bool personneAVisible = !(frame >= 14 && frame <= 16);

        // =====================================================
        // Détection de A
        // =====================================================

        if (personneAVisible)
        {
            objects.emplace_back(
                byte_track::Rect<float>(
                    xA,
                    100.0f,
                    80.0f,
                    180.0f
                ),
                0,
                0.90f
            );
        }


        // =====================================================
        // Détection de B
        // =====================================================

        objects.emplace_back(
            byte_track::Rect<float>(
                xB,
                100.0f,
                80.0f,
                180.0f
            ),
            0,
            0.90f
        );

        // =====================================================
        // Mise à jour de ByteTrack
        // =====================================================

        const auto tracks = tracker.update(objects);

        // =====================================================
        // Affichage
        // =====================================================

        std::cout << "\n========================================\n";
        std::cout << "Frame " << frame << "\n";

        std::cout << "Detection A : x = " << xA << "\n";
        std::cout << "Detection B : x = " << xB << "\n";

        std::cout << "--- Tracks ByteTrack ---\n";

        for (const auto& track : tracks)
        {
            const auto& rect = track->getRect();

            float xTrack = rect.x();

            // -------------------------------------------------
            // Association uniquement pour rendre le test lisible
            // -------------------------------------------------
            //
            // Nous connaissons ici les positions réelles xA et xB
            // puisque nous les avons créées nous-mêmes.
            //
            // On regarde donc de quelle position théorique
            // le track est le plus proche.

            float distanceA = std::abs(xTrack - xA);
            float distanceB = std::abs(xTrack - xB);

            std::string personne;

            if (distanceA < distanceB)
            {
                personne = "Personne A";
            }
            else if (distanceA == distanceB)
            {
                personne = "Personnes se chevauchent";
            }
            else
            {
                personne = "Personne B";
            }

            std::cout
                << personne
                << " | ID = " << track->getTrackId()
                << " | x detection = ";

            if (personne == "Personne A")
            {
                    std::cout << xA;
            }
            else
            {
                std::cout << xB;
            }

            std::cout
                << " | x track = " << xTrack
                << "\n";
        }
    }

    return 0;
}
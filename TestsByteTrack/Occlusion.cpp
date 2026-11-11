#include <iostream>
#include <vector>

#include "ByteTrack/BYTETracker.h"

/* -----------------
Avec ce code d'occlusion, si la personne est cachée de la frame 6 à 14 elle est de nouveau associée à son id d'avant occlusion.
Mais si elle est cachée de la frame 6 à 15, elle est associée à un nouvel id quand elle reapparait.
La memoire de track est de 30 frames. Je n'explique pas pourquoi ByteTrack n'associe plus cette personne a son id alors qu'elle bouge dans la meme direction et a la meme vitesse.
------------------------------*/

int main()
{
    printf("------\nCe test réalise le track de deux personnes dont l'une est cachée pendant un certain nb de frames.\n------\n\n");
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

        // Personne A : va vers la droite
        float xA = 100.0f + frame * 5.0f;

        // Personne B : va vers la gauche
        float xB = 500.0f - frame * 5.0f;

        // --------------------------------------------------
        // Personne A
        // --------------------------------------------------
        //
        // Elle est visible :
        // frames 0 à 5
        //
        // Elle disparaît :
        // frames 6 à 9
        //
        // Elle réapparaît :
        // frame 10 et suivantes
        //

        bool personneAVisible = !(frame >= 6 && frame <= 14);

        if (personneAVisible)
        {
            objects.emplace_back(
                byte_track::Rect<float>(
                    xA,
                    100.0f,
                    80.0f,
                    180.0f
                ),
                0,       // classe person
                0.90f    // confiance
            );
        }

        // --------------------------------------------------
        // Personne B
        // --------------------------------------------------
        //
        // Elle reste toujours visible.
        //

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

        const auto tracks = tracker.update(objects);

        std::cout << "Frame " << frame << "\n";

        if (!personneAVisible)
        {
            std::cout << "Personne A : OCCULTEE\n";
        }

        for (const auto& track : tracks)
        {
            const auto& rect = track->getRect();
            float xTrack = rect.x();

            // On compare la position du track aux positions
            // théoriques de A et B.
            float distanceA = std::abs(xTrack - xA);
            float distanceB = std::abs(xTrack - xB);

            std::string personne;

            if (distanceA < distanceB)
            {
                personne = "Personne A";
            }
            else
            {
                personne = "Personne B";
            }

            std::cout
                << personne
                << " | ID = " << track->getTrackId()
                << " | x = " << xTrack
                << "\n";
        }
    }
}
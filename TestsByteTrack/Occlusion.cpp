#include <iostream>
#include <vector>

#include "ByteTrack/BYTETracker.h"

int main()
{
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

        bool personneAVisible = !(frame >= 6 && frame <= 9);

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
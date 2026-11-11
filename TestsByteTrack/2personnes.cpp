#include <iostream>
#include <vector>

#include "ByteTrack/BYTETracker.h"

int main()
{
    printf("------\nCe test réalise le track de deux personnes détéctées sur 10 frames.\n------\n\n");
    byte_track::BYTETracker tracker(
        30,     // FPS
        30,     // track buffer
        0.5f,   // track threshold
        0.6f,   // high threshold
        0.8f    // match threshold
    );

    for (int frame = 0; frame < 10; frame++)
    {
        std::vector<byte_track::Object> objects;

        objects.emplace_back(
        byte_track::Rect<float>(
            100 + frame * 5,
            100,
            80,
            180
        ),
        0,
        0.90f
        );

    objects.emplace_back(
        byte_track::Rect<float>(
            500 - frame * 5,
            100,
            80,
            180
        ),
        0,
        0.90f
        );

        const auto tracks = tracker.update(objects);

        std::cout << "Frame " << frame << "\n";

        for (const auto& track : tracks)
        {
            const auto& rect = track->getRect();
            std::cout
                << "ID = "
                << track->getTrackId()
                << " | x = " << rect.x()
                << "\n";
        }
    }
}
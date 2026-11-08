#include <iostream>
#include <vector>

#include "ByteTrack/BYTETracker.h"
//#include "../ByteTrack-cpp/include/ByteTrack/BYTETracker.h"

int main()
{
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

        const float x = 100.0f + frame * 10.0f;

        objects.emplace_back(
            byte_track::Rect<float>(
                x,
                100.0f,
                80.0f,
                180.0f
            ),
            0,
            0.90f
        );

        const auto tracks = tracker.update(objects);

        std::cout << "Frame " << frame << "\n";

        for (const auto& track : tracks)
        {
            std::cout
                << "ID = "
                << track->getTrackId()
                << "\n";
        }
    }
}
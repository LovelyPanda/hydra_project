//MyTerrain.hpp

#ifndef MY_TERRAIN_HPP__
#define MY_TERRAIN_HPP__

/**
 * \class MyTerrain
 * \brief Class to represent terrain (container)
 *
 * Contains terrain fragments and all the data needed to
 * load and/or visualize terrain.
 * Keeps links to ChunkedTerrain object and TerrainFragment objects.
 *
 * In fact this terrain is synchronized version of ChunkedTerrain, our loader implementations
 * work with this object and store all the metadata here. All the loaded data 
 * transparently unloads to ChunkedTerrain to be used by manager and renderer (or whoever)
 */

#include <boost/noncopyable.hpp>
#include "common/SharedPtr.hpp"

namespace hydra{
    namespace data{
        class ChunkedTerrain;
        typedef hydra::common::SharedPtr<ChunkedTerrain>::Type ChunkedTerrainPtr;
    }
}

class MyTerrain: private boost::noncopyable{
 
public:
    ///constructor (builds a terrain with link to ChunkedTerrain object)
    MyTerrain(hydra::data::ChunkedTerrainPtr inTerrain);

    ///destructor (cleans everything up)
    ~MyTerrain();


};


#endif

[![discord](https://img.shields.io/discord/495955797872869376.svg?logo=discord "Discord")](https://discord.gg/BfceAsX)
[![travis](https://img.shields.io/travis/caseymcc/voxigen/master.svg?logo=travis&style=flat-square&label=Linux/OSX "Travis CI")](https://travis-ci.org/caseymcc/voxigen)
[![appveyor](https://img.shields.io/appveyor/ci/caseymcc/voxigen/master.svg?logo=appveyor&style=flat-square&label=Windows "AppVeyor CI")](https://ci.appveyor.com/project/caseymcc/voxigen)

# ![logo](https://github.com/caseymcc/voxigen/raw/master/resources/voxigen_64_tr.png) voxigen
Voxel handling library, generating/loading/saving/meshing/memory managment.  

- Documentation - https://github.com/caseymcc/voxigen/wiki
- Video of current version - https://www.youtube.com/watch?v=KFW_KskYbPA  
- Region HeightMap testing - https://www.youtube.com/watch?v=NAaNWuHsJkQ

![screenshot](https://github.com/caseymcc/voxigen/raw/master/resources/screenshot1.png)

Although the library is currently geared towards blocky worlds the hope is to support Marching Cubes and Dual Contouring in the future. Voxigen handles chunking and loading/saving of the world based on a maximum size. The world is not infinite but the size can be chosen as though it seems to be infinite. Also the current generator choses blocks based on a cylinder out of 3D noise allowing the y axis to wrap around. Sphere could be possible but rendering might become an issue.

The plan is to support physics directly into this layer so the world is segmented into regions that the physics will stay stable in. There should be sharing across the segments to make sure the simulation is across the full world.

There is currently a very simple generator in place, hopefully this will get more complex as time goes on. I am hoping to build the generator system to be plugable, allowing outside developers to add their own implmentation with out the worry of complexity the base has.

The library is template based. The type of block is up to the user and is the first argument of the world object, the remaining are the size of the chunks and region size.
```
    World<BlockType, ChunkSizeX, ChunkSizeY, ChunkSizeZ, RegionSizeX, RegionSizeY, RegionSizeZ> world;
```
The world segmentation is chosen at compile-time but the size of the world is a runtime option. Currently it can easily handle a sun sized planet (assuming 0.5 meter cubes). It should be noted that the library assumes right handed world with z as the "up" dimension in the world.

This is still very new and likely at any point things may change and also not work. 

## Current Features:
- Templated Block type and chunk size
- Threaded Generator/Load/Save
  - Simple LOD support
- Plugable generator system
  - Simple Generator
- Segment world for Physics (also increases possible size)
- Simple Loading/Saving
- Naive Render
  - Threaded mesher/buffer upload
  

## Planned Features:
- [ ] Build large scale polygon replacement terrain for lower LOD levels
- [ ] Build 2D imposters for distant regions based on the voxel data
- [ ] Intergration with Lumberyard
- [ ] Add physics (through Lumberyard)
- [ ] More complex base generator

Any comments/suggestions will always be welcome.

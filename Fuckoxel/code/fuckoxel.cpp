#include "fuckoxel_math.h"
#include "fuckoxel_shader.h"

struct stack_allocator
{
    uint64_t Size;
    uint8_t *Base;
    uint64_t Used;
};

static void
InitializeStackAllocator(stack_allocator *Allocator, uint64_t Size, void *Base)
{
    Allocator->Size = Size;
    Allocator->Base = (uint8_t *)Base;
    Allocator->Used = 0;
}

#define PushStruct(Allocator, type) (type *)PushSize(Allocator, sizeof(type))
#define PushArray(Allocator, Count, type) (type *)PushSize(Allocator, (Count)*sizeof(type))
inline void *
PushSize(stack_allocator *Allocator, uint64_t Size)
{
    Assert((Allocator->Used + Size) <= Allocator->Size);

    void *Result = Allocator->Base + Allocator->Used;
    Allocator->Used += Size;

    return(Result);
}

struct stack_temp_memory
{
    stack_allocator *Allocator;
    uint64_t Used;
};

inline stack_temp_memory
BeginTempMemory(stack_allocator *Allocator)
{
    stack_temp_memory Result;
    Result.Allocator = Allocator;
    Result.Used = Allocator->Used;

    return(Result);
}

inline void
EndTempMemory(stack_temp_memory TempMemory)
{
    stack_allocator *Allocator = TempMemory.Allocator;
    Assert(Allocator->Used >= TempMemory.Used);
    Allocator->Used = TempMemory.Used;
}

static void
ZeroSize(void *Memory, uint64_t Size)
{
    uint8_t *Byte = (uint8_t *)Memory;
    while(Size--)
    {
        *Byte++ = 0;
    }
}


struct dynamic_array_vec3
{
    uint32_t MaxEntriesCount;
    uint32_t EntriesCount;
    vec3 *Entries;
};

static void
InitializeDynamicArray(dynamic_array_vec3 *Array, uint32_t MaxEntriesCount = 0)
{
    Array->MaxEntriesCount = MaxEntriesCount;
    Array->EntriesCount = 0;
    Array->Entries = 0;
}

static void
ExpandDynamicArray(dynamic_array_vec3 *Array)
{
    uint32_t NewMaxEntriesCount = Array->MaxEntriesCount ? 2*Array->MaxEntriesCount : 1;
    vec3 *NewMemory = (vec3 *)malloc(NewMaxEntriesCount*sizeof(vec3));
    Assert(NewMemory);

    for(uint32_t EntryIndex = 0;
        EntryIndex < Array->EntriesCount;
        EntryIndex++)
    {
        NewMemory[EntryIndex] = Array->Entries[EntryIndex];
    }

    if(Array->Entries) free(Array->Entries);
    Array->MaxEntriesCount = NewMaxEntriesCount;
    Array->Entries = NewMemory;
}

static void
FreeDynamicArray(dynamic_array_vec3 *Array)
{
    free(Array->Entries);
    Array->Entries = 0;
    Array->MaxEntriesCount = 0;
    Array->EntriesCount = 0;
}

static void
PushEntry(dynamic_array_vec3 *Array, vec3 Entry)
{
    if(Array->MaxEntriesCount <= Array->EntriesCount)
    {
        ExpandDynamicArray(Array);
    }

    Array->Entries[Array->EntriesCount++] = Entry;
}

struct dynamic_array_vec4
{
    uint32_t MaxEntriesCount;
    uint32_t EntriesCount;
    vec4 *Entries;
};

static void
InitializeDynamicArray(dynamic_array_vec4 *Array, uint32_t MaxEntriesCount = 0)
{
    Array->MaxEntriesCount = MaxEntriesCount;
    Array->EntriesCount = 0;
    Array->Entries = 0;
}

static void
ExpandDynamicArray(dynamic_array_vec4 *Array)
{
    uint32_t NewMaxEntriesCount = Array->MaxEntriesCount ? 2*Array->MaxEntriesCount : 1;
    vec4 *NewMemory = (vec4 *)malloc(NewMaxEntriesCount*sizeof(vec4));
    Assert(NewMemory);

    for(uint32_t EntryIndex = 0;
        EntryIndex < Array->EntriesCount;
        EntryIndex++)
    {
        NewMemory[EntryIndex] = Array->Entries[EntryIndex];
    }

    if(Array->Entries) free(Array->Entries);
    Array->MaxEntriesCount = NewMaxEntriesCount;
    Array->Entries = NewMemory;
}

static void
FreeDynamicArray(dynamic_array_vec4 *Array)
{
    free(Array->Entries);
    Array->Entries = 0;
    Array->MaxEntriesCount = 0;
    Array->EntriesCount = 0;
}

static void
PushEntry(dynamic_array_vec4 *Array, vec4 Entry)
{
    if(Array->MaxEntriesCount <= Array->EntriesCount)
    {
        ExpandDynamicArray(Array);
    }

    Array->Entries[Array->EntriesCount++] = Entry;
}

struct chunk;
struct light_node
{
    int8_t LightLevel;
    uint32_t X, Y, Z;
    chunk *Chunk;

    light_node() {}
    light_node(int8_t level, uint32_t x, uint32_t y, uint32_t z, chunk *ch) : 
               LightLevel(level), X(x), Y(y), Z(z), Chunk(ch) {}
};

struct dynamic_array_light_node
{
    uint32_t MaxEntriesCount;
    uint32_t EntriesCount;
    light_node *Entries;
};

static void
InitializeDynamicArray(dynamic_array_light_node *Array, uint32_t MaxEntriesCount = 0)
{
    Array->MaxEntriesCount = MaxEntriesCount;
    Array->EntriesCount = 0;
    Array->Entries = 0;
}

static void
ExpandDynamicArray(dynamic_array_light_node *Array)
{
    uint32_t NewMaxEntriesCount = Array->MaxEntriesCount ? 2*Array->MaxEntriesCount : 1;
    light_node *NewMemory = (light_node *)malloc(NewMaxEntriesCount*sizeof(light_node));
    Assert(NewMemory);

    for(uint32_t EntryIndex = 0;
        EntryIndex < Array->EntriesCount;
        EntryIndex++)
    {
        NewMemory[EntryIndex] = Array->Entries[EntryIndex];
    }

    if(Array->Entries) free(Array->Entries);
    Array->MaxEntriesCount = NewMaxEntriesCount;
    Array->Entries = NewMemory;
}

static void
FreeDynamicArray(dynamic_array_light_node *Array)
{
    free(Array->Entries);
	Array->Entries = 0;
	Array->MaxEntriesCount = 0;
    Array->EntriesCount = 0;
}

static void
PushEntry(dynamic_array_light_node *Array, light_node Entry)
{
    if(Array->MaxEntriesCount <= Array->EntriesCount)
    {
        ExpandDynamicArray(Array);
    }

    Array->Entries[Array->EntriesCount++] = Entry;
}


struct sunlight_bfs_queue
{
    uint32_t FirstIndex;
    dynamic_array_light_node LightNodes;
};
inline void
Enqueue(sunlight_bfs_queue *Queue, light_node LightNode)
{
    PushEntry(&Queue->LightNodes, LightNode);
}

inline light_node
Dequeue(sunlight_bfs_queue *Queue)
{
    Assert(Queue->FirstIndex < Queue->LightNodes.EntriesCount);
    light_node Result = Queue->LightNodes.Entries[Queue->FirstIndex++];

    return(Result);
}

#define CHUNK_DIM_X 32
#define CHUNK_DIM_Z CHUNK_DIM_X
#define CHUNK_DIM_Y 256
#define INVALID_CHUNK_P INT32_MAX

#define IsBlockSolid(Blocks, X, Y, Z) (Blocks[(Y) + (X)*CHUNK_DIM_Y + (Z)*CHUNK_DIM_X*CHUNK_DIM_Y].Type != block_type::BLOCK_AIR)
#define BlockType(Blocks, X, Y, Z) (Blocks[(Y) + (X)*CHUNK_DIM_Y + (Z)*CHUNK_DIM_X*CHUNK_DIM_Y].Type)
#define BlockLightLevel(Blocks, X, Y, Z) (Blocks[(Y) + (X)*CHUNK_DIM_Y + (Z)*CHUNK_DIM_X*CHUNK_DIM_Y].LightLevel)
#define BlockColor(Colors, X, Y, Z) (Colors[(Y) + (X)*CHUNK_DIM_Y + (Z)*CHUNK_DIM_X*CHUNK_DIM_Y])

enum class block_type : uint8_t
{
    BLOCK_AIR,
    BLOCK_SOIL,
    BLOCK_RIVER,
    BLOCK_WOOD,
    BLOCK_LEAVE
};
struct block
{
    block_type Type;
    int8_t LightLevel;
};
struct chunk_blocks_info
{
    block Blocks[CHUNK_DIM_X*CHUNK_DIM_Y*CHUNK_DIM_Z];
    vec3 Colors[CHUNK_DIM_X*CHUNK_DIM_Y*CHUNK_DIM_Z];
    
    chunk_blocks_info *NextFree;
};

enum chunk_flag
{
    CHUNK_GENERATED = 0x1,
    CHUNK_TREES_PLACED = 0x2,
    CHUNK_LIGHT_PROP_STARTED = 0x4,
    CHUNK_LIGHT_PROP_FINISHED = 0x8,
    CHUNK_SETUP = 0x10,
    CHUNK_LOADED = 0x20,
    CHUNK_RECENTLY_USED = 0x40
};
struct chunk
{
    int32_t X, Z;
    vec3 SimP;

    uint8_t Flags;

    chunk_blocks_info *BlocksInfo;
    dynamic_array_vec4 Vertices;
    GLuint VAO, VBO;

    sunlight_bfs_queue SunlightBFSQueue;
    sunlight_bfs_queue SunlightBFSQueueFromOtherChunks;
    // TODO(georgy): Get rid of this HeightMap somehow?
    uint32_t HeightMap[CHUNK_DIM_X*CHUNK_DIM_Z];

    chunk *NextInHash;
};
inline void
SetFlags(chunk* Chunk, uint8_t Flags)
{
	Chunk->Flags |= Flags;
}
inline void
ClearFlags(chunk* Chunk, uint8_t Flags)
{
	Chunk->Flags &= ~Flags;
}
inline bool
CheckFlag(chunk* Chunk, chunk_flag Flag)
{
	bool Result = Chunk->Flags & Flag;
	return(Result);
}

struct world_position
{
    int32_t ChunkX, ChunkZ;
    vec3 Offset;
};

struct world
{
    vec3 ChunkDimInMeters;
    float BlockDimInMeters;

    uint32_t RecentlyUsedChunksCount;
    chunk Chunks[4096];

    chunk_blocks_info *FirstFreeChunkBlocksInfo;

    uint32_t ChunksToGenerateCount;
    chunk *ChunksToGenerate[8];

    uint32_t ChunksToPlaceTreesCount;
    chunk *ChunksToPlaceTrees[8];

    uint32_t ChunksToPropagateLightCount;
    chunk *ChunksToPropagateLight[8];

    uint32_t ChunksToFinishPropagateLightCount;
    chunk *ChunksToFinishPropagateLight[8];

    uint32_t ChunksToSetupCount;
    chunk *ChunksToSetup[8];

    uint32_t ChunksToLoadCount;
    chunk *ChunksToLoad[8];

    // TODO(georgy): Assert that 2048 is enough! (Make this dynamic array?)
    uint32_t ChunksToRenderCount;
    chunk *ChunksToRender[2048];
};

inline void
ResetWorldWork(world *World)
{
    World->ChunksToGenerateCount = 0;
    World->ChunksToPlaceTreesCount = 0;
    World->ChunksToPropagateLightCount = 0;
    World->ChunksToFinishPropagateLightCount = 0;
    World->ChunksToSetupCount = 0;
    World->ChunksToLoadCount = 0;
    World->ChunksToRenderCount = 0;
}

static chunk *
GetChunk(world *World, int32_t ChunkX, int32_t ChunkZ, stack_allocator *WorldAllocator = 0)
{
    float A = 0.6180339887f;
    uint32_t HashValue = Absolute(1123*ChunkX + 719*ChunkZ);
	uint32_t HashIndex = (uint32_t)(ArrayCount(World->Chunks) * ((A * HashValue) - (uint32_t)(A * HashValue)));

    chunk *Chunk = &World->Chunks[HashIndex];
    if(Chunk->X == INVALID_CHUNK_P)
    {
        Chunk->X = ChunkX;
        Chunk->Z = ChunkZ;
    }
    else
    {
        Chunk = Chunk->NextInHash;
        while(Chunk)
        {
            if((Chunk->X == ChunkX) && (Chunk->Z == ChunkZ))
            {
                break;
            }

            Chunk = Chunk->NextInHash;
        }
    }

    if(!Chunk && WorldAllocator)
    {
        Chunk = PushStruct(WorldAllocator, chunk);
        Chunk->X = ChunkX;
        Chunk->Z = ChunkZ;

        Chunk->Flags = 0;
        Chunk->BlocksInfo = 0;
        Chunk->VAO = Chunk->VBO = 0;

        Chunk->SunlightBFSQueue.FirstIndex = 0;
        Chunk->SunlightBFSQueueFromOtherChunks.FirstIndex = 0;
        InitializeDynamicArray(&Chunk->SunlightBFSQueue.LightNodes);
        InitializeDynamicArray(&Chunk->SunlightBFSQueueFromOtherChunks.LightNodes);

        Chunk->NextInHash = World->Chunks[HashIndex].NextInHash;
        World->Chunks[HashIndex].NextInHash = Chunk;
    }

    return(Chunk);
}

static void
RecanonicalizeCoords(world *World, world_position *Pos)
{
    int32_t ChunkXOffset = FloorReal32ToInt32(Pos->Offset.x / World->ChunkDimInMeters.x);
    Pos->ChunkX += ChunkXOffset;

    int32_t ChunkZOffset = FloorReal32ToInt32(Pos->Offset.z / World->ChunkDimInMeters.z);
    Pos->ChunkZ += ChunkZOffset;

    vec3 ChunkOffset = vec3i(ChunkXOffset, 0, ChunkZOffset);
    Pos->Offset = Pos->Offset - Hadamard(ChunkOffset, World->ChunkDimInMeters);
}

static world_position
MapIntoChunkSpace(world *World, world_position *Pos, vec3 Offset)
{
    world_position Result = *Pos;

    Result.Offset += Offset;
    RecanonicalizeCoords(World, &Result);

    return(Result);
}

inline vec3
Substract(world *World, world_position *A, world_position *B)
{
    vec3 ChunkDelta = vec3i(A->ChunkX - B->ChunkX, 0, A->ChunkZ - B->ChunkZ);
    vec3 Result = Hadamard(ChunkDelta, World->ChunkDimInMeters) + (A->Offset - B->Offset);

    return(Result);
}

// NOTE(georgy): Returns chunk for (X, Y, Z) block offset from current chunk
static chunk *
GetChunkForBlock(world *World, chunk *Chunk, int32_t &X, int32_t Y, int32_t &Z)
{
    chunk *Result = Chunk;

    if((X >= 0) && (X < CHUNK_DIM_X) &&
       (Z >= 0) && (Z < CHUNK_DIM_Z))
    {
        // NOTE(georgy): Initial chunk
    }
    else
    {
        int32_t NewChunkX = Chunk->X;
        int32_t NewChunkZ = Chunk->Z;

        int32_t XOffset = FloorReal32ToInt32((float)X / CHUNK_DIM_X);
        int32_t ZOffset = FloorReal32ToInt32((float)Z / CHUNK_DIM_Z);

        NewChunkX += XOffset;
        NewChunkZ += ZOffset;

        X -= CHUNK_DIM_X*XOffset;
        Z -= CHUNK_DIM_Z*ZOffset;

        Result = GetChunk(World, NewChunkX, NewChunkZ);
    }

    return(Result);
}

static int8_t
GetLightLevelBetweenChunks(world *World, chunk *Chunk, int32_t X, int32_t Y, int32_t Z)
{
    Chunk = GetChunkForBlock(World, Chunk, X, Y, Z);
    Assert(Chunk);
    
    int8_t Result;
    if(Y < 0) Result = 0;
    else if(Y >= CHUNK_DIM_Y) Result = 15;
    else Result = BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y, Z);

    return(Result);
}

static bool
IsBlockSolidBetweenChunks(world *World, chunk *Chunk, int32_t X, int32_t Y, int32_t Z)
{
    Chunk = GetChunkForBlock(World, Chunk, X, Y, Z);
    Assert(Chunk);

    bool Result;
    if(Y < 0) Result = true;
    else if(Y >= CHUNK_DIM_Y) Result = false;
    else Result = IsBlockSolid(Chunk->BlocksInfo->Blocks, X, Y, Z);

    return(Result);
}

static uint32_t
GetHeightBetweenChunks(world *World, chunk *Chunk, int32_t X, int32_t Z)
{
    Chunk = GetChunkForBlock(World, Chunk, X, 0, Z);
    Assert(Chunk);

    uint32_t Result = Chunk->HeightMap[Z*CHUNK_DIM_X + X];
    return(Result);
}

static void
SetBlockTypeBetweenChunks(world *World, chunk *Chunk, int32_t X, int32_t Y, int32_t Z, block_type Type)
{
    Chunk = GetChunkForBlock(World, Chunk, X, Y, Z);
    Assert(Chunk);

    BlockType(Chunk->BlocksInfo->Blocks, X, Y, Z) = Type;
}

static void
SetBlockColorBetweenChunks(world *World, chunk *Chunk, int32_t X, int32_t Y, int32_t Z, vec3 Color)
{
    Chunk = GetChunkForBlock(World, Chunk, X, Y, Z);
    Assert(Chunk);

    BlockColor(Chunk->BlocksInfo->Colors, X, Y, Z) = Color;
}

static void
GenerateChunks(world *World, stack_allocator *WorldAllocator)
{
    for(uint32_t ChunkIndex = 0;
        ChunkIndex < World->ChunksToGenerateCount;
        ChunkIndex++)
    {
		chunk* Chunk = World->ChunksToGenerate[ChunkIndex];
        Assert(!CheckFlag(Chunk, CHUNK_GENERATED));

        // TODO(georgy): I think float is not enough for all chunks! Check!
        float BaseX = Chunk->X*World->ChunkDimInMeters.x;
        float BaseZ = Chunk->Z*World->ChunkDimInMeters.z;

        if(!World->FirstFreeChunkBlocksInfo)
        {
            World->FirstFreeChunkBlocksInfo = PushStruct(WorldAllocator, chunk_blocks_info);
        }
        Chunk->BlocksInfo = World->FirstFreeChunkBlocksInfo;
        World->FirstFreeChunkBlocksInfo = World->FirstFreeChunkBlocksInfo->NextFree;
        ZeroSize(Chunk->BlocksInfo, sizeof(chunk_blocks_info));

        for(uint32_t Z = 0; Z < CHUNK_DIM_Z; Z++)
        {
            for(uint32_t X = 0; X < CHUNK_DIM_X; X++)
			{
				float XPos = BaseX + X * World->BlockDimInMeters;
				float ZPos = BaseZ + Z * World->BlockDimInMeters;

                float DistortionAmplitude = 30.0f;
				float DistortionFrequency = 0.5f*0.008333f;
                float XDistortion = DistortionAmplitude*(2.0f*(1.0f - SimplexNoise2DBetween01(DistortionFrequency*XPos + 235.0f, DistortionFrequency*ZPos + 196.0f)) - 1.0f);
                float ZDistortion = DistortionAmplitude*(2.0f*(1.0f - SimplexNoise2DBetween01(DistortionFrequency*XPos + 93.0f, DistortionFrequency*ZPos + 321.0f)) - 1.0f);
                XDistortion = ZDistortion = 0;

                const float StartFrequency = 0.008333f;
                uint32_t Octaves = 4;

                // NOTE(georgy): Ground
                float Value0 = 0;
                float Amplitude = 0.1f;
                float Frequency = 0.5f*StartFrequency;
                for (float Octave = 0; Octave < Octaves; Octave++)
                {
                    float NoiseValue = SimplexNoise2DBetween01(Frequency*(XPos + XDistortion) + 200.0f,
                                                               Frequency*(ZPos + ZDistortion) + 100.0f);
                    Value0 += Amplitude*NoiseValue;

                    Amplitude *= 2.0f;
                    Frequency *= 0.5f;
                }

                // NOTE(georgy): Hills
                float Value1 = 0.0f;
                Amplitude = 0.2f;
                Frequency = 2.0f*StartFrequency;
                for (float Octave = 0; Octave < Octaves; Octave++)
                {
                    float NoiseValue = Absolute(SimplexNoise2D(Frequency*(XPos + XDistortion) + 100.0f,
                                                               Frequency*(ZPos + ZDistortion) + 300.0f));
                    Value1 += Amplitude*NoiseValue;

                    Amplitude *= 2.0f;
                    Frequency *= 0.5;
                }

                // NOTE(georgy): Add some roughness to the hills
                Value1 += 0.04f*SimplexNoise2D(((XPos + XDistortion) / 18.0f) + 100.0f, ((ZPos + ZDistortion) / 12.0f) + 300.0f);

                Frequency *= 2.0f;

                // NOTE(georgy): Height variation on hills
                float Blend = SimplexNoise2D(Frequency*(XPos + XDistortion) + 310.0f, Frequency*(ZPos + ZDistortion) + 470.0f);
                Blend += 1;
                Value1 /= 2.0f*(Octaves - 1);

                // Blend the two values
                Blend = (1.0f - Blend) + 1.0f;
                Blend *= 0.5f;
                Blend = Blend*Blend;

                Value1 *= Blend;

                // Scale appropriately
                float Height = (70.0f*Value0 + 350.0f*Value1);
                Height = Clamp(Height, 0.0f, 255.0f);
                Chunk->HeightMap[Z*CHUNK_DIM_X + X] = (uint32_t)roundf(Height);


                // NOTE(georgy): River generation part
                Frequency = 0.5f*StartFrequency;
                float RiverNoise = SimplexNoise2D(Frequency*XPos + 200.0f,
                                                  Frequency*ZPos + 200.0f);
                
                Frequency *= 2.0f;
                RiverNoise += 0.2f*SimplexNoise2D(Frequency*XPos + 200.0f,
                                                  Frequency*ZPos + 200.0f);

                Frequency *= 2.0f;
                RiverNoise += 0.02f*SimplexNoise2D(Frequency*XPos + 200.0f,
                                                   Frequency*ZPos + 200.0f);

                float MaxRiverHeight = 120.0f;
                float MinRiverHeightBorder = 40.0f;
                float t = Clamp((Height - MinRiverHeightBorder) / (MaxRiverHeight - MinRiverHeightBorder), 0.0f, 1.0f);
                float RiverWidthBorder = Lerp(0.1f, 0.035f, t);

                if((Height < MaxRiverHeight) && (RiverNoise <= RiverWidthBorder) && (RiverNoise >= -RiverWidthBorder))
                {
                    float CutHeightNoise = SimplexNoise2DBetween01(2.0f*Frequency*XPos + 478.0f, 2.0f*Frequency*ZPos + 312.0f);

                    float MaxCutHeight = 6.0f;
                    float CutHeight = (1.0f - t)*MaxCutHeight*CutHeightNoise;
                    CutHeight = Clamp(CutHeight, 1.0f, 5.0f);
                    Chunk->HeightMap[Z*CHUNK_DIM_X + X] -= (uint32_t)CutHeight;

                    for(uint32_t Y = 0; Y <= Chunk->HeightMap[Z*CHUNK_DIM_X + X]; Y++)
                    {
                        Chunk->BlocksInfo->Blocks[Y + X*CHUNK_DIM_Y + Z*CHUNK_DIM_X*CHUNK_DIM_Y].Type = block_type::BLOCK_RIVER;
                        Chunk->BlocksInfo->Colors[Y + X*CHUNK_DIM_Y + Z*CHUNK_DIM_X*CHUNK_DIM_Y] = vec3(0.0f, 0.0f, 1.0f);
                    }
                }
                else if((Height > MaxRiverHeight) && (Height < (MaxRiverHeight + 5.0f)) && 
                        (RiverNoise <= 2.0f*RiverWidthBorder) && (RiverNoise >= -2.0f*RiverWidthBorder))
                {                    
                    Chunk->HeightMap[Z*CHUNK_DIM_X + X] = (uint32_t)MaxRiverHeight;

                    for(uint32_t Y = 0; Y <= Chunk->HeightMap[Z*CHUNK_DIM_X + X]; Y++)
                    {
                        Chunk->BlocksInfo->Blocks[Y + X*CHUNK_DIM_Y + Z*CHUNK_DIM_X*CHUNK_DIM_Y].Type = block_type::BLOCK_RIVER;
                        Chunk->BlocksInfo->Colors[Y + X*CHUNK_DIM_Y + Z*CHUNK_DIM_X*CHUNK_DIM_Y] = vec3(0.0f, 0.0f, 1.0f);
                    }
                }
                else
                {
                    for(uint32_t Y = 0; Y <= Chunk->HeightMap[Z*CHUNK_DIM_X + X]; Y++)
                    {
                        float YPos = Y*World->BlockDimInMeters;
                        Frequency = StartFrequency;
                        float ColorNoise = SimplexNoise3DBetween01(Frequency*XPos + 135.0f,
                                                                   Frequency*YPos + 235.0f,
                                                                   Frequency*ZPos + 335.0f);

                        vec3 Color = vec3(0.0f, 0.0f, 0.0f);
                        if(Y == Chunk->HeightMap[Z*CHUNK_DIM_X + X])
                        {
                            Color = Lerp(vec3(0.768f, 1.0f, 0.992f), vec3(0.615f, 0.894f, 1.0f), t*ColorNoise*ColorNoise);
                        }
                        else
                        {
                            Color = Lerp(vec3(0.65f, 0.65f, 0.65f), vec3(0.75f, 0.75f, 0.775f), ColorNoise*ColorNoise);
                        }

                        Chunk->BlocksInfo->Blocks[Y + X*CHUNK_DIM_Y + Z*CHUNK_DIM_X*CHUNK_DIM_Y].Type = block_type::BLOCK_SOIL;
                        Chunk->BlocksInfo->Colors[Y + X*CHUNK_DIM_Y + Z*CHUNK_DIM_X*CHUNK_DIM_Y] = Color;
                    }
                }
			}
        }

        SetFlags(Chunk, CHUNK_GENERATED);
    }
}

static bool
CanPlaceTree(world *World, chunk *Chunk, float Height, int32_t X, int32_t Z)
{
    for(int32_t ZOffset = -3; (ZOffset <= 3); ZOffset++)
    {
        for(int32_t XOffset = -3; (XOffset <= 3); XOffset++)
        {
            float HeightTest = (float)GetHeightBetweenChunks(World, Chunk, X + XOffset, Z + ZOffset);

            if(Height != HeightTest) return(false);
        }
    }

    return(true);
}

static void
AddLeaves(world *World, chunk *Chunk, int32_t BaseX, int32_t BaseY, int32_t BaseZ, 
          int32_t RadiusX, int32_t RadiusY, int32_t RadiusZ, int32_t YOffsetToCenter,
		  vec3 Color)
{
    vec3 SphereCenter = vec3(World->BlockDimInMeters*BaseX + 0.5f*World->BlockDimInMeters, 
                             World->BlockDimInMeters*(BaseY + YOffsetToCenter/2) + 0.5f*World->BlockDimInMeters, 
                             World->BlockDimInMeters*BaseZ + 0.5f*World->BlockDimInMeters);
    for(int32_t I = -RadiusZ; I <= RadiusZ; I++)
    {
        for(int32_t J = -RadiusX; J <= RadiusX; J++)
        {
            int32_t X = BaseX + J;
            int32_t Z = BaseZ + I;

            for(int32_t K = 0; K < 2*RadiusY; K++)
            {
                int32_t Y = BaseY + K;

                vec3 P = vec3(World->BlockDimInMeters*X + 0.5f*World->BlockDimInMeters, 
                              World->BlockDimInMeters*Y + 0.5f*World->BlockDimInMeters, 
                              World->BlockDimInMeters*Z + 0.5f*World->BlockDimInMeters);
                vec3 ToLeave = P - SphereCenter;

                vec3 RadiusInMeters = World->BlockDimInMeters*vec3i(RadiusX, RadiusY, RadiusZ);
                float EllipsoidEq = Square(ToLeave.x)/Square(RadiusInMeters.x) + 
                                    Square(ToLeave.y)/Square(RadiusInMeters.y) + 
                                    Square(ToLeave.z)/Square(RadiusInMeters.z);

                if(EllipsoidEq <= 1.0f)
                {
                    if(!IsBlockSolidBetweenChunks(World, Chunk, X, Y, Z))
                    {
                        SetBlockTypeBetweenChunks(World, Chunk, X, Y, Z, block_type::BLOCK_LEAVE);
                        SetBlockColorBetweenChunks(World, Chunk, X, Y, Z, Color);
                    }
                }
            }
        }
    }
}

static void
PlaceTreesChunks(world *World)
{
    for(uint32_t ChunkIndex = 0;
        ChunkIndex < World->ChunksToPlaceTreesCount;
        ChunkIndex++)
    {
        chunk *Chunk = World->ChunksToPlaceTrees[ChunkIndex];
        Assert(!CheckFlag(Chunk, CHUNK_TREES_PLACED));

        // TODO(georgy): I think float is not enough for all chunks! Check!
        float BaseX = Chunk->X*World->ChunkDimInMeters.x;
        float BaseZ = Chunk->Z*World->ChunkDimInMeters.z;

        uint32_t TriesToPlaceTree = 0;
        while(TriesToPlaceTree < 5)
        {
            float Frequency = 10.0f*0.008333f;
            float NoiseX = SimplexNoise2DBetween01(Frequency*BaseX + (TriesToPlaceTree+1)*120.23f, Frequency*BaseZ + (TriesToPlaceTree+1)*100.56f);
            float NoiseZ = SimplexNoise2DBetween01(Frequency*BaseX + (TriesToPlaceTree+1)*931.67f, Frequency*BaseZ + (TriesToPlaceTree+1)*456.12f);

            uint32_t TreeX = FloorReal32ToInt32(NoiseX*CHUNK_DIM_X);
            uint32_t TreeZ = FloorReal32ToInt32(NoiseZ*CHUNK_DIM_Z);
            uint32_t Height = Chunk->HeightMap[TreeZ*CHUNK_DIM_X + TreeX];
            bool Placeable = CanPlaceTree(World, Chunk, (float)Height, TreeX, TreeZ);
            if(Placeable && (Height < 80))
            {
                if(BlockType(Chunk->BlocksInfo->Blocks, TreeX, Height, TreeZ) != block_type::BLOCK_RIVER)
                {
                    Frequency *= 2.0f;
                    float TrunkNoise = SimplexNoise2DBetween01(Frequency*BaseX + 555.44f, Frequency*BaseZ + 676.23f);

                    int32_t TrunkHeight = 6 + (int32_t)roundf(6*TrunkNoise);
                    for(int32_t TrunkY = 0; TrunkY < TrunkHeight; TrunkY++)
                    {
                        uint32_t Y = Height + (TrunkY + 1);
                        int32_t Width = Max(3, 5 - TrunkY);

                        for(int32_t LocalX = -Width; LocalX < Width; LocalX++)
                        {
                            for(int32_t LocalZ = -Width; LocalZ < Width; LocalZ++)
                            {
                                if(((LocalX >= 1) || (LocalX < -1)) && ((LocalZ >= 1) || (LocalZ < -1))) continue;

                                int32_t X = TreeX + LocalX;
                                int32_t Z = TreeZ + LocalZ;
                                SetBlockTypeBetweenChunks(World, Chunk, X, Y, Z, block_type::BLOCK_WOOD);
                                SetBlockColorBetweenChunks(World, Chunk, X, Y, Z, vec3(0.46666f, 0.2666f, 0.0f));
                            } 
                        }
                    }

                    Frequency *= 0.4f;
                    float RadiusYNoise = Square(SimplexNoise2DBetween01(Frequency*BaseX + 735.45f, Frequency*BaseZ + 12.23f));
                    float RadiusXNoise = Square(SimplexNoise2DBetween01(Frequency*BaseX + 673.78f, Frequency*BaseZ + 856.13f));
                    float RadiusZNoise = Square(SimplexNoise2DBetween01(Frequency*BaseX + 57.43f, Frequency*BaseZ + 219.68f));
                    int32_t RadiusY = 5 + (int32_t)(roundf(4*RadiusYNoise));
                    int32_t RadiusX = RadiusY + 2 + (int32_t)(roundf(4*RadiusXNoise));
                    int32_t RadiusZ = RadiusY + 2 + (int32_t)(roundf(4*RadiusZNoise));

					Frequency = 12.0f * 0.008333f;
					vec3 Colors[] = { vec3(0.0f, 0.53333f, 0.0f),
									  vec3(1.0f, 0.46666f, 0.0f),
									  vec3(0.8f, 1.0f, 0.0f),
									  vec3(0.3333f, 0.6f, 0.0f) };
					float LeaveColorNoise = SimplexNoise2DBetween01(Frequency * BaseX + 777.23f, Frequency * BaseZ + 666.56f);
					uint32_t ColorIndex = (int32_t)roundf(LeaveColorNoise * (ArrayCount(Colors) - 1));
					AddLeaves(World, Chunk, TreeX, Height + TrunkHeight + 1, TreeZ, RadiusX, RadiusY, RadiusZ, RadiusY, Colors[ColorIndex]);

                    Frequency *= 1.15f;
                    float StepNoise = SimplexNoise2DBetween01(Frequency*BaseX + 749.11f, Frequency*BaseZ + 497.22f);
                    int32_t Steps = 5 + (int32_t)(roundf(3*StepNoise));

                    float Angle = 0.0f;
                    for(int32_t Step = 0; Step < Steps; Step++)
                    {
                        float RadiusYNoise = SimplexNoise2DBetween01(Frequency*BaseX + Step*312.43f, Frequency*BaseZ + Step*78.45f);
                        float RadiusXNoise = SimplexNoise2DBetween01(Frequency*BaseX + Step*941.64f, Frequency*BaseZ + Step*56.234f);
                        float RadiusZNoise = SimplexNoise2DBetween01(Frequency*BaseX + Step*195.47f, Frequency*BaseZ + Step*191.123f);
                        int32_t NewRadiusY = RadiusY - (1 + (int32_t)roundf(2*RadiusYNoise));
                        int32_t NewRadiusX = RadiusX - (1 + (int32_t)roundf(4*RadiusXNoise));
                        int32_t NewRadiusZ = RadiusZ - (1 + (int32_t)roundf(4*RadiusZNoise));

                        float OffsetYNoise = SimplexNoise2DBetween01(Frequency*BaseX + 111.46f, Frequency*BaseZ + 222.12f);
                        vec3 Offset = vec3(7*Sin(Angle), roundf(5*OffsetYNoise) - 2, 7*Cos(Angle));

					    AddLeaves(World, Chunk, TreeX + (int32_t)roundf(Offset.x), Height + TrunkHeight + 1 + (int32_t)roundf(Offset.y), TreeZ + (int32_t)roundf(Offset.z), 
                                  NewRadiusX, NewRadiusY, NewRadiusZ, RadiusY, Colors[ColorIndex]);

                        Angle += 2*PI / Steps;
                    }
                }

                break;
            }

            ++TriesToPlaceTree;
        }

        SetFlags(Chunk, CHUNK_TREES_PLACED);
    }
}

static void
PropagateLightFromBlock(world *World, chunk *Chunk, light_node &LightNode, int32_t XToPropagate, int32_t YToPropagate, int32_t ZToPropagate)
{
    chunk *ChunkToPropagate = GetChunkForBlock(World, Chunk, XToPropagate, YToPropagate, ZToPropagate);
    Assert(ChunkToPropagate);

    int8_t NewLightLevel = ((LightNode.LightLevel - 1) >= 0) ? LightNode.LightLevel - 1 : 0;
    light_node NewLightNode(NewLightLevel, XToPropagate, YToPropagate, ZToPropagate, ChunkToPropagate);
    if(ChunkToPropagate == Chunk)
    {
        ChunkToPropagate->BlocksInfo->Blocks[YToPropagate + XToPropagate*CHUNK_DIM_Y + ZToPropagate*CHUNK_DIM_X*CHUNK_DIM_Y].LightLevel = 
                                    NewLightLevel;
        Enqueue(&ChunkToPropagate->SunlightBFSQueue, NewLightNode);
    }
    else
    {
        Enqueue(&ChunkToPropagate->SunlightBFSQueueFromOtherChunks, NewLightNode);
    }
}

static void
PropagateLightChunks(world *World)
{
    for(uint32_t ChunkIndex = 0;
        ChunkIndex < World->ChunksToPropagateLightCount;
        ChunkIndex++)
    {
        chunk *Chunk = World->ChunksToPropagateLight[ChunkIndex];
        Assert(!CheckFlag(Chunk, CHUNK_LIGHT_PROP_STARTED));

        for(uint32_t Z = 0; Z < CHUNK_DIM_Z; Z++)
        {
            for(uint32_t X = 0; X < CHUNK_DIM_X; X++)
            {
                if(!IsBlockSolid(Chunk->BlocksInfo->Blocks, X, CHUNK_DIM_Y - 1, Z))
                {
                    Chunk->BlocksInfo->Blocks[(CHUNK_DIM_Y - 1) + X*CHUNK_DIM_Y + Z*CHUNK_DIM_X*CHUNK_DIM_Y].LightLevel = 15;

                    light_node LightNode(15, X, CHUNK_DIM_Y - 1, Z, Chunk);
                    Enqueue(&Chunk->SunlightBFSQueue, LightNode);
                }
            }
        }

        while(Chunk->SunlightBFSQueue.FirstIndex != Chunk->SunlightBFSQueue.LightNodes.EntriesCount)
        {
            light_node LightNode = Dequeue(&Chunk->SunlightBFSQueue);
            uint32_t X = LightNode.X;
            uint32_t Y = LightNode.Y;
            uint32_t Z = LightNode.Z;

            if((!IsBlockSolidBetweenChunks(World, Chunk, X - 1, Y, Z)) &&
                ((GetLightLevelBetweenChunks(World, Chunk, X - 1, Y, Z) + 2) <= LightNode.LightLevel))
            {
                PropagateLightFromBlock(World, Chunk, LightNode, X - 1, Y, Z);
            }

            if((!IsBlockSolidBetweenChunks(World, Chunk, X + 1, Y, Z)) &&
                ((GetLightLevelBetweenChunks(World, Chunk, X + 1, Y, Z) + 2) <= LightNode.LightLevel))
            {
                PropagateLightFromBlock(World, Chunk, LightNode, X + 1, Y, Z);
            }

            if((!IsBlockSolidBetweenChunks(World, Chunk, X, Y, Z - 1)) &&
                ((GetLightLevelBetweenChunks(World, Chunk, X, Y, Z - 1) + 2) <= LightNode.LightLevel))
            {
                PropagateLightFromBlock(World, Chunk, LightNode, X, Y, Z - 1);
            }

            if((!IsBlockSolidBetweenChunks(World, Chunk, X, Y, Z + 1)) &&
                ((GetLightLevelBetweenChunks(World, Chunk, X, Y, Z + 1) + 2) <= LightNode.LightLevel))
            {
                PropagateLightFromBlock(World, Chunk, LightNode, X, Y, Z + 1);
            }

            if((Y != 0) && (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X, Y - 1, Z)) &&
                ((BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y-1, Z) + 2) <= LightNode.LightLevel))
            {
                int8_t NewLightLevel = 15;
                if(LightNode.LightLevel < 15)
                {
                    NewLightLevel = ((LightNode.LightLevel - 1) >= 0) ? LightNode.LightLevel - 1 : 0;
                }

                BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y-1, Z) = NewLightLevel;

                light_node NewLightNode(BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y - 1, Z), X, Y - 1, Z, Chunk);
                Enqueue(&Chunk->SunlightBFSQueue, NewLightNode);
            }

            if((Y != (CHUNK_DIM_Y - 1)) && (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X, Y + 1, Z)) &&
                ((BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y+1, Z) + 2) <= LightNode.LightLevel))
            {
                BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y+1, Z) = 
                                        ((LightNode.LightLevel - 1) >= 0) ? LightNode.LightLevel - 1 : 0;

                light_node NewLightNode(BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y + 1, Z), X, Y + 1, Z, Chunk);
                Enqueue(&Chunk->SunlightBFSQueue, NewLightNode);
            }
        }

        SetFlags(Chunk, CHUNK_LIGHT_PROP_STARTED);
    }
}

static void
FinishPropagateLightChunks(world* World)
{
    for(uint32_t ChunkIndex = 0;
        ChunkIndex < World->ChunksToFinishPropagateLightCount;
        ChunkIndex++)
    {
        chunk *Chunk = World->ChunksToFinishPropagateLight[ChunkIndex];
        Assert(!CheckFlag(Chunk, CHUNK_LIGHT_PROP_FINISHED));

        while(Chunk->SunlightBFSQueueFromOtherChunks.FirstIndex != Chunk->SunlightBFSQueueFromOtherChunks.LightNodes.EntriesCount)
        {
            light_node LightNode = Dequeue(&Chunk->SunlightBFSQueueFromOtherChunks);
            uint32_t X = LightNode.X;
            uint32_t Y = LightNode.Y;
            uint32_t Z = LightNode.Z;

            if((LightNode.LightLevel >= BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y, Z)) &&
               !IsBlockSolid(Chunk->BlocksInfo->Blocks, X, Y, Z))
            {
                BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y, Z) = LightNode.LightLevel;

                if((X != 0) && (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X - 1, Y, Z)) &&
                   ((BlockLightLevel(Chunk->BlocksInfo->Blocks, X - 1, Y, Z) + 2) <= LightNode.LightLevel))
                {
                    BlockLightLevel(Chunk->BlocksInfo->Blocks, X-1, Y, Z) = 
                                        ((LightNode.LightLevel - 1) >= 0) ? LightNode.LightLevel - 1 : 0;

                    light_node NewLightNode(BlockLightLevel(Chunk->BlocksInfo->Blocks, X-1, Y, Z), X - 1, Y, Z, Chunk);
                    Enqueue(&Chunk->SunlightBFSQueueFromOtherChunks, NewLightNode);
                }

                if((X != (CHUNK_DIM_X - 1)) && (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X + 1, Y, Z)) &&
                   ((BlockLightLevel(Chunk->BlocksInfo->Blocks, X + 1, Y, Z) + 2) <= LightNode.LightLevel))
                {
                    BlockLightLevel(Chunk->BlocksInfo->Blocks, X+1, Y, Z) = 
                                        ((LightNode.LightLevel - 1) >= 0) ? LightNode.LightLevel - 1 : 0;

                    light_node NewLightNode(BlockLightLevel(Chunk->BlocksInfo->Blocks, X+1, Y, Z), X + 1, Y, Z, Chunk);
                    Enqueue(&Chunk->SunlightBFSQueueFromOtherChunks, NewLightNode);
                }

                if((Z != 0) && (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X, Y, Z - 1)) &&
                   ((BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y, Z - 1) + 2) <= LightNode.LightLevel))
                {
                    BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y, Z - 1) = 
                                        ((LightNode.LightLevel - 1) >= 0) ? LightNode.LightLevel - 1 : 0;

                    light_node NewLightNode(BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y, Z-1), X, Y, Z - 1, Chunk);
                    Enqueue(&Chunk->SunlightBFSQueueFromOtherChunks, NewLightNode);
                }

                if((Z != (CHUNK_DIM_Z - 1)) && (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X, Y, Z + 1)) &&
                   ((BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y, Z + 1) + 2) <= LightNode.LightLevel))
                {
                    BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y, Z + 1) = 
                                        ((LightNode.LightLevel - 1) >= 0) ? LightNode.LightLevel - 1 : 0;

                    light_node NewLightNode(BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y, Z+1), X, Y, Z + 1, Chunk);
                    Enqueue(&Chunk->SunlightBFSQueueFromOtherChunks, NewLightNode);
                }

                if((Y != 0) && (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X, Y - 1, Z)) &&
                   ((BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y-1, Z) + 2) <= LightNode.LightLevel))
                {
                    int8_t NewLightLevel = 15;
                    if(LightNode.LightLevel < 15)
                    {
                        NewLightLevel = ((LightNode.LightLevel - 1) >= 0) ? LightNode.LightLevel - 1 : 0;
                    }

                    BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y-1, Z) = NewLightLevel;

                    light_node NewLightNode(BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y - 1, Z), X, Y - 1, Z, Chunk);
                    Enqueue(&Chunk->SunlightBFSQueue, NewLightNode);
                }

                if((Y != (CHUNK_DIM_Y - 1)) && (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X, Y + 1, Z)) &&
                    ((BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y+1, Z) + 2) <= LightNode.LightLevel))
                {
                    BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y+1, Z) = 
                                            ((LightNode.LightLevel - 1) >= 0) ? LightNode.LightLevel - 1 : 0;

                    light_node NewLightNode(BlockLightLevel(Chunk->BlocksInfo->Blocks, X, Y + 1, Z), X, Y + 1, Z, Chunk);
                    Enqueue(&Chunk->SunlightBFSQueue, NewLightNode);
                }
            }
        }

		SetFlags(Chunk, CHUNK_LIGHT_PROP_FINISHED);
    }
}

inline float
VertexAO(int8_t Side0, int8_t Side1, int8_t Corner)
{
	float AOValues[] = { 1.0f, 0.8f, 0.6f, 0.15f };
	float Result;
	if ((Side0 + Side1) == 2)
	{
		Result = AOValues[3];
	}
	else
	{
		Result = AOValues[Side0 + Side1 + Corner];
	}

	return(Result);
}

static void
PushVoxelFaceVertices(chunk *Chunk, vec4 &A, vec4 &B, vec4 &C, vec4 &D, 
                      vec4 &AColor, vec4 &BColor, vec4 &CColor, vec4 &DColor, bool Order)
{
    if(Order)
    {
        PushEntry(&Chunk->Vertices, A); PushEntry(&Chunk->Vertices, AColor);
        PushEntry(&Chunk->Vertices, B); PushEntry(&Chunk->Vertices, BColor);
        PushEntry(&Chunk->Vertices, C); PushEntry(&Chunk->Vertices, CColor);
        PushEntry(&Chunk->Vertices, C); PushEntry(&Chunk->Vertices, CColor);
        PushEntry(&Chunk->Vertices, B); PushEntry(&Chunk->Vertices, BColor);
        PushEntry(&Chunk->Vertices, D); PushEntry(&Chunk->Vertices, DColor);
    }
    else
    {
        PushEntry(&Chunk->Vertices, B); PushEntry(&Chunk->Vertices, BColor);
        PushEntry(&Chunk->Vertices, D); PushEntry(&Chunk->Vertices, DColor);
        PushEntry(&Chunk->Vertices, A); PushEntry(&Chunk->Vertices, AColor);
        PushEntry(&Chunk->Vertices, A); PushEntry(&Chunk->Vertices, AColor);
        PushEntry(&Chunk->Vertices, D); PushEntry(&Chunk->Vertices, DColor);
        PushEntry(&Chunk->Vertices, C); PushEntry(&Chunk->Vertices, CColor);
    }
}

static void
SetupChunks(world *World)
{
    for(uint32_t ChunkIndex = 0;
        ChunkIndex < World->ChunksToSetupCount;
        ChunkIndex++)
    {
        chunk *Chunk = World->ChunksToSetup[ChunkIndex];
        Assert(!CheckFlag(Chunk, CHUNK_SETUP));

        InitializeDynamicArray(&Chunk->Vertices); 
        for(uint32_t Z = 0; Z < CHUNK_DIM_Z; Z++)
        {
            for(uint32_t X = 0; X < CHUNK_DIM_X; X++)
            {
                for(uint32_t Y = 0; Y < CHUNK_DIM_Y; Y++)
                {
                    if(IsBlockSolid(Chunk->BlocksInfo->Blocks, X, Y, Z))
                    {
                        float BlockDimInMeters = World->BlockDimInMeters;
                        float XPos = X*BlockDimInMeters;
                        float YPos = Y*BlockDimInMeters;
                        float ZPos = Z*BlockDimInMeters;

                        vec3 Color = BlockColor(Chunk->BlocksInfo->Colors, X, Y, Z);

                        if((X == 0) || (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X - 1, Y, Z)))
                        {
                            int8_t Side0, Side1, Corner;
                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z-1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z-1);
                            float AAO = VertexAO(Side0, Side1, Corner);
                            vec4 A = vec4(XPos, YPos, ZPos, AAO);

                            uint32_t ACount = (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z) ? 0 : 1);
                            int8_t ALightLevel = GetLightLevelBetweenChunks(World, Chunk, X-1, Y-1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y-1, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y, Z);
                            float ALightIntensity = ALightLevel / (ACount*15.0f);
                            vec4 AColor = vec4(Color, ALightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z+1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z+1);
                            float BAO = VertexAO(Side0, Side1, Corner);
                            vec4 B = vec4(XPos, YPos, ZPos + BlockDimInMeters, BAO);

                            uint32_t BCount = (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z+1) ? 0 : 1);
                            int8_t BLightLevel = GetLightLevelBetweenChunks(World, Chunk, X-1, Y-1, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y-1, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y, Z+1);
                            float BLightIntensity = BLightLevel / (BCount*15.0f);
                            vec4 BColor = vec4(Color, BLightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z-1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z-1);
                            float CAO = VertexAO(Side0, Side1, Corner);
                            vec4 C = vec4(XPos, YPos + BlockDimInMeters, ZPos, CAO);

                            uint32_t CCount = (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z) ? 0 : 1);
                            int8_t CLightLevel = GetLightLevelBetweenChunks(World, Chunk, X-1, Y, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y+1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y+1, Z);
                            float CLightIntensity = CLightLevel / (CCount*15.0f);
                            vec4 CColor = vec4(Color, CLightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z+1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z+1);
                            float DAO = VertexAO(Side0, Side1, Corner);
                            vec4 D = vec4(XPos, YPos + BlockDimInMeters, ZPos + BlockDimInMeters, DAO);

                            uint32_t DCount = (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z+1) ? 0 : 1);
                            int8_t DLightLevel = GetLightLevelBetweenChunks(World, Chunk, X-1, Y, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y+1, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y+1, Z+1);
                            float DLightIntensity = DLightLevel / (DCount*15.0f);
                            vec4 DColor = vec4(Color, DLightIntensity);

                            PushVoxelFaceVertices(Chunk, A, B, C, D, AColor, BColor, CColor, DColor, ((CAO+BAO) >= (AAO + DAO)));
                        }

                        if((X == (CHUNK_DIM_X - 1)) || (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X + 1, Y, Z)))
                        {
                            int8_t Side0, Side1, Corner;
                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z-1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z-1);
                            float AAO = VertexAO(Side0, Side1, Corner);
                            vec4 A = vec4(XPos + BlockDimInMeters, YPos, ZPos, AAO);
                            
                            uint32_t ACount = (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z) ? 0 : 1);
                            int8_t ALightLevel = GetLightLevelBetweenChunks(World, Chunk, X+1, Y-1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y-1, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y, Z);
                            float ALightIntensity = ALightLevel / (ACount*15.0f);
                            vec4 AColor = vec4(Color, ALightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z-1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z-1);
                            float BAO = VertexAO(Side0, Side1, Corner);
                            vec4 B = vec4(XPos + BlockDimInMeters, YPos + BlockDimInMeters, ZPos, BAO);

                            uint32_t BCount = (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z) ? 0 : 1);
                            int8_t BLightLevel = GetLightLevelBetweenChunks(World, Chunk, X+1, Y, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y+1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y+1, Z);
                            float BLightIntensity = BLightLevel / (BCount*15.0f);
                            vec4 BColor = vec4(Color, BLightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z+1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z+1);
                            float CAO = VertexAO(Side0, Side1, Corner);
                            vec4 C = vec4(XPos + BlockDimInMeters, YPos, ZPos + BlockDimInMeters, CAO);

                            uint32_t CCount = (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z+1) ? 0 : 1);
                            int8_t CLightLevel = GetLightLevelBetweenChunks(World, Chunk, X+1, Y-1, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y-1, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y, Z+1);
                            float CLightIntensity = CLightLevel / (CCount*15.0f);
                            vec4 CColor = vec4(Color, CLightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z+1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z+1);
                            float DAO = VertexAO(Side0, Side1, Corner);
                            vec4 D = vec4(XPos + BlockDimInMeters, YPos + BlockDimInMeters, ZPos + BlockDimInMeters, DAO);

                            uint32_t DCount = (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z+1) ? 0 : 1);
                            int8_t DLightLevel = GetLightLevelBetweenChunks(World, Chunk, X+1, Y, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y+1, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y+1, Z+1);
                            float DLightIntensity = DLightLevel / (DCount*15.0f);
                            vec4 DColor = vec4(Color, DLightIntensity);

                            PushVoxelFaceVertices(Chunk, A, B, C, D, AColor, BColor, CColor, DColor, ((CAO+BAO) >= (AAO + DAO)));
                        }

                        if((Y == 0) || (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X, Y - 1, Z)))
                        {
                            int8_t Side0, Side1, Corner;
                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z-1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z-1);
                            float AAO = VertexAO(Side0, Side1, Corner);
                            vec4 A = vec4(XPos, YPos, ZPos, AAO);

                            uint32_t ACount = (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z) ? 0 : 1);
                            int8_t ALightLevel = GetLightLevelBetweenChunks(World, Chunk, X-1, Y-1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y-1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y-1, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y-1, Z);
                            float ALightIntensity = ALightLevel / (ACount*15.0f);
                            vec4 AColor = vec4(Color, ALightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z-1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z-1);
                            float BAO = VertexAO(Side0, Side1, Corner);
                            vec4 B = vec4(XPos + BlockDimInMeters, YPos, ZPos, BAO);

                            uint32_t BCount = (IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z) ? 0 : 1);
                            int8_t BLightLevel = GetLightLevelBetweenChunks(World, Chunk, X, Y-1, Z-1) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X+1, Y-1, Z-1) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X, Y-1, Z) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X+1, Y-1, Z);
                            float BLightIntensity = BLightLevel / (BCount*15.0f);
                            vec4 BColor = vec4(Color, BLightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z+1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z+1);
                            float CAO = VertexAO(Side0, Side1, Corner);
                            vec4 C = vec4(XPos, YPos, ZPos + BlockDimInMeters, CAO);

                            uint32_t CCount = (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z+1) ? 0 : 1);
                            int8_t CLightLevel = GetLightLevelBetweenChunks(World, Chunk, X-1, Y-1, Z) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X, Y-1, Z) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X-1, Y-1, Z+1) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X, Y-1, Z+1);
                            float CLightIntensity = CLightLevel / (CCount*15.0f);
                            vec4 CColor = vec4(Color, CLightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z+1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z+1);
                            float DAO = VertexAO(Side0, Side1, Corner);
                            vec4 D = vec4(XPos + BlockDimInMeters, YPos, ZPos + BlockDimInMeters, DAO);

                            uint32_t DCount = (IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z+1) ? 0 : 1);
                            int8_t DLightLevel = GetLightLevelBetweenChunks(World, Chunk, X, Y-1, Z) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X+1, Y-1, Z) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X, Y-1, Z+1) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X+1, Y-1, Z+1);
                            float DLightIntensity = DLightLevel / (DCount*15.0f);
                            vec4 DColor = vec4(Color, DLightIntensity);

                            PushVoxelFaceVertices(Chunk, A, B, C, D, AColor, BColor, CColor, DColor, ((CAO+BAO) >= (AAO + DAO)));
                        }

                        if((Y == (CHUNK_DIM_Y - 1)) || (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X, Y + 1, Z)))
                        {
                            int8_t Side0, Side1, Corner;
                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z-1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z-1);
                            float AAO = VertexAO(Side0, Side1, Corner);
                            vec4 A = vec4(XPos, YPos + BlockDimInMeters, ZPos, AAO);

                            uint32_t ACount = (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z) ? 0 : 1);
                            int8_t ALightLevel = GetLightLevelBetweenChunks(World, Chunk, X-1, Y+1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y+1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y+1, Z) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y+1, Z);
                            float ALightIntensity = ALightLevel / (ACount*15.0f);
                            vec4 AColor = vec4(Color, ALightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z+1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z+1);
                            float BAO = VertexAO(Side0, Side1, Corner);
                            vec4 B = vec4(XPos, YPos + BlockDimInMeters, ZPos + BlockDimInMeters, BAO);

                            uint32_t BCount = (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z+1) ? 0 : 1);
                            int8_t BLightLevel = GetLightLevelBetweenChunks(World, Chunk, X-1, Y+1, Z) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X, Y+1, Z) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X-1, Y+1, Z+1) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X, Y+1, Z+1);
                            float BLightIntensity = BLightLevel / (BCount*15.0f);
                            vec4 BColor = vec4(Color, BLightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z-1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z-1);
                            float CAO = VertexAO(Side0, Side1, Corner);
                            vec4 C = vec4(XPos + BlockDimInMeters, YPos + BlockDimInMeters, ZPos, CAO);

                            uint32_t CCount = (IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z) ? 0 : 1);
                            int8_t CLightLevel = GetLightLevelBetweenChunks(World, Chunk, X, Y+1, Z-1) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X+1, Y+1, Z-1) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X, Y+1, Z) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X+1, Y+1, Z);
                            float CLightIntensity = CLightLevel / (CCount*15.0f);
                            vec4 CColor = vec4(Color, CLightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z+1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z+1);
                            float DAO = VertexAO(Side0, Side1, Corner);
                            vec4 D = vec4(XPos + BlockDimInMeters, YPos + BlockDimInMeters, ZPos + BlockDimInMeters, DAO);

                            uint32_t DCount = (IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z+1) ? 0 : 1);
                            int8_t DLightLevel = GetLightLevelBetweenChunks(World, Chunk, X, Y+1, Z) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X+1, Y+1, Z) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X, Y+1, Z+1) +
                                                    GetLightLevelBetweenChunks(World, Chunk, X+1, Y+1, Z+1);
                            float DLightIntensity = DLightLevel / (DCount*15.0f);
                            vec4 DColor = vec4(Color, DLightIntensity);

                            PushVoxelFaceVertices(Chunk, A, B, C, D, AColor, BColor, CColor, DColor, ((CAO+BAO) >= (AAO + DAO)));
                        }

                        if((Z == 0) || (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X, Y, Z - 1)))
                        {
                            int8_t Side0, Side1, Corner;
                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z-1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z-1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z-1);
                            float AAO = VertexAO(Side0, Side1, Corner);
                            vec4 A = vec4(XPos, YPos, ZPos, AAO);
                            
                            uint32_t ACount = (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y, Z-1) ? 0 : 1);
                            int8_t ALightLevel = GetLightLevelBetweenChunks(World, Chunk, X-1, Y-1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y-1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y, Z-1);
                            float ALightIntensity = ALightLevel / (ACount*15.0f);
                            vec4 AColor = vec4(Color, ALightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z-1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z-1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z-1);
                            float BAO = VertexAO(Side0, Side1, Corner);
                            vec4 B = vec4(XPos, YPos + BlockDimInMeters, ZPos, BAO);
                            
                            uint32_t BCount = (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z-1) ? 0 : 1);
                            int8_t BLightLevel = GetLightLevelBetweenChunks(World, Chunk, X-1, Y, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y+1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y+1, Z-1);
                            float BLightIntensity = BLightLevel / (BCount*15.0f);
                            vec4 BColor = vec4(Color, BLightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z-1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z-1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z-1);
                            float CAO = VertexAO(Side0, Side1, Corner);
                            vec4 C = vec4(XPos + BlockDimInMeters, YPos, ZPos, CAO);
                                                        
                            uint32_t CCount = (IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z-1) ? 0 : 1);
                            int8_t CLightLevel = GetLightLevelBetweenChunks(World, Chunk, X, Y-1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y-1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y, Z-1);
                            float CLightIntensity = CLightLevel / (CCount*15.0f);
                            vec4 CColor = vec4(Color, CLightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z-1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z-1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z-1);
                            float DAO = VertexAO(Side0, Side1, Corner);
                            vec4 D = vec4(XPos + BlockDimInMeters, YPos + BlockDimInMeters, ZPos, DAO);

                            uint32_t DCount = (IsBlockSolidBetweenChunks(World, Chunk, X, Y, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z-1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z-1) ? 0 : 1);
                            int8_t DLightLevel = GetLightLevelBetweenChunks(World, Chunk, X, Y, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y+1, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y, Z-1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y+1, Z-1);
                            float DLightIntensity = DLightLevel / (DCount*15.0f);
                            vec4 DColor = vec4(Color, DLightIntensity);

                            PushVoxelFaceVertices(Chunk, A, B, C, D, AColor, BColor, CColor, DColor, ((CAO+BAO) >= (AAO + DAO)));
                        }

                        if((Z == (CHUNK_DIM_Z - 1)) || (!IsBlockSolid(Chunk->BlocksInfo->Blocks, X, Y, Z + 1)))
                        {
                            int8_t Side0, Side1, Corner;
                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z+1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z+1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z+1);
                            float AAO = VertexAO(Side0, Side1, Corner);
                            vec4 A = vec4(XPos, YPos, ZPos + BlockDimInMeters, AAO);
                            
                            uint32_t ACount = (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y-1, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y, Z+1) ? 0 : 1);
                            int8_t ALightLevel = GetLightLevelBetweenChunks(World, Chunk, X-1, Y-1, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y-1, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y, Z+1);
                            float ALightIntensity = ALightLevel / (ACount*15.0f);
                            vec4 AColor = vec4(Color, ALightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z+1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z+1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z+1);
                            float BAO = VertexAO(Side0, Side1, Corner);
                            vec4 B = vec4(XPos + BlockDimInMeters, YPos, ZPos + BlockDimInMeters, BAO);

                            uint32_t BCount = (IsBlockSolidBetweenChunks(World, Chunk, X, Y-1, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y-1, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z+1) ? 0 : 1);
                            int8_t BLightLevel = GetLightLevelBetweenChunks(World, Chunk, X, Y-1, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y-1, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y, Z+1);
                            float BLightIntensity = BLightLevel / (BCount*15.0f);
                            vec4 BColor = vec4(Color, BLightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z+1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z+1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z+1);
                            float CAO = VertexAO(Side0, Side1, Corner);
                            vec4 C = vec4(XPos, YPos + BlockDimInMeters, ZPos + BlockDimInMeters, CAO);
                            
                            uint32_t CCount = (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X-1, Y+1, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z+1) ? 0 : 1);
                            int8_t CLightLevel = GetLightLevelBetweenChunks(World, Chunk, X-1, Y, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X-1, Y+1, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y+1, Z+1);
                            float CLightIntensity = CLightLevel / (CCount*15.0f);
                            vec4 CColor = vec4(Color, CLightIntensity);

                            Side0 = IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z+1);
                            Side1 = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z+1);
                            Corner = IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z+1);
                            float DAO = VertexAO(Side0, Side1, Corner);
                            vec4 D = vec4(XPos + BlockDimInMeters, YPos + BlockDimInMeters, ZPos + BlockDimInMeters, DAO);

                            uint32_t DCount = (IsBlockSolidBetweenChunks(World, Chunk, X, Y, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X, Y+1, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y, Z+1) ? 0 : 1) +
                                              (IsBlockSolidBetweenChunks(World, Chunk, X+1, Y+1, Z+1) ? 0 : 1);
                            int8_t DLightLevel = GetLightLevelBetweenChunks(World, Chunk, X, Y, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X, Y+1, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y, Z+1) +
                                                 GetLightLevelBetweenChunks(World, Chunk, X+1, Y+1, Z+1);
                            float DLightIntensity = DLightLevel / (DCount*15.0f);
                            vec4 DColor = vec4(Color, DLightIntensity);

                            PushVoxelFaceVertices(Chunk, A, B, C, D, AColor, BColor, CColor, DColor, ((CAO+BAO) >= (AAO + DAO)));
                        }
                    }
                }
            }
        }

        SetFlags(Chunk, CHUNK_SETUP);
    }
}

static void
LoadChunks(world *World)
{
    for(uint32_t ChunkIndex = 0;
        ChunkIndex < World->ChunksToLoadCount;
        ChunkIndex++)
    {
        chunk *Chunk = World->ChunksToLoad[ChunkIndex];
        Assert(!CheckFlag(Chunk, CHUNK_LOADED));

        glGenVertexArrays(1, &Chunk->VAO);
        glGenBuffers(1, &Chunk->VBO);
        glBindVertexArray(Chunk->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, Chunk->VBO);
        glBufferData(GL_ARRAY_BUFFER, Chunk->Vertices.EntriesCount*sizeof(vec4), Chunk->Vertices.Entries, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 2*sizeof(vec4), (void*)0);
        glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 2*sizeof(vec4), (void*)(sizeof(vec4)));

        SetFlags(Chunk, CHUNK_LOADED);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void
UnloadChunks(world *World, world_position *MinChunkP, world_position *MaxChunkP)
{
    const uint32_t MaxChunksToFreeCount = 8;
    uint32_t ChunksFreed = 0;
    for(uint32_t ChunkBucket = 0;
        (ChunkBucket < ArrayCount(World->Chunks)) && (ChunksFreed < MaxChunksToFreeCount);
        ChunkBucket++)
    {
        chunk *Chunk = &World->Chunks[ChunkBucket];
        while(Chunk && (ChunksFreed < MaxChunksToFreeCount))
        {
            if(CheckFlag(Chunk, CHUNK_RECENTLY_USED))
            {
                if((Chunk->X < (MinChunkP->ChunkX - 4)) ||
                   (Chunk->Z < (MinChunkP->ChunkZ - 4)) ||
                   (Chunk->X > (MaxChunkP->ChunkX + 4)) ||
                   (Chunk->Z > (MaxChunkP->ChunkZ + 4)))
                {
                    ++ChunksFreed;
                    --World->RecentlyUsedChunksCount;
                    ClearFlags(Chunk, CHUNK_RECENTLY_USED);
                
                    if(CheckFlag(Chunk, CHUNK_GENERATED))
                    {
                        Chunk->BlocksInfo->NextFree = World->FirstFreeChunkBlocksInfo;
                        World->FirstFreeChunkBlocksInfo = Chunk->BlocksInfo;

                        ClearFlags(Chunk, CHUNK_GENERATED|CHUNK_TREES_PLACED);
                    }

                    if(CheckFlag(Chunk, CHUNK_LIGHT_PROP_STARTED))
                    {
						Chunk->SunlightBFSQueue.FirstIndex = 0;
                        FreeDynamicArray(&Chunk->SunlightBFSQueue.LightNodes);
                        ClearFlags(Chunk, CHUNK_LIGHT_PROP_STARTED);
                    }

                    Chunk->SunlightBFSQueueFromOtherChunks.FirstIndex = 0;
                    FreeDynamicArray(&Chunk->SunlightBFSQueueFromOtherChunks.LightNodes);
                    ClearFlags(Chunk, CHUNK_LIGHT_PROP_FINISHED);

                    if(CheckFlag(Chunk, CHUNK_SETUP))
                    {
                        FreeDynamicArray(&Chunk->Vertices);
                        ClearFlags(Chunk, CHUNK_SETUP);
                    }

                    if(CheckFlag(Chunk, CHUNK_LOADED))
                    {
                        glDeleteBuffers(1, &Chunk->VBO);
                        glDeleteVertexArrays(1, &Chunk->VAO);
                        Chunk->VAO = Chunk->VBO = 0;

                        ClearFlags(Chunk, CHUNK_LOADED);
                    }
                }
            }

            Chunk = Chunk->NextInHash;
        }
    }
}

static bool
ChunkFrustumCulling(mat4 &MVP, vec3 ChunkDim)
{
    vec4 ClipSpacePoints[8];

    ClipSpacePoints[0] = MVP * vec4(0.0f, 0.0f, 0.0f, 1.0f);
    ClipSpacePoints[1] = MVP * vec4(0.0f, ChunkDim.y, 0.0f, 1.0f);
    ClipSpacePoints[2] = MVP * vec4(0.0f, 0.0f, ChunkDim.z, 1.0f);
    ClipSpacePoints[3] = MVP * vec4(0.0f, ChunkDim.y, ChunkDim.z, 1.0f);
    ClipSpacePoints[4] = MVP * vec4(ChunkDim.x, 0.0f, 0.0f, 1.0f);
    ClipSpacePoints[5] = MVP * vec4(ChunkDim.x, ChunkDim.y, 0.0f, 1.0f);
    ClipSpacePoints[6] = MVP * vec4(ChunkDim.x, 0.0f, ChunkDim.z, 1.0f);
    ClipSpacePoints[7] = MVP * vec4(ChunkDim.x, ChunkDim.y, ChunkDim.z, 1.0f);

    if((ClipSpacePoints[0].x > ClipSpacePoints[0].w) && (ClipSpacePoints[1].x > ClipSpacePoints[1].w) &&
       (ClipSpacePoints[2].x > ClipSpacePoints[2].w) && (ClipSpacePoints[3].x > ClipSpacePoints[3].w) &&
       (ClipSpacePoints[4].x > ClipSpacePoints[4].w) && (ClipSpacePoints[5].x > ClipSpacePoints[5].w) &&
       (ClipSpacePoints[6].x > ClipSpacePoints[6].w) && (ClipSpacePoints[7].x > ClipSpacePoints[7].w))
    {
        return(false);
    }

    if((ClipSpacePoints[0].x < -ClipSpacePoints[0].w) && (ClipSpacePoints[1].x < -ClipSpacePoints[1].w) &&
       (ClipSpacePoints[2].x < -ClipSpacePoints[2].w) && (ClipSpacePoints[3].x < -ClipSpacePoints[3].w) &&
       (ClipSpacePoints[4].x < -ClipSpacePoints[4].w) && (ClipSpacePoints[5].x < -ClipSpacePoints[5].w) &&
       (ClipSpacePoints[6].x < -ClipSpacePoints[6].w) && (ClipSpacePoints[7].x < -ClipSpacePoints[7].w))
    {
        return(false);
    }

    if((ClipSpacePoints[0].y > ClipSpacePoints[0].w) && (ClipSpacePoints[1].y > ClipSpacePoints[1].w) &&
       (ClipSpacePoints[2].y > ClipSpacePoints[2].w) && (ClipSpacePoints[3].y > ClipSpacePoints[3].w) &&
       (ClipSpacePoints[4].y > ClipSpacePoints[4].w) && (ClipSpacePoints[5].y > ClipSpacePoints[5].w) &&
       (ClipSpacePoints[6].y > ClipSpacePoints[6].w) && (ClipSpacePoints[7].y > ClipSpacePoints[7].w))
    {
        return(false);
    }

    if((ClipSpacePoints[0].y < -ClipSpacePoints[0].w) && (ClipSpacePoints[1].y < -ClipSpacePoints[1].w) &&
       (ClipSpacePoints[2].y < -ClipSpacePoints[2].w) && (ClipSpacePoints[3].y < -ClipSpacePoints[3].w) &&
       (ClipSpacePoints[4].y < -ClipSpacePoints[4].w) && (ClipSpacePoints[5].y < -ClipSpacePoints[5].w) &&
       (ClipSpacePoints[6].y < -ClipSpacePoints[6].w) && (ClipSpacePoints[7].y < -ClipSpacePoints[7].w))
    {
        return(false);
    }

    if((ClipSpacePoints[0].z > ClipSpacePoints[0].w) && (ClipSpacePoints[1].z > ClipSpacePoints[1].w) &&
       (ClipSpacePoints[2].z > ClipSpacePoints[2].w) && (ClipSpacePoints[3].z > ClipSpacePoints[3].w) &&
       (ClipSpacePoints[4].z > ClipSpacePoints[4].w) && (ClipSpacePoints[5].z > ClipSpacePoints[5].w) &&
       (ClipSpacePoints[6].z > ClipSpacePoints[6].w) && (ClipSpacePoints[7].z > ClipSpacePoints[7].w))
    {
        return(false);
    }

    if((ClipSpacePoints[0].z < -ClipSpacePoints[0].w) && (ClipSpacePoints[1].z < -ClipSpacePoints[1].w) &&
       (ClipSpacePoints[2].z < -ClipSpacePoints[2].w) && (ClipSpacePoints[3].z < -ClipSpacePoints[3].w) &&
       (ClipSpacePoints[4].z < -ClipSpacePoints[4].w) && (ClipSpacePoints[5].z < -ClipSpacePoints[5].w) &&
       (ClipSpacePoints[6].z < -ClipSpacePoints[6].w) && (ClipSpacePoints[7].z < -ClipSpacePoints[7].w))
    {
        return(false);
    }

    return(true);
}

static void
MergeSort(uint32_t Count, chunk **Chunks, chunk **TempStorage)
{
    if((Count == 1) || (Count == 0))
    {
        return;
    }
    else if(Count == 2)
    {
        if(LengthSq(Chunks[0]->SimP) > LengthSq(Chunks[1]->SimP))
        {
            chunk *Temp = Chunks[0];
            Chunks[0] = Chunks[1];
            Chunks[1] = Temp;
        }
    }
    else
    {
        uint32_t CountHalf0 = Count / 2;
        uint32_t CountHalf1 = Count - CountHalf0;

        chunk **Half0 = Chunks;
        chunk **Half1 = Chunks + CountHalf0; 
        MergeSort(CountHalf0, Half0, TempStorage);
        MergeSort(CountHalf1, Half1, TempStorage);

        chunk **ReadHalf0 = Half0;
        chunk **ReadHalf1 = Half1;
        chunk **End = Chunks + Count;

        chunk **Out = TempStorage;
        for(uint32_t I = 0; I < Count; I++)
        {
            if(ReadHalf0 == Half1)
            {
                *Out++ = *ReadHalf1++;
            }
            else if(ReadHalf1 == End)
            {
                *Out++ = *ReadHalf0++;
            }
            else if(LengthSq((*ReadHalf0)->SimP) < LengthSq((*ReadHalf1)->SimP))
            {
                *Out++ = *ReadHalf0++;
            }
            else 
            {
                *Out++ = *ReadHalf1++;
            }
        }

        for(uint32_t I = 0; I < Count; I++)
        {
            Chunks[I] = TempStorage[I];
        }
    }
}

static void
RenderChunks(world *World, shader Shader, mat4 VP, stack_allocator *WorldAllocator)
{
    stack_temp_memory TempMemory = BeginTempMemory(WorldAllocator);
    chunk **TempStorage = PushArray(WorldAllocator, World->ChunksToRenderCount, chunk *);
    MergeSort(World->ChunksToRenderCount, World->ChunksToRender, TempStorage);
    EndTempMemory(TempMemory);

    for(uint32_t ChunkIndex = 0;
        ChunkIndex < World->ChunksToRenderCount;
        ChunkIndex++)
    {
        chunk *Chunk = World->ChunksToRender[ChunkIndex];

        mat4 ChunkTranslationMatrix = Translation(Chunk->SimP);
        mat4 MVP = VP * ChunkTranslationMatrix;

        if(ChunkFrustumCulling(MVP, World->ChunkDimInMeters))
        {
            Shader.SetMat4("Model", ChunkTranslationMatrix);
            glBindVertexArray(Chunk->VAO);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)Chunk->Vertices.EntriesCount);
        }
    }

    glBindVertexArray(0);
}

static bool
CheckAllNeighbourChunks(world *World, chunk *Chunk, chunk_flag Flag)
{
    chunk *Left = GetChunk(World, Chunk->X - 1, Chunk->Z);
    chunk *Right = GetChunk(World, Chunk->X + 1, Chunk->Z);
    chunk *Bottom = GetChunk(World, Chunk->X, Chunk->Z + 1);
    chunk *Top = GetChunk(World, Chunk->X, Chunk->Z - 1);

    chunk *LeftTop = GetChunk(World, Chunk->X - 1, Chunk->Z - 1);
    chunk *RightTop = GetChunk(World, Chunk->X + 1, Chunk->Z - 1);
    chunk *LeftBottom = GetChunk(World, Chunk->X - 1, Chunk->Z + 1);
    chunk *RightBottom = GetChunk(World, Chunk->X + 1, Chunk->Z + 1);

    bool Result = (Left ? CheckFlag(Left, Flag) : 0) && (Right ? CheckFlag(Right, Flag) : 0) &&
                  (Bottom ? CheckFlag(Bottom, Flag) : 0) && (Top ? CheckFlag(Top, Flag) : 0) &&
                  (LeftTop ? CheckFlag(LeftTop, Flag) : 0) && (RightTop ? CheckFlag(RightTop, Flag) : 0) &&
                  (LeftBottom ? CheckFlag(LeftBottom, Flag) : 0) && (RightBottom ? CheckFlag(RightBottom, Flag) : 0);
    
    return(Result);
}

// TODO(georgy): Maybe use multithreading here? This function eats some tons of time I believe
static void
BeginSimulation(world *World, world_position *Origin, vec2 Bounds, stack_allocator *WorldAllocator)
{
    world_position MinChunkP = MapIntoChunkSpace(World, Origin, -vec3(Bounds.x, 0.0f, Bounds.y));
    world_position MaxChunkP = MapIntoChunkSpace(World, Origin, vec3(Bounds.x, 0.0f, Bounds.y));

    for(int32_t ChunkZ = MinChunkP.ChunkZ - 1;
        ChunkZ <= MaxChunkP.ChunkZ + 1;
        ChunkZ++)
    {
        for(int32_t ChunkX = MinChunkP.ChunkX - 1;
            ChunkX <= MaxChunkP.ChunkX + 1;
            ChunkX++)
        {
            chunk *Chunk = GetChunk(World, ChunkX, ChunkZ, WorldAllocator);
            
            if(Chunk)
            {
                if(!CheckFlag(Chunk, CHUNK_RECENTLY_USED))
                {
                    ++World->RecentlyUsedChunksCount;
                    SetFlags(Chunk, CHUNK_RECENTLY_USED);
                }

                if(!CheckFlag(Chunk, CHUNK_GENERATED))
                {
                    if(World->ChunksToGenerateCount < ArrayCount(World->ChunksToGenerate))
                    {
                        World->ChunksToGenerate[World->ChunksToGenerateCount++] = Chunk;
                    }
                }
                else if((ChunkX > (MinChunkP.ChunkX - 1)) && (ChunkX < (MaxChunkP.ChunkX + 1)) &&
                        (ChunkZ > (MinChunkP.ChunkZ - 1)) && (ChunkZ < (MaxChunkP.ChunkZ + 1)))
                {
                    if(!CheckFlag(Chunk, CHUNK_TREES_PLACED))
                    {
                        if(CheckAllNeighbourChunks(World, Chunk, CHUNK_GENERATED))
                        {
                            if(World->ChunksToPlaceTreesCount < ArrayCount(World->ChunksToPlaceTrees))
                            {
                                World->ChunksToPlaceTrees[World->ChunksToPlaceTreesCount++] = Chunk;
                            }
                        }
                    }
                    else if(!CheckFlag(Chunk, CHUNK_LIGHT_PROP_STARTED))
                    {
                        if(CheckAllNeighbourChunks(World, Chunk, CHUNK_TREES_PLACED))
                        {
                            if(World->ChunksToPropagateLightCount < ArrayCount(World->ChunksToPropagateLight))
                            {
                                World->ChunksToPropagateLight[World->ChunksToPropagateLightCount++] = Chunk;
                            }
                        }
                    }
                    else if(!CheckFlag(Chunk, CHUNK_LIGHT_PROP_FINISHED))
                    {
                        if(CheckAllNeighbourChunks(World, Chunk, CHUNK_LIGHT_PROP_STARTED))
                        {
                            if(World->ChunksToFinishPropagateLightCount < ArrayCount(World->ChunksToFinishPropagateLight))
                            {
                                World->ChunksToFinishPropagateLight[World->ChunksToFinishPropagateLightCount++] = Chunk;
                            }
                        }
                    }
                    else if(!CheckFlag(Chunk, CHUNK_SETUP))
                    {
                        if(CheckAllNeighbourChunks(World, Chunk, CHUNK_LIGHT_PROP_FINISHED))
                        {
                            if(World->ChunksToSetupCount < ArrayCount(World->ChunksToSetup))
                            {
                                World->ChunksToSetup[World->ChunksToSetupCount++] = Chunk;
                            }
                        }
                    }
                    else if(!CheckFlag(Chunk, CHUNK_LOADED))
                    {
                        if(World->ChunksToLoadCount < ArrayCount(World->ChunksToLoad))
                        {
                            World->ChunksToLoad[World->ChunksToLoadCount++] = Chunk;
                        }
                    }
                    else
                    {
                        Assert(World->ChunksToRenderCount < ArrayCount(World->ChunksToRender));
                        World->ChunksToRender[World->ChunksToRenderCount++] = Chunk;

                        world_position ChunkP = {Chunk->X, Chunk->Z, vec3(0.0f, 0.0f, 0.0f)};
                        Chunk->SimP = Substract(World, &ChunkP, Origin);
                    }
                }
            }
        }
    }

	// TODO(georgy): Need more accurate value for this! Profile! Can be based upon sim region bounds!
#define MAX_CHUNKS_IN_MEMORY 800
    if(World->RecentlyUsedChunksCount > MAX_CHUNKS_IN_MEMORY)
    {
        UnloadChunks(World, &MinChunkP, &MaxChunkP);
    }
}

struct camera
{
    vec3 P;
    vec3 ViewDir;

    float Pitch, Head;

    mat4 GetViewMatrix(void);
};
inline mat4
camera::GetViewMatrix(void)
{
    mat4 Result = LookAt(P, P + ViewDir);
    return(Result);
}

struct game_state
{
    bool IsInitialized;

    camera Camera;
    world_position HeroP;

    shader DefaultShader;

    GLuint CubeVAO, CubeVBO;
};

struct temp_state
{
    bool IsInitialized;

    stack_allocator WorldAllocator;
    world World;
};

static void
GameUpdateAndRender(game_memory *Memory, game_input *Input, uint32_t BufferWidth, uint32_t BufferHeight)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(!GameState->IsInitialized)
    {
        GameState->DefaultShader = shader("shaders\\DefaultVS.glsl", "shaders\\DefaultFS.glsl");

        GameState->Camera = {};
		GameState->Camera.P = vec3(0.0f, 56.0f, 0.0f);

        GameState->IsInitialized = true;
    }

    Assert(sizeof(temp_state) <= Memory->TemporaryStorageSize);
    temp_state *TempState = (temp_state *)Memory->TemporaryStorage;
    if(!TempState->IsInitialized)
    {
        TempState->World.BlockDimInMeters = 0.5f;
        TempState->World.ChunkDimInMeters = vec3(TempState->World.BlockDimInMeters*CHUNK_DIM_X, 
                                                 TempState->World.BlockDimInMeters*CHUNK_DIM_Y, 
                                                 TempState->World.BlockDimInMeters*CHUNK_DIM_Z);
        for(uint32_t HashIndex = 0; 
            HashIndex < ArrayCount(TempState->World.Chunks); 
            HashIndex++)
        {
            TempState->World.Chunks[HashIndex].X = INVALID_CHUNK_P;
        }        

        InitializeStackAllocator(&TempState->WorldAllocator, Memory->TemporaryStorageSize - sizeof(temp_state), 
                                                             (uint8_t *)Memory->TemporaryStorage + sizeof(temp_state));

        TempState->IsInitialized = true;
    }

    camera *Camera = &GameState->Camera;
    world *World = &TempState->World;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Camera->Pitch += -0.1f*Input->MouseDeltaY;
    Camera->Head += -0.1f*Input->MouseDeltaX;
    
    Camera->Pitch = Clamp(Camera->Pitch, -89.0f, 89.0f);

    Camera->ViewDir.x = -Cos(Radians(Camera->Pitch))*Sin(Radians(Camera->Head));
    Camera->ViewDir.y = Sin(Radians(Camera->Pitch));
    Camera->ViewDir.z = -Cos(Radians(Camera->Pitch))*Cos(Radians(Camera->Head));

    vec3 CameraRight = Normalize(Cross(Camera->ViewDir, vec3(0.0f, 1.0f, 0.0f)));

    vec3 Offset = vec3(0.0f, 0.0f, 0.0f);
    if(Input->MoveForward.EndedDown)
    {
        Offset += 40.0f*Input->dt*Camera->ViewDir;
    }
    if(Input->MoveBack.EndedDown)
    {
        Offset -= 40.0f*Input->dt*Camera->ViewDir;
    }
    if(Input->MoveRight.EndedDown)
    {
        Offset += 40.0f*Input->dt*CameraRight;
    }
    if(Input->MoveLeft.EndedDown)
    {
        Offset -= 40.0f*Input->dt*CameraRight;
    }

    BeginSimulation(World, &GameState->HeroP, vec2(100.0f, 100.0f), &TempState->WorldAllocator);
    GenerateChunks(World, &TempState->WorldAllocator);
    PlaceTreesChunks(World);
    PropagateLightChunks(World);
    FinishPropagateLightChunks(World);
    SetupChunks(World);
    LoadChunks(World);

    mat4 PerspectiveProj = Perspective(45.0f, (float)BufferWidth/(float)BufferHeight, 0.1f, 500.0f);
    mat4 View = Camera->GetViewMatrix();

    GameState->DefaultShader.Use();
    GameState->DefaultShader.SetMat4("Projection", PerspectiveProj);
    GameState->DefaultShader.SetMat4("View", View);
    GameState->DefaultShader.SetVec2("Resolution", vec2((float)BufferWidth, (float)BufferHeight));
    RenderChunks(World, GameState->DefaultShader, PerspectiveProj * View, &TempState->WorldAllocator);

    GameState->HeroP = MapIntoChunkSpace(World, &GameState->HeroP, Offset);
    
    ResetWorldWork(World);
}
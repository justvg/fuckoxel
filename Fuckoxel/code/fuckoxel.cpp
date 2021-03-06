#include "fuckoxel_math.h"
#include "fuckoxel_shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static GLuint
LoadTexture(const char *Path)
{
    GLuint TextureID;
    glGenTextures(1, &TextureID);

    int Width, Height, Channels;
    stbi_uc *Data = stbi_load(Path, &Width, &Height, &Channels, 0);
    Assert(Data);
    if(Data)
    {
        GLenum Format;
        if(Channels == 1) Format = GL_RED;
        else if(Channels == 3) Format = GL_RGB;
        else if(Channels == 4) Format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, TextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, Format, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    stbi_image_free(Data);

    return(TextureID);
}

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

inline stack_allocator
SubAlloctor(stack_allocator *Allocator, uint64_t Size)
{
    stack_allocator Result = {};

    Result.Size = Size;
    Result.Base = (uint8_t *)PushSize(Allocator, Size);

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

struct dynamic_array_uint32
{
    uint32_t MaxEntriesCount;
    uint32_t EntriesCount;
    uint32_t *Entries;
};

static void
InitializeDynamicArray(dynamic_array_uint32 *Array, uint32_t MaxEntriesCount = 0)
{
    Array->MaxEntriesCount = MaxEntriesCount;
    Array->EntriesCount = 0;
    Array->Entries = 0;
}

static void
ExpandDynamicArray(dynamic_array_uint32 *Array)
{
    uint32_t NewMaxEntriesCount = Array->MaxEntriesCount ? 2*Array->MaxEntriesCount : 1;
    uint32_t *NewMemory = (uint32_t *)malloc(NewMaxEntriesCount*sizeof(uint32_t));
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
FreeDynamicArray(dynamic_array_uint32 *Array)
{
    free(Array->Entries);
    Array->Entries = 0;
    Array->MaxEntriesCount = 0;
    Array->EntriesCount = 0;
}

static void
PushEntry(dynamic_array_uint32 *Array, uint32_t Entry)
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
    CHUNK_RECENTLY_USED = 0x40,
    CHUNK_MODIFIED = 0x80
};
struct chunk
{
    int32_t X, Z;
    vec3 SimP;

    uint8_t Flags;

    chunk_blocks_info *BlocksInfo;
    dynamic_array_vec4 Vertices;
    dynamic_array_uint32 Indices;
    GLuint VAO, VBO, EBO;

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
    chunk *ChunksToLoad[4];

    // TODO(georgy): Assert that 2048 is enough! (Make this dynamic array?)
    uint32_t ChunksToRenderCount;
    chunk *ChunksToRender[2048];

    uint64_t Lock;
};

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


struct hero
{
    world_position WorldP;

    vec3 P;
    vec3 dP;
    vec3 ddP;

    aabb AABB;

    bool CanJump;
};

struct entity_path_graph_entry
{
    bool Visited;
    entity_path_graph_entry *Parent;
    uint32_t BlockX, BlockY, BlockZ;
};

static entity_path_graph_entry *
GetNodeFromPathGraph(entity_path_graph_entry *Graph, uint32_t Count, int32_t X, int32_t Z)
{
    entity_path_graph_entry *Result = 0;

    if((X >= 0) && (X < CHUNK_DIM_X) &&
        (Z >= 0) && (Z < CHUNK_DIM_Z))
    {
        for(uint32_t NodeIndex = 0; 
            NodeIndex < Count; 
            NodeIndex++)
        {
            entity_path_graph_entry *Node = Graph + NodeIndex;

            if((Node->BlockX == X) &&
            (Node->BlockZ == Z))
            {
                Result = Node;
                break;
            }
        }
    }

    return(Result);
}

struct path_graph_entries_queue
{
    uint32_t Head, Tail;
    entity_path_graph_entry *Entries[256];
};

static bool
IsEmpty(path_graph_entries_queue *Queue)
{
    bool Result = (Queue->Head == Queue->Tail);
    return(Result);
}

static void
Enqueue(path_graph_entries_queue *Queue, entity_path_graph_entry *Entry)
{
    uint32_t NewTail = (Queue->Tail + 1) % ArrayCount(Queue->Entries);
    Assert(NewTail != Queue->Head);

    Queue->Entries[Queue->Tail] = Entry;
    Queue->Tail = NewTail;
}

static entity_path_graph_entry *
Dequeue(path_graph_entries_queue *Queue)
{
    Assert(!IsEmpty(Queue));

    entity_path_graph_entry *Result = Queue->Entries[Queue->Head];
    Queue->Head = (Queue->Head + 1) % ArrayCount(Queue->Entries);

    return(Result);
}

struct game_state
{
    bool IsInitialized;

    camera Camera;
    hero Hero;

    shader DefaultShader;
    shader ScreenShader;

    GLuint CrossHairTexture;
    GLuint QuadVAO, QuadVBO;


    // NOTE(georgy): Test entity path finding stuff
    uint32_t RabbitCurrentPathCount;
    entity_path_graph_entry *RabbitCurrentPath[128];
    uint32_t RabbitGraphNodesCount;
    entity_path_graph_entry RabbitGraph[1024];
    bool RabbitSpawned;
    bool RabbitHasCurrentTargetBlock;
    uint32_t RabbitCurrentBlockX;
    uint32_t RabbitCurrentBlockY;
    uint32_t RabbitCurrentBlockZ;
    world_position RabbitWorldP;
    vec3 RabbitdP;
    GLuint CubeVAO, CubeVBO;
};

struct temp_state
{
    bool IsInitialized;

    job_system_queue *JobSystemQueue;

    stack_allocator WorldAllocator;
    world World;

    stack_allocator TempAllocator;
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
        Chunk->VAO = Chunk->VBO = Chunk->EBO = 0;

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

inline void
BeginWorldLock(world *World)
{
    for(;;)
    {
        if(InterlockedCompareExchange((LONG volatile *)&World->Lock, 1, 0) == 0)
        {
            break;
        }
    }
}

inline void
EndWorldLock(world *World)
{
    _WriteBarrier();
    World->Lock = 0;
}

struct generate_chunk_job
{
    world *World;
    chunk *Chunk;
    stack_allocator *WorldAllocator;
};
static void
GenerateChunk(void *Data)
{
    generate_chunk_job *Job = (generate_chunk_job *)Data;
    world *World = Job->World;
    chunk *Chunk = Job->Chunk;
    stack_allocator *WorldAllocator = Job->WorldAllocator;

    // TODO(georgy): I think float is not enough for all chunks! Check!
    float BaseX = Chunk->X*World->ChunkDimInMeters.x;
    float BaseZ = Chunk->Z*World->ChunkDimInMeters.z;

    BeginWorldLock(World);
    if(!World->FirstFreeChunkBlocksInfo)
    {
        World->FirstFreeChunkBlocksInfo = PushStruct(WorldAllocator, chunk_blocks_info);
    }
    Chunk->BlocksInfo = World->FirstFreeChunkBlocksInfo;
    World->FirstFreeChunkBlocksInfo = World->FirstFreeChunkBlocksInfo->NextFree;
    EndWorldLock(World);
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

    _WriteBarrier();
    SetFlags(Chunk, CHUNK_GENERATED);
}

static void
GenerateChunks(temp_state *TempState)
{
    for(uint32_t ChunkIndex = 0;
        ChunkIndex < TempState->World.ChunksToGenerateCount;
        ChunkIndex++)
    {
		chunk* Chunk = TempState->World.ChunksToGenerate[ChunkIndex];
        Assert(!CheckFlag(Chunk, CHUNK_GENERATED));

        generate_chunk_job *Job = PushStruct(&TempState->TempAllocator, generate_chunk_job);
        Job->World = &TempState->World;
        Job->Chunk = Chunk;
        Job->WorldAllocator = &TempState->WorldAllocator;

        JobSystemAddEntry(TempState->JobSystemQueue, GenerateChunk, Job);
    }

    JobSystemCompleteAllWork(TempState->JobSystemQueue);
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

struct place_trees_job
{
    world *World;
    chunk *Chunk;
};
static void
PlaceTreesChunk(void *Data)
{
    place_trees_job *Job = (place_trees_job *)Data;
    world *World = Job->World;
    chunk *Chunk = Job->Chunk;

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

    _WriteBarrier();
    SetFlags(Chunk, CHUNK_TREES_PLACED);
}

static void
PlaceTreesChunks(temp_state *TempState)
{
    for(uint32_t ChunkIndex = 0;
        ChunkIndex < TempState->World.ChunksToPlaceTreesCount;
        ChunkIndex++)
    {
        chunk *Chunk = TempState->World.ChunksToPlaceTrees[ChunkIndex];
        Assert(!CheckFlag(Chunk, CHUNK_TREES_PLACED));

        place_trees_job *Job = PushStruct(&TempState->TempAllocator, place_trees_job);
        Job->World = &TempState->World;
        Job->Chunk = Chunk;

        JobSystemAddEntry(TempState->JobSystemQueue, PlaceTreesChunk, Job);
    }

    JobSystemCompleteAllWork(TempState->JobSystemQueue);
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
        BeginWorldLock(World);
        Enqueue(&ChunkToPropagate->SunlightBFSQueueFromOtherChunks, NewLightNode);
        EndWorldLock(World);
    }
}

struct propagate_light_chunk_job
{
    world *World;
    chunk *Chunk;
};
static void
PropagateLightChunk(void *Data)
{
    propagate_light_chunk_job *Job = (propagate_light_chunk_job *)Data;
    world *World = Job->World;
    chunk *Chunk = Job->Chunk;

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

    _WriteBarrier();
    SetFlags(Chunk, CHUNK_LIGHT_PROP_STARTED);
}

static void
PropagateLightChunks(temp_state *TempState)
{
    for(uint32_t ChunkIndex = 0;
        ChunkIndex < TempState->World.ChunksToPropagateLightCount;
        ChunkIndex++)
    {
        chunk *Chunk = TempState->World.ChunksToPropagateLight[ChunkIndex];
        Assert(!CheckFlag(Chunk, CHUNK_LIGHT_PROP_STARTED));

        propagate_light_chunk_job *Job = PushStruct(&TempState->TempAllocator, propagate_light_chunk_job);
        Job->World = &TempState->World;
        Job->Chunk = Chunk;

        JobSystemAddEntry(TempState->JobSystemQueue, PropagateLightChunk, Job);        
    }

    JobSystemCompleteAllWork(TempState->JobSystemQueue);
}

struct finish_propagate_light_chunk
{
    chunk *Chunk;
};
static void
FinishPropagateLightChunk(void *Data)
{
    finish_propagate_light_chunk *Job = (finish_propagate_light_chunk *)Data;
    chunk *Chunk = Job->Chunk;

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

    _WriteBarrier();
    SetFlags(Chunk, CHUNK_LIGHT_PROP_FINISHED);
}

static void
FinishPropagateLightChunks(temp_state *TempState)
{
    for(uint32_t ChunkIndex = 0;
        ChunkIndex < TempState->World.ChunksToFinishPropagateLightCount;
        ChunkIndex++)
    {
        chunk *Chunk = TempState->World.ChunksToFinishPropagateLight[ChunkIndex];
        Assert(!CheckFlag(Chunk, CHUNK_LIGHT_PROP_FINISHED));

        finish_propagate_light_chunk *Job = PushStruct(&TempState->TempAllocator, finish_propagate_light_chunk);
        Job->Chunk = Chunk;

        JobSystemAddEntry(TempState->JobSystemQueue, FinishPropagateLightChunk, Job);      
    }

    JobSystemCompleteAllWork(TempState->JobSystemQueue);
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
    uint32_t VerticesCountBeforeQuad = Chunk->Vertices.EntriesCount / 2;
    if(Order)
    {
        PushEntry(&Chunk->Vertices, A); PushEntry(&Chunk->Vertices, AColor);
        PushEntry(&Chunk->Vertices, B); PushEntry(&Chunk->Vertices, BColor);
        PushEntry(&Chunk->Vertices, C); PushEntry(&Chunk->Vertices, CColor);
        PushEntry(&Chunk->Vertices, D); PushEntry(&Chunk->Vertices, DColor);
    }
    else
    {
        PushEntry(&Chunk->Vertices, B); PushEntry(&Chunk->Vertices, BColor);
        PushEntry(&Chunk->Vertices, D); PushEntry(&Chunk->Vertices, DColor);
        PushEntry(&Chunk->Vertices, A); PushEntry(&Chunk->Vertices, AColor);
        PushEntry(&Chunk->Vertices, C); PushEntry(&Chunk->Vertices, CColor);
    }
    
    PushEntry(&Chunk->Indices, VerticesCountBeforeQuad);
    PushEntry(&Chunk->Indices, VerticesCountBeforeQuad + 1);
    PushEntry(&Chunk->Indices, VerticesCountBeforeQuad + 2);
    PushEntry(&Chunk->Indices, VerticesCountBeforeQuad + 2);
    PushEntry(&Chunk->Indices, VerticesCountBeforeQuad + 1);
    PushEntry(&Chunk->Indices, VerticesCountBeforeQuad + 3);
}

static void
GenerateChunkVertices(world *World, chunk *Chunk)
{
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
}

struct setup_chunk_job
{
    world *World;
    chunk *Chunk;
};
static void
SetupChunk(void *Data)
{
    setup_chunk_job *Job = (setup_chunk_job *)Data;
    world *World = Job->World;
    chunk *Chunk = Job->Chunk;

    InitializeDynamicArray(&Chunk->Vertices); 
    InitializeDynamicArray(&Chunk->Indices); 
    GenerateChunkVertices(World, Chunk);

    _WriteBarrier();
    SetFlags(Chunk, CHUNK_SETUP);
}

static void
SetupChunks(temp_state *TempState)
{
    for(uint32_t ChunkIndex = 0;
        ChunkIndex < TempState->World.ChunksToSetupCount;
        ChunkIndex++)
    {
        chunk *Chunk = TempState->World.ChunksToSetup[ChunkIndex];
        Assert(!CheckFlag(Chunk, CHUNK_SETUP));

        setup_chunk_job *Job = PushStruct(&TempState->TempAllocator, setup_chunk_job);
        Job->World = &TempState->World;
        Job->Chunk = Chunk;

        JobSystemAddEntry(TempState->JobSystemQueue, SetupChunk, Job);    
    }

    JobSystemCompleteAllWork(TempState->JobSystemQueue);
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
        glGenBuffers(1, &Chunk->EBO);
        glBindVertexArray(Chunk->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, Chunk->VBO);
        glBufferData(GL_ARRAY_BUFFER, Chunk->Vertices.EntriesCount*sizeof(vec4), Chunk->Vertices.Entries, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 2*sizeof(vec4), (void*)0);
        glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 2*sizeof(vec4), (void*)(sizeof(vec4)));
		glGenBuffers(1, &Chunk->EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, Chunk->Indices.EntriesCount*sizeof(uint32_t), Chunk->Indices.Entries, GL_STATIC_DRAW);

        SetFlags(Chunk, CHUNK_LOADED);
    }

    glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void
UpdateChunk(world *World, chunk *Chunk)
{
    Chunk->Vertices.EntriesCount = 0;
    Chunk->Indices.EntriesCount = 0;
    GenerateChunkVertices(World, Chunk);

    glBindVertexArray(Chunk->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Chunk->VBO);
    glBufferData(GL_ARRAY_BUFFER, Chunk->Vertices.EntriesCount*sizeof(vec4), Chunk->Vertices.Entries, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 2*sizeof(vec4), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 2*sizeof(vec4), (void*)(sizeof(vec4)));
    glGenBuffers(1, &Chunk->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Chunk->Indices.EntriesCount*sizeof(uint32_t), Chunk->Indices.Entries, GL_STATIC_DRAW);

    ClearFlags(Chunk, CHUNK_MODIFIED);
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
                        glDeleteBuffers(1, &Chunk->EBO);
                        glDeleteBuffers(1, &Chunk->VBO);
                        glDeleteVertexArrays(1, &Chunk->VAO);
                        Chunk->VAO = Chunk->VBO = Chunk->EBO = 0;

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
			glDrawElements(GL_TRIANGLES, (GLsizei)Chunk->Indices.EntriesCount, GL_UNSIGNED_INT, 0);
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
                        if(CheckFlag(Chunk, CHUNK_MODIFIED))
                        {
                            UpdateChunk(World, Chunk);
                        }

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

static void
GameUpdateAndRender(game_memory *Memory, game_input *Input, uint32_t BufferWidth, uint32_t BufferHeight)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(!GameState->IsInitialized)
    {
        GameState->DefaultShader = shader("shaders\\DefaultVS.glsl", "shaders\\DefaultFS.glsl");
        GameState->ScreenShader = shader("shaders\\ScreenVS.glsl", "shaders\\ScreenFS.glsl");

        GameState->CrossHairTexture = LoadTexture("textures\\crosshair.png");

        float QuadVertices[] =
        {
            -0.5f, 0.5f,
            -0.5f, -0.5f,
            0.5f, 0.5f,
            0.5f, -0.5f
        };

        glGenVertexArrays(1, &GameState->QuadVAO);
        glGenBuffers(1, &GameState->QuadVBO);
        glBindVertexArray(GameState->QuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, GameState->QuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        
        float CubeVertices[] = {
            // Back face
            -0.5f, -0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            // Front face
            -0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            // Left face
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            // Right face
            0.5f,  0.5f,  0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            // Bottom face
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,
            // Top face
            -0.5f,  0.5f, -0.5f,
            0.5f,  0.5f , 0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f,  0.5f,
	    };

        glGenVertexArrays(1, &GameState->CubeVAO);
        glGenBuffers(1, &GameState->CubeVBO);
        glBindVertexArray(GameState->CubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, GameState->CubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        GameState->Hero = {};
        GameState->Hero.WorldP.Offset.y = 90.0f;
        GameState->Hero.AABB = AABBMinMax(vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.5f));

        GameState->Camera = {};
        GameState->Camera.P = vec3(0.0f, GetHalfDim(GameState->Hero.AABB).y, 0.0f);

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

        TempState->TempAllocator = SubAlloctor(&TempState->WorldAllocator, Megabytes(32));

        TempState->JobSystemQueue = Memory->JobSystemQueue;

        TempState->IsInitialized = true;
    }

    camera *Camera = &GameState->Camera;
    world *World = &TempState->World;
	hero *Hero = &GameState->Hero;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Camera->Pitch += -0.1f*Input->MouseDeltaY;
    Camera->Head += -0.1f*Input->MouseDeltaX;
    
    Camera->Pitch = Clamp(Camera->Pitch, -89.0f, 89.0f);

    Camera->ViewDir.x = -Cos(Radians(Camera->Pitch))*Sin(Radians(Camera->Head));
    Camera->ViewDir.y = Sin(Radians(Camera->Pitch));
    Camera->ViewDir.z = -Cos(Radians(Camera->Pitch))*Cos(Radians(Camera->Head));
    Camera->ViewDir = Normalize(Camera->ViewDir);

    vec3 CameraRight = Normalize(Cross(Camera->ViewDir, vec3(0.0f, 1.0f, 0.0f)));

    vec3 HeroMovementDir = Normalize(vec3(Camera->ViewDir.x, 0.0f, Camera->ViewDir.z));
    vec3 HeroRightDir = Normalize(vec3(CameraRight.x, 0.0f, CameraRight.z));
    Hero->ddP = vec3(0.0f, 0.0f, 0.0f);
    if(Input->MoveForward.EndedDown)
    {
        Hero->ddP += HeroMovementDir;
    }
    if(Input->MoveBack.EndedDown)
    {
        Hero->ddP += -HeroMovementDir;
    }
    if(Input->MoveRight.EndedDown)
    {
        Hero->ddP += HeroRightDir;
    }
    if(Input->MoveLeft.EndedDown)
    {
        Hero->ddP += -HeroRightDir;
    }
    if(WasDown(&Input->MoveUp))
    {
        if(Hero->CanJump)
        {
            Hero->dP.y = 5.0f;
        }
    }

    BeginSimulation(World, &Hero->WorldP, vec2(75.0f, 75.0f), &TempState->WorldAllocator);

    stack_temp_memory TempMemory = BeginTempMemory(&TempState->TempAllocator);
    GenerateChunks(TempState);
    PlaceTreesChunks(TempState);
    PropagateLightChunks(TempState);
    FinishPropagateLightChunks(TempState);
    SetupChunks(TempState);
    LoadChunks(World);

    mat4 PerspectiveProj = Perspective(45.0f, (float)BufferWidth/(float)BufferHeight, 0.1f, 500.0f);
    mat4 View = Camera->GetViewMatrix();

    GameState->DefaultShader.Use();
    GameState->DefaultShader.SetMat4("Projection", PerspectiveProj);
    GameState->DefaultShader.SetMat4("View", View);
    GameState->DefaultShader.SetVec2("Resolution", vec2((float)BufferWidth, (float)BufferHeight));
    RenderChunks(World, GameState->DefaultShader, PerspectiveProj * View, &TempState->WorldAllocator);

    Hero->ddP = 20.0f*NOZ(Hero->ddP);
    vec3 Drag = -2.0f*Hero->dP;
    Drag.y = 0.0f;
    Hero->ddP += Drag;
    Hero->ddP.y += -9.8f;

    Hero->dP += Input->dt*Hero->ddP;
    vec3 HeroDeltaP = Input->dt*Hero->dP;

	aabb HeroAABB = Hero->AABB;
    aabb SweptAABB = {};
    {
        vec3 Min, Max;
        Min.x = (HeroDeltaP.x > 0.0f) ? HeroAABB.Min.x : HeroAABB.Min.x + HeroDeltaP.x;
        Min.y = (HeroDeltaP.y > 0.0f) ? HeroAABB.Min.y : HeroAABB.Min.y + HeroDeltaP.y;
        Min.z = (HeroDeltaP.z > 0.0f) ? HeroAABB.Min.z : HeroAABB.Min.z + HeroDeltaP.z;
        Max.x = (HeroDeltaP.x > 0.0f) ? HeroAABB.Max.x + HeroDeltaP.x : HeroAABB.Max.x;
        Max.y = (HeroDeltaP.y > 0.0f) ? HeroAABB.Max.y + HeroDeltaP.y : HeroAABB.Max.y;
        Max.z = (HeroDeltaP.z > 0.0f) ? HeroAABB.Max.z + HeroDeltaP.z : HeroAABB.Max.z;

        SweptAABB = AABBMinMax(Min, Max);
    }

    struct chunk_broad_phase
    {
        chunk *Chunk;
        uint32_t MinXBlock, MinYBlock, MinZBlock;
        uint32_t MaxXBlock, MaxYBlock, MaxZBlock;
    };
    uint32_t ChunksInBroadPhaseCount = 0;
    chunk_broad_phase ChunksInBroadPhase[8];
    for(int32_t ZOffset = -1; ZOffset <= 1; ZOffset++)
    {
        for(int32_t XOffset = -1; XOffset <= 1; XOffset++)
        {
            chunk *Chunk = GetChunk(World, Hero->WorldP.ChunkX + XOffset, Hero->WorldP.ChunkZ + ZOffset);
            if(Chunk && CheckFlag(Chunk, CHUNK_LOADED))
            {   
                aabb ChunkAABB = AABBMinMax(Chunk->SimP,
                                            Chunk->SimP + World->ChunkDimInMeters);

                if(Intersect(SweptAABB, ChunkAABB))
                {
                    Assert(ChunksInBroadPhaseCount < (ArrayCount(ChunksInBroadPhase)));

                    vec3 MinOffset = SweptAABB.Min - ChunkAABB.Min;
                    MinOffset = Clamp(MinOffset, 0.0f, FLT_MAX);
                    MinOffset *= 1.0f / World->BlockDimInMeters;

                    vec3 MaxOffset = SweptAABB.Max - ChunkAABB.Min;
                    MaxOffset = Clamp(MaxOffset, vec3(0.0f, 0.0f, 0.0f), World->ChunkDimInMeters);
                    MaxOffset *= 1.0f / World->BlockDimInMeters;
                    MaxOffset = Clamp(MaxOffset, vec3(0.0f, 0.0f, 0.0f), vec3i(CHUNK_DIM_X - 1, CHUNK_DIM_Y - 1, CHUNK_DIM_Z - 1));

                    chunk_broad_phase ChunkBroadPhase;
                    ChunkBroadPhase.Chunk = Chunk;
                    ChunkBroadPhase.MinXBlock = (uint32_t)MinOffset.x;
                    ChunkBroadPhase.MinYBlock = (uint32_t)MinOffset.y;
                    ChunkBroadPhase.MinZBlock = (uint32_t)MinOffset.z;
                    ChunkBroadPhase.MaxXBlock = (uint32_t)MaxOffset.x;
                    ChunkBroadPhase.MaxYBlock = (uint32_t)MaxOffset.y;
                    ChunkBroadPhase.MaxZBlock = (uint32_t)MaxOffset.z;
                    ChunksInBroadPhase[ChunksInBroadPhaseCount++] = ChunkBroadPhase;
                }
            }
        }
    }

    uint32_t BlocksInBroadPhaseCount = 0;
    aabb BlocksInBroadPhase[128];
    for(uint32_t ChunkIndex = 0;
        ChunkIndex < ChunksInBroadPhaseCount;
        ChunkIndex++)
    {
        chunk_broad_phase ChunkBroadPhase = ChunksInBroadPhase[ChunkIndex];
        chunk *Chunk = ChunkBroadPhase.Chunk;
        uint32_t MinXBlock = ChunkBroadPhase.MinXBlock;
        uint32_t MinYBlock = ChunkBroadPhase.MinYBlock;
        uint32_t MinZBlock = ChunkBroadPhase.MinZBlock;
        uint32_t MaxXBlock = ChunkBroadPhase.MaxXBlock;
        uint32_t MaxYBlock = ChunkBroadPhase.MaxYBlock;
        uint32_t MaxZBlock = ChunkBroadPhase.MaxZBlock;

        for(uint32_t ZBlock = MinZBlock; ZBlock <= MaxZBlock; ZBlock++)
        {
            for(uint32_t XBlock = MinXBlock; XBlock <= MaxXBlock; XBlock++)
            {
                for(uint32_t YBlock = MinYBlock; YBlock <= MaxYBlock; YBlock++)
                {
                    if(IsBlockSolid(Chunk->BlocksInfo->Blocks, XBlock, YBlock, ZBlock))
                    {
                        vec3 BlockMin = Chunk->SimP + vec3(XBlock*World->BlockDimInMeters,
                                                           YBlock*World->BlockDimInMeters,
                                                           ZBlock*World->BlockDimInMeters);
                        vec3 BlockMax = Chunk->SimP + vec3((XBlock + 1)*World->BlockDimInMeters,
                                                           (YBlock + 1)*World->BlockDimInMeters,
                                                           (ZBlock + 1)*World->BlockDimInMeters);
                        aabb BlockAABB = AABBMinMax(BlockMin, BlockMax);

                        if(Intersect(SweptAABB, BlockAABB))
                        {
                            Assert(BlocksInBroadPhaseCount < ArrayCount(BlocksInBroadPhase));
                            BlocksInBroadPhase[BlocksInBroadPhaseCount++] = BlockAABB;
                        }
                    }
                }    
            }
        }
    }

    for(uint32_t Iteration = 0; Iteration < 4; Iteration++)
    {
        float HeroDeltaLength = Length(HeroDeltaP);
        if(HeroDeltaLength > 0.0f)
        {
            vec3 DesiredP = Hero->P + HeroDeltaP;
            vec3 Normal = vec3(0.0f, 0.0f, 0.0f);
            float t = 1.0f;

            bool WasCollision = false;
            for(uint32_t BlockIndex = 0;
                BlockIndex < BlocksInBroadPhaseCount;
                )
            {
                aabb BlockAABB = BlocksInBroadPhase[BlockIndex];

                vec3 N = vec3(0.0f, 0.0f, 0.0f);
                float tFirst = 0.0f;

                intersect_moving_aabbs_result IntersectRes = IntersectMovingAABBs(HeroAABB, BlockAABB, HeroDeltaP, &N, &tFirst);
                if(IntersectRes == INTERSECT_AT_START)
                {
                    Hero->P += tFirst*N;
                    HeroAABB = Translate(HeroAABB, tFirst*N);
                    DesiredP += tFirst*N;
                }
                else if(IntersectRes == INTERSECT_MOVING)
                {
                    WasCollision = true;

                    if(tFirst < t)
                    {
                        t = tFirst;
                        Normal = N;
                    }

                    BlockIndex++;
                }
                else
                {
                    BlockIndex++;
                }
            }

            Hero->P += t*HeroDeltaP;
            HeroAABB = Translate(HeroAABB, t*HeroDeltaP);
            HeroDeltaP = DesiredP - Hero->P;
            if(WasCollision)
            {
                HeroDeltaP -= 1.01f*Dot(HeroDeltaP, Normal)*Normal;
                Hero->dP -= 1.01f*Dot(Hero->dP, Normal)*Normal;
            }
        }
    }

    vec3 HalfDim = GetHalfDim(Hero->AABB);
    segment Segments[] = 
    {
        {Hero->P, Hero->P + vec3(0.0f, -HalfDim.y - 0.1f, 0.0f)},
        {Hero->P + vec3(-HalfDim.x, 0.0f, -HalfDim.z), Hero->P + vec3(-HalfDim.x, 0.0f, -HalfDim.z) + vec3(0.0f, -HalfDim.y - 0.1f, 0.0f)},
        {Hero->P + vec3(-HalfDim.x, 0.0f, HalfDim.z), Hero->P + vec3(-HalfDim.x, 0.0f, HalfDim.z) + vec3(0.0f, -HalfDim.y - 0.1f, 0.0f)},
        {Hero->P + vec3(HalfDim.x, 0.0f, HalfDim.z), Hero->P + vec3(HalfDim.x, 0.0f, HalfDim.z) + vec3(0.0f, -HalfDim.y - 0.1f, 0.0f)},
        {Hero->P + vec3(HalfDim.x, 0.0f, -HalfDim.z), Hero->P + vec3(HalfDim.x, 0.0f, -HalfDim.z) + vec3(0.0f, -HalfDim.y - 0.1f, 0.0f)},
    };
    bool Intersect = false;
    for(uint32_t SegmentIndex = 0; 
        (SegmentIndex < ArrayCount(Segments)) && !Intersect;
        SegmentIndex++)
    {
        segment Segment = Segments[SegmentIndex];

        for(uint32_t BlockIndex = 0;
            BlockIndex < BlocksInBroadPhaseCount;
            BlockIndex++)
        {
            aabb BlockAABB = BlocksInBroadPhase[BlockIndex];
            
            float t;
            Intersect = IntersectSegmentAABB(Segment, BlockAABB, &t);
            if(Intersect) break;
        }
    }
    Hero->CanJump = Intersect;

    if(WasDown(&Input->MouseRight))
    {
        world_position NewHeroWorldP = MapIntoChunkSpace(World, &Hero->WorldP, Hero->P);

        chunk *Chunk = GetChunk(World, NewHeroWorldP.ChunkX, NewHeroWorldP.ChunkZ);
        int32_t BlockX = (int32_t)(NewHeroWorldP.Offset.x / World->BlockDimInMeters);
        int32_t BlockY = (int32_t)(NewHeroWorldP.Offset.y / World->BlockDimInMeters);
        int32_t BlockZ = (int32_t)(NewHeroWorldP.Offset.z / World->BlockDimInMeters);

        chunk *LastChunk = Chunk;
        int32_t LastFreeBlockX = BlockX;
        int32_t LastFreeBlockY = BlockY;
        int32_t LastFreeBlockZ = BlockZ;

        vec3 RayP = Hero->P + Camera->P;
        vec3 RayDir = Camera->ViewDir;
        for(uint32_t BlockIndex = 0; BlockIndex < 10; BlockIndex++)
        {
            float XBorderDist = FLT_MAX;
            {
                vec3 PlaneN = vec3(1.0f, 0.0f, 0.0f);
                if(RayDir.x > 0.0f)
                {
                    float PlaneD = Chunk->SimP.x + (BlockX + 1)*World->BlockDimInMeters;
                    XBorderDist = (PlaneD - Dot(RayP, PlaneN)) / Dot(RayDir, PlaneN);
                }
                else if(RayDir.x < 0.0f)
                {
                    float PlaneD = Chunk->SimP.x + BlockX*World->BlockDimInMeters;
                    XBorderDist = (PlaneD - Dot(RayP, PlaneN)) / Dot(RayDir, PlaneN);
                }
            }

            float YBorderDist = FLT_MAX;
            {
                vec3 PlaneN = vec3(0.0f, 1.0f, 0.0f);
                if(RayDir.y > 0.0f)
                {
                    float PlaneD = Chunk->SimP.y + (BlockY + 1)*World->BlockDimInMeters;
                    YBorderDist = (PlaneD - Dot(RayP, PlaneN)) / Dot(RayDir, PlaneN);
                }
                else if(RayDir.y < 0.0f)
                {
                    float PlaneD = Chunk->SimP.y + BlockY*World->BlockDimInMeters;
                    YBorderDist = (PlaneD - Dot(RayP, PlaneN)) / Dot(RayDir, PlaneN);
                }
            }
            
            float ZBorderDist = FLT_MAX;
            {
                vec3 PlaneN = vec3(0.0f, 0.0f, 1.0f);
                if(RayDir.z > 0.0f)
                {
                    float PlaneD = Chunk->SimP.z + (BlockZ + 1)*World->BlockDimInMeters;
                    ZBorderDist = (PlaneD - Dot(RayP, PlaneN)) / Dot(RayDir, PlaneN);
                }
                else if(RayDir.z < 0.0f)
                {
                    float PlaneD = Chunk->SimP.z + BlockZ*World->BlockDimInMeters;
                    ZBorderDist = (PlaneD - Dot(RayP, PlaneN)) / Dot(RayDir, PlaneN);
                }
            }

            if(XBorderDist <= YBorderDist)
            {
                if(XBorderDist <= ZBorderDist)
                {
                    BlockX = (RayDir.x > 0.0f) ? BlockX + 1 : BlockX - 1;
                    RayP += RayDir*(XBorderDist);
                    Chunk = GetChunkForBlock(World, Chunk, BlockX, BlockY, BlockZ);
                }
                else
                {
                    BlockZ = (RayDir.z > 0.0f) ? BlockZ + 1 : BlockZ - 1;
                    RayP += RayDir*(ZBorderDist);
                    Chunk = GetChunkForBlock(World, Chunk, BlockX, BlockY, BlockZ);
                }
            }
            else
            {
                if(YBorderDist <= ZBorderDist)
                {
                    BlockY = (RayDir.y > 0.0f) ? BlockY + 1 : BlockY - 1;
                    RayP += RayDir*(YBorderDist);
                }
                else
                {
                    BlockZ = (RayDir.z > 0.0f) ? BlockZ + 1 : BlockZ - 1;
                    RayP += RayDir*(ZBorderDist);
                    Chunk = GetChunkForBlock(World, Chunk, BlockX, BlockY, BlockZ);
                }
            }

            if(IsBlockSolid(Chunk->BlocksInfo->Blocks, BlockX, BlockY, BlockZ))
            {
                BlockType(LastChunk->BlocksInfo->Blocks, LastFreeBlockX, LastFreeBlockY, LastFreeBlockZ) = block_type::BLOCK_SOIL;
                BlockLightLevel(LastChunk->BlocksInfo->Blocks, LastFreeBlockX, LastFreeBlockY, LastFreeBlockZ) = 0;
                BlockColor(LastChunk->BlocksInfo->Colors, LastFreeBlockX, LastFreeBlockY, LastFreeBlockZ) = vec3(1.0f, 0.0f, 0.0f);
                SetFlags(LastChunk, CHUNK_MODIFIED);
                break;
            }
            else
            {
                LastChunk = Chunk;
                LastFreeBlockX = BlockX;
                LastFreeBlockY = BlockY;
                LastFreeBlockZ = BlockZ;
            }
        }
    }
    
    // NOTE(georgy): Test entity path finding stuff
    if(Input->H.EndedDown && !GameState->RabbitSpawned)
    {
        chunk *Chunk = GetChunk(World, 0, 0);

        GameState->RabbitWorldP = {};
        uint32_t BlockX = CHUNK_DIM_X / 2;
		uint32_t BlockZ = CHUNK_DIM_Z / 2;
		uint32_t BlockY = Chunk->HeightMap[BlockZ*CHUNK_DIM_X + BlockX] + 1;
        GameState->RabbitWorldP.Offset = World->BlockDimInMeters*vec3i(BlockX, BlockY, BlockZ) + 
                                         0.5f*vec3(World->BlockDimInMeters, World->BlockDimInMeters, World->BlockDimInMeters);

        GameState->RabbitCurrentBlockX = BlockX;
        GameState->RabbitCurrentBlockY = BlockY;
        GameState->RabbitCurrentBlockZ = BlockZ;

        GameState->RabbitGraphNodesCount = 0;
        for(uint32_t Z = 0; Z < CHUNK_DIM_Z; Z++)
        {
            for(uint32_t X = 0; X < CHUNK_DIM_X; X++)
            {
                uint32_t Y = Chunk->HeightMap[Z*CHUNK_DIM_X + X] + 1;

                GameState->RabbitGraph[GameState->RabbitGraphNodesCount].Visited = false;
                GameState->RabbitGraph[GameState->RabbitGraphNodesCount].BlockX = X;
                GameState->RabbitGraph[GameState->RabbitGraphNodesCount].BlockY = Y;
                GameState->RabbitGraph[GameState->RabbitGraphNodesCount].BlockZ = Z;

                ++GameState->RabbitGraphNodesCount;
            }
        }

        GameState->RabbitSpawned = true;
    }

    if(GameState->RabbitSpawned)
    {
        vec3 ddP = vec3(0.0f, 0.0f, 0.0f);
        if(!GameState->RabbitHasCurrentTargetBlock)
        {
            entity_path_graph_entry *StartNode;
            for(uint32_t NodeIndex = 0; NodeIndex < GameState->RabbitGraphNodesCount; NodeIndex++)
            {
                entity_path_graph_entry *Node = GameState->RabbitGraph + NodeIndex;

                Node->Visited = false;
                Node->Parent = 0;

                if((Node->BlockX == GameState->RabbitCurrentBlockX) &&
                   (Node->BlockY == GameState->RabbitCurrentBlockY) &&
                   (Node->BlockZ == GameState->RabbitCurrentBlockZ))
                {
                    StartNode = Node;
                    StartNode->Visited = true;
                }
            }

            path_graph_entries_queue Queue = {};
            Enqueue(&Queue, StartNode);
            while(!IsEmpty(&Queue))
            {
                entity_path_graph_entry *Node = Dequeue(&Queue);

                int32_t X = (int32_t)Node->BlockX - 1;
                int32_t Z = Node->BlockZ;
                entity_path_graph_entry *LeftNeighbour = GetNodeFromPathGraph(GameState->RabbitGraph, GameState->RabbitGraphNodesCount, X, Z);
                if(LeftNeighbour && (((int32_t)LeftNeighbour->BlockY - (int32_t)Node->BlockY) < 2) && !LeftNeighbour->Visited)
                {
                    LeftNeighbour->Visited = true;
                    LeftNeighbour->Parent = Node;
                    Enqueue(&Queue, LeftNeighbour);
                }


                X = Node->BlockX + 1;
                Z = Node->BlockZ;
                entity_path_graph_entry *RightNeighbour = GetNodeFromPathGraph(GameState->RabbitGraph, GameState->RabbitGraphNodesCount, X, Z);
                if(RightNeighbour && (((int32_t)RightNeighbour->BlockY - (int32_t)Node->BlockY) < 2) && !RightNeighbour->Visited)
                {
                    RightNeighbour->Visited = true;
                    RightNeighbour->Parent = Node;
                    Enqueue(&Queue, RightNeighbour);
                }


                X = Node->BlockX;
                Z = (int32_t)Node->BlockZ - 1;
                entity_path_graph_entry *TopNeighbour = GetNodeFromPathGraph(GameState->RabbitGraph, GameState->RabbitGraphNodesCount, X, Z);
                if(TopNeighbour && (((int32_t)TopNeighbour->BlockY - (int32_t)Node->BlockY) < 2) && !TopNeighbour->Visited)
                {
                    TopNeighbour->Visited = true;
                    TopNeighbour->Parent = Node;
                    Enqueue(&Queue, TopNeighbour);
                }

                X = Node->BlockX;
                Z = Node->BlockZ + 1;
                entity_path_graph_entry *BotNeighbour = GetNodeFromPathGraph(GameState->RabbitGraph, GameState->RabbitGraphNodesCount, X, Z);
                if(BotNeighbour && (((int32_t)BotNeighbour->BlockY - (int32_t)Node->BlockY) < 2) && !BotNeighbour->Visited)
                {
                    BotNeighbour->Visited = true;
                    BotNeighbour->Parent = Node;
                    Enqueue(&Queue, BotNeighbour);
                }
            }
            
            uint32_t TargetBlockX = rand() % CHUNK_DIM_X;
            uint32_t TargetBlockZ = rand() % CHUNK_DIM_Z;
            while((TargetBlockX == GameState->RabbitCurrentBlockX) &&
                  (TargetBlockZ == GameState->RabbitCurrentBlockZ))
            {
                TargetBlockX = rand() % CHUNK_DIM_X;
                TargetBlockZ = rand() % CHUNK_DIM_Z;
            }

            entity_path_graph_entry *TargetNode = GetNodeFromPathGraph(GameState->RabbitGraph, GameState->RabbitGraphNodesCount, 
                                                                       TargetBlockX, TargetBlockZ);
            Assert(TargetNode);

            GameState->RabbitCurrentPathCount = 0;
            entity_path_graph_entry *Node = TargetNode;
            while(Node)
            {
                GameState->RabbitCurrentPath[GameState->RabbitCurrentPathCount++] = Node;
                Node = Node->Parent;
            }
            Assert(GameState->RabbitCurrentPath[GameState->RabbitCurrentPathCount - 1] == StartNode);

            GameState->RabbitHasCurrentTargetBlock = true;
        }
        else
        {
            entity_path_graph_entry *TargetBlock = GameState->RabbitCurrentPath[GameState->RabbitCurrentPathCount - 2];

            vec3 TargetP = World->BlockDimInMeters*vec3i(TargetBlock->BlockX, TargetBlock->BlockY, TargetBlock->BlockZ) + 
                           0.5f*vec3(World->BlockDimInMeters, World->BlockDimInMeters, World->BlockDimInMeters);
            vec3 MovingDir = TargetP - GameState->RabbitWorldP.Offset;

            ddP = 5.0f*NOZ(MovingDir);
        }
        ddP -= 2.0f*GameState->RabbitdP;

        GameState->RabbitdP += Input->dt*ddP;
        GameState->RabbitWorldP.Offset += Input->dt*GameState->RabbitdP;

        GameState->RabbitCurrentBlockX = (uint32_t)(GameState->RabbitWorldP.Offset.x / World->BlockDimInMeters);
        GameState->RabbitCurrentBlockY = (uint32_t)(GameState->RabbitWorldP.Offset.y / World->BlockDimInMeters);
        GameState->RabbitCurrentBlockZ = (uint32_t)(GameState->RabbitWorldP.Offset.z / World->BlockDimInMeters);
        if((GameState->RabbitCurrentBlockX == GameState->RabbitCurrentPath[GameState->RabbitCurrentPathCount - 2]->BlockX) &&
           (GameState->RabbitCurrentBlockY == GameState->RabbitCurrentPath[GameState->RabbitCurrentPathCount - 2]->BlockY) &&
           (GameState->RabbitCurrentBlockZ == GameState->RabbitCurrentPath[GameState->RabbitCurrentPathCount - 2]->BlockZ))
        {
            --GameState->RabbitCurrentPathCount;
            if(GameState->RabbitCurrentPathCount == 1)
            {
                GameState->RabbitHasCurrentTargetBlock = false;
            }
        }

        vec3 RabbitSimP = Substract(World, &GameState->RabbitWorldP, &Hero->WorldP);
        mat4 CubeModel = Translation(RabbitSimP) * Scaling(0.25f);
        GameState->DefaultShader.SetMat4("Model", CubeModel);
        glBindVertexArray(GameState->CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    Hero->WorldP = MapIntoChunkSpace(World, &Hero->WorldP, Hero->P);
    Hero->P = vec3(0.0f, 0.0f, 0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mat4 OrthoProj = Ortho(-0.5f*BufferHeight, 0.5f*BufferHeight, -0.5f*BufferWidth, 0.5f*BufferWidth, -0.1f, -100.0f);
    mat4 Model = Scaling(25.0f);
    GameState->ScreenShader.Use();
    GameState->ScreenShader.SetMat4("Projection", OrthoProj);
    GameState->ScreenShader.SetMat4("Model", Model);
    GameState->ScreenShader.SetI32("Texture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GameState->CrossHairTexture);
    glBindVertexArray(GameState->QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisable(GL_BLEND);

    EndTempMemory(TempMemory);
    ResetWorldWork(World);
}
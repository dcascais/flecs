#include <reflecs/vector.h>
#include <reflecs/map.h>
#include "world.h"
#include "table.h"
#include "entity.h"

const EcsVectorParams entities_vec_params = {
    .element_size = sizeof(EcsEntity),
    .chunk_count = REFLECS_INITIAL_CHUNK_COUNT
};

const EcsVectorParams tables_vec_params = {
    .element_size = sizeof(EcsTable),
    .chunk_count = REFLECS_INITIAL_CHUNK_COUNT
};

const EcsArrayParams entityptr_arr_params = {
    .element_size = sizeof(EcsEntity*)
};

static
int compare_ptr(
    const void *p1,
    const void *p2)
{
    return *(void**)p1 - *(void**)p2;
}

static
uint64_t hash_entity_array(
    EcsEntity** array,
    uint32_t count)
{
    uint64_t hash = 0;
    int i;
    for (i = 0; i < count; i ++) {
        ecs_hash(array[i], sizeof(void*), &hash);
    }
    return hash;
}

static
EcsResult ecs_world_init(
    EcsWorld *world)
{
    uint64_t stage_hash;

    EcsEntity *type = ecs_new(world, "EcsType");
    ecs_add(type, type);
    stage_hash = type->stage_hash;

    EcsTable *table =
        ecs_table_new_w_size(world, stage_hash, sizeof(EcsType));

    ecs_map_set(world->tables_map, stage_hash, table);

    world->type = type;

    if (ecs_commit(world->type) != EcsOk) {
        return EcsError;
    }

    EcsType *type_data = ecs_get(type, type);
    if (!type_data) {
        return EcsError;
    }

    type_data->size = sizeof(EcsType);

    return EcsOk;
}

EcsWorld* ecs_world_new(void)
{
    EcsWorld *result = malloc(sizeof(EcsWorld));
    result->entities = ecs_vector_new(&entities_vec_params);
    result->entities_map = ecs_map_new(REFLECS_INITIAL_ENTITY_COUNT);
    result->tables = ecs_vector_new(&tables_vec_params);
    result->tables_map = ecs_map_new(REFLECS_INITIAL_TABLE_COUNT);
    result->components_map = ecs_map_new(REFLECS_INITIAL_COMPONENT_SET_COUNT);
    ecs_world_init(result);
    return result;
}

uint64_t ecs_world_components_hash(
    EcsWorld *world,
    EcsArray *set,
    EcsEntity *to_add)
{
    uint64_t stage_hash = 0;
    uint32_t count = ecs_array_count(set);
    EcsEntity *new_set[count + 1];
    void *new_buffer = new_set;

    if (to_add) {
        void *buffer = ecs_array_buffer(set);
        if (count) {
            memcpy(new_set, buffer, sizeof(EcsEntity*) * count);
        }
        new_set[count] = to_add;
        qsort(new_set, count + 1, sizeof(EcsEntity*), compare_ptr);
        stage_hash = hash_entity_array(new_set, count + 1);
        count ++;
    } else if (set) {
        void *buffer = ecs_array_buffer(set);
        stage_hash = hash_entity_array(buffer, count);
        new_buffer = buffer;
    } else {
        return 0;
    }

    EcsArray *stage_set = ecs_map_get(world->components_map, stage_hash);
    if (!stage_set) {
        stage_set = ecs_array_new_from_buffer(count, &entityptr_arr_params, new_buffer);
        ecs_map_set(world->components_map, stage_hash, stage_set);
    }

    return stage_hash;
}

EcsArray* ecs_world_get_components(
    EcsWorld *world,
    uint64_t components_hash)
{
    return ecs_map_get(world->components_map, components_hash);
}

EcsTable *ecs_world_lookup_table(
    EcsWorld *world,
    uint64_t components_hash)
{
    return ecs_map_get(world->tables_map, components_hash);
}

EcsTable *ecs_world_create_table(
    EcsWorld *world,
    uint64_t components_hash)
{
    EcsTable *table = ecs_table_new(world, components_hash);
    if (!table) {
        return NULL;
    }

    ecs_map_set(world->tables_map, components_hash, table);
    return table;
}

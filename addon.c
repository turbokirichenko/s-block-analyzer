#define NAPI_VERSION 8
#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

struct s_block_stats
{
    uint8_t **probability_matrix;
    size_t sblock_len;
};

uint8_t xor_bits(const uint8_t value)
{
    uint8_t v = value;
    uint8_t s = 0;
    for (v = value; v; v = v >> 1)
    {
        s ^= v % 2;
    }
    return s;
}

static struct s_block_stats *SBlockCalculations(uint16_t sblock_len, uint8_t *sblock_outputs)
{
    uint8_t **output_matrix = (uint8_t **)malloc(sizeof(uint8_t *) * sblock_len);
    for (uint16_t i = 0; i < sblock_len; ++i)
    {
        output_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * sblock_len);
    }
    uint16_t maskX = 0;
    uint16_t maskY = 0;
    for (uint16_t maskX = 0; maskX < sblock_len; ++maskX)
    {
        for (uint16_t maskY = 0; maskY < sblock_len; ++maskY)
        {
            uint8_t overlap = 0;
            uint8_t i = 0;
            do
            {
                const uint8_t dX = maskX & i;
                const uint8_t dY = maskY & sblock_outputs[i];
                overlap += (xor_bits(dX) == xor_bits(dY)) ? 1 : 0;
            } while (sblock_len - ++i);
            output_matrix[maskX][maskY] = overlap;
        }
    }

    struct s_block_stats *stats = malloc(sizeof(struct s_block_stats));
    stats->probability_matrix = output_matrix;
    stats->sblock_len = sblock_len;
    return stats;
}

static napi_value CheckBlock(napi_env env, napi_callback_info info)
{
    napi_status status;
    size_t argc = 2;
    napi_value args[1];
    status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    assert(status == napi_ok);

    if (argc < 1)
    {
        napi_throw_type_error(env, NULL, "Wrong number of arguments");
        return NULL;
    }

    bool isArgArray;
    status = napi_is_typedarray(env, args[0], &isArgArray);
    assert(status == napi_ok);

    if (!isArgArray)
    {
        napi_throw_type_error(env, NULL, "Wrong type of the second parameter");
        return NULL;
    }

    napi_typedarray_type arr_type;
    size_t sblock_len;
    void *sblock_outputs;
    status = napi_get_typedarray_info(env, args[0], &arr_type, &sblock_len, &sblock_outputs, NULL, NULL);
    uint8_t *sblock = sblock_outputs;
    assert(status == napi_ok);

    if (sblock_len - 1 > UINT8_MAX)
    {
        napi_throw_type_error(env, NULL, "S-block length is more, than 255 bit");
        return NULL;
    }

    // calculate statistics of block
    struct s_block_stats *stats = SBlockCalculations(sblock_len, sblock);

    napi_value output_matrix;
    status = napi_create_array_with_length(env, sblock_len, &output_matrix);
    assert(status == napi_ok);
    // translite result to output matrix
    for (uint16_t i = 0; i < stats->sblock_len; ++i)
    {
        napi_value array;
        status = napi_create_array_with_length(env, stats->sblock_len, &array);
        assert(status == napi_ok);
        for (uint16_t j = 0; j < stats->sblock_len; ++j)
        {
            napi_value number;
            status = napi_create_uint32(env, stats->probability_matrix[i][j], &number);
            assert(status == napi_ok);
            status = napi_set_element(env, array, j, number);
            assert(status == napi_ok);
        }
        status = napi_set_element(env, output_matrix, i, array);
        assert(status == napi_ok);
    }
    // free up memory
    for (uint16_t i = 0; i < stats->sblock_len; ++i)
    {
        free(stats->probability_matrix[i]);
    }
    free(stats->probability_matrix);
    free(stats);
    // result
    return output_matrix;
}

#define DECLARE_NAPI_METHOD(name, func)         \
    {                                           \
        name, 0, func, 0, 0, 0, napi_default, 0 \
    }

static napi_value Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc = DECLARE_NAPI_METHOD("add", CheckBlock);
    status = napi_define_properties(env, exports, 1, &desc);
    assert(status == napi_ok);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
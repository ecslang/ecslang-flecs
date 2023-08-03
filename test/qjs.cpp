#include "quickjs-extern.h"
#include "doctest.h"

TEST_CASE("QuickJS tests") {
    /* Initialize the JS runtime and context */
    JSRuntime *rt = js_runtime_new();
    REQUIRE(rt);

    JSContext *ctx = js_context_new(rt);
    REQUIRE(ctx);

    SUBCASE("value types") {
        CHECK_EQ(js_value_get_type(nullptr), JS_VALUE_TYPE_UNINITIALIZED);

        JSValue* value = js_value_new(ctx);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_UNDEFINED);
        js_value_free(ctx, value);

        // js_value_new_*
        value = js_value_new_uint32(ctx, 42);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_INT);
        CHECK_EQ(js_value_get_int(value), 42);
        CHECK_EQ(js_value_get_float(value), 42.0);
        js_value_free(ctx, value);

        value = js_value_new_uint64(ctx, 42);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_INT);
        CHECK_EQ(js_value_get_int(value), 42);
        CHECK_EQ(js_value_get_float(value), 42.0);
        js_value_free(ctx, value);

        value = js_value_new_int32(ctx, -42);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_INT);
        CHECK_EQ(js_value_get_int(value), -42);
        CHECK_EQ(js_value_get_float(value), -42.0);
        js_value_free(ctx, value);

        value = js_value_new_int64(ctx, -42);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_INT);
        CHECK_EQ(js_value_get_int(value), -42);
        CHECK_EQ(js_value_get_float(value), -42.0);
        js_value_free(ctx, value);

        value = js_value_new_bool(ctx, 0);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_BOOL);
        CHECK_EQ(js_value_get_int(value), 0);
        CHECK_EQ(js_value_get_float(value), 0.0);
        CHECK_EQ(js_value_get_bool(value), 0);
        js_value_free(ctx, value);

        value = js_value_new_bool(ctx, 42);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_BOOL);
        CHECK_EQ(js_value_get_int(value), 1);
        CHECK_EQ(js_value_get_float(value), 1);
        CHECK_EQ(js_value_get_bool(value), 1);
        js_value_free(ctx, value);

        value = js_value_new_int64(ctx, (int64_t)INT32_MAX + 1ll);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_FLOAT64);
        CHECK_EQ(js_value_get_int(value), (int64_t)INT32_MAX + 1ll);
        CHECK_EQ(js_value_get_float(value), (double)((int64_t)INT32_MAX + 1ll));
        js_value_free(ctx, value);

        value = js_value_new_float(ctx, 42.5);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_FLOAT64);
        CHECK_EQ(js_value_get_int(value), 42);
        CHECK_EQ(js_value_get_float(value), 42.5);
        js_value_free(ctx, value);

        // js_value_load_*
        value = js_value_new(ctx);
        js_value_load_uint32(ctx, value, 42);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_INT);
        CHECK_EQ(js_value_get_int(value), 42);
        CHECK_EQ(js_value_get_float(value), 42.0);
        js_value_free(ctx, value);

        value = js_value_new(ctx);
        js_value_load_uint64(ctx, value, 42);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_INT);
        CHECK_EQ(js_value_get_int(value), 42);
        CHECK_EQ(js_value_get_float(value), 42.0);
        js_value_free(ctx, value);

        value = js_value_new(ctx);
        js_value_load_int32(ctx, value, -42);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_INT);
        CHECK_EQ(js_value_get_int(value), -42);
        CHECK_EQ(js_value_get_float(value), -42.0);
        js_value_free(ctx, value);

        value = js_value_new(ctx);
        js_value_load_int64(ctx, value, -42);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_INT);
        CHECK_EQ(js_value_get_int(value), -42);
        CHECK_EQ(js_value_get_float(value), -42.0);
        js_value_free(ctx, value);

        value = js_value_new(ctx);
        js_value_load_bool(ctx, value, 0);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_BOOL);
        CHECK_EQ(js_value_get_int(value), 0);
        CHECK_EQ(js_value_get_float(value), 0.0);
        CHECK_EQ(js_value_get_bool(value), 0);
        js_value_free(ctx, value);

        value = js_value_new(ctx);
        js_value_load_bool(ctx, value, 42);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_BOOL);
        CHECK_EQ(js_value_get_int(value), 1);
        CHECK_EQ(js_value_get_float(value), 1);
        CHECK_EQ(js_value_get_bool(value), 1);
        js_value_free(ctx, value);

        value = js_value_new(ctx);
        js_value_load_int64(ctx, value, (int64_t)INT32_MAX + 1ll);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_FLOAT64);
        CHECK_EQ(js_value_get_int(value), (int64_t)INT32_MAX + 1ll);
        CHECK_EQ(js_value_get_float(value), (double)((int64_t)INT32_MAX + 1ll));
        js_value_free(ctx, value);

        value = js_value_new(ctx);
        js_value_load_float(ctx, value, 42.5);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_FLOAT64);
        CHECK_EQ(js_value_get_int(value), 42);
        CHECK_EQ(js_value_get_float(value), 42.5);
        js_value_free(ctx, value);
    }
//    test_string(ctx);
//    test_eval(ctx);
//    test_objects(ctx);
//    test_globals(ctx);
//    test_eval_ctx(ctx);
//    test_cfunction(ctx);
//    test_json(ctx);

    /* Clean up */
    js_context_free(ctx);
    js_runtime_free(rt);
}

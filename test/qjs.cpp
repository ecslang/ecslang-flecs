#include <cstring>

#include "quickjs-extern.h"
#include "doctest.h"

#define JS_LOAD_JSON(ctx, out, json) \
    REQUIRE(js_json_parse_cstr(ctx, out, json) == JS_OK) \

static int test_counter = 0;

static JSValue inc_counter(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic, JSValue *func_data)
{
    printf("inc_counter called\n");

    void* userPtr = JS_VALUE_GET_PTR(func_data[0]);

    int* counter = (int*)userPtr;
    ++(*counter);

    return JS_UNDEFINED;
}

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
    SUBCASE("strings") {
        JSValue* value = js_value_new(ctx);
        const char* str;
        REQUIRE(value);

        str = js_str_from_value(ctx, value);
        REQUIRE(str);
        CHECK_EQ(str, doctest::String("undefined"));
        js_str_free(ctx, str);
        js_value_free(ctx, value);

        const char* testStr = "Hello";
        size_t testStrLen = strlen(testStr);

        value = js_value_new_str(ctx, testStr, testStrLen);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_STRING);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String(testStr));
        js_str_free(ctx, str);
        js_value_free(ctx, value);

        value = js_value_new_cstr(ctx, "Hello, World!");
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_STRING);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String("Hello, World!"));
        js_str_free(ctx, str);
        js_value_free(ctx, value);

        value = js_value_new(ctx);
        js_value_load_str(ctx, value, testStr, testStrLen);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_STRING);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String(testStr));
        js_str_free(ctx, str);
        js_value_free(ctx, value);

        value = js_value_new(ctx);
        js_value_load_cstr(ctx, value, "Hello, World!");
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_STRING);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String("Hello, World!"));
        js_str_free(ctx, str);
        js_value_free(ctx, value);

        // empty string
        value = js_value_new_str(ctx, nullptr, 0);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_STRING);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String(""));
        js_str_free(ctx, str);
        js_value_free(ctx, value);

        value = js_value_new_cstr(ctx, "");
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_STRING);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String(""));
        js_str_free(ctx, str);
        js_value_free(ctx, value);

        value = js_value_new(ctx);
        js_value_load_str(ctx, value, NULL, 0);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_STRING);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String(""));
        js_str_free(ctx, str);
        js_value_free(ctx, value);

        value = js_value_new(ctx);
        js_value_load_cstr(ctx, value, "");
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_STRING);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String(""));
        js_str_free(ctx, str);
        js_value_free(ctx, value);

        // test unicode
        testStr = "M\xc3\xabt\xc3\xa0l H\xc3\xab\xc3\xa0""d";
        testStrLen = strlen(testStr);

        value = js_value_new_str(ctx, testStr, testStrLen);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_STRING);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String(testStr));
        js_str_free(ctx, str);
        js_value_free(ctx, value);

        value = js_value_new_cstr(ctx, testStr);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_STRING);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String(testStr));
        js_str_free(ctx, str);
        js_value_free(ctx, value);

        value = js_value_new(ctx);
        js_value_load_str(ctx, value, testStr, testStrLen);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_STRING);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String(testStr));
        js_str_free(ctx, str);
        js_value_free(ctx, value);

        value = js_value_new(ctx);
        js_value_load_cstr(ctx, value, testStr);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_STRING);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String(testStr));
        js_str_free(ctx, str);
        js_value_free(ctx, value);
    }

    SUBCASE("eval") {
        JSValue* value = js_value_new(ctx);
        REQUIRE(value);

        const char *evalStr = "40 + 2";
        int eval_flags = JS_EVAL_TYPE_GLOBAL;  // Use the correct flag based on your needs
        const char *filename = "<eval>";
        int err = js_eval(ctx, value, evalStr, strlen(evalStr), filename, eval_flags);
        REQUIRE(err == 0);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_INT);

        const char* str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String("42"));
        js_str_free(ctx, str);

        /* Test js_eval_cstr */
        err = js_eval_cstr(ctx, value, "40.5 + 2", filename, eval_flags);
        REQUIRE(err == 0);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_FLOAT64);

        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String("42.5"));
        js_str_free(ctx, str);

        /* Test js_eval_cstr throwing */
        err = js_eval_cstr(ctx, value, "throw 'error'", filename, eval_flags);
        CHECK_EQ(err, JS_ERR_EXCEPTION);

        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String("error"));
        js_str_free(ctx, str);

        js_value_free(ctx, value);
    }

    SUBCASE("objects") {
        JSValue *value = js_value_new(ctx);
        JSValue *obj;
        JSValue *temp;
        const char *str;

        obj = js_value_new_obj(ctx);
        CHECK_EQ(js_value_get_type(obj), JS_VALUE_TYPE_OBJECT);
        CHECK(js_value_get_obj_ref(obj));
        js_value_free(ctx, obj);

        obj = js_value_new(ctx);
        CHECK_EQ(js_value_get_type(obj), JS_VALUE_TYPE_UNDEFINED);
        js_obj_create(ctx, obj);
        CHECK_EQ(js_value_get_type(obj), JS_VALUE_TYPE_OBJECT);
        CHECK(js_value_get_obj_ref(obj));
        js_value_free(ctx, obj);

        obj = js_value_new_obj(ctx);
        js_value_load_cstr(ctx, value, "Hello");
        js_obj_set_property(ctx, obj, "a", value);
        js_value_load_int32(ctx, value, 2);
        js_obj_set_property(ctx, obj, "b", value);

        // read property a
        js_obj_read_property(ctx, value, obj, "a");
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_STRING);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String("Hello"));
        js_str_free(ctx, str);

        // read property b
        js_obj_read_property(ctx, value, obj, "b");
        CHECK_EQ(js_value_get_int(value), 2);

        // get property b
        temp = js_obj_get_property(ctx, obj, "b");
        CHECK(temp);
        CHECK_EQ(js_value_get_int(temp), 2);
        js_value_free(ctx, temp);

        js_value_free(ctx, obj);

        js_value_free(ctx, value);
    }

    SUBCASE("globals") {
        JSValue *value = js_value_new(ctx);
        JSValue *globals = js_value_new_ref_globals(ctx);
        CHECK_EQ(js_value_get_type(globals), JS_VALUE_TYPE_OBJECT);
        REQUIRE(js_value_get_obj_ref(globals));

        js_value_load_ref_globals(ctx, value);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_OBJECT);
        CHECK_EQ(js_value_get_obj_ref(value), js_value_get_obj_ref(globals));

        js_value_free(ctx, globals);
        js_value_free(ctx, value);
    }

    SUBCASE("eval_ctx") {
        JSValue* value = js_value_new(ctx);
        REQUIRE(value);

        JSValue* self = js_value_new(ctx);
        REQUIRE(self);
        JS_LOAD_JSON(ctx, self, "{\"a\":\"Hello\",\"b\":2}");

        const char *evalStr = "this.b";
        int err = js_eval_this(ctx, self, value, evalStr, strlen(evalStr), "<eval>", JS_EVAL_TYPE_GLOBAL);
        REQUIRE(err == 0);
        CHECK_EQ(js_value_get_type(value), JS_VALUE_TYPE_INT);
        const char* str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String("2"));
        js_str_free(ctx, str);

        err = js_eval_this_cstr(ctx, self, value, "this.a", "<eval>", JS_EVAL_TYPE_GLOBAL);
        REQUIRE(err == 0);
        str = js_str_from_value(ctx, value);
        CHECK_EQ(str, doctest::String("Hello"));
        js_str_free(ctx, str);

        js_value_free(ctx, self);
        js_value_free(ctx, value);
    }

    SUBCASE("cfunction") {
        JSValue* globals = js_value_new_ref_globals(ctx);
        // JSValue* user_ptr = js_value_new_ptr(ctx, &test_counter);

        int counter = test_counter;

//    JS_SetPropertyStr(ctx, globals, "inc_counter",
//        JS_NewCFunctionData(ctx, inc_counter, 0, 0, 1, user_ptr));
        js_value_set_property_func(ctx, globals, "inc_counter", inc_counter, 0, &test_counter);

        CHECK_EQ(test_counter, counter);

        int err = js_eval_cstr(ctx, NULL, "inc_counter()", "<eval>", JS_EVAL_TYPE_GLOBAL);
        REQUIRE(err == 0);

        CHECK_EQ(test_counter, counter+1);

        js_value_free(ctx, globals);
    }

    SUBCASE("json") {
        JSValue* value = js_value_new(ctx);
        int err;

        const char *jsonOut;
        const char *jsonIn = "{\"a\":\"Hello\",\"b\":2}";

        err = js_json_parse_str(ctx, value, jsonIn, strlen(jsonIn));
        REQUIRE_EQ(err, JS_OK);
        jsonOut = js_json_stringify(ctx, value);
        CHECK_EQ(jsonOut, doctest::String(jsonIn));
        js_str_free(ctx, jsonOut);

        err = js_json_parse_cstr(ctx, value, jsonIn);
        REQUIRE_EQ(err, JS_OK);
        jsonOut = js_json_stringify(ctx, value);
        CHECK_EQ(jsonOut, doctest::String(jsonIn));
        js_str_free(ctx, jsonOut);

        JS_LOAD_JSON(ctx, value, jsonIn);
        jsonOut = js_json_stringify(ctx, value);
        CHECK_EQ(jsonOut, doctest::String(jsonIn));
        js_str_free(ctx, jsonOut);

        js_value_free(ctx, value);
    }

    /* Clean up */
    js_context_free(ctx);
    js_runtime_free(rt);
}

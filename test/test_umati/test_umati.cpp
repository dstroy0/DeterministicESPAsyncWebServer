// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for the umati (OPC UA for Machine Tools, OPC 40501-1) MachineTool model:
// the Browse hierarchy shape + the Read resolver over a bound UmatiMachineTool.

#include "services/umati/umati.h"
#include <string.h>
#include <unity.h>

// Node ids (must track the internal enum in umati.cpp).
enum
{
    N_MACHINETOOL = 5000,
    N_IDENTIFICATION = 5100,
    N_ID_MANUFACTURER = 5101,
    N_ID_YEAR = 5104,
    N_MONITORING = 5200,
    N_MON_MACHINE = 5210,
    N_MON_OPMODE = 5211,
    N_MON_CHANNEL = 5220,
    N_CH_STATE = 5221,
    N_CH_FEEDOVR = 5222,
    N_CH_ACTIVEPROG = 5224,
    N_MON_SPINDLE = 5230,
    N_SP_SPEED = 5231,
    N_SP_ROTATING = 5233,
    N_MON_AXIS_X = 5240,
    N_AX_X_POS = 5241,
    N_PRODUCTION = 5300,
    N_PROD_PARTCOUNT = 5302,
    N_NOTIFICATION = 5400,
    N_NOTIF_MESSAGE = 5401,
    N_NOTIF_SEVERITY = 5402,
};

static UmatiMachineTool g_mt;

void setUp(void)
{
    memset(&g_mt, 0, sizeof(g_mt));
    g_mt.name = "CNC-1";
    g_mt.ident.manufacturer = "Acme Machines";
    g_mt.ident.model = "AX-500";
    g_mt.ident.serial_number = "SN-00042";
    g_mt.ident.software_revision = "2.7.1";
    g_mt.ident.product_instance_uri = "urn:acme:AX-500:SN-00042";
    g_mt.ident.year_of_construction = 2026;
    g_mt.operation_mode = UmatiOperationMode::UMATI_OP_AUTOMATIC;
    g_mt.power_on_duration_s = 12345.0;
    g_mt.channel.state = UmatiChannelState::UMATI_CH_RUNNING;
    g_mt.channel.feed_override = 85.0;
    g_mt.channel.rapid_override = 100.0;
    g_mt.channel.active_program = "PART_A.NC";
    g_mt.spindle.rotation_speed = 1200.0;
    g_mt.spindle.override_value = 90.0;
    g_mt.spindle.is_rotating = true;
    g_mt.axis_x.actual_position = 10.5;
    g_mt.axis_y.actual_position = -3.25;
    g_mt.axis_z.actual_position = 42.0;
    g_mt.active_program = "PART_A.NC";
    g_mt.produced_part_count = 7;
    g_mt.message_text = "Door open";
    g_mt.message_severity = 500;
    dws_umati_bind(&g_mt);
}
void tearDown(void)
{
}

// Browse a node into a fixed buffer; returns the count.
static int32_t browse(uint16_t ns, uint32_t id, OpcUaReference *refs, uint32_t cap)
{
    return dws_umati_browse(ns, id, refs, cap);
}
static const OpcUaReference *find_ref(const OpcUaReference *refs, int32_t n, const char *name)
{
    for (int32_t i = 0; i < n; i++)
        if (refs[i].browse_name && strcmp(refs[i].browse_name, name) == 0)
            return &refs[i];
    return nullptr;
}

// --- Browse hierarchy -------------------------------------------------------
static void test_browse_objects_folder_has_machinetool(void)
{
    OpcUaReference refs[8];
    int32_t n = browse(0, 85, refs, 8); // Objects folder
    TEST_ASSERT_EQUAL_INT32(1, n);
    TEST_ASSERT_EQUAL_UINT32(N_MACHINETOOL, refs[0].target_id);
    TEST_ASSERT_EQUAL_UINT32(DWS_UMATI_NS, refs[0].target_ns);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_NODECLASS_OBJECT, refs[0].node_class);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_REFTYPE_ORGANIZES, refs[0].ref_type_id); // Organizes, not HasComponent
    TEST_ASSERT_EQUAL_STRING("CNC-1", refs[0].browse_name);                 // uses mt->name
}

static void test_browse_machinetool_components(void)
{
    OpcUaReference refs[8];
    int32_t n = browse(DWS_UMATI_NS, N_MACHINETOOL, refs, 8);
    TEST_ASSERT_EQUAL_INT32(4, n); // Identification, Monitoring, Production, Notification
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "Identification"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "Monitoring"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "Production"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "Notification"));
    // every component is HasComponent + Object
    TEST_ASSERT_EQUAL_UINT32(OPCUA_REFTYPE_HAS_COMPONENT, refs[0].ref_type_id);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_NODECLASS_OBJECT, refs[0].node_class);
}

static void test_browse_identification_variables(void)
{
    OpcUaReference refs[8];
    int32_t n = browse(DWS_UMATI_NS, N_IDENTIFICATION, refs, 8);
    TEST_ASSERT_EQUAL_INT32(6, n);
    const OpcUaReference *man = find_ref(refs, n, "Manufacturer");
    TEST_ASSERT_NOT_NULL(man);
    TEST_ASSERT_EQUAL_UINT32(N_ID_MANUFACTURER, man->target_id);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_NODECLASS_VARIABLE, man->node_class);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_TYPEDEF_BASE_DATA_VARIABLE, man->type_def_id);
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "SerialNumber"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "YearOfConstruction"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "ProductInstanceUri"));
}

static void test_browse_monitoring_and_children(void)
{
    OpcUaReference refs[8];
    int32_t n = browse(DWS_UMATI_NS, N_MONITORING, refs, 8);
    TEST_ASSERT_EQUAL_INT32(6, n); // MachineTool, Channel, Spindle, Axis_X/Y/Z
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "Channel"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "Spindle"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "Axis_X"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "Axis_Z"));

    n = browse(DWS_UMATI_NS, N_MON_CHANNEL, refs, 8);
    TEST_ASSERT_EQUAL_INT32(4, n); // ChannelState, FeedOverride, RapidOverride, ActiveProgram
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "ChannelState"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "FeedOverride"));

    n = browse(DWS_UMATI_NS, N_MON_SPINDLE, refs, 8);
    TEST_ASSERT_EQUAL_INT32(3, n); // RotationSpeed, OverrideValue, IsRotating

    n = browse(DWS_UMATI_NS, N_MON_AXIS_X, refs, 8);
    TEST_ASSERT_EQUAL_INT32(1, n); // ActualPosition
    TEST_ASSERT_EQUAL_STRING("ActualPosition", refs[0].browse_name);
    TEST_ASSERT_EQUAL_UINT32(N_AX_X_POS, refs[0].target_id);
}

static void test_browse_leaf_and_unknown_return_negative(void)
{
    OpcUaReference refs[4];
    TEST_ASSERT_EQUAL_INT32(-1, browse(DWS_UMATI_NS, N_SP_SPEED, refs, 4)); // a leaf Variable has no children
    TEST_ASSERT_EQUAL_INT32(-1, browse(DWS_UMATI_NS, 999999, refs, 4));     // unknown node
    TEST_ASSERT_EQUAL_INT32(-1, browse(7, N_MACHINETOOL, refs, 4));         // wrong namespace
}

// --- Read resolver ----------------------------------------------------------
static void test_read_identification(void)
{
    OpcUaVariant v;
    TEST_ASSERT_TRUE(dws_umati_read(DWS_UMATI_NS, N_ID_MANUFACTURER, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_STRING, v.type);
    TEST_ASSERT_EQUAL_STRING("Acme Machines", v.str);

    TEST_ASSERT_TRUE(dws_umati_read(DWS_UMATI_NS, N_ID_YEAR, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_UINT32, v.type);
    TEST_ASSERT_EQUAL_UINT32(2026, v.u32);
}

static void test_read_monitoring_values(void)
{
    OpcUaVariant v;
    TEST_ASSERT_TRUE(dws_umati_read(DWS_UMATI_NS, N_MON_OPMODE, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_INT32, v.type);
    TEST_ASSERT_EQUAL_INT32((int32_t)UmatiOperationMode::UMATI_OP_AUTOMATIC, v.i32);

    TEST_ASSERT_TRUE(dws_umati_read(DWS_UMATI_NS, N_CH_STATE, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL_INT32((int32_t)UmatiChannelState::UMATI_CH_RUNNING, v.i32);

    TEST_ASSERT_TRUE(dws_umati_read(DWS_UMATI_NS, N_CH_FEEDOVR, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_DOUBLE, v.type);
    TEST_ASSERT_TRUE(v.f64 == 85.0); // exact value; the suite compares doubles with == (Unity double asserts off)

    TEST_ASSERT_TRUE(dws_umati_read(DWS_UMATI_NS, N_SP_SPEED, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_TRUE(v.f64 == 1200.0);

    TEST_ASSERT_TRUE(dws_umati_read(DWS_UMATI_NS, N_SP_ROTATING, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_BOOL, v.type);
    TEST_ASSERT_TRUE(v.b);

    TEST_ASSERT_TRUE(dws_umati_read(DWS_UMATI_NS, N_AX_X_POS, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_TRUE(v.f64 == 10.5);

    TEST_ASSERT_TRUE(dws_umati_read(DWS_UMATI_NS, N_CH_ACTIVEPROG, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL_STRING("PART_A.NC", v.str);
}

static void test_read_production_and_notification(void)
{
    OpcUaVariant v;
    TEST_ASSERT_TRUE(dws_umati_read(DWS_UMATI_NS, N_PROD_PARTCOUNT, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL_UINT32(7, v.u32);
    TEST_ASSERT_TRUE(dws_umati_read(DWS_UMATI_NS, N_NOTIF_MESSAGE, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL_STRING("Door open", v.str);
    TEST_ASSERT_TRUE(dws_umati_read(DWS_UMATI_NS, N_NOTIF_SEVERITY, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL_UINT32(500, v.u32);
}

static void test_read_null_string_served_as_empty(void)
{
    g_mt.ident.model = nullptr; // a null field must not crash - served as ""
    OpcUaVariant v;
    TEST_ASSERT_TRUE(dws_umati_read(DWS_UMATI_NS, 5102 /*Model*/, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_STRING, v.type);
    TEST_ASSERT_EQUAL_STRING("", v.str);
    TEST_ASSERT_EQUAL_INT32(0, v.str_len);
}

static void test_read_rejects_unknown_ns_attr_and_node(void)
{
    OpcUaVariant v;
    TEST_ASSERT_FALSE(dws_umati_read(DWS_UMATI_NS, 999999, OPCUA_ATTR_VALUE, &v));         // unknown node
    TEST_ASSERT_FALSE(dws_umati_read(7, N_ID_MANUFACTURER, OPCUA_ATTR_VALUE, &v));         // wrong namespace
    TEST_ASSERT_FALSE(dws_umati_read(DWS_UMATI_NS, N_ID_MANUFACTURER, 12 /*!Value*/, &v)); // not the Value attribute
}

static void test_read_before_bind_is_a_clean_miss(void)
{
    dws_umati_bind(nullptr); // no model bound
    OpcUaVariant v;
    TEST_ASSERT_FALSE(dws_umati_read(DWS_UMATI_NS, N_ID_MANUFACTURER, OPCUA_ATTR_VALUE, &v));
    OpcUaReference refs[4];
    TEST_ASSERT_EQUAL_INT32(-1, dws_umati_browse(0, 85, refs, 4));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_browse_objects_folder_has_machinetool);
    RUN_TEST(test_browse_machinetool_components);
    RUN_TEST(test_browse_identification_variables);
    RUN_TEST(test_browse_monitoring_and_children);
    RUN_TEST(test_browse_leaf_and_unknown_return_negative);
    RUN_TEST(test_read_identification);
    RUN_TEST(test_read_monitoring_values);
    RUN_TEST(test_read_production_and_notification);
    RUN_TEST(test_read_null_string_served_as_empty);
    RUN_TEST(test_read_rejects_unknown_ns_attr_and_node);
    RUN_TEST(test_read_before_bind_is_a_clean_miss);
    return UNITY_END();
}

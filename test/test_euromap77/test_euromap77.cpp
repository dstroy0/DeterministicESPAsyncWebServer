// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for the EUROMAP 77 (OPC 40077) IMM_MES_Interface model: the Browse hierarchy shape + the
// Read resolver over a bound EmImm, including the UInt64 production counters.

#include "services/euromap77/euromap77.h"
#include <string.h>
#include <unity.h>

// Node ids (must track the internal enum in euromap77.cpp).
enum
{
    N_IMM = 7000,
    N_MACHINEINFO = 7100,
    N_MI_MANUFACTURER = 7101,
    N_MI_MODEL = 7102,
    N_MI_MANUFACTURERURI = 7108,
    N_MACHINESTATUS = 7200,
    N_MS_ISPRESENT = 7201,
    N_MS_MACHINEMODE = 7202,
    N_JOBS = 7300,
    N_ACTIVEJOB = 7310,
    N_AJ_JOBNAME = 7311,
    N_AJ_EXPECTEDCYCLE = 7316,
    N_AJ_NUMCAVITIES = 7317,
    N_AJ_NOMINALPARTS = 7318,
    N_ACTIVEJOBVALUES = 7320,
    N_AJV_JOBCYCLECOUNTER = 7321,
    N_AJV_LASTCYCLETIME = 7323,
    N_AJV_JOBPARTSCOUNTER = 7325,
    N_AJV_JOBSTATUS = 7328,
};

static EmImm g_imm;

void setUp(void)
{
    memset(&g_imm, 0, sizeof(g_imm));
    g_imm.name = "IMM-1";
    g_imm.info.manufacturer = "Acme Plastics";
    g_imm.info.model = "IM-200";
    g_imm.info.serial_number = "SN-IMM-0042";
    g_imm.info.product_code = "IM200-STD";
    g_imm.info.hardware_revision = "H1";
    g_imm.info.software_revision = "3.4.0";
    g_imm.info.device_revision = "D2";
    g_imm.info.manufacturer_uri = "urn:acme:plastics";
    g_imm.status.is_present = true;
    g_imm.status.machine_mode = EmMachineMode::EM_MODE_AUTOMATIC;
    g_imm.active_job.job_name = "JOB-A";
    g_imm.active_job.job_description = "widget run";
    g_imm.active_job.material = "ABS";
    g_imm.active_job.product_name = "Widget";
    g_imm.active_job.mould_id = "MLD-9";
    g_imm.active_job.expected_cycle_time = 12.5;
    g_imm.active_job.num_cavities = 4;
    g_imm.active_job.nominal_parts = 100000ULL;
    g_imm.active_job_values.job_cycle_counter = 5000000001ULL; // > 2^32 to prove UInt64
    g_imm.active_job_values.machine_cycle_counter = 9000000002ULL;
    g_imm.active_job_values.last_cycle_time = 12.7;
    g_imm.active_job_values.average_cycle_time = 12.6;
    g_imm.active_job_values.job_parts_counter = 20000000004ULL;
    g_imm.active_job_values.job_good_parts_counter = 19000000003ULL;
    g_imm.active_job_values.job_bad_parts_counter = 1000000001ULL;
    g_imm.active_job_values.job_status = EmJobStatus::EM_JOB_IN_PRODUCTION;
    dws_em77_bind(&g_imm);
}
void tearDown(void)
{
}

static int32_t browse(uint16_t ns, uint32_t id, OpcUaReference *refs, uint32_t cap)
{
    return dws_em77_browse(ns, id, refs, cap);
}
static const OpcUaReference *find_ref(const OpcUaReference *refs, int32_t n, const char *name)
{
    for (int32_t i = 0; i < n; i++)
        if (refs[i].browse_name && strcmp(refs[i].browse_name, name) == 0)
            return &refs[i];
    return nullptr;
}

// --- Browse hierarchy -------------------------------------------------------
static void test_browse_objects_folder_has_interface(void)
{
    OpcUaReference refs[8];
    int32_t n = browse(0, 85, refs, 8); // Objects folder
    TEST_ASSERT_EQUAL_INT32(1, n);
    TEST_ASSERT_EQUAL_UINT32(N_IMM, refs[0].target_id);
    TEST_ASSERT_EQUAL_UINT32(DWS_EM77_NS, refs[0].target_ns);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_NODECLASS_OBJECT, refs[0].node_class);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_REFTYPE_ORGANIZES, refs[0].ref_type_id);
    TEST_ASSERT_EQUAL_STRING("IMM-1", refs[0].browse_name); // uses imm->name
}

static void test_browse_interface_components(void)
{
    OpcUaReference refs[8];
    int32_t n = browse(DWS_EM77_NS, N_IMM, refs, 8);
    TEST_ASSERT_EQUAL_INT32(3, n); // MachineInformation, MachineStatus, Jobs
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "MachineInformation"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "MachineStatus"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "Jobs"));
}

static void test_browse_machineinformation(void)
{
    OpcUaReference refs[8];
    int32_t n = browse(DWS_EM77_NS, N_MACHINEINFO, refs, 8);
    TEST_ASSERT_EQUAL_INT32(8, n);
    const OpcUaReference *man = find_ref(refs, n, "Manufacturer");
    TEST_ASSERT_NOT_NULL(man);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_NODECLASS_VARIABLE, man->node_class);
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "SerialNumber"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "ProductCode"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "ManufacturerUri"));
}

static void test_browse_status_and_jobs(void)
{
    OpcUaReference refs[8];
    int32_t n = browse(DWS_EM77_NS, N_MACHINESTATUS, refs, 8);
    TEST_ASSERT_EQUAL_INT32(2, n); // IsPresent, MachineMode
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "IsPresent"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "MachineMode"));

    n = browse(DWS_EM77_NS, N_JOBS, refs, 8);
    TEST_ASSERT_EQUAL_INT32(2, n); // ActiveJob, ActiveJobValues
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "ActiveJob"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "ActiveJobValues"));
}

static void test_browse_activejob_and_values(void)
{
    OpcUaReference refs[8];
    int32_t n = browse(DWS_EM77_NS, N_ACTIVEJOB, refs, 8);
    TEST_ASSERT_EQUAL_INT32(8, n);
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "JobName"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "ExpectedCycleTime"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "NumCavities"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "NominalParts"));

    n = browse(DWS_EM77_NS, N_ACTIVEJOBVALUES, refs, 8);
    TEST_ASSERT_EQUAL_INT32(8, n); // every container <= DWS_OPCUA_REF_MAX
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "JobCycleCounter"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "MachineCycleCounter"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "JobGoodPartsCounter"));
    TEST_ASSERT_NOT_NULL(find_ref(refs, n, "JobStatus"));
}

static void test_browse_leaf_and_unknown_return_negative(void)
{
    OpcUaReference refs[4];
    TEST_ASSERT_EQUAL_INT32(-1, browse(DWS_EM77_NS, N_MI_MANUFACTURER, refs, 4)); // a leaf Variable
    TEST_ASSERT_EQUAL_INT32(-1, browse(DWS_EM77_NS, 999999, refs, 4));            // unknown node
    TEST_ASSERT_EQUAL_INT32(-1, browse(7, N_IMM, refs, 4));                       // wrong namespace
}

// --- Read resolver ----------------------------------------------------------
static void test_read_identity_and_status(void)
{
    OpcUaVariant v;
    TEST_ASSERT_TRUE(dws_em77_read(DWS_EM77_NS, N_MI_MANUFACTURER, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_STRING, v.type);
    TEST_ASSERT_EQUAL_STRING("Acme Plastics", v.str);

    TEST_ASSERT_TRUE(dws_em77_read(DWS_EM77_NS, N_MS_ISPRESENT, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_BOOL, v.type);
    TEST_ASSERT_TRUE(v.b);

    TEST_ASSERT_TRUE(dws_em77_read(DWS_EM77_NS, N_MS_MACHINEMODE, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_INT32, v.type);
    TEST_ASSERT_EQUAL_INT32((int32_t)EmMachineMode::EM_MODE_AUTOMATIC, v.i32);
}

static void test_read_job_and_counters(void)
{
    OpcUaVariant v;
    TEST_ASSERT_TRUE(dws_em77_read(DWS_EM77_NS, N_AJ_JOBNAME, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL_STRING("JOB-A", v.str);

    TEST_ASSERT_TRUE(dws_em77_read(DWS_EM77_NS, N_AJ_EXPECTEDCYCLE, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_DOUBLE, v.type);
    TEST_ASSERT_TRUE(v.f64 == 12.5);

    TEST_ASSERT_TRUE(dws_em77_read(DWS_EM77_NS, N_AJ_NUMCAVITIES, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_UINT32, v.type);
    TEST_ASSERT_EQUAL_UINT32(4, v.u32);

    // UInt64 fields (prove the new Variant type carries a value > 2^32)
    TEST_ASSERT_TRUE(dws_em77_read(DWS_EM77_NS, N_AJ_NOMINALPARTS, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_UINT64, v.type);
    TEST_ASSERT_TRUE(v.u64 == 100000ULL);

    TEST_ASSERT_TRUE(dws_em77_read(DWS_EM77_NS, N_AJV_JOBCYCLECOUNTER, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_UINT64, v.type);
    TEST_ASSERT_TRUE(v.u64 == 5000000001ULL);

    TEST_ASSERT_TRUE(dws_em77_read(DWS_EM77_NS, N_AJV_JOBPARTSCOUNTER, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_TRUE(v.u64 == 20000000004ULL);

    TEST_ASSERT_TRUE(dws_em77_read(DWS_EM77_NS, N_AJV_LASTCYCLETIME, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_TRUE(v.f64 == 12.7);

    TEST_ASSERT_TRUE(dws_em77_read(DWS_EM77_NS, N_AJV_JOBSTATUS, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL_INT32((int32_t)EmJobStatus::EM_JOB_IN_PRODUCTION, v.i32);
}

static void test_read_null_string_served_as_empty(void)
{
    g_imm.info.model = nullptr; // a null field must not crash - served as ""
    OpcUaVariant v;
    TEST_ASSERT_TRUE(dws_em77_read(DWS_EM77_NS, N_MI_MODEL, OPCUA_ATTR_VALUE, &v));
    TEST_ASSERT_EQUAL(OpcUaVariantType::OPCUA_VAR_STRING, v.type);
    TEST_ASSERT_EQUAL_STRING("", v.str);
    TEST_ASSERT_EQUAL_INT32(0, v.str_len);
}

static void test_read_rejects_unknown_ns_attr_and_node(void)
{
    OpcUaVariant v;
    TEST_ASSERT_FALSE(dws_em77_read(DWS_EM77_NS, 999999, OPCUA_ATTR_VALUE, &v));         // unknown node
    TEST_ASSERT_FALSE(dws_em77_read(7, N_MI_MANUFACTURER, OPCUA_ATTR_VALUE, &v));        // wrong namespace
    TEST_ASSERT_FALSE(dws_em77_read(DWS_EM77_NS, N_MI_MANUFACTURER, 12 /*!Value*/, &v)); // not the Value attribute
}

static void test_read_before_bind_is_a_clean_miss(void)
{
    dws_em77_bind(nullptr); // no model bound
    OpcUaVariant v;
    TEST_ASSERT_FALSE(dws_em77_read(DWS_EM77_NS, N_MI_MANUFACTURER, OPCUA_ATTR_VALUE, &v));
    OpcUaReference refs[4];
    TEST_ASSERT_EQUAL_INT32(-1, dws_em77_browse(0, 85, refs, 4));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_browse_objects_folder_has_interface);
    RUN_TEST(test_browse_interface_components);
    RUN_TEST(test_browse_machineinformation);
    RUN_TEST(test_browse_status_and_jobs);
    RUN_TEST(test_browse_activejob_and_values);
    RUN_TEST(test_browse_leaf_and_unknown_return_negative);
    RUN_TEST(test_read_identity_and_status);
    RUN_TEST(test_read_job_and_counters);
    RUN_TEST(test_read_null_string_served_as_empty);
    RUN_TEST(test_read_rejects_unknown_ns_attr_and_node);
    RUN_TEST(test_read_before_bind_is_a_clean_miss);
    return UNITY_END();
}

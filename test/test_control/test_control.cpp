// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the PID control law (services/control): P / I / D terms, output clamping,
// anti-windup (conditional integration), derivative-on-measurement, feed-forward, the batched
// update, and the inline control-law primitives. Pure host tests.

#include "services/control/control.h"
#include <math.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static bool near_f(float a, float b)
{
    return fabsf(a - b) < 1e-4f;
}

void test_proportional_only()
{
    Pid p;
    pid_init(&p, 2.0f, 0.0f, 0.0f);
    TEST_ASSERT_TRUE(near_f(pid_update(&p, 10.0f, 0.0f, 0.1f), 20.0f)); // kp*error = 2*10
}

void test_integral_accumulates()
{
    Pid p;
    pid_init(&p, 1.0f, 10.0f, 0.0f);
    TEST_ASSERT_TRUE(near_f(pid_update(&p, 1.0f, 0.0f, 0.1f), 2.0f)); // P=1, I=10*1*0.1=1
    TEST_ASSERT_TRUE(near_f(pid_update(&p, 1.0f, 0.0f, 0.1f), 3.0f)); // P=1, I=2
}

void test_feedforward()
{
    Pid p;
    pid_init(&p, 0.0f, 0.0f, 0.0f);
    pid_set_feedforward(&p, 0.5f);
    TEST_ASSERT_TRUE(near_f(pid_update(&p, 8.0f, 0.0f, 0.1f), 4.0f)); // kff*setpoint = 0.5*8
}

void test_output_clamp_and_antiwindup()
{
    Pid p;
    pid_init(&p, 1.0f, 10.0f, 0.0f);
    pid_set_output_limits(&p, 0.0f, 5.0f);
    TEST_ASSERT_TRUE(near_f(pid_update(&p, 100.0f, 0.0f, 0.1f), 5.0f)); // saturates high
    float integ1 = p.integ;
    TEST_ASSERT_TRUE(near_f(pid_update(&p, 100.0f, 0.0f, 0.1f), 5.0f));
    TEST_ASSERT_TRUE(p.integ == integ1);     // integrator frozen while saturated (no windup)
    TEST_ASSERT_TRUE(fabsf(p.integ) < 1.0f); // did not accumulate
}

void test_antiwindup_recovers()
{
    // Once the error reverses, the (un-wound) integrator resumes normally.
    Pid p;
    pid_init(&p, 1.0f, 10.0f, 0.0f);
    pid_set_output_limits(&p, 0.0f, 5.0f);
    pid_update(&p, 100.0f, 0.0f, 0.1f);            // saturate, integrator frozen at 0
    float out = pid_update(&p, 0.0f, 10.0f, 0.1f); // error now -10, should be able to leave the rail
    TEST_ASSERT_TRUE(out < 5.0f);
}

void test_derivative_on_measurement()
{
    Pid p;
    pid_init(&p, 0.0f, 0.0f, 1.0f);
    TEST_ASSERT_TRUE(near_f(pid_update(&p, 0.0f, 0.0f, 0.1f), 0.0f));   // first step primes, no D
    TEST_ASSERT_TRUE(near_f(pid_update(&p, 0.0f, 1.0f, 0.1f), -10.0f)); // d=-(1-0)/0.1
}

void test_setpoint_step_no_kick()
{
    // A setpoint step must NOT produce a derivative kick (D acts on measurement only).
    Pid p;
    pid_init(&p, 0.0f, 0.0f, 5.0f);
    pid_update(&p, 0.0f, 0.0f, 0.1f);                                  // prime
    TEST_ASSERT_TRUE(near_f(pid_update(&p, 99.0f, 0.0f, 0.1f), 0.0f)); // setpoint jumped, meas flat -> D=0
}

void test_derivative_filter()
{
    Pid p;
    pid_init(&p, 0.0f, 0.0f, 1.0f);
    pid_set_derivative_filter(&p, 0.5f); // EMA alpha
    pid_update(&p, 0.0f, 0.0f, 0.1f);    // prime, d_filt=0
    // raw d = -(1-0)/0.1 = -10; filtered = 0 + 0.5*(-10 - 0) = -5
    TEST_ASSERT_TRUE(near_f(pid_update(&p, 0.0f, 1.0f, 0.1f), -5.0f));
}

void test_reset_and_guards()
{
    Pid p;
    pid_init(&p, 1.0f, 10.0f, 0.0f);
    pid_update(&p, 5.0f, 0.0f, 0.1f);
    TEST_ASSERT_TRUE(p.integ != 0.0f);
    pid_reset(&p);
    TEST_ASSERT_TRUE(p.integ == 0.0f);
    TEST_ASSERT_FALSE(p.primed);
    TEST_ASSERT_TRUE(near_f(pid_update(&p, 1.0f, 0.0f, 0.0f), 0.0f)); // dt<=0 returns 0
    TEST_ASSERT_TRUE(near_f(pid_update(nullptr, 1.0f, 0.0f, 0.1f), 0.0f));
}

void test_batched_update()
{
    Pid axes[2];
    pid_init(&axes[0], 1.0f, 0.0f, 0.0f);
    pid_init(&axes[1], 3.0f, 0.0f, 0.0f);
    float sp[2] = {5.0f, 2.0f};
    float meas[2] = {0.0f, 0.0f};
    float out[2] = {0.0f, 0.0f};
    pid_update_n(axes, sp, meas, 0.1f, out, 2);
    TEST_ASSERT_TRUE(near_f(out[0], 5.0f));
    TEST_ASSERT_TRUE(near_f(out[1], 6.0f));
}

void test_control_primitives()
{
    TEST_ASSERT_TRUE(near_f(control_clamp(7.0f, 0.0f, 5.0f), 5.0f));
    TEST_ASSERT_TRUE(near_f(control_clamp(-1.0f, 0.0f, 5.0f), 0.0f));
    TEST_ASSERT_TRUE(near_f(control_deadband(0.3f, 0.5f), 0.0f));
    TEST_ASSERT_TRUE(near_f(control_deadband(1.5f, 0.5f), 1.0f));
    TEST_ASSERT_TRUE(near_f(control_deadband(-1.5f, 0.5f), -1.0f));
    TEST_ASSERT_TRUE(near_f(control_slew(10.0f, 0.0f, 2.0f), 2.0f));
    TEST_ASSERT_TRUE(near_f(control_slew(1.0f, 0.0f, 2.0f), 1.0f)); // within step -> reach target
    TEST_ASSERT_TRUE(near_f(control_lpf(0.0f, 10.0f, 0.25f), 2.5f));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_proportional_only);
    RUN_TEST(test_integral_accumulates);
    RUN_TEST(test_feedforward);
    RUN_TEST(test_output_clamp_and_antiwindup);
    RUN_TEST(test_antiwindup_recovers);
    RUN_TEST(test_derivative_on_measurement);
    RUN_TEST(test_setpoint_step_no_kick);
    RUN_TEST(test_derivative_filter);
    RUN_TEST(test_reset_and_guards);
    RUN_TEST(test_batched_update);
    RUN_TEST(test_control_primitives);
    return UNITY_END();
}

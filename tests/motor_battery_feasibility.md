# Alfred 1 — Motor Load & Battery Feasibility Test

**Project:** Alfred 1 Autonomous Robot  
**Engineer:** Eric Thomas  
**Date:** _(add test date)_  
**Revision:** 1.0

---

## Objective
Determine whether the current **12.8 V 12 Ah LiFePO₄ (≈ 3 lb)** battery provides acceptable weight, power capacity, and performance for Alfred 1 without overloading the two 12 V DC gear motors or the TB6612FNG motor driver.

---

## Equipment Required
- Alfred 1 prototype with frame, motors, battery, and Pi system
- Digital multimeter (≥ 10 A DC)
- Spring or digital luggage scale (0 – 5 kg range)
- Ruler or calipers (for wheel radius)
- Stopwatch / timer app
- Notebook or spreadsheet
- *Optional:* clamp-on DC ammeter for safer current readings

---

## Safety Precautions
1. Use a **fused or bench-supply source** during early tests.  
2. **Limit stall current tests ≤ 1 second** to avoid damage.  
3. Keep hands, wires, and clothing clear of moving wheels.  
4. Work on a **flat, non-slip surface**.

---

## Test 1 – Free-Run Current & Speed
**Purpose:** Establish baseline no-load draw per motor.

**Procedure**
1. Power one motor directly from 12 V DC through an ammeter.  
2. Record **no-load current (I_free)** and speed (if measurable).  
3. Repeat for second motor.

**Expected Result:** I_free ≈ 0.10 – 0.15 A per motor (± 10 %).

| Motor | I_free (A) | RPM | Notes |
|:------|:-----------:|:----:|:------|
| Left |             |     |     |
| Right |             |     |     |

---

## Test 2 – Stall Current & Torque
**Purpose:** Measure maximum torque and current at stall.

**Procedure**
1. Attach scale tangentially to wheel rim.  
2. Increase tension until wheel stops.  
3. Record **stall force (F_stall)** and **stall current (I_stall)**.  
4. Measure wheel radius (r).

**Calculation:**  τ = F_stall × r

**Expected Result:** Driver current < 3.2 A peak (TB6612FNG limit).

| Motor | F_stall (N) | r (m) | τ (N·m) | I_stall (A) | Notes |
|:------|:-------------:|:-----:|:---------:|:-------------:|:------|
| Left |     |     |     |     |     |
| Right |     |     |     |     |     |

---

## Test 3 – Static Load Test (with Battery)
**Purpose:** Verify ability to start motion under full system weight.

**Procedure**
1. Mount 3 lb battery and electronics.  
2. Place robot on flat surface.  
3. Drive forward at 50 % PWM for 5 s.  
4. Record:
   - **Start current (I_start)**
   - **Peak current**
   - Qualitative motion (smooth / hesitant / stall)

**Pass Criteria:** Robot starts reliably; Peak current < 2.5 A total.

| Trial | I_start (A) | I_peak (A) | Motion | Pass/Fail | Notes |
|:-------|:------------:|:-----------:|:-------:|:-----------:|:------|
| 1 |     |     |     |     |     |

---

## Test 4 – Ramp Performance
**Purpose:** Evaluate climbing capability.

**Procedure**
1. Prepare ramp inclines ≈ 5°, 10°, 15°.  
2. Drive robot up each incline.  
3. Record max angle climbed and current draw.

**Pass Criteria:** Climbs 10° ramp smoothly.

| Angle (°) | Result | I_avg (A) | Pass/Fail | Notes |
|:-----------:|:-------:|:-----------:|:-----------:|:------|
| 5 |     |     |     |     |
| 10 |     |     |     |     |
| 15 |     |     |     |     |

---

## Test 5 – Runtime Estimation
**Purpose:** Estimate continuous operation time.

**Procedure**
1. Fully charge battery.  
2. Drive robot on level surface for 5 min at moderate speed.  
3. Measure **average current (I_avg)**.  

**Calculation:**  t_est = 12 Ah / I_avg  (hours)

| I_avg (A) | t_est (h) | Observed Runtime (h) | Notes |
|:-----------:|:----------:|:--------------------:|:------|
|     |     |     |     |

---

## Test 6 – Data Logging & Review
1. Compile all raw data and calculations.  
2. Summarize in the following table:

| Test | Parameter | Result | Pass/Fail | Notes |
|:------|:------------|:---------|:-----------:|:------|
| 1 | I_free (A) |     |     |     |
| 2 | τ_stall (N·m) |     |     |     |
| 3 | I_start (A) |     |     |     |
| 4 | Max Incline (°) |     |     |     |
| 5 | Runtime (h) |     |     |     |

---

## Conclusion Criteria
The 12.8 V 12 Ah battery is **approved for Phase 1 prototyping** if:
- Static and ramp tests pass  
- Motor current ≤ TB6612FNG safe range  
- Estimated runtime > 2 h  

If criteria fail → switch to lighter RC battery and repeat.

---

## Sign-off
| Engineer | Date | Signature |
|:----------|:------|:-----------|
| Eric Thomas |     |     |

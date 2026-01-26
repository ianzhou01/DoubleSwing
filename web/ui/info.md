# DoubleSwing — Model Notes & Implementation Details

## 1. Overview

**DoubleSwing** is an interactive, real-time simulation of a planar double pendulum with rigid rods.
The system is modeled using classical mechanics and integrated numerically at high frequency, with
additional logic to support direct user interaction (dragging the bobs) while preserving stability.

This panel explains:
- The physical model and governing equations
- The numerical integration method
- How interactive dragging is implemented
- Accuracy considerations and known limitations

The simulation continues to run in the background while this panel is open.

---

## 2. Physical Model

The system consists of:
- Two point masses $m_1, m_2$
- Two massless rigid rods of lengths $L_1, L_2$
- Motion constrained to a vertical plane
- Gravitational acceleration $g$
- A simple damping term applied to angular velocities

The configuration is fully described by the generalized coordinates:

$$
\theta_1(t), \quad \theta_2(t)
$$

where each angle is measured from the vertical.

---

## 3. Kinematics

We describe the motion using generalized coordinates  
$\theta_1(t)$ and $\theta_2(t)$, measured from the vertical.

The Cartesian positions of the two bobs are:

$$
x_1 = L_1 \sin \theta_1
$$

$$
y_1 = -L_1 \cos \theta_1
$$

$$
x_2 = x_1 + L_2 \sin \theta_2
$$

$$
y_2 = y_1 - L_2 \cos \theta_2
$$

The velocity is the time derivative of the position. Differentiating the above expressions gives:

$$
\dot{x}_1 = \dot{\theta}_1 \ L_1 \cos \theta_1
$$

$$
\dot{y}_1 = \dot{\theta}_1 \ L_1 \sin \theta_1
$$

$$
\dot{x}_2 = \dot{x}_1 + \dot{\theta}_2 \ L_2 \cos \theta_2
$$

$$
\dot{y}_2 = \dot{y}_1 + \dot{\theta}_2 \ L_2 \sin \theta_2
$$

The acceleration is the second time derivative of the position:

$$
\ddot{x}_1 = -\dot{\theta}_1^2 \ L_1 \sin \theta_1 + \ddot{\theta}_1 \ L_1 \cos \theta_1
$$

$$
\ddot{y}_1 = \dot{\theta}_1^2 \ L_1 \cos \theta_1 + \ddot{\theta}_1 \ L_1 \sin \theta_1
$$

$$
\ddot{x}_2 = \ddot{x}_1 - \dot{\theta}_2^2 \ L_2 \sin \theta_2\ + \ddot{\theta}_2 \ L_2 \cos \theta_2
$$

$$
\ddot{y}_2 = \ddot{y}_1 + \dot{\theta}_2^2 \ L_2 \cos \theta_2 + \ddot{\theta}_2 \ L_2 \sin \theta_2
$$

---

### Notes

- These expressions are used to construct the **kinetic energy** of the system.
- The accelerations are **not integrated directly** in the simulation; instead, the equations of motion for $\theta_1$ and $\theta_2$ are derived via the Euler–Lagrange equations.
- Explicit Cartesian accelerations are shown here for completeness and physical intuition.

---

## 4. Energies

### Kinetic Energy

$$
T =
\frac{1}{2} m_1 (\dot{x}_1^2 + \dot{y}_1^2)
+
\frac{1}{2} m_2 (\dot{x}_2^2 + \dot{y}_2^2)
$$

### Potential Energy

$$
V =
m_1 g y_1 + m_2 g y_2
$$

#### Normalization
In this simulation, gravitational potential energy is defined so that it is **always non-negative** and equals zero when the system is in its lowest vertical configuration.

Rather than using absolute height relative to an arbitrary world origin, we measure height *relative to each pendulum’s lowest possible position*. This avoids negative potential energy values and simplifies interpretation of energy plots.

The potential energy is computed as:

$$
V =
(m_1 + m_2)\, g\, L_1 \left( 1 - \cos \theta_1 \right)
\ + \
m_2\, g\, L_2 \left( 1 - \cos \theta_2 \right)
$$

#### Interpretation

- For each rod, the vertical height contribution is  
  $$
  \Delta h = L (1 - \cos \theta)
  $$
  which is always $\ge 0$.
- When $\theta = 0$ (pendulum hanging straight down),  
  $$
  \cos \theta = 1 \ \Rightarrow\ V = 0
  $$
- As the pendulum rises, $1 - \cos \theta$ increases smoothly, producing a monotonic increase in potential energy.



This corresponds directly to the engine code:

```cpp
const double pe = (m1 + m2)*g*l1*(1 - cos(th1)) + m2*g*l2*(1 - cos(th2));
```
---

## 5. Equations of Motion

The equations of motion are derived using the Euler–Lagrange equations:

```math
\frac{d}{dt}\left(\frac{\partial \mathcal{L}}{\partial \dot{\theta}_i}\right)
-
\frac{\partial \mathcal{L}}{\partial \theta_i}
= Q_i
```

where:
- $\mathcal{L} = T - V$
- $Q_i$ represents non-conservative generalized forces

After simplification, this yields a coupled, nonlinear system of second-order ODEs of the form:

$$
\ddot{\theta}_1 = f_1(\theta_1, \theta_2, \dot{\theta}_1, \dot{\theta}_2)
$$
$$
\ddot{\theta}_2 = f_2(\theta_1, \theta_2, \dot{\theta}_1, \dot{\theta}_2)
$$

These equations are evaluated in the engine at each integration step.

---

## 6. Damping Model

A simple linear damping term is applied to angular velocities:

$$
\dot{\theta}_i \leftarrow \dot{\theta}_i - \gamma \dot{\theta}_i
$$

This is **not** a full aerodynamic drag model and does not depend on bob radius, shape, or velocity
squared. It exists primarily to:
- Prevent unbounded energy growth from numerical error
- Allow visually stable long-running simulations

---

## 7. Numerical Integration (RK4)

The system is integrated using **fourth-order Runge–Kutta (RK4)** with a fixed timestep:

$$
\Delta t = \frac{1}{240} \text{ seconds}
$$

RK4 evaluates the derivative at four intermediate stages per step, significantly reducing local
truncation error compared to Euler or semi-implicit methods.

This choice balances:
- Stability for chaotic motion
- Performance in WebAssembly
- Responsiveness during user interaction

---

## 8. Interactive Dragging Logic

### Dragging Bob 1 (`step_drag_p1`)

When the user drags the first bob:
- $\theta_1$ is set directly from the pointer position
- Angular velocity $\dot{\theta}_1$ is estimated using a filtered finite difference
- The engine advances one step using these externally imposed values

This is **not physically equivalent** to applying a force or torque. Instead, it acts as a
constraint override designed for responsiveness and control.

### Limitations of Dragging

- Momentum is not strictly conserved during dragging
- Angular acceleration is noisy and intentionally suppressed
- Dragging bob 2 is handled more approximately by temporarily overriding state variables

This design prioritizes user feel and stability over physical purity.

---

## 9. Energy Visualization

The energy bar displays the relative proportions of kinetic and potential energy:

$$
\text{KE fraction} = \frac{T}{T + V}, \quad
\text{PE fraction} = \frac{V}{T + V}
$$

If total energy approaches zero, the bar defaults to a balanced state to avoid numerical artifacts.

---

## 10. Accuracy & Limitations

This simulation is intended as an educational and exploratory tool, not a precision physics solver.

Known limitations include (but are not limited to :p):
- No elastic rods or rod mass
- No collision handling
- Simplified damping
- Chaotic sensitivity to timestep size
- Interaction logic that breaks strict Hamiltonian structure

Still, the system captures:
- Qualitative chaotic behavior
- Energy exchange dynamics
- Sensitivity to initial conditions

---

## 11. Notes

Double pendulums are chaotic systems. Small numerical errors, small timestep changes, or
tiny interaction impulses can lead to visibly divergent trajectories over time.

This is not a bug; it is a defining feature of the physics.
= Scanline Sweeper
By: Rook&Possum

== Quadratic Bezier (`qb`)
$
  B(t) &= (c_0 + c_2 - 2c_1)t^2 + 2(c_1 - c_0)t + c_0 \
$

== Make `qb` monotonic
This preprocessing step is intended to make sure that `qb` is monotonic in both the $x$ and $y$-axis. If the bezier is not as such, then it needs to be split in such a way that it transforms into smaller bezier curves that are monotonic. The steps below find "intersection" or split point. In the equation below, $T$ represents the target y-value.

#figure(
  $
    A t^2 + B t + C &= T \
    A t^2 + B t + (C - T) &= 0 \
    A &= (c_0 + c_2 - 2c_1) \
    B &= 2(c_1 - c_0) \
    C &= c_0 - T
  $,
  caption: [Equation to find split point]
)

If $A approx 0$, the equation approximately becomes linear.
$
  B t + C &= 0 \
  t &= -C / B = (T - c_0) / (2(c_1 - c_0))
$

General solution
$
  t &= (-b plus.minus sqrt(b^2 - 4a c)) / (2a)
$
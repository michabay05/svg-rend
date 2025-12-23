// #show math.equation.where(block: true): set align(left)

= Quadratic Bezier

#line(length: 100%)

== Setup: Linear interpolation (Lerp)
Let $A$ and $B$ be the start and end points and $t$ be $RR$ $in [0, 1]$.
$
    L(A, B, t) = A + (B - A)t
$

== Quadratic Bezier (in terms of Lerp)
Let $A$ and $C$ be the start and end points, $B$ be the control point, and $t$ be $RR$ $in [0, 1]$.
$
    L_0 &= L(A, B, t) \
    L_1 &= L(B, C, t) \
    Q(A, B, C, t) &= L(L_0, L_1, t)
$

== Quadratic Bezier (explicit form)
$
    L_0 &= L(A, B, t) = A + (B - A)t \
    L_1 &= L(B, C, t) = B + (C - B)t \ \
    Q(A, B, C, t) &= L(L_0, L_1, t) \
    &= L_0 + (L_1 - L_0)t \
    &= [A + (B - A)t] + ([B + (C - B)t] - [A + (B - A)t])t \
    &= [A + B t - A t] + ([B + C t - B t] - [A + B t - A t])t \
    &= A + B t - A t + B t + C t^2 - B t^2 - A t - B t^2 + A t^2 \
    bold(Q(A, B, C, t) &= (A - 2B + C)t^2 + (2B - 2A)t + A)
$

=== Find roots using quadratic formula
$
    Q(A, B, C, t) &= (A - 2B + C)t^2 + (2B - 2A)t + A \
    a &= A - 2B + C \
    b &= 2B - 2A \
    c &= A\
    "Quadratic formula": t &= (-b plus.minus sqrt(b^2 - 4a c)) / (2a)
$

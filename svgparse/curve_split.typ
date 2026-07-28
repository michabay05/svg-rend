#set underline(offset: 3pt)
#set enum(indent: 1em)
#set list(indent: 1em)
#set math.mat(delim: "[")
#set math.vec(delim: "[")
#show link: set text(fill: blue)
#show link: underline

= Subdividing Cubic Bezier

#underline[Objective]: For a given cubic bezier, find a collection of _monotonic_ quadratic beziers that closely approximate the original bezier. 

The general subdivision process goes as follows:
- Sort $T$ in ascending order
- Deduplicate values, if needed.
- Compute the start and end control points for each curve segment
- Find the middle control point on individual segments.

== Definitions
Let $P = [p_i | 0 <= i < N]$ where $N$ is the number of control points for a given order bezier. Although the focus of this paper are cubic beziers, I included linear and quadratic beziers, just for the sake of completeness.

=== First-Order Bezier ($N = 2$)
$
    L(t, P) &= p_0 + (p_1 - p_0)t \
    &= (1-t)p_0 + t p_1
$

=== Second-Order Bezier ($N = 3$)
$
    L_0 &= L(t, [p_0, p_1]) \
    L_1 &= L(t, [p_1, p_2]) \ \

    Q (t, P) &= L(t, [L_0, L_1]) \
    &= (p_0 - 2p_1 + p_2)t^2 + (2p_1 - 2p_0)t + p_0 \ \

    Q'(t, P) &= 2(p_0 - 2p_1 + p_2)t + (2p_1 - 2p_0)
$

=== Third-Order Bezier ($N = 4$)
$
    Q_0 &= Q(t, [p_0, p_1, p_2]) \
    Q_1 &= Q(t, [p_1, p_2, p_3]) \ \

    C (t, P) &= L(t, [Q_0, Q_1]) \
    &= (1-t)^3 p_0 + 3(1-t)^2t p_1 + 3(1-t)t^2 p_2 + t^3 p_3 \
    &= p_0 - 3t p_0 + 3t^2 p_0 - t^3 p_0 + 3t p_1 - 6t^2 p_1 + 3 t^3 p_1 + 3t^2 p_2 - 3t^3 p_2 + t^3p_3 \
    &= (-p_0 + 3p_1 - 3p_2 + p_3)t^3 + (3p_0 - 6p_1 + 3p_2)t^2 + (-3p_0 + 3p_1)t + (p_0) \
    C'(t, P) &= 3(-p_0 + 3p_1 - 3p_2 + p_3)t^2 + 6(p_0 - 2p_1 + p_2)t + 3(-p_0 + p_1) \ \

    C''(t, P) &= 6(-p_0 + 3p_1 - 3p_2 + p_3)t + 6(p_0 - 2p_1 + p_2) \
$

== Inflection point
Before approximating a segments of a cubic bezier, the inflection points need to be identified as quadratic beziers are unable to replicate a cubic bezier with a quadratic bezier. Thus, the cubic bezier needs to be split at the points of inflection; then, the split segments can be approximated better with quadratic beziers.

Since inflection points take place where the curvature, $kappa$, equals zero.

$
    C(t) &= (x(t), y(t)) \
    kappa &= (x' y'' - x'' y')/((x')^2 + (y')^2)^(3"/"2) \
$
$ x' y'' - x'' y' = 0 $

Because the equations become messy, let's fold some of the constant terms into a single constant.
$
    a_i &= (u_i, v_i) \
    a_0 &= 3(-p_0 + 3p_1 - 3p_2 + p_3) \
    a_1 &= 6(p_0 - 2p_1 + p_2) \
    a_2 &= 3(-p_0 + p_1) \

    C'(t, P) &= a_0t^2 + a_1t + a_2 \
    C''(t, P) &= 2a_0t + a_1 \

$

where $u_i$ and $v_i$ represent the $x$ and $y$-components of $a_i$.

Let's plug these condensed values back in and find the solutions.
$
    x' y'' - x'' y' &= 0 \
    (u_0t^2 + u_1t + u_2)(2v_0t + v_1) - (2u_0t + u_1)(v_0t^2 + v_1t + v_2) &= 0 \
    2v_0t(u_0t^2 + u_1t + u_2) + v_1(u_0t^2 + u_1t + u_2) \
        - 2u_0t (v_0t^2 + v_1t + v_2) - u_1 (v_0t^2 + v_1t + v_2) &= 0 \
    (2v_0u_0t^3 + 2v_0u_1t^2 + 2v_0u_2t) + (v_1u_0t^2 + v_1u_1t + v_1u_2) \
        - (2u_0v_0t^3 + 2u_0v_1t^2 + 2u_0v_2t) - (u_1v_0t^2 + u_1v_1t + u_1v_2) &= 0 \
    (v_0u_1t^2 + 2v_0u_2t) + (-v_1u_0t^2 + v_1u_2) - 2u_0v_2t - u_1v_2 &= 0 \
    (v_0u_1 - v_1u_0)t^2 + (2v_0u_2 - 2u_0v_2)t + (v_1u_2 - u_1v_2) &= 0
$

Now, solve for $t$ using the quadratic equation.
$
    (v_0u_1 - v_1u_0)t^2 + (2v_0u_2 - 2u_0v_2)t + (v_1u_2 - u_1v_2) = 0 \
    t_i = (-(2v_0u_2 - 2u_0v_2) plus.minus sqrt((2v_0u_2 - 2u_0v_2)^2 - 4(v_0u_1 - v_1u_0)(v_1u_2 - u_1v_2)))/(2(v_0u_1 - v_1u_0)) \
$

The solutions above now denote exactly where on the cubic bezier the point of inflection can be found. Given the quadratic nature of the equation, the discriminant can used to determine the number of inflection points.

If two inflections points are found, there exists three quadratic beziers that can closely approximate the original cubic bezier. For one inflection point, there exist two quadratic beziers. If no inflection point exists, then there exists a single quadratic bezier that can approximate the original cubic bezier.

== Monotonicity
Since the primary objective of this paper is to split the cubic bezier into _monotonic_ quadratic beziers. In addition to splitting at points of inflection, the segments need to be split at extrema points. Splitting at these extrema points will guarantee a collection of _monotonic_ subsegments.

$
    x'(t) &= u_0t^2 + u_1t + u_2 = 0 \
    y'(t) &= v_0t^2 + v_1t + v_2 = 0 \ \

    t_x &= (-u_1 plus.minus sqrt((u_1)^2 - 4u_0u_2)) / (2 u_0) \
    t_y &= (-v_1 plus.minus sqrt((v_1)^2 - 4v_0v_2)) / (2 v_0) \
$

== Split points
$ T &= {t_i, t_x, t_y} inter (0, 1) $

The set $T$ contains the inflection points and extrema (in both axes), which fall within the open interval $(0, 1)$. Values beyond the aforementioned interval or _undefined_ values should excluded $T$.

== Middle control point
After splitting the cubic bezier into segments, finding the _on-curve_ start and end point is relatively trivial. To find the middle control point, use the Tangent Intersection method, which follows the process below:
- Compute the tangent vectors of the start and end points
- Find the intersection point of the two tangent vectors (aka. rays)

$
    "Start ray"&: alpha_0 + alpha t \
    "End ray"&: beta_0 + beta s \

    alpha_(x 0) + alpha_x t &= beta_(x 0) + beta_x s \
    alpha_(y 0) + alpha_y t &= beta_(y 0) + beta_y s \ \

    alpha_x t  - beta_x s &= beta_(x 0) - alpha_(x 0) \
    alpha_y t  - beta_y s &= beta_(y 0) - alpha_(y 0) \ \

    mat(alpha_x, -beta_x; alpha_y, -beta_y)vec(t, s) &= mat(
        beta_(x 0) - alpha_(x 0);
        beta_(y 0) - alpha_(y 0)
    ) \
    vec(t, s) &= mat(alpha_x, -beta_x; alpha_y, -beta_y)^(-1) mat(
        beta_(x 0) - alpha_(x 0);
        beta_(y 0) - alpha_(y 0)
    ) \ \

    q_m &= alpha_0 + t alpha = beta_0 + s beta
$

The point of intersection $q_m$ will then be used as the middle control point.

// #link("https://www.desmos.com/calculator/tdbgepxkzs")[Unpolished desmos playground]

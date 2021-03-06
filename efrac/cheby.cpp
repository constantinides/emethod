#include "cheby.h"

void changeOfVariable(std::vector<mpfr::mpreal> &out,
                      std::vector<mpfr::mpreal> const &in,
                      std::pair<mpfr::mpreal, mpfr::mpreal> const &type)
{
    using mpfr::mpreal;
    out = in;
    for (std::size_t i = 0u; i < in.size(); ++i)
        out[i] = fma((type.second - type.first) / 2, in[i],
                     (type.second + type.first) / 2);
}

void evaluateClenshaw(mpfr::mpreal &result, std::vector<mpfr::mpreal> &p,
                      mpfr::mpreal &x,
                      std::pair<mpfr::mpreal, mpfr::mpreal> const &type)
{
    using mpfr::mpreal;
    mpreal bn1, bn2, bn;
    mpreal buffer;

    bn1 = 0;
    bn2 = 0;

    // compute the value of (2*x - b - a)/(b - a) in the temporary
    // variable buffer
    buffer = (x * 2 - type.second - type.first) / (type.second - type.first);

    int n = (int)p.size() - 1;
    for (int k = n; k >= 0; --k)
    {
        bn = buffer * 2;
        bn = bn * bn1 - bn2 + p[k];
        // update values
        bn2 = bn1;
        bn1 = bn;
    }

    // set the value for the result (line 8 which outputs the value
    // of the CI at x)
    result = bn1 - buffer * bn2;
}

void evaluateClenshaw(mpfr::mpreal &result, std::vector<mpfr::mpreal> &p,
                      mpfr::mpreal &x)
{
    using mpfr::mpreal;
    mpreal bn1, bn2, bn;

    int n = (int)p.size() - 1;
    bn2 = 0;
    bn1 = p[n];
    for (int k = n - 1; k >= 1; --k)
    {
        bn = x * 2;
        bn = bn * bn1 - bn2 + p[k];
        // update values
        bn2 = bn1;
        bn1 = bn;
    }

    result = x * bn1 - bn2 + p[0];
}

void evaluateClenshaw2ndKind(mpfr::mpreal &result, std::vector<mpfr::mpreal> &p,
                             mpfr::mpreal &x)
{
    using mpfr::mpreal;
    mpreal bn1, bn2, bn;

    int n = (int)p.size() - 1;
    bn2 = 0;
    bn1 = p[n];
    for (int k = n - 1; k >= 1; --k)
    {
        bn = x * 2;
        bn = bn * bn1 - bn2 + p[k];
        // update values
        bn2 = bn1;
        bn1 = bn;
    }

    result = (x << 1) * bn1 - bn2 + p[0];
}

void generateEquidistantNodes(std::vector<mpfr::mpreal> &v, std::size_t n)
{
    using mpfr::mpreal;
    mpreal pi = mpfr::const_pi();

    v.resize(n);
    // store the points in the vector v as v[i] = i * pi / (n-1)
    for (int i = 0; i < n; ++i)
    {
        v[n - 1 - i] = pi * i;
        v[n - 1 - i] /= (n - 1);
    }
}

void generateChebyshevPoints(std::vector<mpfr::mpreal> &x, std::size_t n)
{
    using mpfr::mpreal;
    generateEquidistantNodes(x, n);
    for (std::size_t i{0}; i < n; ++i)
    {
        x[i] = mpfr::cos(x[i]);
    }
}

void generateChebyshevCoefficients(std::vector<mpfr::mpreal> &c,
                                   std::vector<mpfr::mpreal> &fv, std::size_t n)
{
    using mpfr::mpreal;
    std::vector<mpreal> v(n + 1);
    generateEquidistantNodes(v, n + 1);

    mpreal buffer;

    // halve the first and last coefficients
    mpfr::mpreal oldValue1 = fv[0];
    mpfr::mpreal oldValue2 = fv[n];
    fv[0] /= 2;
    fv[n] /= 2;

    for (std::size_t i = 0u; i <= n; ++i)
    {
        buffer = mpfr::cos(v[i]); // compute the actual value at the Chebyshev
        // node cos(i * pi / n)

        evaluateClenshaw(c[i], fv, buffer); // evaluate the current coefficient
        // using Clenshaw
        if (i == 0u || i == n)
        {
            c[i] /= n;
        }
        else
        {
            c[i] <<= 1;
            c[i] /= n;
        }
    }
    fv[0] = oldValue1;
    fv[n] = oldValue2;
}

// function that generates the coefficients of the derivative of a given CI
void derivativeCoefficients1stKind(std::vector<mpfr::mpreal> &derivC,
                                   std::vector<mpfr::mpreal> &c)
{
    using mpfr::mpreal;
    int n = (int)c.size() - 2;
    derivC[n] = c[n + 1] * (2 * (n + 1));
    derivC[n - 1] = c[n] * (2 * n);
    for (int i = n - 2; i >= 0; --i)
    {
        derivC[i] = 2 * (i + 1);
        derivC[i] = fma(derivC[i], c[i + 1], derivC[i + 2]);
    }
    derivC[0] >>= 1;
}

// use the formula (T_n(x))' = n * U_{n-1}(x)
void derivativeCoefficients2ndKind(std::vector<mpfr::mpreal> &derivC,
                                   std::vector<mpfr::mpreal> &c)
{
    std::size_t n = c.size() - 1;
    for (std::size_t i = n; i > 0u; --i)
        derivC[i - 1] = c[i] * i;
}

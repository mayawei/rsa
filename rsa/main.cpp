#include<stdio.h>
#include<stdlib.h>
#include<openssl/bn.h>
#include<iostream>
BIGNUM * rsa_en(BIGNUM * m, BIGNUM * e, BIGNUM * n)
{
	BIGNUM * ret = BN_new();
	BN_one(ret);
	BIGNUM * b = BN_new();
	BN_copy(b, m);
	BN_CTX * ctx = BN_CTX_new();
	for (int i = 0; i < BN_num_bits(e); i++)
	{
		if (BN_is_bit_set(e, i))
		{
			BN_mod_mul(ret, ret, b, n, ctx);
			BN_mod_mul(b, b, b, n, ctx);
		}
		else
		{
			BN_mod_mul(b, b, b, n, ctx);
		}
	}
	BN_CTX_free(ctx);
	BN_free(b);
	return ret;
}

int Miller(BIGNUM * bn, int rounds)
{
	BIGNUM *temp = BN_new();
	BN_zero(temp);
	BN_add_word(temp, 2);
	if (!BN_cmp(bn, temp))
	{
		BN_free(temp);
		return 1;
	}
	if (!BN_cmp(bn, BN_value_one()) || !BN_is_odd(bn))
	{
		BN_free(temp);
		return 0;
	}
	BN_copy(temp, bn);
	BN_sub_word(temp, 1);
	int t;
	for (t = 0; !BN_is_bit_set(temp, 0); t++)
	{
		BN_rshift1(temp , temp);
	}
	BIGNUM* a = BN_new();
	BIGNUM* x = BN_new();
	BIGNUM* y = BN_new();
	BN_CTX* ctx = BN_CTX_new();
	BIGNUM* _n = BN_new();
	BN_copy(_n, bn);
	BN_sub_word(_n, 1);
	for (int i = 0; i < rounds; i++)
	{
		BN_rand_range(a, bn);
		x = rsa_en(a, temp,    bn);
		for (int j = 0; j < t; j++)
		{
			BN_mod_mul(y, x, x, bn, ctx);
			if (!BN_cmp(y, BN_value_one()) && BN_cmp(x, BN_value_one()) && BN_cmp(x, _n))
			{
				BN_free(a);
				BN_free(temp);
				BN_free(x);
				BN_free(y);
				BN_free(_n);
				BN_CTX_free(ctx);
				return 0;
			}
			BN_copy(x, y);
		}
		if (BN_cmp(y, BN_value_one()))
		{
			BN_free(a);
			BN_free(temp);
			BN_free(x);
			BN_free(y);
			BN_free(_n);
			BN_CTX_free(ctx);
			return 0;
		}
	}
	BN_free(a);
	BN_free(temp);
	BN_free(x);
	BN_free(y);
	BN_free(_n);
	BN_CTX_free(ctx);
	return 1;
}

BIGNUM * generateprime(int size)
{
	BIGNUM * x = BN_new();
	while (1)
	{
		BN_rand(x, size, 1, 1);
		if (Miller(x, 20))
		{
			break;
		}
	}
	return x;
};



BIGNUM * MontMod(BIGNUM * a, BIGNUM * b, BIGNUM * n)
{
	BN_CTX * ctx = BN_CTX_new();
	BIGNUM* t = BN_new();
	BIGNUM* m = BN_new();
	BIGNUM* r = BN_new();
	BIGNUM* n_1 = BN_new();
	BIGNUM* n2 = BN_new();
	BIGNUM* u = BN_new();
	BIGNUM* a2 = BN_new();
	int s = BN_num_bits(n);
	BN_zero(r);
	BN_set_bit(r, s);
	BN_lshift(a2, a, s);
	BN_mod(a2, a2, n, ctx);
	BN_mul(t, a2, b, ctx);
	BN_mod_inverse(n_1, n, r, ctx);
	BN_sub(n2, r, n_1);
	BN_mul(m, t, n2, ctx);
	int j = BN_num_bits(m);
	for (int i = s; i < j; i++)
	{
		BN_clear_bit(m, i);
	}
	BN_mul(u, m, n, ctx);
	BN_add(u, u, t);
	BN_rshift(u, u, s);

	if (BN_cmp(u, n) >= 0)
	{
		BN_sub(r, u, n);
	}
	else
	{
		BN_copy(r, u);
	}
	BN_free(t);
	BN_free(u);
	BN_free(m);
	BN_CTX_free(ctx);
	return r;
}

BIGNUM * rsa_en_mon(BIGNUM * m, BIGNUM * e, BIGNUM * n)
{
	BIGNUM * ret = BN_new();
	BN_one(ret);
	BIGNUM * b = BN_new();
	BN_copy(b, m);
	BN_CTX * ctx = BN_CTX_new();
	for (int i = 0; i < BN_num_bits(e); i++)
	{
		if (BN_is_bit_set(e, i))
		{
			ret = MontMod(ret, b, n);
			b = MontMod(b, b, n);
		}
		else
		{
			b = MontMod(b, b, n);
		}
	}
	BN_CTX_free(ctx);
	BN_free(b);
	return ret;
}


BIGNUM * Ch_remainder(BIGNUM * p, BIGNUM * q, BIGNUM * a, BIGNUM * e, BIGNUM * n)
{
	BIGNUM* q1 = BN_new();
	BIGNUM* p1 = BN_new();
	BN_CTX* ctx = BN_CTX_new();
	BN_mod_inverse(q1, q, p, ctx);
	BN_mod_inverse(p1, p, q, ctx);
	BIGNUM* a1 = rsa_en(a, e, p);
	BIGNUM* a2 = rsa_en(a, e, p);
	BIGNUM* x1 = BN_new();
	BIGNUM* x2 = BN_new();
	BN_mod_mul(x1, a1, q, n, ctx);
	BN_mod_mul(x2, a2, p, n, ctx);
	BN_mod_mul(x1, x1, q1, n, ctx);
	BN_mod_mul(x2, x2, p1, n, ctx);
	BIGNUM* ret = BN_new();
	BN_mod_add(ret, x1, x2, n, ctx);
	BN_free(x1);
	BN_free(x2);
	BN_free(q1);
	BN_free(p1);
	BN_free(a1);
	BN_free(a2);
	return ret;
}


BIGNUM * Ch_remainder_mon(BIGNUM * p, BIGNUM * q, BIGNUM * a, BIGNUM * e, BIGNUM * n)
{
	BIGNUM* q1 = BN_new();
	BIGNUM* p1 = BN_new();
	BN_CTX* ctx = BN_CTX_new();
	BN_mod_inverse(q1, q, p, ctx);
	BN_mod_inverse(p1, p, q, ctx);
	BIGNUM* a1 = rsa_en_mon(a, e, p);
	BIGNUM* a2 = rsa_en_mon(a, e, p);
	BIGNUM* x1 = BN_new();
	BIGNUM* x2 = BN_new();
	x1 = MontMod(a1, q, n);
	x2 = MontMod(a2, p, n);
	x1 = MontMod(x1, q1, n);
	x2 = MontMod(x2, p1, n);
	BIGNUM* ret = BN_new();
	BN_mod_add(ret, x1, x2, n, ctx);
	BN_free(x1);
	BN_free(x2);
	BN_free(q1);
	BN_free(p1);
	BN_free(a1);
	BN_free(a2);
	return ret;
}



int main()
{
	char input[] = "abcc12321424ac";
	BIGNUM *e = BN_new();
	BN_zero(e);
	BN_add_word(e, 65537);
	BIGNUM *m = BN_new();
	BN_hex2bn(&m, input);
	BIGNUM *c = BN_new();
	BIGNUM *n = BN_new();
	BIGNUM *d = BN_new();
	BIGNUM *p = BN_new();
	BIGNUM *p1 = BN_new();
	BIGNUM *q = BN_new();
	BIGNUM *q1 = BN_new();
	BIGNUM *mid = BN_new();
	BIGNUM *phi = BN_new();
	BN_CTX *ctx = BN_CTX_new();
	p = generateprime(1024);
	q = generateprime(1024);
	//BN_generate_prime(p, 1024, 1, NULL, NULL, NULL, NULL);
	//BN_generate_prime(q, 1024, 1, NULL, NULL, NULL, NULL);
	BN_copy(p1, p);
	BN_copy(q1, q);
	BN_sub_word(p1, 1);
	BN_sub_word(q1, 1);
	BN_mul(phi, p1, q1, ctx);
	BN_free(p1);
	BN_free(q1);
	BN_mul(n, p, q, ctx);
	BN_mod_inverse(d, e, phi, ctx);
	c = rsa_en(m, e, n);

	struct timespec tmv1, tmv2, tmv3, tmv4;



	timespec_get(&tmv1, 1);
	m = Ch_remainder(p, q, c, d, n);
	timespec_get(&tmv2, 1);
	double tb = (tmv2.tv_nsec - tmv1.tv_nsec)*0.000000001+ tmv2.tv_sec - tmv1.tv_sec;



	timespec_get(&tmv3, 1);
	m = Ch_remainder_mon(p, q, c, d, n);
	timespec_get(&tmv4, 1);
	double tb2 = (tmv4.tv_nsec - tmv3.tv_nsec)*0.000000001 + tmv4.tv_sec - tmv3.tv_sec;




	std::cout << BN_bn2hex(m) << std::endl<<"�Դ�:"<<tb<<"s"<<std::endl<<"�ɸ�����"<<tb2<<"s"<<std::endl;
	//BN_mod_exp(m, c, d, n, ctx);
	BN_free(p);
	BN_free(q);
	BN_free(phi);



	return 0;
}
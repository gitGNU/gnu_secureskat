/*******************************************************************************
   This file is part of libTMCG.

 Copyright (C) 2004 Heiko Stamer, <stamer@gaos.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*******************************************************************************/

#ifndef INCLUDED_TMCG_PublicKey_HH
	#define INCLUDED_TMCG_PublicKey_HH

	// config.h
	#if HAVE_CONFIG_H
		#include "config.h"
	#endif

	// C++/STL header
	#include <cstdio>
	#include <cstdlib>
	#include <cassert>
	#include <string>
	#include <sstream>
	#include <iostream>
	#include <vector>
	
	// GNU multiple precision library
	#include <gmp.h>

	#include "mpz_srandom.h"
	#include "parse_helper.hh"

struct TMCG_PublicKey
{
	string							name, email, type, nizk, sig;
	mpz_t							m, y;
	
	TMCG_PublicKey
		()
	{
		mpz_init(m), mpz_init(y);
	}
	
	TMCG_PublicKey
		(const TMCG_SecretKey &skey):
			name(skey.name), email(skey.email), type(skey.type),
			nizk(skey.nizk), sig(skey.sig)
	{
		mpz_init_set(m, skey.m);
		mpz_init_set(y, skey.y);
	}
	
	bool check
		()
	{
		mpz_t foo, bar;
		string s = nizk;
		size_t stage1_size = 0, stage2_size = 0, stage3_size = 0;
		size_t mnsize = mpz_sizeinbase(m, 2L) / 8;
		char *ec, *mn = new char[mnsize];
		
		mpz_init(foo), mpz_init(bar);
		try
		{
			// sanity check, whether y \in Z^\circ
			if (mpz_jacobi(y, m) != 1)
				throw false;
			
			// sanity check, whether m \in ODD (odd numbers)
			if (!mpz_odd_p(m))
				throw false;
			
			// sanity check, whether m \not\in P (prime)
			// (here is a very small probability of false-negativ behaviour,
			// FIX: give a short witness in public key)
			if (mpz_probab_prime_p(m, 500))
				throw false;
			
			// check self-signature
			ostringstream data;
			data << name << "|" << email << "|" << type << "|" << m << "|" <<
				y << "|" << nizk << "|";
			if (!verify(data.str(), sig))
				throw false;
			
			// check, whether m \not\in FP (fermat primes: m = 2^k + 1)
			mpz_set(foo, m);
			mpz_sub_ui(foo, foo, 1L);
			unsigned long int k = mpz_sizeinbase(m, 2L);
			mpz_set_ui(bar, 2L);
			mpz_pow_ui(bar, bar, k);
			if (!mpz_cmp(foo, bar))
			{
				// check, whether k is power of two
				mpz_set_ui(foo, k);
				unsigned long int l = mpz_sizeinbase(foo, 2L);
				mpz_set_ui(bar, 2L);
				mpz_pow_ui(bar, bar, l);
				if (!mpz_cmp(foo, bar))
				{
					// check, whether m is not equal to 5L
					if (!mpz_cmp_ui(m, 5L))
						throw false;
					
					// check, whether 5^2^(k/2) \equiv -1 (mod m) [Pepin's prime test]
					mpz_set_ui(foo, 2L);
					mpz_powm_ui(foo, foo, (k / 2), m);
					mpz_set_ui(bar, 5L);
					mpz_powm(foo, bar, foo, m);
					mpz_set_si(bar, -1L);
					if (mpz_congruent_p(foo, bar, m))
						throw false;
				}
			}
			
			// check magic of NIZK
			if (!cm(s, "nzk", '^'))
				throw false;
			
			// initalize NIZK proof input
			ostringstream input;
			input << m << "^" << y;
			
			// get security parameter of STAGE1
			if (gs(s, '^') == NULL)
				throw false;
			stage1_size = strtoul(gs(s, '^'), &ec, 10);
			if ((*ec != '\0') || (stage1_size <= 0) || (!nx(s, '^')))
				throw false;
			
			// check security constraint of STAGE1
			if (stage1_size < nizk_stage1)
				throw false;
			
			// STAGE1: m is Square Free
			for (size_t i = 0; i < stage1_size; i++)
			{
				// common random number foo \in Z^*_m (build from hash function g)
				do
				{
					g(mn, mnsize, (input.str()).c_str(), (input.str()).length());
					mpz_import(foo, 1, -1, mnsize, 1, 0, mn);
					mpz_mod(foo, foo, m);
					mpz_gcd(bar, foo, m);
					input << foo;
				}
				while (mpz_cmp_ui(bar, 1L));
				
				// read NIZK proof
				if (gs(s, '^') == NULL)
					throw false;
				if ((mpz_set_str(bar, gs(s, '^'), TMCG_MPZ_IO_BASE) < 0) || (!nx(s, '^')))
					throw false;
				
				// check, whether bar^m mod m is equal to foo
				mpz_powm(bar, bar, m, m);
				if (mpz_cmp(foo, bar))
					throw false;
			}
			
			// get security parameter of STAGE2
			if (gs(s, '^') == NULL)
				throw false;
			stage2_size = strtoul(gs(s, '^'), &ec, 10);
			if ((*ec != '\0') || (stage2_size <= 0) || (!nx(s, '^')))
				throw false;
			
			// check security constraint of STAGE2
			if (stage2_size < nizk_stage2)
				throw false;
			
			// STAGE2: m is Prime Power Product
			for (size_t i = 0; i < stage2_size; i++)
			{
				// common random number foo \in Z^*_m (build from hash function g)
				do
				{
					g(mn, mnsize, (input.str()).c_str(), (input.str()).length());
					mpz_import(foo, 1, -1, mnsize, 1, 0, mn);
					mpz_mod(foo, foo, m);
					mpz_gcd(bar, foo, m);
					input << foo;
				}
				while (mpz_cmp_ui(bar, 1L));
				
				// read NIZK proof
				if (gs(s, '^') == NULL)
					throw false;
				if ((mpz_set_str(bar, gs(s, '^'), TMCG_MPZ_IO_BASE) < 0) || (!nx(s, '^')))
					throw false;
				
				// check, whether bar^2 \equiv +-foo or \equiv +-2foo (mod m)
				mpz_mul(bar, bar, bar);
				mpz_mod(bar, bar, m);
				if (!mpz_congruent_p(bar, foo, m))
				{
					mpz_neg(foo, foo);
					if (!mpz_congruent_p(bar, foo, m))
					{
						mpz_mul_2exp(foo, foo, 1L);
						if (!mpz_congruent_p(bar, foo, m))
						{
							mpz_neg(foo, foo);
							if (!mpz_congruent_p(bar, foo, m))
								throw false;
						}
					}
				}
			}
			
			// get security parameter of STAGE3
			if (gs(s, '^') == NULL)
				throw false;
			stage3_size = strtoul(gs(s, '^'), &ec, 10);
			if ((*ec != '\0') || (stage3_size <= 0) || (!nx(s, '^')))
				throw false;
			
			// check security constraint of STAGE3
			if (stage3_size < nizk_stage3)
				throw false;
			
			// STAGE3: y \in NQR^\circ_m
			for (size_t i = 0; i < stage3_size; i++)
			{
				// common random number foo \in Z^\circ_m (build from hash function g)
				do
				{
					g(mn, mnsize, (input.str()).c_str(), (input.str()).length());
					mpz_import(foo, 1, -1, mnsize, 1, 0, mn);
					mpz_mod(foo, foo, m);
					input << foo;
				}
				while (mpz_jacobi(foo, m) != 1);
				
				// read NIZK proof
				if (gs(s, '^') == NULL)
					throw false;
				if ((mpz_set_str(bar, gs(s, '^'), TMCG_MPZ_IO_BASE) < 0) || (!nx(s, '^')))
					throw false;
				
				// check congruence [Goldwasser-Micali NIZK proof for NQR]
				mpz_mul(bar, bar, bar);
				mpz_mod(bar, bar, m);
				if (!mpz_congruent_p(bar, foo, m))
				{
					mpz_mul(foo, foo, y);
					mpz_mod(foo, foo, m);
					if (!mpz_congruent_p(bar, foo, m))
						throw false;
				}
			}
			
			// finish
			throw true;
		}
		catch (bool return_value)
		{
			delete [] mn;
			mpz_clear(foo), mpz_clear(bar);
			return return_value;
		}
	}
	
	string selfid
		()
	{
		string s = sig;
		
		// maybe a self signature
		if (s == "")
			return string("SELFSIG-SELFSIG-SELFSIG-SELFSIG-SELFSIG-SELFSIG");
		
		// check magic
		if (!cm(s, "sig", '|'))
			return string("NULL");
		
		// skip the keyID
		if (!nx(s, '|'))
			return string("NULL");
		
		// get the sigID
		return string(gs(s, '|'));
	}
	
	string keyid
		()
	{
		ostringstream data;
		string tmp = selfid();
		
		data << "ID" << TMCG_KeyIDSize << "^" << tmp.substr(tmp.length() -
			((TMCG_KeyIDSize < tmp.length()) ? TMCG_KeyIDSize : tmp.length()),
			(TMCG_KeyIDSize < tmp.length()) ? TMCG_KeyIDSize : tmp.length());
		return data.str();
	}
	
	string sigid
		(string s)
	{
		// check magic
		if (!cm(s, "sig", '|'))
			return string("NULL");
		
		// get the keyID
		return string(gs(s, '|'));
	}
	
	bool import
		(string s)
	{
		try
		{
			// check magic
			if (!cm(s, "pub", '|'))
				throw false;
			
			// name
			name = gs(s, '|');
			if ((gs(s, '|') == NULL) || (!nx(s, '|')))
				throw false;
			
			// email
			email = gs(s, '|');
			if ((gs(s, '|') == NULL) || (!nx(s, '|')))
				throw false;
			
			// type
			type = gs(s, '|');
			if ((gs(s, '|') == NULL) || (!nx(s, '|')))
				throw false;
			
			// m
			if ((mpz_set_str(m, gs(s, '|'), TMCG_MPZ_IO_BASE) < 0) || (!nx(s, '|')))
				throw false;
			
			// y
			if ((mpz_set_str(y, gs(s, '|'), TMCG_MPZ_IO_BASE) < 0) || (!nx(s, '|')))
				throw false;
			
			// NIZK
			key.nizk = gs(s, '|');
			if ((gs(s, '|') == NULL) || (!nx(s, '|')))
				throw false;
			
			// sig
			sig = s;
			
			throw true;
		}
		catch (bool return_value)
		{
			return return_value;
		}
	}
	
	string encrypt
		(const string &value)
	{
		mpz_t vdata;
		size_t rabin_s2 = 2 * rabin_s0;
		size_t rabin_s1 = (mpz_sizeinbase(key.m, 2L) / 8) - rabin_s2;
		
		assert(rabin_s2 < (mpz_sizeinbase(key.m, 2L) / 16));
		assert(rabin_s2 < rabin_s1);
		assert(rabin_s0 < (mpz_sizeinbase(key.m, 2L) / 32));
		
		char *r = new char[rabin_s1];
		gcry_randomize((unsigned char*)r, rabin_s1, GCRY_STRONG_RANDOM);
		
		char *Mt = new char[rabin_s2], *g12 = new char[rabin_s2];
		memcpy(Mt, value, rabin_s0), memset(Mt + rabin_s0, 0, rabin_s0);
		g(g12, rabin_s2, r, rabin_s1);
		
		for (size_t i = 0; i < rabin_s2; i++)
			Mt[i] ^= g12[i];
		
		char *yy = new char[rabin_s2 + rabin_s1];
		memcpy(yy, Mt, rabin_s2), memcpy(yy + rabin_s2, r, rabin_s1);
		mpz_init(vdata);
		mpz_import(vdata, 1, -1, rabin_s2 + rabin_s1, 1, 0, yy);
		delete [] yy, delete [] g12, delete [] Mt, delete [] r;
		
		// apply RABIN function vdata = vdata^2 mod m
		mpz_mul(vdata, vdata, vdata);
		mpz_mod(vdata, vdata, m);
		
		ostringstream ost;
		ost << "enc|" << keyid() << "|" << vdata << "|";
		mpz_clear(vdata);
		
		return ost.str();
	}
	
	bool verify
		(const string &data, string s)
	{
		mpz_t foo;
		
		mpz_init(foo);
		try
		{
			// check magic
			if (!cm(s, "sig", '|'))
				throw false;
			
			// check keyID
			if (!cm(s, keyid().c_str(), '|'))
				throw false;
			
			// value
			if ((mpz_set_str(foo, gs(s, '|'), TMCG_MPZ_IO_BASE) < 0) || (!nx(s, '|')))
				throw false;
			
			// verify signature
			size_t mdsize = gcry_md_get_algo_dlen(TMCG_GCRY_MD_ALGO);
			size_t mnsize = mpz_sizeinbase(m, 2L) / 8;
			
			assert(mpz_sizeinbase(m, 2L) > (mnsize * 8));
			assert(mnsize > (mdsize + rabin_k0));
			
			mpz_mul(foo, foo, foo);
			mpz_mod(foo, foo, m);
			
			char *w = new char[mdsize], *r = new char[rabin_k0];
			char *gamma = new char[mnsize - mdsize - rabin_k0];
			char *yy = new char[mnsize + 1024];
			size_t cnt = 1;
			mpz_export(yy, &cnt, -1, mnsize, 1, 0, foo);
			memcpy(w, yy, mdsize);
			memcpy(r, yy + mdsize, rabin_k0);
			memcpy(gamma, yy + mdsize + rabin_k0, mnsize - mdsize - rabin_k0);
			
			char *g12 = new char[mnsize];
			g(g12, mnsize - mdsize, w, mdsize);
			
			for (size_t i = 0; i < rabin_k0; i++)
				r[i] ^= g12[i];
			
			char *Mr = new char[data.length() + rabin_k0];
			memcpy(Mr, data.c_str(), data.length());
			memcpy(Mr + data.length(), r, rabin_k0);
			
			char *w2 = new char[mdsize];
			h(w2, Mr, data.length() + rabin_k0);
			
			bool ok = (memcmp(w, w2, mdsize) == 0) && 
				(memcmp(gamma, g12 + rabin_k0, mnsize - mdsize - rabin_k0) == 0);
			delete [] yy, delete [] w, delete [] r, delete [] gamma, 
				delete [] g12, delete [] Mr, delete [] w2;
			
			throw ok;
		}
		catch (bool return_value)
		{
			mpz_clear(foo);
			return return_value;
		}
	}
	
	~TMCG_PublicKey
		()
	{
		mpz_clear(m), mpz_clear(y);
	}
};

friend ostream& operator<< 
	(ostream &out, const TMCG_PublicKey &key)
{
	return out << "pub|" << key.name << "|" << key.email << "|" << key.type <<
		"|" << key.m << "|" << key.y << "|" << key.nizk << "|" << key.sig;
}

#endif

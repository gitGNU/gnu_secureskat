/*******************************************************************************
   This file is part of libTMCG.

 Copyright (C) 2002-2004 Heiko Stamer, <stamer@gaos.org>

   libTMCG is free software; you can redistribute it and/or modify
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

#ifndef INCLUDED_mpz_shash_H
	#define INCLUDED_mpz_shash_H

void mpz_shash
	(mpz_ptr r, mpz_srcptr a1, mpz_srcptr a2, mpz_srcptr a3)
{
	std::string c_tmp;
	char *vtmp = NULL, *digest = NULL, *hex_digest = NULL;
	unsigned int hash_size = gcry_md_get_algo_dlen(GCRY_MD_RMD160);
	size_t max_size = mpz_sizeinbase(a1, 16);
	max_size = std::max(max_size, mpz_sizeinbase(a2, 16));
	max_size = std::max(max_size, mpz_sizeinbase(a3, 16));
	vtmp = new char[2 * max_size + 1];
	digest = new char[hash_size];
	hex_digest = new char[2 * hash_size + 1];
	
	/* concatenate arguments */
	c_tmp += mpz_get_str(vtmp, 16, a1);
	c_tmp += "|";
	c_tmp += mpz_get_str(vtmp, 16, a2);
	c_tmp += "|";
	c_tmp += mpz_get_str(vtmp, 16, a3);
	c_tmp += "|";
	
	/* hash arguments */
	gcry_md_hash_buffer(GCRY_MD_RMD160, digest, c_tmp.c_str(), c_tmp.length());
	
	/* convert digest to hex */
	for (unsigned int i = 0; i < hash_size; i++)
		snprintf(hex_digest + (2 * i), 3, "%02x", (unsigned char)digest[i]);
	
	/* import hex string */
	mpz_set_str(r, hex_digest, 16);
	
	delete [] vtmp, delete [] digest, delete [] hex_digest;
}

void mpz_shash
	(mpz_ptr r, mpz_srcptr a1, mpz_srcptr a2, mpz_srcptr a3, mpz_srcptr a4,
	mpz_srcptr a5, mpz_srcptr a6)
{
	std::string c_tmp;
	char *vtmp = NULL, *digest = NULL, *hex_digest = NULL;
	unsigned int hash_size = gcry_md_get_algo_dlen(GCRY_MD_RMD160);
	size_t max_size = mpz_sizeinbase(a1, 16);
	max_size = std::max(max_size, mpz_sizeinbase(a2, 16));
	max_size = std::max(max_size, mpz_sizeinbase(a3, 16));
	max_size = std::max(max_size, mpz_sizeinbase(a4, 16));
	max_size = std::max(max_size, mpz_sizeinbase(a5, 16));
	max_size = std::max(max_size, mpz_sizeinbase(a6, 16));
	vtmp = new char[2 * max_size + 1];
	digest = new char[hash_size];
	hex_digest = new char[2 * hash_size + 1];
	
	/* concatenate arguments */
	c_tmp += mpz_get_str(vtmp, 16, a1);
	c_tmp += "|";
	c_tmp += mpz_get_str(vtmp, 16, a2);
	c_tmp += "|";
	c_tmp += mpz_get_str(vtmp, 16, a3);
	c_tmp += "|";
	c_tmp += mpz_get_str(vtmp, 16, a4);
	c_tmp += "|";
	c_tmp += mpz_get_str(vtmp, 16, a5);
	c_tmp += "|";
	c_tmp += mpz_get_str(vtmp, 16, a6);
	c_tmp += "|";
	
	/* hash arguments */
	gcry_md_hash_buffer(GCRY_MD_RMD160, digest, c_tmp.c_str(), c_tmp.length());
	
	/* convert digest to hex */
	for (unsigned int i = 0; i < hash_size; i++)
		snprintf(hex_digest + (2 * i), 3, "%02x", (unsigned char)digest[i]);
	
	/* import hex string */
	mpz_set_str(r, hex_digest, 16);
	
	delete [] vtmp, delete [] digest, delete [] hex_digest;
}

#endif
// This file is part of SCIL.
//
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with SCIL.  If not, see <http://www.gnu.org/licenses/>.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <assert.h>

static uint8_t get_needed_bit_count(const double min_value, const double max_value, const double absolute_tolerance){

    assert(max_value > min_value);
    assert(absolute_tolerance > 0);

    return (uint8_t)ceil(log2(1 + (max_value - min_value) / (2 * absolute_tolerance)));
}

static size_t round_up_byte(const size_t bits){

    uint8_t a = bits % 8;
    if(a == 0)
        return bits / 8;
    return 1 + (bits - a) / 8;
}

static uint64_t int_repres(const double num, const double min, const double absolute_tolerance){

    assert(num >= min);

    return (uint64_t)round((num - min) / (2 * absolute_tolerance));
}

static double double_repres(const uint64_t num, const double min, const double absolute_tolerance){

    return min + (double)num * 2 * absolute_tolerance;
}

// Testing-----------------------------------------

static int get_needed_bit_count_test(){

    int failed = 0;

    failed += 1 != get_needed_bit_count(1, 2, 0.5);
    failed += 2 != get_needed_bit_count(1, 3, 0.5);
    failed += 2 != get_needed_bit_count(1, 4, 0.5);
    failed += 3 != get_needed_bit_count(1, 5, 0.5);
    failed += 3 != get_needed_bit_count(1, 8, 0.5);
    failed += 4 != get_needed_bit_count(1, 9, 0.5);
    failed += 4 != get_needed_bit_count(1, 16, 0.5);
    failed += 5 != get_needed_bit_count(1, 17, 0.5);
    failed += 5 != get_needed_bit_count(1, 32, 0.5);
    failed += 6 != get_needed_bit_count(1, 33, 0.5);
    failed += 6 != get_needed_bit_count(1, 64, 0.5);
    failed += 7 != get_needed_bit_count(1, 65, 0.5);
    failed += 7 != get_needed_bit_count(1, 128, 0.5);
    failed += 8 != get_needed_bit_count(1, 129, 0.5);
    failed += 8 != get_needed_bit_count(1, 256, 0.5);
    failed += 9 != get_needed_bit_count(1, 257, 0.5);
    failed += 9 != get_needed_bit_count(1, 512, 0.5);
    failed += 10 != get_needed_bit_count(1, 513, 0.5);
    failed += 10 != get_needed_bit_count(1, 1024, 0.5);
    failed += 11 != get_needed_bit_count(1, 1025, 0.5);
    failed += 11 != get_needed_bit_count(1, 2048, 0.5);
    failed += 12 != get_needed_bit_count(1, 2049, 0.5);
    failed += 12 != get_needed_bit_count(1, 4096, 0.5);
    failed += 13 != get_needed_bit_count(1, 4097, 0.5);
    failed += 13 != get_needed_bit_count(1, 8192, 0.5);

    return failed;
}

static int round_up_byte_test(){

    int failed = 0;

    failed += 0 != round_up_byte(0);
    failed += 1 != round_up_byte(1);
    failed += 1 != round_up_byte(8);
    failed += 2 != round_up_byte(9);
    failed += 2 != round_up_byte(16);
    failed += 3 != round_up_byte(17);
    failed += 3 != round_up_byte(24);
    failed += 4 != round_up_byte(25);
    failed += 4 != round_up_byte(32);
    failed += 5 != round_up_byte(33);
    failed += 5 != round_up_byte(40);
    failed += 6 != round_up_byte(41);
    failed += 6 != round_up_byte(48);
    failed += 7 != round_up_byte(49);
    failed += 7 != round_up_byte(56);
    failed += 8 != round_up_byte(57);
    failed += 8 != round_up_byte(64);
    failed += 9 != round_up_byte(65);
    failed += 9 != round_up_byte(72);
    failed += 10 != round_up_byte(73);
    failed += 10 != round_up_byte(80);

    return failed;
}

#pragma GCC diagnostic ignored "-Wfloat-equal"
static int int_repres_test(){

    int failed = 0;

    for(int i = -500; i < 500; ++i){

    	srand(time(NULL));

    	double r = rand() / RAND_MAX;

	    failed += 0x00000000 != int_repres(0.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00000001 != int_repres(1.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x0000000F != int_repres(15.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00000010 != int_repres(16.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x000000FF != int_repres(255.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00000100 != int_repres(256.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00000FFF != int_repres(4095.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00001000 != int_repres(4096.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x0000FFFF != int_repres(65535.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00010000 != int_repres(65536.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x000FFFFF != int_repres(1048575.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00100000 != int_repres(1048576.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00FFFFFF != int_repres(16777215.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x01000000 != int_repres(16777216.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x0FFFFFFF != int_repres(268435455.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x10000000 != int_repres(268435456.0+r*i, 0.0+r*i, 0.5);
	    failed += 0xFFFFFFFF != int_repres(4294967295.0+r*i, 0.0+r*i, 0.5);
    }

    return failed;
}

#pragma GCC diagnostic ignored "-Wfloat-equal"
static int double_repres_test(){

	int failed = 0;

    for(int i = -500; i < 500; ++i){

    	srand(time(NULL));

    	double r = rand() / RAND_MAX;

	    failed += 0.0+r*i != double_repres(0x00000000, 0.0+r*i, 0.5);
	    failed += 1.0+r*i != double_repres(0x00000001, 0.0+r*i, 0.5);
	    failed += 15.0+r*i != double_repres(0x0000000F, 0.0+r*i, 0.5);
	    failed += 16.0+r*i != double_repres(0x00000010, 0.0+r*i, 0.5);
	    failed += 255.0+r*i != double_repres(0x000000FF, 0.0+r*i, 0.5);
	    failed += 256.0+r*i != double_repres(0x00000100, 0.0+r*i, 0.5);
	    failed += 4095.0+r*i != double_repres(0x00000FFF, 0.0+r*i, 0.5);
	    failed += 4096.0+r*i != double_repres(0x00001000, 0.0+r*i, 0.5);
	    failed += 65535.0+r*i != double_repres(0x0000FFFF, 0.0+r*i, 0.5);
	    failed += 65536.0+r*i != double_repres(0x00010000, 0.0+r*i, 0.5);
	    failed += 1048575.0+r*i != double_repres(0x000FFFFF, 0.0+r*i, 0.5);
	    failed += 1048576.0+r*i != double_repres(0x00100000, 0.0+r*i, 0.5);
	    failed += 16777215.0+r*i != double_repres(0x00FFFFFF, 0.0+r*i, 0.5);
	    failed += 16777216.0+r*i != double_repres(0x01000000, 0.0+r*i, 0.5);
	    failed += 268435455.0+r*i != double_repres(0x0FFFFFFF, 0.0+r*i, 0.5);
	    failed += 268435456.0+r*i != double_repres(0x10000000, 0.0+r*i, 0.5);
	    failed += 4294967295.0+r*i != double_repres(0xFFFFFFFF, 0.0+r*i, 0.5);

    }

    return failed;
}

int main(int argc, char ** argv){

    printf("get_needed_bit_count:\t%d of 25 tests failed\n", get_needed_bit_count_test());
    printf("round_up_byte:\t\t%d of 21 tests failed\n", round_up_byte_test());
    printf("int_repres_test:\t%d of 17000 tests failed\n", int_repres_test());
    printf("double_repres_test:\t%d of 17000 tests failed\n", double_repres_test());

    return 0;

}

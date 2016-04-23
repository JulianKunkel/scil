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

#include <math.h>

#include <string.h>

//#include "algo-abstol.h"
#include <algo/algo-abstol.c>

static int get_needed_bit_count_float_test(){

    int failed = 0;

    failed += 1 != get_needed_bit_count_float(1, 2, 0.5);
    failed += 2 != get_needed_bit_count_float(1, 3, 0.5);
    failed += 2 != get_needed_bit_count_float(1, 4, 0.5);
    failed += 3 != get_needed_bit_count_float(1, 5, 0.5);
    failed += 3 != get_needed_bit_count_float(1, 8, 0.5);
    failed += 4 != get_needed_bit_count_float(1, 9, 0.5);
    failed += 4 != get_needed_bit_count_float(1, 16, 0.5);
    failed += 5 != get_needed_bit_count_float(1, 17, 0.5);
    failed += 5 != get_needed_bit_count_float(1, 32, 0.5);
    failed += 6 != get_needed_bit_count_float(1, 33, 0.5);
    failed += 6 != get_needed_bit_count_float(1, 64, 0.5);
    failed += 7 != get_needed_bit_count_float(1, 65, 0.5);
    failed += 7 != get_needed_bit_count_float(1, 128, 0.5);
    failed += 8 != get_needed_bit_count_float(1, 129, 0.5);
    failed += 8 != get_needed_bit_count_float(1, 256, 0.5);
    failed += 9 != get_needed_bit_count_float(1, 257, 0.5);
    failed += 9 != get_needed_bit_count_float(1, 512, 0.5);
    failed += 10 != get_needed_bit_count_float(1, 513, 0.5);
    failed += 10 != get_needed_bit_count_float(1, 1024, 0.5);
    failed += 11 != get_needed_bit_count_float(1, 1025, 0.5);
    failed += 11 != get_needed_bit_count_float(1, 2048, 0.5);
    failed += 12 != get_needed_bit_count_float(1, 2049, 0.5);
    failed += 12 != get_needed_bit_count_float(1, 4096, 0.5);
    failed += 13 != get_needed_bit_count_float(1, 4097, 0.5);
    failed += 13 != get_needed_bit_count_float(1, 8192, 0.5);

    return failed;
}

static int get_needed_bit_count_double_test(){

    int failed = 0;

    failed += 1 != get_needed_bit_count_double(1, 2, 0.5);
    failed += 2 != get_needed_bit_count_double(1, 3, 0.5);
    failed += 2 != get_needed_bit_count_double(1, 4, 0.5);
    failed += 3 != get_needed_bit_count_double(1, 5, 0.5);
    failed += 3 != get_needed_bit_count_double(1, 8, 0.5);
    failed += 4 != get_needed_bit_count_double(1, 9, 0.5);
    failed += 4 != get_needed_bit_count_double(1, 16, 0.5);
    failed += 5 != get_needed_bit_count_double(1, 17, 0.5);
    failed += 5 != get_needed_bit_count_double(1, 32, 0.5);
    failed += 6 != get_needed_bit_count_double(1, 33, 0.5);
    failed += 6 != get_needed_bit_count_double(1, 64, 0.5);
    failed += 7 != get_needed_bit_count_double(1, 65, 0.5);
    failed += 7 != get_needed_bit_count_double(1, 128, 0.5);
    failed += 8 != get_needed_bit_count_double(1, 129, 0.5);
    failed += 8 != get_needed_bit_count_double(1, 256, 0.5);
    failed += 9 != get_needed_bit_count_double(1, 257, 0.5);
    failed += 9 != get_needed_bit_count_double(1, 512, 0.5);
    failed += 10 != get_needed_bit_count_double(1, 513, 0.5);
    failed += 10 != get_needed_bit_count_double(1, 1024, 0.5);
    failed += 11 != get_needed_bit_count_double(1, 1025, 0.5);
    failed += 11 != get_needed_bit_count_double(1, 2048, 0.5);
    failed += 12 != get_needed_bit_count_double(1, 2049, 0.5);
    failed += 12 != get_needed_bit_count_double(1, 4096, 0.5);
    failed += 13 != get_needed_bit_count_double(1, 4097, 0.5);
    failed += 13 != get_needed_bit_count_double(1, 8192, 0.5);

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
static int int_repres_float_test(){

    int failed = 0;

    for(int i = -500; i < 500; ++i){

    	srand(time(NULL));

    	double r = rand() / RAND_MAX;

	    failed += 0x00000000 != int_repres_float(0.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00000001 != int_repres_float(1.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x0000000F != int_repres_float(15.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00000010 != int_repres_float(16.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x000000FF != int_repres_float(255.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00000100 != int_repres_float(256.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00000FFF != int_repres_float(4095.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00001000 != int_repres_float(4096.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x0000FFFF != int_repres_float(65535.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00010000 != int_repres_float(65536.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x000FFFFF != int_repres_float(1048575.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00100000 != int_repres_float(1048576.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00FFFFFF != int_repres_float(16777215.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x01000000 != int_repres_float(16777216.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x0FFFFFFF != int_repres_float(268435455.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x10000000 != int_repres_float(268435456.0+r*i, 0.0+r*i, 0.5);
	    failed += 0xFFFFFFFF != int_repres_float(4294967295.0+r*i, 0.0+r*i, 0.5);
    }

    return failed;
}

#pragma GCC diagnostic ignored "-Wfloat-equal"
static int int_repres_double_test(){

    int failed = 0;

    for(int i = -500; i < 500; ++i){

    	srand(time(NULL));

    	double r = rand() / RAND_MAX;

	    failed += 0x00000000 != int_repres_double(0.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00000001 != int_repres_double(1.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x0000000F != int_repres_double(15.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00000010 != int_repres_double(16.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x000000FF != int_repres_double(255.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00000100 != int_repres_double(256.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00000FFF != int_repres_double(4095.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00001000 != int_repres_double(4096.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x0000FFFF != int_repres_double(65535.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00010000 != int_repres_double(65536.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x000FFFFF != int_repres_double(1048575.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00100000 != int_repres_double(1048576.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x00FFFFFF != int_repres_double(16777215.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x01000000 != int_repres_double(16777216.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x0FFFFFFF != int_repres_double(268435455.0+r*i, 0.0+r*i, 0.5);
	    failed += 0x10000000 != int_repres_double(268435456.0+r*i, 0.0+r*i, 0.5);
	    failed += 0xFFFFFFFF != int_repres_double(4294967295.0+r*i, 0.0+r*i, 0.5);
    }

    return failed;
}

#pragma GCC diagnostic ignored "-Wfloat-equal"
static int DataType_repres_float_test(){

	int failed = 0;

    for(int i = -500; i < 500; ++i){

    	srand(time(NULL));

    	float r = rand() / RAND_MAX;

	    failed += 0.0f+r*i != DataType_repres_float(0x00000000, 0.0f+r*i, 0.5);
	    failed += 1.0f+r*i != DataType_repres_float(0x00000001, 0.0f+r*i, 0.5);
	    failed += 15.0f+r*i != DataType_repres_float(0x0000000F, 0.0f+r*i, 0.5);
	    failed += 16.0f+r*i != DataType_repres_float(0x00000010, 0.0f+r*i, 0.5);
	    failed += 255.0f+r*i != DataType_repres_float(0x000000FF, 0.0f+r*i, 0.5);
	    failed += 256.0f+r*i != DataType_repres_float(0x00000100, 0.0f+r*i, 0.5);
	    failed += 4095.0f+r*i != DataType_repres_float(0x00000FFF, 0.0f+r*i, 0.5);
	    failed += 4096.0f+r*i != DataType_repres_float(0x00001000, 0.0f+r*i, 0.5);
	    failed += 65535.0f+r*i != DataType_repres_float(0x0000FFFF, 0.0f+r*i, 0.5);
	    failed += 65536.0f+r*i != DataType_repres_float(0x00010000, 0.0f+r*i, 0.5);
	    failed += 1048575.0f+r*i != DataType_repres_float(0x000FFFFF, 0.0f+r*i, 0.5);
	    failed += 1048576.0f+r*i != DataType_repres_float(0x00100000, 0.0f+r*i, 0.5);
	    failed += 16777215.0f+r*i != DataType_repres_float(0x00FFFFFF, 0.0f+r*i, 0.5);
	    failed += 16777216.0f+r*i != DataType_repres_float(0x01000000, 0.0f+r*i, 0.5);
	    failed += 268435455.0f+r*i != DataType_repres_float(0x0FFFFFFF, 0.0f+r*i, 0.5);
	    failed += 268435456.0f+r*i != DataType_repres_float(0x10000000, 0.0f+r*i, 0.5);
	    failed += 4294967295.0f+r*i != DataType_repres_float(0xFFFFFFFF, 0.0f+r*i, 0.5);

    }

    return failed;
}

#pragma GCC diagnostic ignored "-Wfloat-equal"
static int DataType_repres_double_test(){

	int failed = 0;

    for(int i = -500; i < 500; ++i){

    	srand(time(NULL));

    	double r = rand() / RAND_MAX;

	    failed += 0.0+r*i != DataType_repres_double(0x00000000, 0.0+r*i, 0.5);
	    failed += 1.0+r*i != DataType_repres_double(0x00000001, 0.0+r*i, 0.5);
	    failed += 15.0+r*i != DataType_repres_double(0x0000000F, 0.0+r*i, 0.5);
	    failed += 16.0+r*i != DataType_repres_double(0x00000010, 0.0+r*i, 0.5);
	    failed += 255.0+r*i != DataType_repres_double(0x000000FF, 0.0+r*i, 0.5);
	    failed += 256.0+r*i != DataType_repres_double(0x00000100, 0.0+r*i, 0.5);
	    failed += 4095.0+r*i != DataType_repres_double(0x00000FFF, 0.0+r*i, 0.5);
	    failed += 4096.0+r*i != DataType_repres_double(0x00001000, 0.0+r*i, 0.5);
	    failed += 65535.0+r*i != DataType_repres_double(0x0000FFFF, 0.0+r*i, 0.5);
	    failed += 65536.0+r*i != DataType_repres_double(0x00010000, 0.0+r*i, 0.5);
	    failed += 1048575.0+r*i != DataType_repres_double(0x000FFFFF, 0.0+r*i, 0.5);
	    failed += 1048576.0+r*i != DataType_repres_double(0x00100000, 0.0+r*i, 0.5);
	    failed += 16777215.0+r*i != DataType_repres_double(0x00FFFFFF, 0.0+r*i, 0.5);
	    failed += 16777216.0+r*i != DataType_repres_double(0x01000000, 0.0+r*i, 0.5);
	    failed += 268435455.0+r*i != DataType_repres_double(0x0FFFFFFF, 0.0+r*i, 0.5);
	    failed += 268435456.0+r*i != DataType_repres_double(0x10000000, 0.0+r*i, 0.5);
	    failed += 4294967295.0+r*i != DataType_repres_double(0xFFFFFFFF, 0.0+r*i, 0.5);

    }

    printf("%d\n", failed);

    return failed;
}

static void print_usage(){
    printf("Usage:\n \
        \tThis test requires exactly one argument\n \
        \t-gnbcf:\tTest get_needed_bit_count_float function\n \
        \t-gnbcd:\tTest get_needed_bit_count_double function\n \
        \t-rub:\tTest round_up_byte function\n \
        \t-irf:\t Test int_repres_float function\n \
        \t-ird:\t Test int_repres_double function\n \
        \t-drf:\t Test DataType_repres_float function\n \
        \t-drd:\t Test DataType_repres_double function\n\n \
        \tThe test either returns -1 for argument errors or a positive integer of the count of test failures.\n");
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char ** argv){

    // printf("%s\n", argv[1]);

    if(argc != 2){
        print_usage();
        return -1;
    }

    if      (strcmp(argv[1], "-gnbcf") == 0) return get_needed_bit_count_float_test();
    else if (strcmp(argv[1], "-gnbcd") == 0) return get_needed_bit_count_double_test();
    else if (strcmp(argv[1], "-rub") == 0)   return round_up_byte_test();
    else if (strcmp(argv[1], "-irf") == 0)   return int_repres_float_test();
    else if (strcmp(argv[1], "-ird") == 0)   return int_repres_double_test();
    else if (strcmp(argv[1], "-drf") == 0)   return DataType_repres_float_test();
    else if (strcmp(argv[1], "-drd") == 0)   return DataType_repres_double_test();

    print_usage();
    return -1;
}

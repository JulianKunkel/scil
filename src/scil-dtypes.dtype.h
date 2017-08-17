#include <scil-util.h>

//Supported datatypes: float double
// Repeat for each data type

#pragma GCC diagnostic ignored "-Wfloat-equal"
static void scil_determine_accuracy_<DATATYPE>(const <DATATYPE> *data_1, const <DATATYPE> *data_2, const size_t length, const double relative_err_finest_abs_tolerance, scil_user_hints_t * a){
	for(size_t i = 0; i < length; i++ ){
		const <DATATYPE> c1 = data_1[i];
		const <DATATYPE> c2 = data_2[i];
		const double err = c2 > c1 ? c2 - c1 : c1 - c2;
		scil_user_hints_t cur;
		cur.absolute_tolerance = err;
		// determine significant digits
		{
			datatype_cast_<DATATYPE> f1, f2;
			f1.f = c1;
			f2.f = c2;
			//printf("checking %f %f\n", (double) c1, (double) c2);

			if (f1.p.sign != f2.p.sign ){
				cur.significant_bits = 0;
				//printf("fall exponent different\n");
			}else{
				// check exponent
				uint64_t diff = (uint64_t) f1.p.exponent - (uint64_t) f2.p.exponent;
				if(diff > 1){
					cur.significant_bits = 0;
					break;
				}
				if(diff == 1){
					// wrap around
					f1.p.mantissa -= 1;
				}

				uint64_t res = f1.p.mantissa > f2.p.mantissa ? f1.p.mantissa - f2.p.mantissa : f2.p.mantissa - f1.p.mantissa;
				// check mantissa, bit by bit
				cur.significant_bits = MANTISSA_LENGTH_<DATATYPE_UPPER>;
				for(int m = MANTISSA_LENGTH_<DATATYPE_UPPER> - 1; m >= 0; m--){
					if( ((res >> m) & 1) != 0){
						cur.significant_bits = MANTISSA_LENGTH_<DATATYPE_UPPER> - (int) m;
						//printf("significant bits:%d\n", cur.significant_bits);
						break;
					}
				}
				//printf("fall %d\n", cur.significant_bits);
			}
		}
		// determine relative tolerance
		cur.relative_tolerance_percent = 0;
		cur.relative_err_finest_abs_tolerance = 0;
		if (err >= relative_err_finest_abs_tolerance){
			if (c1 == 0 && c2 != 0){
				cur.relative_tolerance_percent = INFINITY;
			}else{
				// sign check not needed
				cur.relative_tolerance_percent = fabs(1 - c2 / c1);
			}
		}else{
			cur.relative_err_finest_abs_tolerance = err;
		}
		a->absolute_tolerance = max(cur.absolute_tolerance, a->absolute_tolerance);
		a->relative_err_finest_abs_tolerance = max(cur.relative_err_finest_abs_tolerance, a->relative_err_finest_abs_tolerance);
		a->relative_tolerance_percent = max(cur.relative_tolerance_percent, a->relative_tolerance_percent);
		a->significant_bits = min(cur.significant_bits, a->significant_bits);
	}
}
// End repeat

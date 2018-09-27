//Supported datatypes: int8_t int16_t int32_t int64_t
// Repeat for each data type

#pragma GCC diagnostic ignored "-Wfloat-equal"
static void scil_determine_accuracy_<DATATYPE>(const <DATATYPE> *data_1, const <DATATYPE> *data_2, const size_t length, const double relative_err_finest_abs_tolerance, scil_user_hints_t * a, scil_validate_params_t * out_validation){
	for(size_t i = 0; i < length; i++ ){
		const <DATATYPE> c1 = data_1[i];
		const <DATATYPE> c2 = data_2[i];
		const <DATATYPE> err = c2 > c1 ? c2 - c1 : c1 - c2;

		scil_user_hints_t cur;
		cur.absolute_tolerance = err;
		// determine significant digits
		{
			cur.significant_digits = sizeof(<DATATYPE>)*8;
			for(int m = cur.significant_digits - 1 ; m >= 0; m--){
				int b1 = (c1>>m) & (1);
				int b2 = (c2>>m) & (1);
				if( b1 != b2){
					cur.significant_digits = cur.significant_digits - (int) m;
					break;
				}
			}
		}
		// determine relative tolerance
		cur.relative_tolerance_percent = 0;
		cur.relative_err_finest_abs_tolerance = 0;
		if (err >= (<DATATYPE>) relative_err_finest_abs_tolerance){
			if (c1 == 0 && c2 != 0){
				cur.relative_tolerance_percent = INFINITY;
			}else{
				cur.relative_tolerance_percent = fabs(1 - c2 / (float) c1);
			}
		}else{
			cur.relative_err_finest_abs_tolerance = err;
		}

                if(cur.absolute_tolerance > a->absolute_tolerance){
                        out_validation->absolute_tolerance_idx = i; 
                }
                if(cur.relative_err_finest_abs_tolerance > a->relative_err_finest_abs_tolerance){
                        out_validation->relative_err_finest_abs_tolerance_idx = i;
                }
                if(cur.relative_tolerance_percent > a->relative_tolerance_percent){
                        out_validation->relative_tolerance_percent_idx = i;
                }

                a->absolute_tolerance = max(cur.absolute_tolerance, a->absolute_tolerance);
                a->relative_err_finest_abs_tolerance = max(cur.relative_err_finest_abs_tolerance, a->relative_err_finest_abs_tolerance);
                a->relative_tolerance_percent = max(cur.relative_tolerance_percent, a->relative_tolerance_percent);
		a->significant_bits = min(cur.significant_digits, a->significant_bits);
	}
}
// End repeat

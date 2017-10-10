module scil
  double PRECISION :: SCIL_ACCURACY_DBL_IGNORE = 0d+0
  double PRECISION :: SCIL_ACCURACY_DBL_FINEST = 1d-307
  integer :: SCIL_ACCURACY_INT_IGNORE = 0
  integer :: SCIL_ACCURACY_INT_FINEST = -1


  interface
    subroutine h5pset_scil_compression_hints_f(prp_id, relative_tolerance_percent, &
      relative_err_finest_abs_tolerance, absolute_tolerance, significant_digits, significant_bits &
      )
      USE H5GLOBAL
      INTEGER(HID_T), INTENT(IN) :: prp_id
      double PRECISION, INTENT(IN) :: relative_tolerance_percent
      double PRECISION, INTENT(IN) :: relative_err_finest_abs_tolerance
      double PRECISION, INTENT(IN) :: absolute_tolerance
      integer, INTENT(IN) :: significant_digits
      integer, INTENT(IN) :: significant_bits
    end subroutine h5pset_scil_compression_hints_f
  end interface

end module scil

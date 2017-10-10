module scil

  double PRECISION :: SCIL_ACCURACY_DBL_IGNORE = 0d+0
  double PRECISION :: SCIL_ACCURACY_DBL_FINEST = 1d-307
  integer :: SCIL_ACCURACY_INT_IGNORE = 0
  integer :: SCIL_ACCURACY_INT_FINEST = -1


  interface
    subroutine h5pset_scil_compression_hints_f(relative_tolerance_percent, &
      relative_err_finest_abs_tolerance, absolute_tolerance, significant_digits, significant_bits &
      )
      double PRECISION :: relative_tolerance_percent
      double PRECISION :: relative_err_finest_abs_tolerance
      double PRECISION :: absolute_tolerance
      integer :: significant_digits
      integer :: significant_bits
    end subroutine h5pset_scil_compression_hints_f
  end interface

end module scil

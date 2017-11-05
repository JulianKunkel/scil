// This header is needed to remove the cyclic dependenncies of scil-context,
// scil-algorithm and scil-chain

#ifndef SCIL_CCA_H
#define SCIL_CCA_H

#include <scil-datatypes.h>
#include <scil-user-hints.h>
#include <scil-dict.h>
#include <scil-dims.h>
#include <scil-compressor.h>

// at most we support chaining of 10 preconditioners
#define PRECONDITIONER_LIMIT 10

typedef struct scil_compression_chain {
  struct scil_compression_algorithm* pre_cond_first[PRECONDITIONER_LIMIT]; // preconditioners first stage
  struct scil_compression_algorithm* converter;
  struct scil_compression_algorithm* pre_cond_second[PRECONDITIONER_LIMIT]; // preconditioners second stage
  struct scil_compression_algorithm* data_compressor; // datatype compressor
  struct scil_compression_algorithm* byte_compressor; // byte compressor

  char precond_first_count;
  char precond_second_count;
  char total_size; // includes data and byte compressors
  char is_lossy;
} scil_compression_chain_t;


int scilU_chain_create(scil_compression_chain_t* chain, const char* str_in);

int scilU_chain_is_applicable(const scil_compression_chain_t* chain, SCIL_Datatype_t datatype);

#endif // SCIL_CCA_H

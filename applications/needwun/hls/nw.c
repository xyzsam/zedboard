#include "nw.h"

#define MATCH_SCORE 1
#define MISMATCH_SCORE -1
#define GAP_SCORE -1

#define ALIGN '\\'
#define SKIPA '^'
#define SKIPB '<'

#define MAX(A,B) ( ((A)>(B))?(A):(B) )

void needwun_kernel(char SEQA[ALEN], char SEQB[BLEN],
             char alignedA[ALEN+BLEN], char alignedB[ALEN+BLEN],
             int M[(ALEN+1)*(BLEN+1)], char ptr[(ALEN+1)*(BLEN+1)]){

    int score, up_left, up, left, max;
    int row, row_up, r;
    int a_idx, b_idx;
    int a_str_idx, b_str_idx;

    init_row: for(a_idx=0; a_idx<(ALEN+1); a_idx++){
        M[a_idx] = a_idx * GAP_SCORE;
    }
    init_col: for(b_idx=0; b_idx<(BLEN+1); b_idx++){
        M[b_idx*(ALEN+1)] = b_idx * GAP_SCORE;
    }

    // Matrix filling loop
    fill_out: for(b_idx=1; b_idx<(BLEN+1); b_idx++){
        fill_in: for(a_idx=1; a_idx<(ALEN+1); a_idx++){
            if(SEQA[a_idx-1] == SEQB[b_idx-1]){
                score = MATCH_SCORE;
            } else {
                score = MISMATCH_SCORE;
            }

            row_up = (b_idx-1)*(ALEN+1);
            row = (b_idx)*(ALEN+1);

            up_left = M[row_up + (a_idx-1)] + score;
            up      = M[row_up + (a_idx  )] + GAP_SCORE;
            left    = M[row    + (a_idx-1)] + GAP_SCORE;

            max = MAX(up_left, MAX(up, left));

            M[row + a_idx] = max;
            if(max == left){
                ptr[row + a_idx] = SKIPB;
            } else if(max == up){
                ptr[row + a_idx] = SKIPA;
            } else{
                ptr[row + a_idx] = ALIGN;
            }
        }
    }

    // TraceBack (n.b. aligned sequences are backwards to avoid string appending)
    a_idx = ALEN;
    b_idx = BLEN;
    a_str_idx = 0;
    b_str_idx = 0;

    trace: while(a_idx>0 || b_idx>0) {
        r = b_idx*(ALEN+1);
        if (ptr[r + a_idx] == ALIGN){
            alignedA[a_str_idx++] = SEQA[a_idx-1];
            alignedB[b_str_idx++] = SEQB[b_idx-1];
            a_idx--;
            b_idx--;
        }
        else if (ptr[r + a_idx] == SKIPB){
            alignedA[a_str_idx++] = SEQA[a_idx-1];
            alignedB[b_str_idx++] = '-';
            a_idx--;
        }
        else{ // SKIPA
            alignedA[a_str_idx++] = '-';
            alignedB[b_str_idx++] = SEQB[b_idx-1];
            b_idx--;
        }
    }

    // Pad the result
    pad_a: for( ; a_str_idx<ALEN+BLEN; a_str_idx++ ) {
      alignedA[a_str_idx] = '_';
    }
    pad_b: for( ; b_str_idx<ALEN+BLEN; b_str_idx++ ) {
      alignedB[b_str_idx] = '_';
    }
}

void needwun(int32_t * in_stream, int32_t * out_stream) {
#pragma HLS INTERFACE s_axilite bundle=BUS_A port=return
#pragma HLS INTERFACE axis bundle=INPUT_STREAM depth=64 port=in_stream
#pragma HLS INTERFACE axis bundle=OUTPUT_STREAM depth=128 port=out_stream
  int i, j;
  int32_t temp;
  char SEQA[ALEN];
  char SEQB[BLEN];
  char alignedA[ALEN + BLEN];
  char alignedB[ALEN + BLEN];
  int M[(ALEN + 1) * (BLEN + 1)];
  char ptr[(ALEN + 1) * (BLEN + 1)];

  for (i = 0; i < ALEN; i+=4) {
#pragma HLS PIPELINE II=1
    temp = in_stream[i/4];
    for (j = 0; j < 4; j++) {
#pragma HLS UNROLL
      SEQA[i+j] = (temp >> (j*8)) & 0xff;
    }
  }
  for (i = 0; i < BLEN; i+=4) {
#pragma HLS PIPELINE II=1
    temp = in_stream[(i+ALEN)/4];
    for (j = 0; j < 4; j++) {
      SEQB[i+j] = (temp >> (j*8)) & 0xff;
    }
  }

  needwun_kernel(SEQA, SEQB, alignedA, alignedB, M, ptr);

  for (i = 0; i < ALEN+BLEN; i+=4) {
#pragma HLS PIPELINE II=1
    temp = 0;
    for (j = 0; j < 4; j++) {
#pragma HLS UNROLL
      temp |= (alignedA[i+j] << (j*8));
    }
    out_stream[i/4] = temp;
  }
  for (i = 0; i < ALEN+BLEN; i+=4) {
#pragma HLS PIPELINE II=1
    temp = 0;
    for (j = 0; j < 4; j++) {
#pragma HLS UNROLL
      temp |= (alignedB[i+j] << (j*8));
    }
    out_stream[(i+ALEN+BLEN)/4] = temp;
  }
}

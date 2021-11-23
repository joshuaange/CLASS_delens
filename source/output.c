/** @file output.c Documented output module
 *
 * Julien Lesgourgues, 26.08.2010
 *
 * This module writes the output in files.
 *
 * The following functions can be called from other modules or from the main:
 *
 * -# output_init() (must be called after harmonic_init())
 * -# output_total_cl_at_l() (can be called even before output_init())
 *
 * No memory needs to be deallocated after that,
 * hence there is no output_free() routine like in other modules.
 */

#include "output.h"

int output_total_cl_at_l(
                         struct harmonic * phr,
                         struct lensing * ple,
                         struct output * pop,
                         int l,
                         double * cl
                         ){
    
    double ** cl_md_ic; /* array with argument
                         cl_md_ic[index_md][index_ic1_ic2*phr->ct_size+index_ct] */
    
    double ** cl_md;    /* array with argument
                         cl_md[index_md][index_ct] */
    
    int index_md;
    
    if (ple->has_lensed_cls == _TRUE_) {
        class_call(lensing_cl_at_l(ple,
                                   l,
                                   cl),
                   ple->error_message,
                   pop->error_message);
    }
    else {
        
        class_alloc(cl_md_ic,
                    phr->md_size*sizeof(double *),
                    pop->error_message);
        
        class_alloc(cl_md,
                    phr->md_size*sizeof(double *),
                    pop->error_message);
        
        for (index_md = 0; index_md < phr->md_size; index_md++) {
            
            if (phr->md_size > 1)
                
                class_alloc(cl_md[index_md],
                            phr->ct_size*sizeof(double),
                            ple->error_message);
            
            if (phr->ic_size[index_md] > 1)
                
                class_alloc(cl_md_ic[index_md],
                            phr->ic_ic_size[index_md]*phr->ct_size*sizeof(double),
                            ple->error_message);
        }
        
        class_call(harmonic_cl_at_l(phr,
                                    (double)l,
                                    cl,
                                    cl_md,
                                    cl_md_ic),
                   phr->error_message,
                   pop->error_message);
        
        for (index_md = 0; index_md < phr->md_size; index_md++) {
            
            if (phr->md_size > 1)
                free(cl_md[index_md]);
            
            if (phr->ic_size[index_md] > 1)
                free(cl_md_ic[index_md]);
            
        }
        
        free(cl_md_ic);
        free(cl_md);
        
    }
    
    return _SUCCESS_;
    
}

/**
 * This routine writes the output in files.
 *
 *
 * @param pba Input: pointer to background structure (needed for calling harmonic_pk_at_z())
 * @param pth Input: pointer to thermodynamics structure
 * @param ppt Input: pointer perturbation structure
 * @param ppm Input: pointer to primordial structure
 * @param ptr Input: pointer to transfer structure
 * @param phr Input: pointer to harmonic structure
 * @param pfo Input: pointer to fourier structure
 * @param ple Input: pointer to lensing structure
 * @param psd Input: pointer to distortions structure
 * @param pop Input: pointer to output structure
 */

int output_init(
                struct background * pba,
                struct thermodynamics * pth,
                struct perturbations * ppt,
                struct primordial * ppm,
                struct transfer * ptr,
                struct harmonic * phr,
                struct fourier * pfo,
                struct lensing * ple,
                struct distortions * psd,
                struct output * pop
                ) {
    
    /** Summary: */
    
    /** - check that we really want to output at least one file */
    
    if ((ppt->has_cls == _FALSE_) && (ppt->has_pk_matter == _FALSE_) && (ppt->has_density_transfers == _FALSE_) && (ppt->has_velocity_transfers == _FALSE_) && (pop->write_background == _FALSE_) && (pop->write_thermodynamics == _FALSE_) && (pop->write_primordial == _FALSE_)) {
        if (pop->output_verbose > 0)
            printf("No output files requested. Output module skipped.\n");
        return _SUCCESS_;
    }
    else {
        if (pop->output_verbose > 0)
            printf("Writing output files in %s... \n",pop->root);
    }
    
//     printf("we are now output.c 7\n");
//     printf("we are now output.c 7\n");
//     printf("we are now output.c 7\n");
    
    /** - deal with all anisotropy power spectra \f$ C_l\f$'s */
    
    if (ppt->has_cls == _TRUE_) {
        
        class_call(output_cl(pba,ppt,phr,ple,pop),
                   pop->error_message,
                   pop->error_message);
    }
    
//     printf("we are now output.c 8\n");
//     printf("we are now output.c 8\n");
//     printf("we are now output.c 8\n");
    
    
    /** - deal with all Fourier matter power spectra P(k)'s */
    
    if (ppt->has_pk_matter == _TRUE_) {
        
        class_call(output_pk(pba,ppt,pfo,pop,pk_linear),
                   pop->error_message,
                   pop->error_message);
        
        if (pfo->method != nl_none) {
            
            class_call(output_pk(pba,ppt,pfo,pop,pk_nonlinear),
                       pop->error_message,
                       pop->error_message);
            
        }
    }
    
    /** - deal with density and matter power spectra */
    
    if ((ppt->has_density_transfers == _TRUE_) || (ppt->has_velocity_transfers == _TRUE_)) {
        
        class_call(output_tk(pba,ppt,pop),
                   pop->error_message,
                   pop->error_message);
        
    }
    
    /** - deal with background quantities */
    
    if (pop->write_background == _TRUE_) {
        
        class_call(output_background(pba,pop),
                   pop->error_message,
                   pop->error_message);
        
    }
    
    /** - deal with thermodynamics quantities */
    
    if (pop->write_thermodynamics == _TRUE_) {
        
        class_call(output_thermodynamics(pba,pth,pop),
                   pop->error_message,
                   pop->error_message);
        
    }
    
    /** - deal with perturbation quantities */
    
    if (pop->write_perturbations == _TRUE_ && ppt->has_perturbations) {
        
        class_call(output_perturbations(pba,ppt,pop),
                   pop->error_message,
                   pop->error_message);
        
    }
    
    /** - deal with primordial spectra */
    
    if (pop->write_primordial == _TRUE_ && ppt->has_perturbations) {
        
        class_call(output_primordial(ppt,ppm,pop),
                   pop->error_message,
                   pop->error_message);
        
    }
    
    /** - deal with heating */
    
    if (pop->write_exotic_injection == _TRUE_ || pop->write_noninjection == _TRUE_) {
        
        class_call(output_heating(&(pth->in),&(psd->ni),pop),
                   pop->error_message,
                   pop->error_message);
    }
    
    /** - deal with spectral distortions */
    
    if (pop->write_distortions == _TRUE_) {
        
        class_call(output_distortions(psd,pop),
                   pop->error_message,
                   pop->error_message);
    }
    
    return _SUCCESS_;
    
}

/**
 * This routines writes the output in files for anisotropy power spectra \f$ C_l\f$'s.
 *
 * @param pba Input: pointer to background structure (needed for \f$ T_{cmb}\f$)
 * @param ppt Input: pointer perturbation structure
 * @param phr Input: pointer to harmonic structure
 * @param ple Input: pointer to lensing structure
 * @param pop Input: pointer to output structure
 */

int output_cl(
              struct background * pba,
              struct perturbations * ppt,
              struct harmonic * phr,
              struct lensing * ple,
              struct output * pop
              ) {
    
    /** Summary: */
    
    /** - define local variables */
    
    FILE *** out_md_ic; /* array of pointers to files with argument
                         out_md_ic[index_md][index_ic1_ic2]
                         (will contain cl's for each mode and pairs of initial conditions) */
    
    FILE ** out_md;     /* array of pointers to files with argument
                         out_md[index_md]
                         (will contain cl's for each mode, summed eventually over ic's) */
    
    FILE * out;         /* (will contain total cl's, summed eventually over modes and ic's) */
    
    FILE * out_lensed;         /* (will contain total lensed cl's) */
    
    FILE * out_delensed; /* DLM */
    
    FILE * out_rcn_noise; /* DLM */
    
    FILE * out_spec_noise; /* DLM */
    
    FILE * out_cl_tt_derv; /* DLM */
    FILE * out_cl_te_derv; /* DLM */
    FILE * out_cl_ee_derv; /* DLM */
    FILE * out_cl_bb_derv; /* DLM */
    
    FILE * out_cl_tt_derv_tt; /* DLM */
    FILE * out_cl_te_derv_te; /* DLM */
    FILE * out_cl_ee_derv_ee; /* DLM */
    FILE * out_cl_ee_derv_bb; /* DLM */
    FILE * out_cl_bb_derv_ee; /* DLM */
    FILE * out_cl_bb_derv_bb; /* DLM */
    
    FILE * out_cl_dl_tt_derv_tt; /* DLM */
    FILE * out_cl_dl_te_derv_te; /* DLM */
    FILE * out_cl_dl_ee_derv_ee; /* DLM */
    FILE * out_cl_dl_ee_derv_bb; /* DLM */
    FILE * out_cl_dl_bb_derv_ee; /* DLM */
    FILE * out_cl_dl_bb_derv_bb; /* DLM */
    
    double ** cl_md_ic; /* array with argument
                         cl_md_ic[index_md][index_ic1_ic2*phr->ct_size+index_ct] */
    
    double ** cl_md;    /* array with argument
                         cl_md[index_md][index_ct] */
    
    double * cl_tot;    /* array with argument
                         cl_tot[index_ct] */
    
    
    double * cl_tot_dl; /* DLM */
    double * nl_tot_dl; /* DLM */
    
    double cl_delensed_derv; /* DLM */
    
    double cl_lensed_derv_tt_tt; /* DLM */
    double cl_lensed_derv_te_te; /* DLM */
    double cl_lensed_derv_ee_ee; /* DLM */
    double cl_lensed_derv_ee_bb; /* DLM */
    double cl_lensed_derv_bb_ee; /* DLM */
    double cl_lensed_derv_bb_bb; /* DLM */
    
    double cl_delensed_derv_tt_tt; /* DLM */
    double cl_delensed_derv_te_te; /* DLM */
    double cl_delensed_derv_ee_ee; /* DLM */
    double cl_delensed_derv_ee_bb; /* DLM */
    double cl_delensed_derv_bb_ee; /* DLM */
    double cl_delensed_derv_bb_bb; /* DLM */
    
    int index_md;
    int index_ic1,index_ic2,index_ic1_ic2;
    int l;
    
    int k; /* DLM */
    char derv_buffer[_COLUMNWIDTH_*2*ple->l_delensed_max];
    int buf_length;
    
    FileName file_name;
    char first_line[_LINE_LENGTH_MAX_];
    
    char derv_type_string[50]; /* DLM */
    
    /** - first, allocate all arrays of files and \f$ C_l\f$'s */
    
    class_alloc(out_md_ic,
                phr->md_size*sizeof(FILE * *),
                pop->error_message);
    
    class_alloc(cl_md_ic,
                phr->md_size*sizeof(double *),
                pop->error_message);
    
    class_alloc(out_md,
                phr->md_size*sizeof(FILE *),
                pop->error_message);
    
    class_alloc(cl_md,
                phr->md_size*sizeof(double *),
                pop->error_message);
    
    for (index_md = 0; index_md < ppt->md_size; index_md++) {
        
        class_alloc(out_md_ic[index_md],
                    phr->ic_ic_size[index_md]*sizeof(FILE *),
                    pop->error_message);
        
    }
//     printf("we are inside output.c inside 1 \n");
//     printf("we are inside output.c inside 1 \n");
//     printf("we are inside output.c inside 1 \n");
    
    //   /** - second, open only the relevant files, and write a heading in each of them */
    
    sprintf(file_name,"%s%s",pop->root,"cl.dat");
    
    class_call(output_open_cl_file(phr,
                                   pop,
                                   &out,
                                   file_name,
                                   "total [l(l+1)/2pi] C_l's",
                                   phr->l_max_tot
                                   ),
               pop->error_message,
               pop->error_message);
    
    class_alloc(cl_tot,
                phr->ct_size*sizeof(double),
                pop->error_message);
    
    class_alloc(cl_tot_dl,
                ple->dlt_size*sizeof(double),
                pop->error_message); /* DLM */
    
    class_alloc(nl_tot_dl,
                ple->nlt_size*sizeof(double),
                pop->error_message); /* DLM */
    
    //   printf('we are inside output.c inside 2');
    //   printf('we are inside output.c inside 2');
    //   printf('we are inside output.c inside 2');
    
    if (ple->has_lensed_cls == _TRUE_) {
        
        sprintf(file_name,"%s%s",pop->root,"cl_lensed.dat");
        
        class_call(output_open_cl_file(phr,
                                       pop,
                                       &out_lensed,
                                       file_name,
                                       "total lensed [l(l+1)/2pi] C_l's",
                                       ple->l_lensed_max
                                       ),
                   pop->error_message,
                   pop->error_message);
    }
    
    if (ple->has_delensed_cls == _TRUE_) {
        
        sprintf(file_name,"%s%s",pop->root,"cl_delensed.dat"); /* DLM */
        
        class_call(output_open_cl_file(phr,
                                       pop,
                                       &out_delensed,
                                       file_name,
                                       "DLM: total delensed [l(l+1)/2pi] C_l's",
                                       ple->l_delensed_max
                                       ),
                   pop->error_message,
                   pop->error_message); /* DLM */
        
    }
    
    if (ple->lens_rec_noise_type == internal_rn) { /* DLM */
        
        sprintf(file_name,"%s%s",pop->root,"lensing_noise_rcn.dat"); /* DLM */
        class_call(output_open_nl_lens_file(phr,
                                            pop,
                                            ple,
                                            &out_rcn_noise,
                                            file_name,
                                            "DLM: Best estimate lensing noise spectra [l(l+1)/2pi] N_l's",
                                            ple->l_delensed_max
                                            ),
                   pop->error_message,
                   pop->error_message); /* DLM */
        
    }
    
    if (ple->output_spectra_noise == _TRUE_) { /* DLM */
        
        sprintf(file_name,"%s%s",pop->root,"spectra_noise.dat"); /* DLM */
        
        class_call(output_open_nl_tp_file(pop,
                                          ple,
                                          &out_spec_noise,
                                          file_name,
                                          "DLM: Noise spectra for temperature and polarization"
                                          ),
                   pop->error_message,
                   pop->error_message); /* DLM */
    }
    
    
    if (ple->calculate_pderivaties == _TRUE_ && ple->output_derivatives == _TRUE_) { /* DLM */
        
        if (ple->derv_type == delensed) {strcpy(derv_type_string, "delensed");}
        if (ple->derv_type == lensed) {strcpy(derv_type_string, "lensed");}
        
        /* sprintf(file_name,"%s%s",pop->root,"dClTTdCldd_delensed.dat"); /* DLM */
        sprintf(file_name,"%s%s%s%s",pop->root,"dClTTdCldd_", derv_type_string, ".dat");
        
        class_call(output_open_cl_derv_file(pop,
                                            ple,
                                            &out_cl_tt_derv,
                                            file_name,
                                            "DLM: Derivative of the Cl_TT spectrum with respect to deflection spectrum Cl_dd. Rows are L multipoles at the corresponding Cl_dd spectrum; and columns are the l multipoles for the corresponding Cl_TT spectrum."
                                            ),
                   pop->error_message,
                   pop->error_message); /* DLM */
        
        if(ple->calculate_derviaties_wrt_unlensed == _TRUE_){
            if(ple->lensed_wrt_unlensed == _TRUE_){
                sprintf(file_name,"%s%s%s%s",pop->root,"dClTTdClTT_", "lensed", ".dat");
                
                class_call(output_open_cl_derv_file(pop,
                                                    ple,
                                                    &out_cl_tt_derv_tt,
                                                    file_name,
                                                    "DLM: Derivative of the Cl_TT spectrum with respect to unlensed Cl_TT. Rows are L multipoles at the corresponding Cl_TT, unlensed spectrum; and columns are the l multipoles for the corresponding lensed Cl_TT spectrum."
                                                    ),
                           pop->error_message,
                           pop->error_message); /* DLM */
                
                // finish up the output file
            }
            if(ple->delensed_wrt_unlensed == _TRUE_){
                sprintf(file_name,"%s%s%s%s",pop->root,"dClTTdClTT_", "delensed", ".dat");
                
                class_call(output_open_cl_derv_file(pop,
                                                    ple,
                                                    &out_cl_dl_tt_derv_tt,
                                                    file_name,
                                                    "DLM: Derivative of the Cl_TT spectrum with respect to unlensed Cl_TT. Rows are L multipoles at the corresponding Cl_TT, unlensed spectrum; and columns are the l multipoles for the corresponding delensed Cl_TT spectrum."
                                                    ),
                           pop->error_message,
                           pop->error_message); /* DLM */
            }
        }
        
        /* sprintf(file_name,"%s%s",pop->root,"dClTEdCldd_delensed.dat"); /* DLM */
        sprintf(file_name,"%s%s%s%s",pop->root,"dClTEdCldd_", derv_type_string, ".dat");
        
        class_call(output_open_cl_derv_file(pop,
                                            ple,
                                            &out_cl_te_derv,
                                            file_name,
                                            "DLM: Derivative of the Cl_TE spectrum with respect to deflection spectrum Cl_dd. Rows are L multipoles at the corresponding Cl_dd spectrum; and columns are the l multipoles for the corresponding Cl_TE spectrum."
                                            ),
                   pop->error_message,
                   pop->error_message); /* DLM */
        
        if(ple->calculate_derviaties_wrt_unlensed == _TRUE_){
            if(ple->lensed_wrt_unlensed == _TRUE_){
                sprintf(file_name,"%s%s%s%s",pop->root,"dClTEdClTE_", "lensed", ".dat");
                
                class_call(output_open_cl_derv_file(pop,
                                                    ple,
                                                    &out_cl_te_derv_te,
                                                    file_name,
                                                    "DLM: Derivative of the Cl_TE spectrum with respect to unlensed Cl_TE. Rows are L multipoles at the corresponding Cl_TE, unlensed spectrum; and columns are the l multipoles for the corresponding lensed Cl_TE spectrum."
                                                    ),
                           pop->error_message,
                           pop->error_message); /* DLM */
            }
            if(ple->delensed_wrt_unlensed == _TRUE_){
                
                sprintf(file_name,"%s%s%s%s",pop->root,"dClTEdClTE_", "delensed", ".dat");
                
                class_call(output_open_cl_derv_file(pop,
                                                    ple,
                                                    &out_cl_dl_te_derv_te,
                                                    file_name,
                                                    "DLM: Derivative of the Cl_TE spectrum with respect to unlensed Cl_TE. Rows are L multipoles at the corresponding Cl_TE, unlensed spectrum; and columns are the l multipoles for the corresponding delensed Cl_TE spectrum."
                                                    ),
                           pop->error_message,
                           pop->error_message); /* DLM */
                
            }
        }
        /* sprintf(file_name,"%s%s",pop->root,"dClEEdCldd_delensed.dat"); /* DLM */
        sprintf(file_name,"%s%s%s%s",pop->root,"dClEEdCldd_", derv_type_string, ".dat");
        
        class_call(output_open_cl_derv_file(pop,
                                            ple,
                                            &out_cl_ee_derv,
                                            file_name,
                                            "DLM: Derivative of the Cl_EE spectrum with respect to deflection spectrum Cl_dd. Rows are L multipoles at the corresponding Cl_dd spectrum; and columns are the l multipoles for the corresponding Cl_EE spectrum."
                                            ),
                   pop->error_message,
                   pop->error_message); /* DLM */
        
        if(ple->calculate_derviaties_wrt_unlensed == _TRUE_){
            if(ple->lensed_wrt_unlensed == _TRUE_){
                
                sprintf(file_name,"%s%s%s%s",pop->root,"dClEEdClEE_", "lensed", ".dat");
                
                class_call(output_open_cl_derv_file(pop,
                                                    ple,
                                                    &out_cl_ee_derv_ee,
                                                    file_name,
                                                    "DLM: Derivative of the Cl_EE spectrum with respect to unlensed Cl_EE. Rows are L multipoles at the corresponding Cl_EE, unlensed spectrum; and columns are the l multipoles for the corresponding lensed Cl_EE spectrum."
                                                    ),
                           pop->error_message,
                           pop->error_message); /* DLM */
                
                sprintf(file_name,"%s%s%s%s",pop->root,"dClEEdClBB_", "lensed", ".dat");
                
                class_call(output_open_cl_derv_file(pop,
                                                    ple,
                                                    &out_cl_ee_derv_bb,
                                                    file_name,
                                                    "DLM: Derivative of the Cl_EE spectrum with respect to unlensed Cl_BB. Rows are L multipoles at the corresponding Cl_BB, unlensed spectrum; and columns are the l multipoles for the corresponding lensed Cl_EE spectrum."
                                                    ),
                           pop->error_message,
                           pop->error_message); /* DLM */
            }
            if(ple->delensed_wrt_unlensed == _TRUE_){
                
                sprintf(file_name,"%s%s%s%s",pop->root,"dClEEdClEE_", "delensed", ".dat");
                
                class_call(output_open_cl_derv_file(pop,
                                                    ple,
                                                    &out_cl_dl_ee_derv_ee,
                                                    file_name,
                                                    "DLM: Derivative of the Cl_EE spectrum with respect to unlensed Cl_EE. Rows are L multipoles at the corresponding Cl_EE, unlensed spectrum; and columns are the l multipoles for the corresponding delensed Cl_EE spectrum."
                                                    ),
                           pop->error_message,
                           pop->error_message); /* DLM */
                
                sprintf(file_name,"%s%s%s%s",pop->root,"dClEEdClBB_", "delensed", ".dat");
                
                class_call(output_open_cl_derv_file(pop,
                                                    ple,
                                                    &out_cl_dl_ee_derv_bb,
                                                    file_name,
                                                    "DLM: Derivative of the Cl_EE spectrum with respect to unlensed Cl_BB. Rows are L multipoles at the corresponding Cl_BB, unlensed spectrum; and columns are the l multipoles for the corresponding delensed Cl_EE spectrum."
                                                    ),
                           pop->error_message,
                           pop->error_message); /* DLM */
            }
        }
        
        sprintf(file_name,"%s%s%s%s",pop->root,"dClBBdCldd_", derv_type_string, ".dat");
        
        class_call(output_open_cl_derv_file(pop,
                                            ple,
                                            &out_cl_bb_derv,
                                            file_name,
                                            "DLM: Derivative of the Cl_BB spectrum with respect to deflection spectrum Cl_dd. Rows are L multipoles at the corresponding Cl_dd spectrum; and columns are the l multipoles for the corresponding Cl_BB spectrum."
                                            ),
                   pop->error_message,
                   pop->error_message); /* DLM */
        
        if(ple->calculate_derviaties_wrt_unlensed == _TRUE_){
            if(ple->lensed_wrt_unlensed == _TRUE_){
                
                sprintf(file_name,"%s%s%s%s",pop->root,"dClBBdClEE_", "lensed", ".dat");
                
                class_call(output_open_cl_derv_file(pop,
                                                    ple,
                                                    &out_cl_bb_derv_ee,
                                                    file_name,
                                                    "DLM: Derivative of the Cl_BB spectrum with respect to unlensed Cl_EE. Rows are L multipoles at the corresponding Cl_EE, unlensed spectrum; and columns are the l multipoles for the corresponding lensed Cl_BB spectrum."
                                                    ),
                           pop->error_message,
                           pop->error_message); /* DLM */
                
                sprintf(file_name,"%s%s%s%s",pop->root,"dClBBdClBB_", "lensed", ".dat");
                
                class_call(output_open_cl_derv_file(pop,
                                                    ple,
                                                    &out_cl_bb_derv_bb,
                                                    file_name,
                                                    "DLM: Derivative of the Cl_BB spectrum with respect to unlensed Cl_BB. Rows are L multipoles at the corresponding Cl_BB, unlensed spectrum; and columns are the l multipoles for the corresponding lensed Cl_BB spectrum."
                                                    ),
                           pop->error_message,
                           pop->error_message); /* DLM */
            }
            if(ple->delensed_wrt_unlensed == _TRUE_){
                sprintf(file_name,"%s%s%s%s",pop->root,"dClBBdClEE_", "delensed", ".dat");
                
                class_call(output_open_cl_derv_file(pop,
                                                    ple,
                                                    &out_cl_dl_bb_derv_ee,
                                                    file_name,
                                                    "DLM: Derivative of the Cl_BB spectrum with respect to unlensed Cl_EE. Rows are L multipoles at the corresponding Cl_EE, unlensed spectrum; and columns are the l multipoles for the corresponding delensed Cl_BB spectrum."
                                                    ),
                           pop->error_message,
                           pop->error_message); /* DLM */
                
                sprintf(file_name,"%s%s%s%s",pop->root,"dClBBdClBB_", "delensed", ".dat");
                
                class_call(output_open_cl_derv_file(pop,
                                                    ple,
                                                    &out_cl_dl_bb_derv_bb,
                                                    file_name,
                                                    "DLM: Derivative of the Cl_BB spectrum with respect to unlensed Cl_BB. Rows are L multipoles at the corresponding Cl_BB, unlensed spectrum; and columns are the l multipoles for the corresponding delensed Cl_BB spectrum."
                                                    ),
                           pop->error_message,
                           pop->error_message); /* DLM */
            }
        }
    }
    
    
    if (ppt->md_size > 1) {
        
        for (index_md = 0; index_md < ppt->md_size; index_md++) {
            
            if (_scalars_) {
                
                sprintf(file_name,"%s%s",pop->root,"cls.dat");
                strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar mode");
                
            }
            
            if (_tensors_) {
                
                sprintf(file_name,"%s%s",pop->root,"clt.dat");
                strcpy(first_line,"[l(l+1)/2pi] C_l's for tensor mode");
                
            }
            
            class_call(output_open_cl_file(phr,
                                           pop,
                                           &(out_md[index_md]),
                                           file_name,
                                           first_line,
                                           phr->l_max[index_md]
                                           ),
                       pop->error_message,
                       pop->error_message);
            
            class_alloc(cl_md[index_md],
                        phr->ct_size*sizeof(double),
                        pop->error_message);
            
        }
    }
    
    for (index_md = 0; index_md < ppt->md_size; index_md++) {
        
        if (ppt->ic_size[index_md] > 1) {
            
            for (index_ic1 = 0; index_ic1 < ppt->ic_size[index_md]; index_ic1++) {
                
                for (index_ic2 = index_ic1; index_ic2 < ppt->ic_size[index_md]; index_ic2++) {
                    
                    if (_scalars_) {
                        
                        if ((ppt->has_ad == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_ad) && (index_ic2 == ppt->index_ic_ad)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_ad.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar adiabatic (AD) mode");
                        }
                        
                        if ((ppt->has_bi == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_bi) && (index_ic2 == ppt->index_ic_bi)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_bi.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar baryon isocurvature (BI) mode");
                        }
                        
                        if ((ppt->has_cdi == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_cdi) && (index_ic2 == ppt->index_ic_cdi)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_cdi.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar CDM isocurvature (CDI) mode");
                        }
                        
                        if ((ppt->has_nid == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_nid) && (index_ic2 == ppt->index_ic_nid)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_nid.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar neutrino density isocurvature (NID) mode");
                        }
                        
                        if ((ppt->has_niv == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_niv) && (index_ic2 == ppt->index_ic_niv)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_niv.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar neutrino velocity isocurvature (NIV) mode");
                        }
                        
                        if ((ppt->has_ad == _TRUE_) &&
                            (ppt->has_bi == _TRUE_) && (index_ic1 == ppt->index_ic_ad) && (index_ic2 == ppt->index_ic_bi)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_ad_bi.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar cross ADxBI mode");
                        }
                        
                        if ((ppt->has_ad == _TRUE_) && (ppt->has_cdi == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_ad) && (index_ic2 == ppt->index_ic_cdi)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_ad_cdi.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar cross ADxCDI mode");
                        }
                        
                        if ((ppt->has_ad == _TRUE_) && (ppt->has_nid == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_ad) && (index_ic2 == ppt->index_ic_nid)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_ad_nid.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar cross ADxNID mode");
                        }
                        
                        if ((ppt->has_ad == _TRUE_) && (ppt->has_niv == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_ad) && (index_ic2 == ppt->index_ic_niv)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_ad_niv.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar cross ADxNIV mode");
                        }
                        
                        if ((ppt->has_bi == _TRUE_) && (ppt->has_cdi == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_bi) && (index_ic2 == ppt->index_ic_cdi)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_bi_cdi.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar cross BIxCDI mode");
                        }
                        
                        if ((ppt->has_bi == _TRUE_) && (ppt->has_nid == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_bi) && (index_ic2 == ppt->index_ic_nid)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_bi_nid.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar cross BIxNID mode");
                        }
                        
                        if ((ppt->has_bi == _TRUE_) && (ppt->has_niv == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_bi) && (index_ic2 == ppt->index_ic_niv)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_bi_niv.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar cross BIxNIV mode");
                        }
                        
                        if ((ppt->has_cdi == _TRUE_) && (ppt->has_nid == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_cdi) && (index_ic2 == ppt->index_ic_nid)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_cdi_nid.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar cross CDIxNID mode");
                        }
                        
                        if ((ppt->has_cdi == _TRUE_) && (ppt->has_niv == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_cdi) && (index_ic2 == ppt->index_ic_niv)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_cdi_niv.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar cross CDIxNIV mode");
                        }
                        
                        if ((ppt->has_nid == _TRUE_) && (ppt->has_niv == _TRUE_) &&
                            (index_ic1 == ppt->index_ic_nid) && (index_ic2 == ppt->index_ic_niv)) {
                            
                            sprintf(file_name,"%s%s",pop->root,"cls_nid_niv.dat");
                            strcpy(first_line,"[l(l+1)/2pi] C_l's for scalar cross NIDxNIV mode");
                        }
                        
                    }
                    
                    if (_tensors_) {
                        
                        class_test(0==0,
                                   pop->error_message,
                                   "Seems that we have mixed initial conditions for tensors? Should not happen!\n");
                        
                    }
                    
                    index_ic1_ic2 = index_symmetric_matrix(index_ic1,index_ic2,phr->ic_size[index_md]);
                    
                    if (phr->is_non_zero[index_md][index_ic1_ic2] == _TRUE_) {
                        
                        class_call(output_open_cl_file(phr,
                                                       pop,
                                                       &(out_md_ic[index_md][index_ic1_ic2]),
                                                       file_name,
                                                       first_line,
                                                       phr->l_max[index_md]
                                                       ),
                                   pop->error_message,
                                   pop->error_message);
                        
                    }
                }
            }
            
            class_alloc(cl_md_ic[index_md],
                        phr->ic_ic_size[index_md]*phr->ct_size*sizeof(double),
                        pop->error_message);
        }
    }
    
    /** - third, perform loop over l. For each multipole, get all \f$ C_l\f$'s
     by calling harmonic_cl_at_l() and distribute the results to
     relevant files */
    
    for (l = 2; l <= phr->l_max_tot; l++) {
        
        class_call(harmonic_cl_at_l(phr,(double)l,cl_tot,cl_md,cl_md_ic),
                   phr->error_message,
                   pop->error_message);
        
        class_call(output_one_line_of_cl(pba,phr,pop,out,(double)l,cl_tot,phr->ct_size),
                   pop->error_message,
                   pop->error_message);
        
        if ((ple->has_lensed_cls == _TRUE_) && (l<=ple->l_lensed_max)) {
            
            class_call(lensing_cl_at_l(ple,
                                       (double)l,
                                       cl_tot),
                       ple->error_message,
                       pop->error_message);
            
            class_call(output_one_line_of_cl(pba,phr,pop,out_lensed,l,cl_tot,phr->ct_size),
                       pop->error_message,
                       pop->error_message);
            
            if (ple->has_lensed_cls==_TRUE_ && (l<=ple->l_delensed_max) && ple->has_delensed_cls == _TRUE_){ /* DLM */
                
                class_call(delensing_cl_at_l(ple,
                                             (double)l,
                                             cl_tot_dl),
                           ple->error_message,
                           ple->error_message); /* DLM */
                
                class_call(output_one_line_of_cl(pba,phr,pop,out_delensed,(double)l,cl_tot_dl,ple->dlt_size),
                           pop->error_message,
                           pop->error_message); /* DLM */
            }
            
            if (ple->lens_rec_noise_type == internal_rn && (l<=ple->l_delensed_max)){ /* DLM */
                
                class_call(lensing_reconst_nl_at_l(ple,
                                                   (double)l,
                                                   nl_tot_dl),
                           ple->error_message,
                           ple->error_message); /* DLM */
                
                class_call(output_one_line_of_nl(pba,phr,pop,out_rcn_noise,(double)l,nl_tot_dl,ple->nlt_size),
                           pop->error_message,
                           pop->error_message); /* DLM */
                
            }
            
            if (ple->output_spectra_noise == _TRUE_) { /* DLM */
                
                class_call(output_one_line_of_nl_tp(pop,
                                                    out_spec_noise,
                                                    (double)l,
                                                    ple->pk_tn[l]*pow(pba->T_cmb*1.e6,2),
                                                    ple->pk_pn[l]*pow(pba->T_cmb*1.e6,2)),
                           pop->error_message,
                           pop->error_message); /* DLM */
                
            }
            
            if (ple->calculate_pderivaties == _TRUE_ &&
                ple->output_derivatives == _TRUE_ && l<=ple->l_delensed_max) { /* DLM */
                
                //----------------------------------------//
                derv_buffer[0]='\0'; /* DLM */
                buf_length = 0;
                
                for (k=2; k<=ple->l_delensed_max; k++) {
                    
                    if((k-2)%ple->derv_binedges!=0) continue;
                    
                    class_call(delensing_derv_cl_tt_at_l(ple,
                                                         (double)k,
                                                         (double)l,
                                                         &cl_delensed_derv),
                               ple->error_message,
                               ple->error_message);
                    buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                            cl_delensed_derv/(double)(k*(k+1.))*pow(phr->T_cmb*1.e6,2)); 
                }
                buf_length += sprintf(derv_buffer+buf_length, "\n");
                fprintf(out_cl_tt_derv, derv_buffer);
                //----------------------------------------//
                    
                    /*
                    class_call(output_one_line_of_cl_derv(pop,
                                                          ple,
                                                          out_cl_tt_derv,
                                                          cl_delensed_derv/(double)(k*(k+1.))*pow(phr->T_cmb*1.e6,2),
                                                          k,
                                                          ple->l_delensed_max),
                               pop->error_message,
                               pop->error_message); 
                    */
                //----------------------------------------//
                derv_buffer[0]='\0'; /* DLM */
                buf_length = 0;
                
                for (k=2; k<=ple->l_delensed_max; k++) {
                    
                    if((k-2)%ple->derv_binedges!=0) continue;
                    
                    class_call(delensing_derv_cl_te_at_l(ple,
                                                         (double)k,
                                                         (double)l,
                                                         &cl_delensed_derv),
                               ple->error_message,
                               ple->error_message);
                    buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                            cl_delensed_derv/(double)(k*(k+1.))*pow(phr->T_cmb*1.e6,2)); 
                }
                buf_length += sprintf(derv_buffer+buf_length, "\n");
                fprintf(out_cl_te_derv, derv_buffer);
                //----------------------------------------//

                    /*
                    class_call(output_one_line_of_cl_derv(pop,
                                                          ple,
                                                          out_cl_te_derv,
                                                          cl_delensed_derv/(double)(k*(k+1.))*pow(phr->T_cmb*1.e6,2),
                                                          k,
                                                          ple->l_delensed_max),
                               pop->error_message,
                               pop->error_message); 
                    */
                //----------------------------------------//
                derv_buffer[0]='\0'; /* DLM */
                buf_length = 0;
                
                for (k=2; k<=ple->l_delensed_max; k++) {
                    
                    if((k-2)%ple->derv_binedges!=0) continue;
                    
                    class_call(delensing_derv_cl_ee_at_l(ple,
                                                         (double)k,
                                                         (double)l,
                                                         &cl_delensed_derv),
                               ple->error_message,
                               ple->error_message);
                    buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                            cl_delensed_derv/(double)(k*(k+1.))*pow(phr->T_cmb*1.e6,2)); 
                }
                buf_length += sprintf(derv_buffer+buf_length, "\n");
                fprintf(out_cl_ee_derv, derv_buffer);
                //----------------------------------------//
                    /*                    
                    class_call(output_one_line_of_cl_derv(pop,
                                                          ple,
                                                          out_cl_ee_derv,
                                                          cl_delensed_derv/(double)(k*(k+1.))*pow(phr->T_cmb*1.e6,2),
                                                          k,
                                                          ple->l_delensed_max),
                               pop->error_message,
                               pop->error_message); 
                    */
                //----------------------------------------//
                derv_buffer[0]='\0'; /* DLM */
                buf_length = 0;
                
                for (k=2; k<=ple->l_delensed_max; k++) {
                    
                    if((k-2)%ple->derv_binedges!=0) continue;
                    
                    class_call(delensing_derv_cl_bb_at_l(ple,
                                                         (double)k,
                                                         (double)l,
                                                         &cl_delensed_derv),
                               ple->error_message,
                               ple->error_message);
                    buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                            cl_delensed_derv/(double)(k*(k+1.))*pow(phr->T_cmb*1.e6,2)); 
                }
                buf_length += sprintf(derv_buffer+buf_length, "\n");
                fprintf(out_cl_bb_derv, derv_buffer);
                //----------------------------------------//
                    /*    
                    class_call(output_one_line_of_cl_derv(pop,
                                                          ple,
                                                          out_cl_bb_derv,
                                                          cl_delensed_derv/(double)(k*(k+1.))*pow(phr->T_cmb*1.e6,2),
                                                          k,
                                                          ple->l_delensed_max),
                               pop->error_message,
                               pop->error_message); 
                    */
                    //----------------------------------------//
                }
                
                if(ple->calculate_derviaties_wrt_unlensed == _TRUE_){
                    if(ple->lensed_wrt_unlensed == _TRUE_){
                        //----------------------------------------//
                        derv_buffer[0]='\0'; /* DLM */
                        buf_length = 0;
                        
                        for (k=2; k<=ple->l_lensed_max; k++) {
                            
                            if((k-2)%ple->derv_binedges!=0) continue;
                            
                            /*
                            class_call(lensing_derv_cl_tt_tt_at_l(ple,
                                                                  (double)k,
                                                                  (double)l,
                                                                  &cl_lensed_derv_tt_tt),
                                       ple->error_message,
                                       ple->error_message);
                            */
                            // dCl^{TT}/dCk^{TT_unlensed}
                            // Index ordering here chosen for contiguous memory blocks for output
                            // Consistent with derv_all functions in lensing.c
                            buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                                    ple->cl_lens_derv_TT_TT[l][k]); 
                        }
                        buf_length += sprintf(derv_buffer+buf_length, "\n");
                        fprintf(out_cl_tt_derv_tt, derv_buffer);
                        //----------------------------------------//
                            
                            /*
                            class_call(output_one_line_of_cl_derv(pop,
                                                                  ple,
                                                                  out_cl_tt_derv_tt,
                                                                  cl_lensed_derv_tt_tt,
                                                                  k,
                                                                  ple->l_delensed_max),
                                       pop->error_message,
                                       pop->error_message); 
                            */
                        //----------------------------------------//
                        derv_buffer[0]='\0'; /* DLM */
                        buf_length = 0;
                        
                        for (k=2; k<=ple->l_lensed_max; k++) {
                            
                            if((k-2)%ple->derv_binedges!=0) continue;
                            
                            /*
                            class_call(lensing_derv_cl_te_te_at_l(ple,
                                                                  (double)k,
                                                                  (double)l,
                                                                  &cl_lensed_derv_te_te),
                                       ple->error_message,
                                       ple->error_message);
                            */ 
                            buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                                    ple->cl_lens_derv_TE_TE[l][k]); 
                        }
                        buf_length += sprintf(derv_buffer+buf_length, "\n");
                        fprintf(out_cl_te_derv_te, derv_buffer);
                        //----------------------------------------//

                            /*
                            class_call(output_one_line_of_cl_derv(pop,
                                                                  ple,
                                                                  out_cl_te_derv_te,
                                                                  cl_lensed_derv_te_te,
                                                                  k,
                                                                  ple->l_delensed_max),
                                       pop->error_message,
                                       pop->error_message); 
                            */
                        //----------------------------------------//
                        derv_buffer[0]='\0'; /* DLM */
                        buf_length = 0;
                        
                        for (k=2; k<=ple->l_lensed_max; k++) {
                            
                            if((k-2)%ple->derv_binedges!=0) continue;
                            
                            /*
                            class_call(lensing_derv_cl_ee_ee_at_l(ple,
                                                                  (double)k,
                                                                  (double)l,
                                                                  &cl_lensed_derv_ee_ee),
                                       ple->error_message,
                                       ple->error_message);
                            */
                            buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                                    ple->cl_lens_derv_EE_EE[l][k]); 
                        }
                        buf_length += sprintf(derv_buffer+buf_length, "\n");
                        fprintf(out_cl_ee_derv_ee, derv_buffer);
                        //----------------------------------------//

                            /*
                            class_call(output_one_line_of_cl_derv(pop,
                                                                  ple,
                                                                  out_cl_ee_derv_ee,
                                                                  cl_lensed_derv_ee_ee,
                                                                  k,
                                                                  ple->l_delensed_max),
                                       pop->error_message,
                                       pop->error_message); 
                            */
                        //----------------------------------------//
                        derv_buffer[0]='\0'; /* DLM */
                        buf_length = 0;
                        
                        for (k=2; k<=ple->l_lensed_max; k++) {
                            
                            if((k-2)%ple->derv_binedges!=0) continue;
                            
                            /*
                            class_call(lensing_derv_cl_ee_bb_at_l(ple,
                                                                  (double)k,
                                                                  (double)l,
                                                                  &cl_lensed_derv_ee_bb),
                                       ple->error_message,
                                       ple->error_message);
                            */
                            buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                                    ple->cl_lens_derv_EE_BB[l][k]); 
                        }
                        buf_length += sprintf(derv_buffer+buf_length, "\n");
                        fprintf(out_cl_ee_derv_bb, derv_buffer);
                        //----------------------------------------//

                            /*                            
                            class_call(output_one_line_of_cl_derv(pop,
                                                                  ple,
                                                                  out_cl_ee_derv_bb,
                                                                  cl_lensed_derv_ee_bb,
                                                                  k,
                                                                  ple->l_delensed_max),
                                       pop->error_message,
                                       pop->error_message); 
                            */
                        //----------------------------------------//
                        derv_buffer[0]='\0'; /* DLM */
                        buf_length = 0;
                        
                        for (k=2; k<=ple->l_lensed_max; k++) {
                            
                            if((k-2)%ple->derv_binedges!=0) continue;
                            
                            /*
                            class_call(lensing_derv_cl_bb_ee_at_l(ple,
                                                                  (double)k,
                                                                  (double)l,
                                                                  &cl_lensed_derv_bb_ee),
                                       ple->error_message,
                                       ple->error_message);
                            */
                            buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                                    ple->cl_lens_derv_BB_EE[l][k]); 
                        }
                        buf_length += sprintf(derv_buffer+buf_length, "\n");
                        fprintf(out_cl_bb_derv_ee, derv_buffer);
                        //----------------------------------------//

                            /*                            
                            class_call(output_one_line_of_cl_derv(pop,
                                                                  ple,
                                                                  out_cl_bb_derv_ee,
                                                                  cl_lensed_derv_bb_ee,
                                                                  k,
                                                                  ple->l_delensed_max),
                                       pop->error_message,
                                       pop->error_message); 
                            */
                        //----------------------------------------//
                        derv_buffer[0]='\0'; /* DLM */
                        buf_length = 0;
                        
                        for (k=2; k<=ple->l_lensed_max; k++) {
                            
                            if((k-2)%ple->derv_binedges!=0) continue;
                            
                            /*
                            class_call(lensing_derv_cl_bb_bb_at_l(ple,
                                                                  (double)k,
                                                                  (double)l,
                                                                  &cl_lensed_derv_bb_bb),
                                       ple->error_message,
                                       ple->error_message);
                            */
                            buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                                    ple->cl_lens_derv_BB_BB[l][k]); 
                        }
                        buf_length += sprintf(derv_buffer+buf_length, "\n");
                        fprintf(out_cl_bb_derv_bb, derv_buffer);
                        //----------------------------------------//

                            /*
                            class_call(output_one_line_of_cl_derv(pop,
                                                                  ple,
                                                                  out_cl_bb_derv_bb,
                                                                  cl_lensed_derv_bb_bb,
                                                                  k,
                                                                  ple->l_delensed_max),
                                       pop->error_message,
                                       pop->error_message); 
                            */
                            
                    }
                        
                        //----------------------------------------//
                        //----------------------------------------//
                    
                    if(ple->delensed_wrt_unlensed == _TRUE_){
                        //----------------------------------------//
                        derv_buffer[0]='\0'; /* DLM */
                        buf_length = 0;
                        
                        for (k=2; k<=ple->l_lensed_max; k++) {
                            
                            if((k-2)%ple->derv_binedges!=0) continue;
                            
                            /*
                            class_call(lensing_derv_dl_cl_tt_tt_at_l(ple,
                                                                  (double)k,
                                                                  (double)l,
                                                                  &cl_delensed_derv_tt_tt),
                                       ple->error_message,
                                       ple->error_message);
                            */
                            buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                                    ple->cl_delens_derv_TT_TT[l][k]); 
                        }
                        buf_length += sprintf(derv_buffer+buf_length, "\n");
                        fprintf(out_cl_dl_tt_derv_tt, derv_buffer);
                        //----------------------------------------//
                            
                            /*
                            class_call(output_one_line_of_cl_derv(pop,
                                                                  ple,
                                                                  out_cl_dl_tt_derv_tt,
                                                                  cl_delensed_derv_tt_tt,
                                                                  k,
                                                                  ple->l_delensed_max),
                                       pop->error_message,
                                       pop->error_message); 
                            */
                            
                        //----------------------------------------//
                        derv_buffer[0]='\0'; /* DLM */
                        buf_length = 0;
                        
                        for (k=2; k<=ple->l_lensed_max; k++) {
                            
                            if((k-2)%ple->derv_binedges!=0) continue;
                            
                            /*
                            class_call(lensing_derv_dl_cl_te_te_at_l(ple,
                                                                  (double)k,
                                                                  (double)l,
                                                                  &cl_delensed_derv_te_te),
                                       ple->error_message,
                                       ple->error_message);
                            */
                            buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                                    ple->cl_delens_derv_TE_TE[l][k]); 
                        }
                        buf_length += sprintf(derv_buffer+buf_length, "\n");
                        fprintf(out_cl_dl_te_derv_te, derv_buffer);
                        //----------------------------------------//
                            
                            /*
                            class_call(output_one_line_of_cl_derv(pop,
                                                                  ple,
                                                                  out_cl_dl_te_derv_te,
                                                                  cl_delensed_derv_te_te,
                                                                  k,
                                                                  ple->l_delensed_max),
                                       pop->error_message,
                                       pop->error_message); 
                            */
                            
                        //----------------------------------------//
                        derv_buffer[0]='\0'; /* DLM */
                        buf_length = 0;
                        
                        for (k=2; k<=ple->l_lensed_max; k++) {
                            
                            if((k-2)%ple->derv_binedges!=0) continue;
                            
                            /*
                            class_call(lensing_derv_dl_cl_ee_ee_at_l(ple,
                                                                  (double)k,
                                                                  (double)l,
                                                                  &cl_delensed_derv_ee_ee),
                                       ple->error_message,
                                       ple->error_message);
                            */
                            buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                                    ple->cl_delens_derv_EE_EE[l][k]); 
                        }
                        buf_length += sprintf(derv_buffer+buf_length, "\n");
                        fprintf(out_cl_dl_ee_derv_ee, derv_buffer);
                        //----------------------------------------//

                            /*                            
                            class_call(output_one_line_of_cl_derv(pop,
                                                                  ple,
                                                                  out_cl_dl_ee_derv_ee,
                                                                  cl_delensed_derv_ee_ee,
                                                                  k,
                                                                  ple->l_delensed_max),
                                       pop->error_message,
                                       pop->error_message); 
                            */
                            
                        //----------------------------------------//
                        derv_buffer[0]='\0'; /* DLM */
                        buf_length = 0;
                        
                        for (k=2; k<=ple->l_lensed_max; k++) {
                            
                            if((k-2)%ple->derv_binedges!=0) continue;
                            
                            /*
                            class_call(lensing_derv_dl_cl_ee_bb_at_l(ple,
                                                                  (double)k,
                                                                  (double)l,
                                                                  &cl_delensed_derv_ee_bb),
                                       ple->error_message,
                                       ple->error_message);
                            */
                            buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                                    ple->cl_delens_derv_EE_BB[l][k]); 
                        }
                        buf_length += sprintf(derv_buffer+buf_length, "\n");
                        fprintf(out_cl_dl_ee_derv_bb, derv_buffer);
                        //----------------------------------------//

                            /*
                            class_call(output_one_line_of_cl_derv(pop,
                                                                  ple,
                                                                  out_cl_dl_ee_derv_bb,
                                                                  cl_delensed_derv_ee_bb,
                                                                  k,
                                                                  ple->l_delensed_max),
                                       pop->error_message,
                                       pop->error_message); 
                            */

                        //----------------------------------------//
                        derv_buffer[0]='\0'; /* DLM */
                        buf_length = 0;
                        
                        for (k=2; k<=ple->l_lensed_max; k++) {
                            
                            if((k-2)%ple->derv_binedges!=0) continue;
                             /*
                            class_call(lensing_derv_dl_cl_bb_ee_at_l(ple,
                                                                  (double)k,
                                                                  (double)l,
                                                                  &cl_delensed_derv_bb_ee),
                                       ple->error_message,
                                       ple->error_message);
                            */
                            buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                                    ple->cl_delens_derv_BB_EE[l][k]); 
                        }
                        buf_length += sprintf(derv_buffer+buf_length, "\n");
                        fprintf(out_cl_dl_bb_derv_ee, derv_buffer);
                        //----------------------------------------//

                            /*
                            class_call(output_one_line_of_cl_derv(pop,
                                                                  ple,
                                                                  out_cl_dl_bb_derv_ee,
                                                                  cl_delensed_derv_bb_ee,
                                                                  k,
                                                                  ple->l_delensed_max),
                                       pop->error_message,
                                       pop->error_message); 
                            */
                        //----------------------------------------//
                        derv_buffer[0]='\0'; /* DLM */
                        buf_length = 0;
                        
                        for (k=2; k<=ple->l_lensed_max; k++) {
                            
                            if((k-2)%ple->derv_binedges!=0) continue;
                            
                            /*
                            class_call(lensing_derv_dl_cl_bb_bb_at_l(ple,
                                                                  (double)k,
                                                                  (double)l,
                                                                  &cl_delensed_derv_bb_bb),
                                       ple->error_message,
                                       ple->error_message);
                            */
                            buf_length += sprintf(derv_buffer+buf_length, "%*.*e ",_COLUMNWIDTH_,_OUTPUTPRECISION_,
                                                    ple->cl_delens_derv_BB_BB[l][k]); 
                        }
                        buf_length += sprintf(derv_buffer+buf_length, "\n");
                        fprintf(out_cl_dl_bb_derv_bb, derv_buffer);
                        //----------------------------------------//

                            /*
                            class_call(output_one_line_of_cl_derv(pop,
                                                                  ple,
                                                                  out_cl_dl_bb_derv_bb,
                                                                  cl_delensed_derv_bb_bb,
                                                                  k,
                                                                  ple->l_delensed_max),
                                       pop->error_message,
                                       pop->error_message); 
                            */
                         
                    }
                }
            }
        
        
        if (ppt->md_size > 1) {
            for (index_md = 0; index_md < ppt->md_size; index_md++) {
                if (l <= phr->l_max[index_md]) {
                    
                    class_call(output_one_line_of_cl(pba,phr,pop,out_md[index_md],l,cl_md[index_md],phr->ct_size),
                               pop->error_message,
                               pop->error_message);
                }
            }
        }
        
        for (index_md = 0; index_md < ppt->md_size; index_md++) {
            if ((ppt->ic_size[index_md] > 1) && (l <= phr->l_max[index_md])) {
                for (index_ic1_ic2 = 0; index_ic1_ic2 < phr->ic_ic_size[index_md]; index_ic1_ic2++) {
                    if (phr->is_non_zero[index_md][index_ic1_ic2] == _TRUE_) {
                        
                        class_call(output_one_line_of_cl(pba,phr,pop,out_md_ic[index_md][index_ic1_ic2],l,&(cl_md_ic[index_md][index_ic1_ic2*phr->ct_size]),phr->ct_size),
                                   pop->error_message,
                                   pop->error_message);
                    }
                }
            }
        }
    }
    
    /** - finally, close files and free arrays of files and \f$ C_l\f$'s */
    
    for (index_md = 0; index_md < ppt->md_size; index_md++) {
        if (ppt->ic_size[index_md] > 1) {
            for (index_ic1_ic2 = 0; index_ic1_ic2 < phr->ic_ic_size[index_md]; index_ic1_ic2++) {
                if (phr->is_non_zero[index_md][index_ic1_ic2] == _TRUE_) {
                    fclose(out_md_ic[index_md][index_ic1_ic2]);
                }
            }
            free(cl_md_ic[index_md]);
        }
    }
    if (ppt->md_size > 1) {
        for (index_md = 0; index_md < ppt->md_size; index_md++) {
            fclose(out_md[index_md]);
            free(cl_md[index_md]);
        }
    }
    fclose(out);
    if (ple->has_lensed_cls == _TRUE_) {
        fclose(out_lensed);
    }
    free(cl_tot);
    
    
    if (ple->has_delensed_cls == _TRUE_) {
        fclose(out_delensed);
    }
    free(cl_tot_dl);
    
    if (ple->lens_rec_noise_type == internal_rn) {
        fclose(out_rcn_noise);
    }
    free(nl_tot_dl);
    
    if (ple->output_spectra_noise == _TRUE_) {
        fclose(out_spec_noise);
    }
    
    if (ple->calculate_pderivaties == _TRUE_ && ple->output_derivatives == _TRUE_) {
        fclose(out_cl_tt_derv);
        fclose(out_cl_te_derv);
        fclose(out_cl_ee_derv);
        fclose(out_cl_bb_derv);
        if(ple->calculate_derviaties_wrt_unlensed == _TRUE_){
            if(ple->lensed_wrt_unlensed == _TRUE_){
                fclose(out_cl_tt_derv_tt);
                fclose(out_cl_te_derv_te);
                fclose(out_cl_ee_derv_ee);
                fclose(out_cl_ee_derv_bb);
                fclose(out_cl_bb_derv_ee);
                fclose(out_cl_bb_derv_bb);
            }
            if(ple->delensed_wrt_unlensed == _TRUE_){        
                fclose(out_cl_dl_tt_derv_tt);
                fclose(out_cl_dl_te_derv_te);
                fclose(out_cl_dl_ee_derv_ee);
                fclose(out_cl_dl_ee_derv_bb);
                fclose(out_cl_dl_bb_derv_ee);
                fclose(out_cl_dl_bb_derv_bb);
            }
        }
    }
    
    
    for (index_md = 0; index_md < ppt->md_size; index_md++) {
        free(out_md_ic[index_md]);
    }
    free(out_md_ic);
    free(cl_md_ic);
    free(out_md);
    free(cl_md);
    
    return _SUCCESS_;
    
}

/**
 * This routines writes the output in files for Fourier matter power spectra P(k)'s
 * (linear or non-linear)
 *
 * @param pba       Input: pointer to background structure (needed for calling harmonic_pk_at_z())
 * @param ppt       Input: pointer perturbation structure
 * @param pfo       Input: pointer to fourier structure
 * @param pop       Input: pointer to output structure
 * @param pk_output Input: pk_linear or pk_nonlinear
 */

int output_pk(
              struct background * pba,
              struct perturbations * ppt,
              struct fourier * pfo,
              struct output * pop,
              enum pk_outputs pk_output
              ) {
    
    /** Summary: */
    
    /** - define local variables */
    
    FILE ** out_pk_ic = NULL;  /* out_pk_ic[index_ic1_ic2] is a pointer to a file with P(k) for each pair of ic */
    FILE * out_pk;             /* out_pk[index_pk] is a pointer to a file with total P(k) summed over ic */
    
    double * ln_pk_ic = NULL;  /* array ln_pk_ic[index_k * pfo->ic_ic_size + index_ic1_ic2] */
    double * ln_pk;            /* array ln_pk[index_k] */
    
    int index_ic1,index_ic2;
    int index_ic1_ic2=0;
    int index_k;
    int index_z;
    int index_pk;
    
    FileName file_name;
    
    char redshift_suffix[7]; // 7 is enough to write "z%d_" as long as there are at most 10'000 bins
    char type_suffix[9];     // 6 is enough to write "pk_cb_nl" plus closing character \0
    char first_line[_LINE_LENGTH_MAX_];
    short do_ic = _FALSE_;
    
    /** - preliminary: check whether we need to output the decomposition into contributions from each initial condition */
    
    if ((pk_output == pk_linear) && (pfo->ic_size > 1))
        do_ic = _TRUE_;
    
    /** - allocate arrays to store the P(k) */
    
    class_alloc(ln_pk,
                pfo->k_size*sizeof(double),
                pop->error_message);
    
    if (do_ic == _TRUE_) {
        
        class_alloc(ln_pk_ic,
                    pfo->k_size*pfo->ic_ic_size*sizeof(double),
                    pop->error_message);
        
        /** - allocate pointer to output files */
        
        class_alloc(out_pk_ic,
                    pfo->ic_ic_size*sizeof(FILE *),
                    pop->error_message);
    }
    
    /** - loop over pk type (_cb, _m) */
    
    for (index_pk=0; index_pk<pfo->pk_size; index_pk++) {
        
        if ((pfo->has_pk_m == _TRUE_) && (index_pk == pfo->index_pk_m)) {
            if (pk_output == pk_linear)
                sprintf(type_suffix,"pk");
            else
                sprintf(type_suffix,"pk_nl");
        }
        if ((pfo->has_pk_cb == _TRUE_) && (index_pk == pfo->index_pk_cb)) {
            if (pk_output == pk_linear)
                sprintf(type_suffix,"pk_cb");
            else
                sprintf(type_suffix,"pk_cb_nl");
        }
        
        /** - loop over z */
        
        for (index_z = 0; index_z < pop->z_pk_num; index_z++) {
            
            /** - first, check that requested redshift z_pk is consistent */
            
            class_test((pop->z_pk[index_z] > ppt->z_max_pk),
                       pop->error_message,
                       "P(k,z) computed up to z=%f but requested at z=%f. Must increase z_max_pk in precision file.",ppt->z_max_pk,pop->z_pk[index_z]);
            
            if (pop->z_pk_num == 1)
                redshift_suffix[0]='\0';
            else
                sprintf(redshift_suffix,"z%d_",index_z+1);
            
            /** - second, open only the relevant files and write a header in each of them */
            
            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,".dat");
            
            class_call(output_open_pk_file(pba,
                                           pfo,
                                           pop,
                                           &out_pk,
                                           file_name,
                                           "",
                                           pop->z_pk[index_z]
                                           ),
                       pop->error_message,
                       pop->error_message);
            
            if (do_ic == _TRUE_) {
                
                for (index_ic1 = 0; index_ic1 < pfo->ic_size; index_ic1++) {
                    
                    for (index_ic2 = index_ic1; index_ic2 < pfo->ic_size; index_ic2++) {
                        
                        if ((ppt->has_ad == _TRUE_) && (index_ic1 == ppt->index_ic_ad) && (index_ic2 == ppt->index_ic_ad)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_ad.dat");
                            strcpy(first_line,"for adiabatic (AD) mode ");
                        }
                        
                        if ((ppt->has_bi == _TRUE_) && (index_ic1 == ppt->index_ic_bi) && (index_ic2 == ppt->index_ic_bi)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_bi.dat");
                            strcpy(first_line,"for baryon isocurvature (BI) mode ");
                        }
                        
                        if ((ppt->has_cdi == _TRUE_) && (index_ic1 == ppt->index_ic_cdi) && (index_ic2 == ppt->index_ic_cdi)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_cdi.dat");
                            strcpy(first_line,"for CDM isocurvature (CDI) mode ");
                        }
                        
                        if ((ppt->has_nid == _TRUE_) && (index_ic1 == ppt->index_ic_nid) && (index_ic2 == ppt->index_ic_nid)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_nid.dat");
                            strcpy(first_line,"for neutrino density isocurvature (NID) mode ");
                        }
                        
                        if ((ppt->has_niv == _TRUE_) && (index_ic1 == ppt->index_ic_niv) && (index_ic2 == ppt->index_ic_niv)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_niv.dat");
                            strcpy(first_line,"for neutrino velocity isocurvature (NIV) mode ");
                        }
                        
                        if ((ppt->has_ad == _TRUE_) && (ppt->has_bi == _TRUE_) && (index_ic1 == ppt->index_ic_ad) && (index_ic2 == ppt->index_ic_bi)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_ad_bi.dat");
                            strcpy(first_line,"for cross ADxBI mode ");
                        }
                        
                        if ((ppt->has_ad == _TRUE_) && (ppt->has_cdi == _TRUE_) && (index_ic1 == ppt->index_ic_ad) && (index_ic2 == ppt->index_ic_cdi)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_ad_cdi.dat");
                            strcpy(first_line,"for cross ADxCDI mode ");
                        }
                        
                        if ((ppt->has_ad == _TRUE_) && (ppt->has_nid == _TRUE_) && (index_ic1 == ppt->index_ic_ad) && (index_ic2 == ppt->index_ic_nid)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_ad_nid.dat");
                            strcpy(first_line,"for scalar cross ADxNID mode ");
                        }
                        
                        if ((ppt->has_ad == _TRUE_) && (ppt->has_niv == _TRUE_) && (index_ic1 == ppt->index_ic_ad) && (index_ic2 == ppt->index_ic_niv)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_ad_niv.dat");
                            strcpy(first_line,"for cross ADxNIV mode ");
                        }
                        
                        if ((ppt->has_bi == _TRUE_) && (ppt->has_cdi == _TRUE_) && (index_ic1 == ppt->index_ic_bi) && (index_ic2 == ppt->index_ic_cdi)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_bi_cdi.dat");
                            strcpy(first_line,"for cross BIxCDI mode ");
                        }
                        
                        if ((ppt->has_bi == _TRUE_) && (ppt->has_nid == _TRUE_) && (index_ic1 == ppt->index_ic_bi) && (index_ic2 == ppt->index_ic_nid)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_bi_nid.dat");
                            strcpy(first_line,"for cross BIxNID mode ");
                        }
                        
                        if ((ppt->has_bi == _TRUE_) && (ppt->has_niv == _TRUE_) && (index_ic1 == ppt->index_ic_bi) && (index_ic2 == ppt->index_ic_niv)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_bi_niv.dat");
                            strcpy(first_line,"for cross BIxNIV mode ");
                        }
                        
                        if ((ppt->has_cdi == _TRUE_) && (ppt->has_nid == _TRUE_) && (index_ic1 == ppt->index_ic_cdi) && (index_ic2 == ppt->index_ic_nid)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_cdi_nid.dat");
                            strcpy(first_line,"for cross CDIxNID mode ");
                        }
                        
                        if ((ppt->has_cdi == _TRUE_) && (ppt->has_niv == _TRUE_) && (index_ic1 == ppt->index_ic_cdi) && (index_ic2 == ppt->index_ic_niv)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_cdi_niv.dat");
                            strcpy(first_line,"for cross CDIxNIV mode ");
                        }
                        
                        if ((ppt->has_nid == _TRUE_) && (ppt->has_niv == _TRUE_) && (index_ic1 == ppt->index_ic_nid) && (index_ic2 == ppt->index_ic_niv)) {
                            sprintf(file_name,"%s%s%s%s",pop->root,redshift_suffix,type_suffix,"_nid_niv.dat");
                            strcpy(first_line,"for cross NIDxNIV mode ");
                        }
                        
                        index_ic1_ic2 = index_symmetric_matrix(index_ic1,index_ic2,pfo->ic_size);
                        
                        if (pfo->is_non_zero[index_ic1_ic2] == _TRUE_) {
                            
                            class_call(output_open_pk_file(pba,
                                                           pfo,
                                                           pop,
                                                           &(out_pk_ic[index_ic1_ic2]),
                                                           file_name,
                                                           first_line,
                                                           pop->z_pk[index_z]
                                                           ),
                                       pop->error_message,
                                       pop->error_message);
                        }
                    }
                }
            }
            
            /** - third, compute P(k) for each k */
            
            class_call(fourier_pk_at_z(pba,
                                       pfo,
                                       logarithmic,
                                       pk_output,
                                       pop->z_pk[index_z],
                                       index_pk,
                                       ln_pk,
                                       ln_pk_ic
                                       ),
                       pfo->error_message,
                       pop->error_message);
            
            /** - fourth, write in files */
            
            for (index_k=0; index_k<pfo->k_size; index_k++) {
                
                class_call(output_one_line_of_pk(out_pk,
                                                 exp(pfo->ln_k[index_k])/pba->h,
                                                 exp(ln_pk[index_k])*pow(pba->h,3)
                                                 ),
                           pop->error_message,
                           pop->error_message);
                
                if (do_ic == _TRUE_) {
                    
                    for (index_ic1_ic2 = 0; index_ic1_ic2 < pfo->ic_ic_size; index_ic1_ic2++) {
                        
                        if (pfo->is_non_zero[index_ic1_ic2] == _TRUE_) {
                            
                            class_call(output_one_line_of_pk(out_pk_ic[index_ic1_ic2],
                                                             exp(pfo->ln_k[index_k])/pba->h,
                                                             exp(ln_pk_ic[index_k * pfo->ic_ic_size + index_ic1_ic2])*pow(pba->h,3)),
                                       pop->error_message,
                                       pop->error_message);
                        }
                    }
                }
            } /* end loop over k */
            
            /** - fifth, close files */
            
            fclose(out_pk);
            
            if (do_ic == _TRUE_) {
                for (index_ic1_ic2 = 0; index_ic1_ic2 < pfo->ic_ic_size; index_ic1_ic2++) {
                    if (pfo->is_non_zero[index_ic1_ic2] == _TRUE_) {
                        fclose(out_pk_ic[index_ic1_ic2]);
                    }
                }
            }
            
        } /* end loop over index_z */
        
    } /* end loop over index_pk */
    
    /* free arrays and pointers */
    free(ln_pk);
    if (pk_output == pk_linear) {
        free(ln_pk_ic);
        free(out_pk_ic);
    }
    
    return _SUCCESS_;
}

/**
 * This routines writes the output in files for matter transfer functions \f$ T_i(k)\f$'s.
 *
 * @param pba Input: pointer to background structure (needed for calling harmonic_pk_at_z())
 * @param ppt Input: pointer perturbation structure
 * @param pop Input: pointer to output structure
 */

int output_tk(
              struct background * pba,
              struct perturbations * ppt,
              struct output * pop
              ) {
    
    /** Summary: */
    
    /** - define local variables */
    char titles[_MAXTITLESTRINGLENGTH_]={0};
    double * data;
    int size_data, number_of_titles;
    
    FILE * tkfile;
    
    int index_md;
    int index_ic;
    int index_z;
    
    double z;
    
    FileName file_name;
    char redshift_suffix[7]; // 7 is enough to write "z%d_" as long as there are at most 10'000 bins
    char first_line[_LINE_LENGTH_MAX_];
    char ic_suffix[4];   // 4 is enough to write "ad", "bi", "cdi", "nid", "niv", ...
    
    
    index_md=ppt->index_md_scalars;
    
    if (pop->output_format == camb_format) {
        
        class_test(pba->N_ncdm>1,
                   pop->error_message,
                   "you wish to output the transfer functions in CMBFAST/CAMB format but you have more than one non-cold dark matter (ncdm) species. The two are not compatible (since CMBFAST/CAMB only have one ncdm species): switch to CLASS output format or keep only on ncdm species");
        
        class_test(ppt->has_velocity_transfers == _TRUE_,
                   pop->error_message,
                   "you wish to output the transfer functions in CMBFAST/CAMB format, but you requested velocity transfer functions. The two are not compatible (since CMBFAST/CAMB do not compute velocity transfer functions): switch to CLASS output format, or ask only for density transfer function");
    }
    
    
    class_call(perturbations_output_titles(pba,ppt,pop->output_format,titles),
               pba->error_message,
               pop->error_message);
    number_of_titles = get_number_of_titles(titles);
    size_data = number_of_titles*ppt->k_size[index_md];
    
    class_alloc(data, sizeof(double)*ppt->ic_size[index_md]*size_data, pop->error_message);
    
    for (index_z = 0; index_z < pop->z_pk_num; index_z++) {
        
        z = pop->z_pk[index_z];
        
        /** - first, check that requested redshift z_pk is consistent */
        
        class_test((pop->z_pk[index_z] > ppt->z_max_pk),
                   pop->error_message,
                   "T_i(k,z) computed up to z=%f but requested at z=%f. Must increase z_max_pk in precision file.",ppt->z_max_pk,pop->z_pk[index_z]);
        
        if (pop->z_pk_num == 1)
            redshift_suffix[0]='\0';
        else
            sprintf(redshift_suffix,"z%d_",index_z+1);
        
        /** - second, open only the relevant files, and write a heading in each of them */
        
        class_call(perturbations_output_data(pba,
                                             ppt,
                                             pop->output_format,
                                             pop->z_pk[index_z],
                                             number_of_titles,
                                             data
                                             ),
                   ppt->error_message,
                   pop->error_message);
        
        for (index_ic = 0; index_ic < ppt->ic_size[index_md]; index_ic++) {
            
            class_call(perturbations_output_firstline_and_ic_suffix(ppt, index_ic, first_line, ic_suffix),
                       ppt->error_message, pop->error_message);
            
            if ((ppt->has_ad == _TRUE_) && (ppt->ic_size[index_md] == 1) )
                sprintf(file_name,"%s%s%s",pop->root,redshift_suffix,"tk.dat");
            else
                sprintf(file_name,"%s%s%s%s%s",pop->root,redshift_suffix,"tk_",ic_suffix,".dat");
            
            class_open(tkfile, file_name, "w", pop->error_message);
            
            if (pop->write_header == _TRUE_) {
                if (pop->output_format == class_format) {
                    fprintf(tkfile,"# Transfer functions T_i(k) %sat redshift z=%g\n",first_line,z);
                    fprintf(tkfile,"# for k=%g to %g h/Mpc,\n",ppt->k[index_md][0]/pba->h,ppt->k[index_md][ppt->k_size[index_md]-1]/pba->h);
                    fprintf(tkfile,"# number of wavenumbers equal to %d\n",ppt->k_size[index_md]);
                    if (ppt->has_density_transfers == _TRUE_) {
                        fprintf(tkfile,"# d_i   stands for (delta rho_i/rho_i)(k,z) with above normalization \n");
                        fprintf(tkfile,"# d_tot stands for (delta rho_tot/rho_tot)(k,z) with rho_Lambda NOT included in rho_tot\n");
                        fprintf(tkfile,"# (note that this differs from the transfer function output from CAMB/CMBFAST, which gives the same\n");
                        fprintf(tkfile,"#  quantities divided by -k^2 with k in Mpc^-1; use format=camb to match CAMB)\n");
                    }
                    if (ppt->has_velocity_transfers == _TRUE_) {
                        fprintf(tkfile,"# t_i   stands for theta_i(k,z) with above normalization \n");
                        fprintf(tkfile,"# t_tot stands for (sum_i [rho_i+p_i] theta_i)/(sum_i [rho_i+p_i]))(k,z)\n");
                    }
                    fprintf(tkfile,"#\n");
                }
                else if (pop->output_format == camb_format) {
                    
                    fprintf(tkfile,"# Rescaled matter transfer functions [-T_i(k)/k^2] %sat redshift z=%g\n",first_line,z);
                    fprintf(tkfile,"# for k=%g to %g h/Mpc,\n",ppt->k[index_md][0]/pba->h,ppt->k[index_md][ppt->k_size[index_md]-1]/pba->h);
                    fprintf(tkfile,"# number of wavenumbers equal to %d\n",ppt->k_size[index_md]);
                    fprintf(tkfile,"# T_i   stands for (delta rho_i/rho_i)(k,z) with above normalization \n");
                    fprintf(tkfile,"# The rescaling factor [-1/k^2] with k in 1/Mpc is here to match the CMBFAST/CAMB output convention\n");
                    fprintf(tkfile,"#\n");
                    fprintf(tkfile,"#");
                    fprintf(tkfile,"\n");
                    
                }
            }
            
            output_print_data(tkfile,
                              titles,
                              data+index_ic*size_data,
                              size_data);
            
            /** - free memory and close files */
            fclose(tkfile);
            
        }
        
    }
    
    free(data);
    
    return _SUCCESS_;
    
}

int output_background(
                      struct background * pba,
                      struct output * pop
                      ) {
    
    FILE * backfile;
    FileName file_name;
    
    char titles[_MAXTITLESTRINGLENGTH_]={0};
    double * data;
    int size_data, number_of_titles;
    
    class_call(background_output_titles(pba,titles),
               pba->error_message,
               pop->error_message);
    number_of_titles = get_number_of_titles(titles);
    size_data = number_of_titles*pba->bt_size;
    class_alloc(data,sizeof(double)*size_data,pop->error_message);
    class_call(background_output_data(pba,
                                      number_of_titles,
                                      data),
               pba->error_message,
               pop->error_message);
    
    sprintf(file_name,"%s%s",pop->root,"background.dat");
    class_open(backfile,file_name,"w",pop->error_message);
    
    if (pop->write_header == _TRUE_) {
        fprintf(backfile,"# Table of selected background quantities\n");
        fprintf(backfile,"# All densities are multiplied by (8piG/3) (below, shortcut notation (.) for this factor) \n");
        fprintf(backfile,"# Densities are in units [Mpc^-2] while all distances are in [Mpc]. \n");
        if (pba->has_scf == _TRUE_){
            fprintf(backfile,"# The units of phi, tau in the derivatives and the potential V are the following:\n");
            fprintf(backfile,"# --> phi is given in units of the reduced Planck mass m_Pl = (8 pi G)^(-1/2)\n");
            fprintf(backfile,"# --> tau in the derivative of V(phi) is given in units of Mpc.\n");
            fprintf(backfile,"# --> the potential V(phi) is given in units of m_Pl^2/Mpc^2.\n");
        }
    }
    
    output_print_data(backfile,
                      titles,
                      data,
                      size_data);
    
    free(data);
    fclose(backfile);
    
    return _SUCCESS_;
    
}

int output_thermodynamics(
                          struct background * pba,
                          struct thermodynamics * pth,
                          struct output * pop
                          ) {
    
    FileName file_name;
    FILE * thermofile;
    char titles[_MAXTITLESTRINGLENGTH_]={0};
    double * data;
    int size_data, number_of_titles;
    
    class_call(thermodynamics_output_titles(pba,pth,titles),
               pth->error_message,
               pop->error_message);
    number_of_titles = get_number_of_titles(titles);
    size_data = number_of_titles*pth->tt_size;
    class_alloc(data,sizeof(double)*size_data,pop->error_message);
    class_call(thermodynamics_output_data(pba,
                                          pth,
                                          number_of_titles,
                                          data),
               pth->error_message,
               pop->error_message);
    
    sprintf(file_name,"%s%s",pop->root,"thermodynamics.dat");
    class_open(thermofile,file_name,"w",pop->error_message);
    
    if (pop->write_header == _TRUE_) {
        fprintf(thermofile,"# Table of selected thermodynamics quantities\n");
        fprintf(thermofile,"# The following notation is used in column titles:\n");
        fprintf(thermofile,"#         x_e = electron ionization fraction\n");
        fprintf(thermofile,"#      -kappa = optical depth\n");
        fprintf(thermofile,"#      kappa' = Thomson scattering rate, prime denotes conformal time derivatives\n");
        fprintf(thermofile,"#           g = kappa' e^-kappa = visibility function \n");
        fprintf(thermofile,"#          Tb = baryon temperature \n");
        fprintf(thermofile,"#         w_b = baryon equation of state parameter \n");
        fprintf(thermofile,"#       c_b^2 = baryon sound speed squared \n");
        fprintf(thermofile,"#       tau_d = baryon drag optical depth \n");
        if (pth->compute_damping_scale == _TRUE_)
            fprintf(thermofile,"#         r_d = approximate comoving value of photon damping scale \n");
        if(pba->has_idm_dr == _TRUE_) {
            fprintf(thermofile,"#  dmu_idm_dr = scattering rate of idr with idm_dr (i.e. idr opacity to idm_dr scattering) (units 1/Mpc)\n");
            fprintf(thermofile,"# ddmu_idm_dr = derivative of this rate\n");
            fprintf(thermofile,"#  tau_idm_dr = optical depth of idm_dr (due to interactions with idr) \n");
            fprintf(thermofile,"#     tau_idr = optical depth of idr (due to self-interactions) \n");
            fprintf(thermofile,"#    g_idm_dr = visibility function of idm_idr \n");
            fprintf(thermofile,"#  c_idm_dr^2 = interacting dark matter squared sound speed \n");
            fprintf(thermofile,"#    T_idm_dr = temperature of DM interacting with DR \n");
            fprintf(thermofile,"#     dmu_idr = idr self-interaction rate \n");
        }
    }
    
    output_print_data(thermofile,
                      titles,
                      data,
                      size_data);
    
    free(data);
    fclose(thermofile);
    
    return _SUCCESS_;
    
}


int output_perturbations(
                         struct background * pba,
                         struct perturbations * ppt,
                         struct output * pop
                         ) {
    
    FILE * out;
    FileName file_name;
    int index_ikout, index_md;
    double k;
    
    for (index_ikout=0; index_ikout<ppt->k_output_values_num; index_ikout++){
        
        if (ppt->has_scalars == _TRUE_){
            index_md = ppt->index_md_scalars;
            k = ppt->k[index_md][ppt->index_k_output_values[index_md*ppt->k_output_values_num+index_ikout]];
            sprintf(file_name,"%s%s%d%s",pop->root,"perturbations_k",index_ikout,"_s.dat");
            class_open(out, file_name, "w", ppt->error_message);
            fprintf(out,"#scalar perturbations for mode k = %.*e Mpc^(-1)\n",_OUTPUTPRECISION_,k);
            output_print_data(out,
                              ppt->scalar_titles,
                              ppt->scalar_perturbations_data[index_ikout],
                              ppt->size_scalar_perturbation_data[index_ikout]);
            
            fclose(out);
        }
        if (ppt->has_vectors == _TRUE_){
            index_md = ppt->index_md_vectors;
            k = ppt->k[index_md][ppt->index_k_output_values[index_md*ppt->k_output_values_num+index_ikout]];
            sprintf(file_name,"%s%s%d%s",pop->root,"perturbations_k",index_ikout,"_v.dat");
            class_open(out, file_name, "w", ppt->error_message);
            fprintf(out,"#vector perturbations for mode k = %.*e Mpc^(-1)\n",_OUTPUTPRECISION_,k);
            output_print_data(out,
                              ppt->vector_titles,
                              ppt->vector_perturbations_data[index_ikout],
                              ppt->size_vector_perturbation_data[index_ikout]);
            
            fclose(out);
        }
        if (ppt->has_tensors == _TRUE_){
            index_md = ppt->index_md_tensors;
            k = ppt->k[index_md][ppt->index_k_output_values[index_md*ppt->k_output_values_num+index_ikout]];
            sprintf(file_name,"%s%s%d%s",pop->root,"perturbations_k",index_ikout,"_t.dat");
            class_open(out, file_name, "w", ppt->error_message);
            fprintf(out,"#tensor perturbations for mode k = %.*e Mpc^(-1)\n",_OUTPUTPRECISION_,k);
            output_print_data(out,
                              ppt->tensor_titles,
                              ppt->tensor_perturbations_data[index_ikout],
                              ppt->size_tensor_perturbation_data[index_ikout]);
            
            fclose(out);
        }
        
        
    }
    return _SUCCESS_;
    
}

int output_primordial(
                      struct perturbations * ppt,
                      struct primordial * ppm,
                      struct output * pop
                      ) {
    FileName file_name;
    FILE * out;
    char titles[_MAXTITLESTRINGLENGTH_]={0};
    double * data;
    int size_data, number_of_titles;
    
    sprintf(file_name,"%s%s",pop->root,"primordial_Pk.dat");
    
    class_call(primordial_output_titles(ppt,ppm,titles),
               ppm->error_message,
               pop->error_message);
    number_of_titles = get_number_of_titles(titles);
    size_data = number_of_titles*ppm->lnk_size;
    class_alloc(data,sizeof(double)*size_data,pop->error_message);
    class_call(primordial_output_data(ppt,
                                      ppm,
                                      number_of_titles,
                                      data),
               ppm->error_message,
               pop->error_message);
    
    class_open(out,file_name,"w",pop->error_message);
    if (pop->write_header == _TRUE_) {
        fprintf(out,"# Dimensionless primordial spectrum, equal to [k^3/2pi^2] P(k) \n");
    }
    
    output_print_data(out,
                      titles,
                      data,
                      size_data);
    
    free(data);
    fclose(out);
    
    return _SUCCESS_;
}

int output_heating(struct injection* pin, struct noninjection* pni, struct output * pop) {
    
    /** Local variables*/
    FileName file_name_injection;
    FILE * out_injection;
    FileName file_name_noninjection;
    FILE * out_noninjection;
    
    char titles_injection[_MAXTITLESTRINGLENGTH_]={0};
    
    double * data_injection;
    int size_data_injection;
    int number_of_titles_injection;
    
    char titles_noninjection[_MAXTITLESTRINGLENGTH_]={0};
    
    double * data_noninjection;
    int size_data_noninjection;
    int number_of_titles_noninjection;
    
    if(pop->write_exotic_injection == _TRUE_){
        
        /* File name */
        sprintf(file_name_injection,"%s%s",pop->root,"exotic_injection.dat");
        
        /* Titles */
        class_call(injection_output_titles(pin,titles_injection),
                   pin->error_message,
                   pin->error_message);
        number_of_titles_injection = get_number_of_titles(titles_injection);
        
        /* Data array */
        size_data_injection = number_of_titles_injection*pin->z_size;
        class_alloc(data_injection,
                    sizeof(double)*size_data_injection,
                    pop->error_message);
        class_call(injection_output_data(pin,
                                         number_of_titles_injection,
                                         data_injection),
                   pin->error_message,
                   pop->error_message);
        
        /* File IO */
        class_open(out_injection,
                   file_name_injection,
                   "w",
                   pop->error_message);
        
        if(pop->write_header == _TRUE_){
            fprintf(out_injection,"# Table of energy injection and deposition from exotic processes \n");
            fprintf(out_injection,"# Heat is dE/dt|dep_h\n");
        }
        
        output_print_data(out_injection,
                          titles_injection,
                          data_injection,
                          size_data_injection);
        free(data_injection);
        fclose(out_injection);
        
    }
    
    if(pop->write_noninjection == _TRUE_){
        
        /* File name */
        sprintf(file_name_noninjection,"%s%s",pop->root,"photon_noninjection.dat");
        
        /* Titles */
        class_call(noninjection_output_titles(pni,titles_noninjection),
                   pni->error_message,
                   pni->error_message);
        number_of_titles_noninjection = get_number_of_titles(titles_noninjection);
        
        /* Data array */
        size_data_noninjection = number_of_titles_noninjection*pin->z_size;
        class_alloc(data_noninjection,
                    sizeof(double)*size_data_noninjection,
                    pop->error_message);
        class_call(noninjection_output_data(pni,
                                            number_of_titles_noninjection,
                                            data_noninjection),
                   pni->error_message,
                   pop->error_message);
        
        /* File IO */
        class_open(out_noninjection,
                   file_name_noninjection,
                   "w",
                   pop->error_message);
        
        if(pop->write_header == _TRUE_){
            fprintf(out_noninjection,"# Table of non-injected energy influencing the photon spectral distortions \n");
        }
        
        output_print_data(out_noninjection,
                          titles_noninjection,
                          data_noninjection,
                          size_data_noninjection);
        free(data_noninjection);
        fclose(out_noninjection);
        
    }
    
    return _SUCCESS_;
}

int output_distortions(
                       struct distortions * psd,
                       struct output * pop
                       ) {
    
    /** Local variables*/
    FileName file_name_heat, file_name_distortion;
    FILE * out_heat, * out_distortion;
    
    char titles_heat[_MAXTITLESTRINGLENGTH_]={0};
    char titles_distortion[_MAXTITLESTRINGLENGTH_]={0};
    
    double * data_heat, * data_distortion;
    int size_data_heat, size_data_distortion;
    int number_of_titles_heat, number_of_titles_distortion;
    
    if(pop->write_distortions==_TRUE_ && psd->has_distortions == _TRUE_){
        
        /* File name */
        sprintf(file_name_heat,"%s%s",pop->root,"sd_heating.dat");
        
        /* Titles */
        class_call(distortions_output_heat_titles(psd,titles_heat),
                   psd->error_message,
                   pop->error_message);
        number_of_titles_heat = get_number_of_titles(titles_heat);
        
        /* Data array */
        size_data_heat = number_of_titles_heat*psd->z_size;
        class_alloc(data_heat,
                    sizeof(double)*size_data_heat,
                    pop->error_message);
        class_call(distortions_output_heat_data(psd,
                                                number_of_titles_heat,
                                                data_heat),
                   psd->error_message,
                   pop->error_message);
        
        /* File IO */
        class_open(out_heat,
                   file_name_heat,
                   "w",
                   pop->error_message);
        
        if(pop->write_header == _TRUE_){
            fprintf(out_heat,"# Heat is d(Q/rho)/dz\n");
            fprintf(out_heat,"# LHeat is d(Q/rho)/dlnz\n");
            fprintf(out_heat,"#\n");
        }
        
        output_print_data(out_heat,
                          titles_heat,
                          data_heat,
                          size_data_heat);
        free(data_heat);
        fclose(out_heat);
        
        /* File name */
        sprintf(file_name_distortion,"%s%s",pop->root,"sd_distortions.dat");
        
        /* Titles */
        class_call(distortions_output_sd_titles(psd,titles_distortion),
                   psd->error_message,
                   pop->error_message);
        number_of_titles_distortion = get_number_of_titles(titles_distortion);
        
        /* Data array */
        size_data_distortion = number_of_titles_distortion*psd->x_size;
        class_alloc(data_distortion,
                    sizeof(double)*size_data_distortion,
                    pop->error_message);
        class_call(distortions_output_sd_data(psd,
                                              number_of_titles_distortion,
                                              data_distortion),
                   psd->error_message,
                   pop->error_message);
        
        /* File IO */
        class_open(out_distortion,
                   file_name_distortion,
                   "w",
                   pop->error_message);
        
        if(pop->write_header == _TRUE_){
            fprintf(out_distortion,"# SD_tot is the amplitude of the overall spectral distortion (SD)\n");
            fprintf(out_distortion,"# The SD[i] are the amplitudes of the individual SDs\n");
            fprintf(out_distortion,"# The SDs are given in units [10^-26 W m^-2 Hz^-1 sr^-1] \n");
            fprintf(out_distortion,"#\n");
        }
        
        output_print_data(out_distortion,
                          titles_distortion,
                          data_distortion,
                          size_data_distortion);
        free(data_distortion);
        fclose(out_distortion);
    }
    
    return _SUCCESS_;
}


int output_print_data(FILE *out,
                      char titles[_MAXTITLESTRINGLENGTH_],
                      double *dataptr,
                      int size_dataptr){
    int colnum=1, number_of_titles;
    int index_title, index_tau;
    char thetitle[_MAXTITLESTRINGLENGTH_];
    char *pch;
    
    /** Summary*/
    
    /** - First we print the titles */
    fprintf(out,"#");
    
    strcpy(thetitle,titles);
    pch = strtok(thetitle,_DELIMITER_);
    while (pch != NULL){
        class_fprintf_columntitle(out, pch, _TRUE_, colnum);
        pch = strtok(NULL,_DELIMITER_);
    }
    fprintf(out,"\n");
    
    /** - Then we print the data */
    number_of_titles = colnum-1;
    if (number_of_titles>0){
        for (index_tau=0; index_tau<size_dataptr/number_of_titles; index_tau++){
            fprintf(out," ");
            for (index_title=0; index_title<number_of_titles; index_title++){
                class_fprintf_double(out, dataptr[index_tau*number_of_titles+index_title], _TRUE_);
            }
            fprintf(out,"\n");
        }
    }
    return _SUCCESS_;
}


/**
 * This routine opens one file where some \f$ C_l\f$'s will be written, and writes
 * a heading with some general information concerning its content.
 *
 * @param phr        Input: pointer to harmonic structure
 * @param pop        Input: pointer to output structure
 * @param clfile     Output: returned pointer to file pointer
 * @param filename   Input: name of the file
 * @param first_line Input: text describing the content (mode, initial condition..)
 * @param lmax       Input: last multipole in the file (the first one is assumed to be 2)
 * @return the error status
 */

int output_open_cl_file(
                        struct harmonic * phr,
                        struct output * pop,
                        FILE * * clfile,
                        FileName filename,
                        char * first_line,
                        int lmax
                        ) {
    /** Summary */
    
    int index_d1,index_d2;
    int colnum = 1;
    char tmp[60]; //A fixed number here is ok, since it should just correspond to the largest string which is printed to tmp.
    
    class_open(*clfile,filename,"w",pop->error_message);
    
    if (pop->write_header == _TRUE_) {
        
        /** - First we deal with the entries that are dependent of format type */
        
        if (pop->output_format == class_format) {
            fprintf(*clfile,"# dimensionless %s\n",first_line);
        }
        if (pop->output_format == camb_format) {
            fprintf(*clfile,"# %s (units: [microK]^2)\n",first_line);
        }
        
        fprintf(*clfile,"# for l=2 to %d, i.e. number of multipoles equal to %d\n",lmax,lmax-1);
        fprintf(*clfile,"#\n");
        
        if (pop->output_format == class_format) {
            fprintf(*clfile,"# -> if you prefer output in CAMB/HealPix/LensPix units/order, set 'format' to 'camb' in input file\n");
        }
        
        fprintf(*clfile,"# -> if you don't want to see such a header, set 'headers' to 'no' in input file\n");
        
        if (phr->has_pp == _TRUE_) {
            if (pop->output_format == class_format) {
                fprintf(*clfile,"# -> for CMB lensing (phi), these are C_l^phi-phi for the lensing potential.\n");
            }
            if (pop->output_format == camb_format) {
                fprintf(*clfile,"# -> for CMB lensing (d), these are C_l^dd for the deflection field.\n");
            }
        }
        
        if (phr->has_ll == _TRUE_) {
            fprintf(*clfile,"# -> for galaxy lensing (lens[i]), these are C_l^phi-phi for the lensing potential.\n");
        }
        
        if (phr->has_pp == _TRUE_ || phr->has_ll == _TRUE_) {
            fprintf(*clfile,"#    Remember the conversion factors:\n");
            fprintf(*clfile,"#    C_l^dd (deflection) = l(l+1) C_l^phi-phi\n");
            fprintf(*clfile,"#    C_l^gg (shear/convergence) = 1/4 (l(l+1))^2 C_l^phi-phi\n");
        }
        
        fprintf(*clfile,"#\n");
        
        if (0==1){
            fprintf(*clfile,"#");
            class_fprintf_columntitle(*clfile,"l",_TRUE_,colnum);
        }
        else{
            fprintf(*clfile,"# 1:l ");
            colnum++;
        }
        if (pop->output_format == class_format) {
            class_fprintf_columntitle(*clfile,"TT",phr->has_tt,colnum);
            class_fprintf_columntitle(*clfile,"EE",phr->has_ee,colnum);
            class_fprintf_columntitle(*clfile,"TE",phr->has_te,colnum);
            class_fprintf_columntitle(*clfile,"BB",phr->has_bb,colnum);
            class_fprintf_columntitle(*clfile,"phiphi",phr->has_pp,colnum);
            class_fprintf_columntitle(*clfile,"TPhi",phr->has_tp,colnum);
            class_fprintf_columntitle(*clfile,"Ephi",phr->has_ep,colnum);
        }
        else if (pop->output_format == camb_format) {
            class_fprintf_columntitle(*clfile,"TT",phr->has_tt,colnum);
            class_fprintf_columntitle(*clfile,"EE",phr->has_ee,colnum);
            class_fprintf_columntitle(*clfile,"BB",phr->has_bb,colnum);
            class_fprintf_columntitle(*clfile,"TE",phr->has_te,colnum);
            class_fprintf_columntitle(*clfile,"dd",phr->has_pp,colnum);
            class_fprintf_columntitle(*clfile,"dT",phr->has_tp,colnum);
            class_fprintf_columntitle(*clfile,"dE",phr->has_ep,colnum);
        }
        
        /** - Next deal with entries that are independent of format type */
        
        if (phr->has_dd == _TRUE_){
            for (index_d1=0; index_d1<phr->d_size; index_d1++){
                for (index_d2=index_d1; index_d2<=MIN(index_d1+phr->non_diag,phr->d_size-1); index_d2++){
                    sprintf(tmp,"dens[%d]-dens[%d]",index_d1+1,index_d2+1);
                    class_fprintf_columntitle(*clfile,tmp,_TRUE_,colnum);
                }
            }
        }
        if (phr->has_td == _TRUE_){
            for (index_d1=0; index_d1<phr->d_size; index_d1++){
                sprintf(tmp,"T-dens[%d]",index_d1+1);
                class_fprintf_columntitle(*clfile,tmp,_TRUE_,colnum);
            }
        }
        if (phr->has_pd == _TRUE_){
            for (index_d1=0; index_d1<phr->d_size; index_d1++){
                sprintf(tmp,"phi-dens[%d]",index_d1+1);
                class_fprintf_columntitle(*clfile,tmp,_TRUE_,colnum);
            }
        }
        if (phr->has_ll == _TRUE_){
            for (index_d1=0; index_d1<phr->d_size; index_d1++){
                for (index_d2=index_d1; index_d2<=MIN(index_d1+phr->non_diag,phr->d_size-1); index_d2++){
                    sprintf(tmp,"lens[%d]-lens[%d]",index_d1+1,index_d2+1);
                    class_fprintf_columntitle(*clfile,tmp,_TRUE_,colnum);
                }
            }
        }
        if (phr->has_tl == _TRUE_){
            for (index_d1=0; index_d1<phr->d_size; index_d1++){
                sprintf(tmp,"T-lens[%d]",index_d1+1);
                class_fprintf_columntitle(*clfile,tmp,_TRUE_,colnum);
            }
        }
        if (phr->has_dl == _TRUE_){
            for (index_d1=0; index_d1<phr->d_size; index_d1++){
                for (index_d2=MAX(index_d1-phr->non_diag,0); index_d2<=MIN(index_d1+phr->non_diag,phr->d_size-1); index_d2++) {
                    sprintf(tmp,"dens[%d]-lens[%d]",index_d1+1,index_d2+1);
                    class_fprintf_columntitle(*clfile,tmp,_TRUE_,colnum);
                }
            }
        }
        fprintf(*clfile,"\n");
    }
    
    return _SUCCESS_;
    
}


int output_open_nl_lens_file(
                             struct harmonic * phr,
                             struct output * pop,
                             struct lensing * ple,
                             FILE * * clfile,
                             FileName filename,
                             char * first_line,
                             int lmax
                             ) {
    /** Summary */
    int index_d1,index_d2;
    int colnum = 1;
    char tmp[60]; //A fixed number here is ok, since it should just correspond to the largest string which is printed to tmp.
    
    class_open(*clfile,filename,"w",pop->error_message);
    
    if (pop->write_header == _TRUE_) {
        
        fprintf(*clfile,"# %s\n",first_line);
        
        /** - First we deal with the entries that are dependent of format type */
        
        fprintf(*clfile,"# for l=2 to %d, i.e. number of multipoles equal to %d\n",lmax,lmax-1);
        fprintf(*clfile,"#\n");
        
        fprintf(*clfile,"# -> if you don't want to see such a header, set 'headers' to 'no' in input file\n");
        
        fprintf(*clfile,"#\n");
        
        if (0==1){
            fprintf(*clfile,"#");
            class_fprintf_columntitle(*clfile,"l",_TRUE_,colnum);
        }
        else{
            fprintf(*clfile,"# 1:l ");
            colnum++;
        }
        if (ple->has_nl_all == _TRUE_) {
            class_fprintf_columntitle(*clfile,"MV (ALL)",phr->has_tt,colnum);
            class_fprintf_columntitle(*clfile,"TT-TT",phr->has_tt,colnum);
            class_fprintf_columntitle(*clfile,"TE-TE",phr->has_ee,colnum);
            class_fprintf_columntitle(*clfile,"EE-EE",phr->has_bb,colnum);
            class_fprintf_columntitle(*clfile,"BB-BB",phr->has_te,colnum);
            class_fprintf_columntitle(*clfile,"EB-EB",phr->has_pp,colnum);
            class_fprintf_columntitle(*clfile,"TB-TB",phr->has_tp,colnum);
            class_fprintf_columntitle(*clfile,"TT-TE",phr->has_ep,colnum);
            class_fprintf_columntitle(*clfile,"TT-EE",phr->has_ep,colnum);
            class_fprintf_columntitle(*clfile,"TE-EE",phr->has_ep,colnum);
            class_fprintf_columntitle(*clfile,"TB-EB",phr->has_ep,colnum);
        }
        else if (ple->has_nl_diag == _TRUE_) {
            class_fprintf_columntitle(*clfile,"MV (DIAG)",phr->has_tt,colnum);
            class_fprintf_columntitle(*clfile,"TT-TT",phr->has_tt,colnum);
            class_fprintf_columntitle(*clfile,"TE-TE",phr->has_ee,colnum);
            class_fprintf_columntitle(*clfile,"EE-EE",phr->has_bb,colnum);
            class_fprintf_columntitle(*clfile,"BB-BB",phr->has_te,colnum);
            class_fprintf_columntitle(*clfile,"EB-EB",phr->has_pp,colnum);
            class_fprintf_columntitle(*clfile,"TB-TB",phr->has_tp,colnum);
        }
        else if (ple->has_nl_eb == _TRUE_) {
            class_fprintf_columntitle(*clfile,"MV (EB)",phr->has_tt,colnum);
            class_fprintf_columntitle(*clfile,"EB-EB",phr->has_pp,colnum);
        }
        
        fprintf(*clfile,"\n");
    }
    
    return _SUCCESS_;
    
}

/**
 * This routine write one line with l and all \f$ C_l\f$'s for all types (TT, TE...)
 *
 * @param pba        Input: pointer to background structure (needed for \f$ T_{cmb}\f$)
 * @param phr        Input: pointer to harmonic structure
 * @param pop        Input: pointer to output structure
 * @param clfile  Input: file pointer
 * @param l       Input: multipole
 * @param cl      Input: \f$ C_l\f$'s for all types
 * @param ct_size Input: number of types
 * @return the error status
 */

int output_one_line_of_cl(
                          struct background * pba,
                          struct harmonic * phr,
                          struct output * pop,
                          FILE * clfile,
                          double l,
                          double * cl, /* array with argument cl[index_ct] */
                          int ct_size
                          ) {
    int index_ct, index_ct_rest;
    double factor;
    
    factor = l*(l+1)/2./_PI_;
    
    fprintf(clfile," ");
    
    if (0==1){
        class_fprintf_int(clfile, (int)l, _TRUE_);
    }
    else{
        fprintf(clfile,"%4d ",(int)l);
    }
    
    if (pop->output_format == class_format) {
        
        for (index_ct=0; index_ct < ct_size; index_ct++) {
            class_fprintf_double(clfile, factor*cl[index_ct], _TRUE_);
        }
        fprintf(clfile,"\n");
    }
    
    if (pop->output_format == camb_format) {
        class_fprintf_double(clfile, factor*pow(pba->T_cmb*1.e6,2)*cl[phr->index_ct_tt], phr->has_tt);
        class_fprintf_double(clfile, factor*pow(pba->T_cmb*1.e6,2)*cl[phr->index_ct_ee], phr->has_ee);
        class_fprintf_double(clfile, factor*pow(pba->T_cmb*1.e6,2)*cl[phr->index_ct_bb], phr->has_bb);
        class_fprintf_double(clfile, factor*pow(pba->T_cmb*1.e6,2)*cl[phr->index_ct_te], phr->has_te);
        class_fprintf_double(clfile, l*(l+1)*factor*cl[phr->index_ct_pp], phr->has_pp);
        class_fprintf_double(clfile, sqrt(l*(l+1))*factor*pba->T_cmb*1.e6*cl[phr->index_ct_tp], phr->has_tp);
        class_fprintf_double(clfile, sqrt(l*(l+1))*factor*pba->T_cmb*1.e6*cl[phr->index_ct_ep], phr->has_ep);
        index_ct_rest = 0;
        if (phr->has_tt == _TRUE_)
            index_ct_rest++;
        if (phr->has_ee == _TRUE_)
            index_ct_rest++;
        if (phr->has_bb == _TRUE_)
            index_ct_rest++;
        if (phr->has_te == _TRUE_)
            index_ct_rest++;
        if (phr->has_pp == _TRUE_)
            index_ct_rest++;
        if (phr->has_tp == _TRUE_)
            index_ct_rest++;
        if (phr->has_ep == _TRUE_)
            index_ct_rest++;
        /* Now print the remaining (if any) entries:*/
        for (index_ct=index_ct_rest; index_ct < ct_size; index_ct++) {
            class_fprintf_double(clfile, factor*cl[index_ct], _TRUE_);
        }
        
        fprintf(clfile,"\n");
        
    }
    return _SUCCESS_;
    
}


/** DLM
 * This routine write one line with l and all \f$ N_l\f$'s (lensing reconstruction noise spectra) for all types (TT, TE...)
 *
 * @param pba        Input: pointer to background structure (needed for \f$ T_{cmb}\f$)
 * @param phr        Input: pointer to harmonic structure
 * @param pop        Input: pointer to output structure
 * @param nlfile  Input: file pointer
 * @param l       Input: multipole
 * @param nl      Input: \f$ N_l\f$'s for all types
 * @param nlt_size Input: number of types
 * @return the error status
 */

int output_one_line_of_nl(/* NOISE FILE */
                          struct background * pba,
                          struct harmonic * phr,
                          struct output * pop,
                          FILE * clfile,
                          double l,
                          double * nl, /* array with argument cl[index_ct] */
                          int nlt_size
                          ) {
    int index_ct, index_ct_rest;
    double factor;
    
    factor = l*(l+1)/2./_PI_;
    
    fprintf(clfile," ");
    
    if (0==1){
        class_fprintf_int(clfile, (int)l, _TRUE_);
    }
    else{
        fprintf(clfile,"%4d ",(int)l);
    }
    
    for (index_ct=0; index_ct < nlt_size; index_ct++) {
        class_fprintf_double(clfile, factor*nl[index_ct], _TRUE_);
    }
    fprintf(clfile,"\n");
    
    return _SUCCESS_;
    
}

/**
 * This routine opens one file where some P(k)'s will be written, and writes
 * a heading with some general information concerning its content.
 *
 * @param pba        Input: pointer to background structure (needed for h)
 * @param pfo        Input: pointer to fourier structure
 * @param pop        Input: pointer to output structure
 * @param pkfile     Output: returned pointer to file pointer
 * @param filename   Input: name of the file
 * @param first_line Input: text describing the content (initial conditions, ...)
 * @param z          Input: redshift of the output
 * @return the error status
 */

int output_open_pk_file(
                        struct background * pba,
                        struct fourier * pfo,
                        struct output * pop,
                        FILE * * pkfile,
                        FileName filename,
                        char * first_line,
                        double z
                        ) {
    
    int colnum = 1;
    class_open(*pkfile,filename,"w",pop->error_message);
    
    if (pop->write_header == _TRUE_) {
        fprintf(*pkfile,"# Matter power spectrum P(k) %sat redshift z=%g\n",first_line,z);
        fprintf(*pkfile,"# for k=%g to %g h/Mpc,\n",
                exp(pfo->ln_k[0])/pba->h,
                exp(pfo->ln_k[pfo->k_size-1])/pba->h);
        fprintf(*pkfile,"# number of wavenumbers equal to %d\n",pfo->k_size);
        
        fprintf(*pkfile,"#");
        class_fprintf_columntitle(*pkfile,"k (h/Mpc)",_TRUE_,colnum);
        class_fprintf_columntitle(*pkfile,"P (Mpc/h)^3",_TRUE_,colnum);
        
        fprintf(*pkfile,"\n");
    }
    
    return _SUCCESS_;
}


/**  DLM
 * This routine opens one file where temperature and polarization noise will be written, and writes
 * a heading with some general information concerning its content.
 *
 * @param ple        Lensing: getting the maximum multipole number
 * @param filename   Input: name of the file
 * @param first_line Input: text describing the content (initial conditions, ...)
 * @return the error status
 */

int output_open_nl_tp_file(
                           struct output * pop,
                           struct lensing * ple,
                           FILE * * pkfile,
                           FileName filename,
                           char * first_line
                           ) {
    
    int colnum = 1;
    class_open(*pkfile,filename,"w",pop->error_message);
    
    if (pop->write_header == _TRUE_) {
        fprintf(*pkfile,"# Intensity and polariztion noise spectra, %s\n",first_line);
        fprintf(*pkfile,"# for l=2 to %d,\n",
                ple->l_lensed_max);
        fprintf(*pkfile,"#");
        class_fprintf_columntitle(*pkfile,"l ",_TRUE_,colnum);
        class_fprintf_columntitle(*pkfile,"Nl (temp)",_TRUE_,colnum);
        class_fprintf_columntitle(*pkfile,"Nl (pol)",_TRUE_,colnum);
        fprintf(*pkfile,"\n");
    }
    
    return _SUCCESS_;
}

int output_open_cl_derv_file(
                             struct output * pop,
                             struct lensing * ple,
                             FILE * * pkfile,
                             FileName filename,
                             char * first_line
                             ) {
    
    int colnum = 1;
    class_open(*pkfile,filename,"w",pop->error_message);
    
    if (pop->write_header == _TRUE_) {
        fprintf(*pkfile,"# %s\n",first_line);
        fprintf(*pkfile,"# for l=2 to %d, printing if Mod[l,%d]==0. \n",ple->l_delensed_max,ple->derv_binedges);
    }
    
    return _SUCCESS_;
}



/**
 * This routine writes one line with k and P(k)
 *
 * @param pkfile  Input: file pointer
 * @param one_k   Input: wavenumber
 * @param one_pk  Input: matter power spectrum
 * @return the error status
 */

int output_one_line_of_pk(
                          FILE * pkfile,
                          double one_k,
                          double one_pk
                          ) {
    
    fprintf(pkfile," ");
    class_fprintf_double(pkfile,one_k,_TRUE_);
    class_fprintf_double(pkfile,one_pk,_TRUE_);
    fprintf(pkfile,"\n");
    
    return _SUCCESS_;
    
}

/** DLM
 * This routine writes one line with l, temp-noise and pol-noise.
 *
 * @param nlfile  Input: file pointer
 * @param one_l   Input: multipole
 * @param one_nl  Input: temperature and polarization noise
 * @return the error status
 */

int output_one_line_of_nl_tp(
                             struct output * pop,
                             FILE * noisefile,
                             double one_l,
                             double one_nt,
                             double one_np
                             ) {
    
    fprintf(noisefile," ");
    class_fprintf_double(noisefile,one_l,_TRUE_);
    class_fprintf_double(noisefile,one_nt,_TRUE_);
    class_fprintf_double(noisefile,one_np,_TRUE_);
    fprintf(noisefile,"\n");
    
    return _SUCCESS_;
    
}

/** DLM
 * This routine write one line with l and all \f$ dC_l/dC_dd\f$'s partial derivative of the spectra w.r.t. lensing spectrum
 *
 * @param pop        Input: pointer to output structure
 * @param nlfile  Input: file pointer
 * @param l       Input: multipole
 * @return the error status
 */

int output_one_line_of_cl_derv(
                               struct output * pop,
                               struct lensing * ple,
                               FILE * clfile,
                               double cl_derv,
                               int lcurr,
                               int lsize) {
    
    fprintf(clfile," ");
    
    class_fprintf_double(clfile, cl_derv, _TRUE_);
    
    if(lcurr > lsize - ple->derv_binedges || lcurr == lsize)fprintf(clfile,"\n");
    
    return _SUCCESS_;
    
}

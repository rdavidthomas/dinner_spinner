/* 
 * File:   spinner.h
 * Author: David Thomas
 *
 * Created on December 17, 2016, 4:40 PM
 */

#ifndef SPINNER_H
#define SPINNER_H

#ifdef __cplusplus
extern "C" {
#endif

    /* modules forward declaration */
    struct spin_sel;
    
    /* typedef just to make types clear */
    typedef struct spin_sel* spin_t;
    typedef void* spin_sel_ptr_t;
    
    spin_t spin_new();
    
    int spin_set_selection_list(spin_t self, 
                                spin_sel_ptr_t selections[], 
                                int number_of_selections);
    
    spin_sel_ptr_t spin_get_rand_selection(spin_t self);
    
    void spin_destroy(spin_t* self);
    

#ifdef __cplusplus
}
#endif

#endif /* SPINNER_H */


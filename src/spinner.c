#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "spinner.h"

#define MAGIC_KEY 0x5350494E // 'SPIN'

struct spin_sel {
    uint32_t magic_key;
    spin_sel_ptr_t* cur_sel_list;
    size_t length_of_list;
};


static inline void _check_module_ptr(spin_t s) {
    assert(s != NULL);
    assert(s->magic_key == MAGIC_KEY);
}

spin_t spin_new() {
    spin_t s =  calloc(1, sizeof(struct spin_sel));
    if( s == NULL ) return NULL;
    s->magic_key = MAGIC_KEY;
    s->cur_sel_list = NULL;
    s->length_of_list = 0;
    return s;
}
    
int spin_set_selection_list(spin_t self, 
                            spin_sel_ptr_t selections[], 
                            int number_of_selections)
{
    int idx = 0;
    spin_sel_ptr_t tmp_memptr;
    
    _check_module_ptr(self);
    if(selections == NULL)
        { return -EINVAL; }
    if( number_of_selections <= 0) 
        { return -EINVAL; }
    
    for(idx = 0; idx < number_of_selections; idx++) {
        if( selections[idx] == NULL ){
            return -EINVAL;
        }
    }
    tmp_memptr = realloc(self->cur_sel_list, 
                         number_of_selections*sizeof(spin_sel_ptr_t));
    if(tmp_memptr == NULL) {
        // out of memory???
        return -errno;
    }
    self->cur_sel_list = tmp_memptr;
    self->length_of_list = number_of_selections;
    memcpy(self->cur_sel_list, selections, number_of_selections*sizeof(spin_sel_ptr_t));
    return 0;
}

spin_sel_ptr_t spin_get_rand_selection(spin_t self)
{
    uintptr_t selection = NULL;
    
    _check_module_ptr(self);
    if(( self->length_of_list == 0)
            || (self->cur_sel_list == NULL)) {
        errno = ENOMSG;
        return NULL;  // not sure the best errno to describe this.
    }
    // cast to uintptr_t so void** can be dereferenced.
    selection = (uintptr_t)self->cur_sel_list[rand() % self->length_of_list];
    return (spin_sel_ptr_t)selection;
}

void spin_destroy(spin_t* self)
{
    if(self != NULL) {
        if(*self != NULL) {
            spin_t s = *self;
            s->magic_key = 0;
            s->length_of_list = 0;
            s->cur_sel_list = NULL;
            free(s);
        }
    }
}

#undef MAGIC_KEY //undef to ensure no interaction with external code.
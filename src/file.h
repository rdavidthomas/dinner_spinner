/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   file.h
 * Author: rthomas74
 *
 * Created on December 17, 2016, 7:17 PM
 */

#ifndef FILE_H
#define FILE_H

#ifdef __cplusplus
extern "C" {
#endif

    
    
    struct meal_s {
        char name[64];
    };
    
    typedef struct meal_s* meal_list_t;
    
    meal_list_t load_meal_list(char* fname);
    int save_meal_list(meal_list_t meal_list, int number_of_meals, char* fname);
    int append_meal(struct meal_s* meal, char* fname);
    int release_loaded_meal_list(meal_list_t meal_list);


#ifdef __cplusplus
}
#endif

#endif /* FILE_H */


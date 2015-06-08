/*
 * Copyright (C) 2007 TIMA Laboratory                                    
 *                                                                       
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, either version 3 of the License, or     
 * (at your option) any later version.                                   
 *                                                                       
 * This program is distributed in the hope that it will be useful,       
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
 * GNU General Public License for more details.                          
 *                                                                       
 * You should have received a copy of the GNU General Public License     
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#ifndef MULTIMEDIACARD_MODULE_H
#define MULTIMEDIACARD_MODULE_H

#include <stdint.h>
#include <MultiMediaCard/Card.h>
#include <MultiMediaCard/Operation.h>
#include <DnaTools/DnaTools.h>

typedef struct _mmc_extension
{
  extension_t extension;
  status_t (* create) (mmc_card_t * card, mmc_callbacks_t callbacks);
  status_t (* destroy) (mmc_card_t card);
  status_t (* execute) (mmc_card_t card, mmc_operation_t operation,
    void * buffer, int64_t block_start, int32_t block_count);
}
mmc_extension_t;

#endif


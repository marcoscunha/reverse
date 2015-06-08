
/* *************************************************************************
 * Copyright (C) 2008 TIMA Laboratory                                    *
 * Author: Alexandre CHAGOYA-GARZON
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/
#ifndef __TOPOLOGY_h
#define __TOPOLOGY_h

/* Cartesian topology defines */

#define MAX_CART_DIMENSIONS 5

typedef uint32_t dnp_cart_coords_t[MAX_CART_DIMENSIONS];
typedef uint8_t  dnp_cart_period_t[MAX_CART_DIMENSIONS];

typedef struct
{
    uint32_t ndims;
    uint32_t nnodes;
    dnp_cart_coords_t  dim; // size for each dimension
    dnp_cart_period_t  periods;
} cart_topology_t;


/* Cartesian topogy support functions */
int32_t dnp_topo_init();


/** cart_get_size
 *    @param dim : requested dimension 
 *    @returns  value of the size of the given dimension
 *    Description: sets the dimension to value in the coordinates
 */
uint32_t dnp_cart_get_size(uint32_t dim);




/* utilities to compare/convert coordinates (common to all)*/


/** dnp_cart_coords
 *    @param rank : rank
 *    @param out_coords : computed coordinates
 *    @returns  : 0 in case of success 
 *    Description: computes the coordinates  corresponding to the
 *    given rank in the configured cartesian topology
 */
int32_t dnp_dnp_cart_coords(uint32_t rank, dnp_cart_coords_t *out_coords);

/**cart_get_local_coords
 *    @param coords : coordinates
 *    @param topo : topology
 *    @returns  : 0 in case of success 
 *    Description: computes the rank  corresponding to the
 *    coordinates in the given topology
 */
void dnp_cart_get_local_coords(dnp_cart_coords_t *coords);


/** hal_topo_rank
 *    @param in_coords: given coordinates
 *    @param rank : pointer to the computed rank
 *    @param topo : topology
 *    @returns  : 0 in case of success 
 *    Description: computes the rank  corresponding to the
 *    coordinates in the given topology
 */
int32_t dnp_cart_rank(dnp_cart_coords_t *in_coords, uint32_t *rank);


/** topo_coordscmp
 *    @param coords1, coords2 : coordinates
 *    @param topo:              topology
 *    @returns  : 1 if equal, 0 else 
 *    Description: compares 2 coordinates in the given topology
 */
uint8_t dnp_dnp_cart_coordscmp(dnp_cart_coords_t *coords1, dnp_cart_coords_t *coords2);

/**cart_setdim
 *    @param val : value 
 *    @param dim : dimension to set
 *    @param coords : coordinates
 *    Description: sets the dimension to value in the coordinates
 */
void dnp_cart_set_dim(uint32_t val, uint32_t dim , dnp_cart_coords_t *coords);

/**cart_getdim
 *    @param dim : dimension to set
 *    @param coords : coordinates
 *    @returns  value of the requested dimension
 *    Description: sets the dimension to value in the coordinates
 */
uint32_t dnp_cart_get_dim(uint32_t dim , dnp_cart_coords_t *coords);

#endif

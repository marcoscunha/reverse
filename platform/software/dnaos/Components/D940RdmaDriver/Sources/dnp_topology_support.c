/* *************************************************************************
 * Copyright (C) 2010 TIMA Laboratory                                    *
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

#include <Processor/Processor.h>
#include <DnaTools/DnaTools.h>
#include <Private/Dnp.h>

//       Static variables
// declaration  of the cartesian topology.
static dnp_cart_topology_t cart_topo;
// declaration of the  variable to store local coordinates
static dnp_cart_coords_t  local_coords;



int32_t dnp_topo_init()
{
   uint32_t nnodes = 1;

   cart_topo . ndims = TOPOLOGY_INFO . mesh_ndims;

   for (int i=0; i <  cart_topo . ndims; i++)
   {
     nnodes = nnodes *  TOPOLOGY_INFO . mesh_dims[i];
     cart_topo . dim[i] = TOPOLOGY_INFO . mesh_dims[i];
     cart_topo . periods[i] = 1; // all dims are cyclic (torus)
   }
   
   cart_topo .nnodes = nnodes;

   // Compute and keep the local coordinates
   dnp_cart_coords(RDMA_COMMON . local_rank, &local_coords);
   
   return 0;
}

/** cart_get_size
 *    @param dim : dimension for which the size is requested 
 *    @returns  size of the requested dimension
 *    Description: gets the size of a given dimension in current topology
 */
uint32_t dnp_cart_get_size(uint32_t dim)
{
    return  (dim <  cart_topo . ndims) ? cart_topo . dim[dim] : 0;
}



/** cart_coords
 *    @param rank : rank
 *    @param out_coords : computed coordinates
 *    @returns  : 0 in case of success 
 *    Description: computes the coordinates  corresponding to the
 *    given rank in the given topology
 */
int32_t dnp_cart_coords(uint32_t rank, cart_coords_t *out_coords)
{
    uint32_t nnodes = cart_topo . nnodes;
    
    for (uint32_t i=0; i < cart_topo . ndims; i++)
    { 
        nnodes = nnodes / cart_topo . dim[i];
        (*out_coords)[cart_topo . ndims - i - 1] = (uint32_t)(rank / nnodes); 
        rank = rank % nnodes;
    }

    return 0;
}

/**cart_get_local_coords
 *    @param coords : coordinates
 *    @param topo : topology
 *    @returns  : 0 in case of success 
 *    Description: computes the rank  corresponding to the
 *    coordinates in the given topology
 */
void dnp_cart_get_local_coords(cart_coords_t *coords)
{
    for (uint32_t i=0; i < cart_topo . ndims; i++)
    (*coords)[i] = local_coords[i];
}




/** topo_rank
 *    @param in_coords: given coordinates
 *    @param rank : pointer to the computed rank
 *    @returns  : 0 in case of success 
 *    Description: computes the rank  corresponding to the
 *    coordinates in the given topology
 */
int32_t dnp_cart_rank(cart_coords_t *in_coords, uint32_t *rank)
{
    *rank = 0;

    for (int i=0; i <  cart_topo . ndims; i++)
    {
        *rank = *rank + (cart_topo . dim[i]) * ((*in_coords)[i]); 
    }

    return 0;
}




/** cart_coordscmp
 *    @param coords1, coords2 : coordinates
 *    @returns  : 1 if equal, 0 else 
 *    Description: compares 2 coordinates in the given topology
 */
uint8_t dnp_cart_coordscmp(cart_coords_t *coords1, cart_coords_t *coords2)
{
        
    for (int i=0; i < cart_topo . ndims; i++)
    {
        if ((*coords1)[i] != (*coords2)[i])
        {
            return 0; 
        }
    }

    return 1;
}

/**cart_setdim
 *    @param val : value 
 *    @param dim : dimension to set
 *    @param coords : coordinates
 *    Description: sets the dimension to value in the coordinates
 */
void dnp_cart_set_dim(uint32_t val, uint32_t dim , cart_coords_t *coords)
{
    if  (dim <  cart_topo . ndims)
        (*coords)[dim] = val;
}

/**cart_getdim
 *    @param dim : dimension to set
 *    @param coords : coordinates
 *    @returns  value of the requested dimension
 *    Description: sets the dimension to value in the coordinates
 */
uint32_t dnp_cart_get_dim(uint32_t dim , cart_coords_t *coords)
{
    return  (dim <  cart_topo . ndims) ?  (*coords)[dim] : 0;
}






/*
 *    dnp_mesh_init
 *    Configures the lattice size register
 */

void dnp_mesh_init()
{
    DNP_REG latticesize, mychipsize;

    // create register value      
    latticesize   =   ( (cart_get_size(0) << X_SIZE_OFFS)   & X_SIZE_MASK) |
                      ( (cart_get_size(1) << Y_SIZE_OFFS )  & Y_SIZE_MASK) |
                      ( (cart_get_size(2) << Z_SIZE_OFFS )  & Z_SIZE_MASK);

   // NOTE: to be changed for shapotto
    mychipsize    =   ( (1 << X_SIZE_OFFS)   & X_SIZE_MASK) |
                      ( (1 << Y_SIZE_OFFS )  & Y_SIZE_MASK) |
                      ( (1 << Z_SIZE_OFFS )  & Z_SIZE_MASK);



    // write it to the DNP register
    dnp_reg_write((DNP_REG *)DNP_REG_LATTICE_SIZE, latticesize);
    dnp_reg_write((DNP_REG *)DNP_REG_MYCHIP_SIZE, mychipsize);
}

/*
 *  dnp_mesh_set_local
 *  
 */
void dnp_mesh_set_local()
{
    cart_coords_t local_coords;
    DNP_REG coordsme, mychip3dcoord;

    cart_get_local_coords(&local_coords);

    // create register value      
    coordsme   = ((cart_get_dim(0, &local_coords) << X_OFFS)  & X_MASK) |
                 ((cart_get_dim(1, &local_coords) << Y_OFFS)  & Y_MASK) |
                 ((cart_get_dim(2, &local_coords) << Z_OFFS)  & Z_MASK);

   // to change for SHAPOTTO
    mychip3dcoord = coordsme;

    // write it to the DNP register
    dnp_reg_write((DNP_REG *)DNP_REG_3DCOORD_ME, coordsme);
    dnp_reg_write((DNP_REG *)DNP_REG_MYCHIP_3DCOORD, mychip3dcoord);
}


